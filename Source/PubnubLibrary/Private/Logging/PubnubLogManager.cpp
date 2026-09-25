// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "Logging/PubnubLogManager.h"

#include <pubnub/providers/logger.h>

// Bridge structure that embeds the new C-Core `pubnub_logger_provider_t` as its
// first member. This lets the C-Core hand us the provider's `self` pointer, which
// we reinterpret back to this bridge to recover the owning UPubnubLogManager.
// This mirrors the "extended struct with vtable first" pattern documented in
// providers/logger.h.
struct FPubnubCCoreLoggerBridge
{
	pubnub_logger_provider_t Base;
	UPubnubLogManager* Manager;
};

static const TArray<FString> FalseCCoreLogPhrases = {
	TEXT("errno=0('No error')"),
	TEXT("errno=9('Bad file descriptor')"),
	TEXT("errno=2('No such file or directory')"),
	TEXT("errno=35('Resource temporarily unavailable')")
};

static bool ShouldCCoreLogBeSkipped(const FString& Message)
{
	for (const FString& LogSkipPhrase : FalseCCoreLogPhrases)
	{
		if (Message.Contains(LogSkipPhrase))
		{
			return true;
		}
	}
	return false;
}

static EPubnubLogLevel ConvertCCoreLogLevel(const pubnub_log_level_t InLevel)
{
	switch (InLevel)
	{
	case PUBNUB_LOG_LEVEL_TRACE:
		return EPubnubLogLevel::PLL_Trace;
	case PUBNUB_LOG_LEVEL_DEBUG:
		return EPubnubLogLevel::PLL_Debug;
	case PUBNUB_LOG_LEVEL_INFO:
		return EPubnubLogLevel::PLL_Info;
	case PUBNUB_LOG_LEVEL_WARNING:
		return EPubnubLogLevel::PLL_Warning;
	case PUBNUB_LOG_LEVEL_ERROR:
		return EPubnubLogLevel::PLL_Error;
	case PUBNUB_LOG_LEVEL_NONE:
	default:
		return EPubnubLogLevel::PLL_None;
	}
}

static FString BuildCCoreCallsite(const pubnub_log_entry_t* Entry)
{
	if (!Entry || !Entry->file)
	{
		return TEXT("");
	}
	return FString::Printf(TEXT("%s:%d"), UTF8_TO_TCHAR(Entry->file), Entry->line);
}

// Renders a single primitive value to a string. Values other than primitives
// (map/array) are handled recursively by AppendStructuredLogValueLines.
static FString PrimitiveLogValueToString(const pubnub_log_value_t* Value)
{
	if (!Value)
	{
		return TEXT("null");
	}

	switch (pubnub_log_value_type(Value))
	{
	case PUBNUB_LOG_VALUE_NULL:
		return TEXT("null");
	case PUBNUB_LOG_VALUE_BOOL:
		return pubnub_log_value_get_bool(Value) ? TEXT("true") : TEXT("false");
	case PUBNUB_LOG_VALUE_NUMBER:
		return FString::Printf(TEXT("%lld"), static_cast<long long>(pubnub_log_value_get_number(Value)));
	case PUBNUB_LOG_VALUE_STRING:
	{
		const char* StringValue = pubnub_log_value_get_string(Value, nullptr);
		return UTF8_TO_TCHAR(StringValue ? StringValue : "");
	}
	default:
		return TEXT("<complex>");
	}
}

// Recursively renders a structured value tree into indented text lines.
//
// The new C-Core models maps as a linked list of entry nodes (each of type
// PUBNUB_LOG_VALUE_MAP carrying `data.map_val.key` + `data.map_val.value`),
// chained via `next`. Arrays are a container node (type PUBNUB_LOG_VALUE_ARRAY)
// whose `data.array_val.head` points to the first element; elements are chained
// via `next` as well. Both are walked iteratively here via pubnub_log_value_next().
static void AppendStructuredLogValueLines(const pubnub_log_value_t* Value, const FString& Indent, FString& OutText)
{
	if (!Value)
	{
		OutText += Indent + TEXT("null\n");
		return;
	}

	const pubnub_log_value_type_t ValueType = pubnub_log_value_type(Value);

	if (ValueType == PUBNUB_LOG_VALUE_MAP || ValueType == PUBNUB_LOG_VALUE_ARRAY)
	{
		if (ValueType == PUBNUB_LOG_VALUE_ARRAY)
		{
			// Array container: iterate elements from the head pointer.
			const pubnub_log_value_t* Head = Value->data.array_val.head;
			if (!Head)
			{
				OutText += Indent + TEXT("[]\n");
			}
			int32 Index = 0;
			for (const pubnub_log_value_t* Element = Head; Element; Element = pubnub_log_value_next(Element), ++Index)
			{
				const pubnub_log_value_type_t ElementType = pubnub_log_value_type(Element);
				if (ElementType == PUBNUB_LOG_VALUE_MAP || ElementType == PUBNUB_LOG_VALUE_ARRAY)
				{
					OutText += FString::Printf(TEXT("%s[%d]:\n"), *Indent, Index);
					AppendStructuredLogValueLines(Element, Indent + TEXT("  "), OutText);
				}
				else
				{
					OutText += FString::Printf(TEXT("%s- %s\n"), *Indent, *PrimitiveLogValueToString(Element));
				}
			}
		}
		else
		{
			// Map: linked list of key/value entry nodes.
			bool bAnyEntry = false;
			for (const pubnub_log_value_t* Node = Value; Node && pubnub_log_value_type(Node) == PUBNUB_LOG_VALUE_MAP; Node = pubnub_log_value_next(Node))
			{
				const char* KeyValue = pubnub_log_value_key(Node);
				if (!KeyValue)
				{
					continue;
				}
				bAnyEntry = true;

				const FString Key = UTF8_TO_TCHAR(KeyValue);
				const pubnub_log_value_t* ChildValue = Node->data.map_val.value;
				const pubnub_log_value_type_t ChildType = pubnub_log_value_type(ChildValue);

				if (ChildType == PUBNUB_LOG_VALUE_MAP || ChildType == PUBNUB_LOG_VALUE_ARRAY)
				{
					OutText += FString::Printf(TEXT("%s%s:\n"), *Indent, *Key);
					AppendStructuredLogValueLines(ChildValue, Indent + TEXT("  "), OutText);
				}
				else
				{
					OutText += FString::Printf(TEXT("%s%s: %s\n"), *Indent, *Key, *PrimitiveLogValueToString(ChildValue));
				}
			}

			if (!bAnyEntry)
			{
				OutText += Indent + TEXT("{}\n");
			}
		}
	}
	else
	{
		// Root value is a primitive.
		OutText += Indent + PrimitiveLogValueToString(Value) + TEXT("\n");
	}
}

static FString BuildCCoreObjectMessage(const pubnub_log_entry_object_t* ObjectMessage)
{
	const FString Label = UTF8_TO_TCHAR(ObjectMessage->label ? ObjectMessage->label : "");
	FString Output = Label.IsEmpty() ? TEXT("C-Core object log.") : Label;

	if (ObjectMessage->data)
	{
		FString StructuredLines;
		AppendStructuredLogValueLines(ObjectMessage->data, TEXT("  "), StructuredLines);
		StructuredLines.TrimEndInline();
		if (!StructuredLines.IsEmpty())
		{
			Output += TEXT("\n");
			Output += StructuredLines;
		}
	}

	return Output;
}

static FString BuildCCoreLogMessage(const pubnub_log_entry_t* Entry)
{
	if (!Entry)
	{
		return TEXT("Unknown C-Core log message.");
	}

	switch (Entry->type)
	{
	case PUBNUB_LOG_ENTRY_TEXT:
	{
		const pubnub_log_entry_text_t* TextEntry = reinterpret_cast<const pubnub_log_entry_text_t*>(Entry);
		return UTF8_TO_TCHAR(TextEntry->message ? TextEntry->message : "");
	}
	case PUBNUB_LOG_ENTRY_ERROR:
	{
		const pubnub_log_entry_error_t* ErrorEntry = reinterpret_cast<const pubnub_log_entry_error_t*>(Entry);
		const FString MessageText = UTF8_TO_TCHAR(ErrorEntry->error_message ? ErrorEntry->error_message : "");

		FString DetailsText;
		if (ErrorEntry->details)
		{
			AppendStructuredLogValueLines(ErrorEntry->details, TEXT("  "), DetailsText);
			DetailsText.TrimEndInline();
		}

		return DetailsText.IsEmpty()
			? FString::Printf(TEXT("Error %d: %s"), ErrorEntry->error_code, *MessageText)
			: FString::Printf(TEXT("Error %d: %s (%s)"), ErrorEntry->error_code, *MessageText, *DetailsText);
	}
	case PUBNUB_LOG_ENTRY_NET_REQ:
	{
		const pubnub_log_entry_net_request_t* Request = reinterpret_cast<const pubnub_log_entry_net_request_t*>(Entry);
		const FString Method = UTF8_TO_TCHAR(Request->method ? Request->method : "");
		const FString Url = UTF8_TO_TCHAR(Request->url ? Request->url : "");
		return FString::Printf(TEXT("Network request: %s %s"), *Method, *Url);
	}
	case PUBNUB_LOG_ENTRY_NET_RESP:
	{
		const pubnub_log_entry_net_response_t* Response = reinterpret_cast<const pubnub_log_entry_net_response_t*>(Entry);
		const FString Url = UTF8_TO_TCHAR(Response->url ? Response->url : "");
		return FString::Printf(TEXT("Network response: %s status=%d"), *Url, Response->status_code);
	}
	case PUBNUB_LOG_ENTRY_OBJECT:
	{
		const pubnub_log_entry_object_t* ObjectEntry = reinterpret_cast<const pubnub_log_entry_object_t*>(Entry);
		return BuildCCoreObjectMessage(ObjectEntry);
	}
	default:
		return TEXT("Unknown C-Core log message type.");
	}
}

// Single log callback for the new C-Core provider. Dispatched by the C-Core's
// internal mux, which already enriches the entry with context_id and timestamp_ms.
static void ForwardCCoreLog(pubnub_logger_provider_t* Self, const pubnub_log_entry_t* Entry)
{
	if (!Self || !Entry)
	{
		return;
	}

	FPubnubCCoreLoggerBridge* Bridge = reinterpret_cast<FPubnubCCoreLoggerBridge*>(Self);
	if (Bridge && Bridge->Manager)
	{
		Bridge->Manager->HandleCCoreLog(Entry);
	}
}

static FString NormalizeCCoreEmitterID(const char* RawID)
{
	const FString Parsed = UTF8_TO_TCHAR(RawID ? RawID : "");
	if (Parsed.IsEmpty())
	{
		return TEXT("PubNub-unknown");
	}
	return Parsed.StartsWith(TEXT("PubNub")) ? Parsed : FString::Printf(TEXT("PubNub-%s"), *Parsed);
}

void UPubnubLogManager::BeginDestroy()
{
	if (CCoreLoggerBridge)
	{
		delete CCoreLoggerBridge;
		CCoreLoggerBridge = nullptr;
	}

	Super::BeginDestroy();
}

void UPubnubLogManager::AddLogger(const TScriptInterface<IPubnubLoggerInterface>& Logger)
{
	UObject* LoggerObject = Logger.GetObject();
	if (!LoggerObject || !LoggerObject->GetClass()->ImplementsInterface(UPubnubLoggerInterface::StaticClass()))
	{
		return;
	}

	for (const TObjectPtr<UObject>& ExistingLogger : LoggerObjects)
	{
		if (ExistingLogger == LoggerObject)
		{
			return;
		}
	}

	LoggerObjects.Add(LoggerObject);
}

void UPubnubLogManager::RemoveLogger(const TScriptInterface<IPubnubLoggerInterface>& Logger)
{
	UObject* LoggerObject = Logger.GetObject();
	if (!LoggerObject)
	{
		return;
	}

	LoggerObjects.RemoveAll([LoggerObject](const TObjectPtr<UObject>& Entry)
	{
		return Entry == LoggerObject;
	});
}

void UPubnubLogManager::ClearLoggers()
{
	LoggerObjects.Reset();
}

TArray<TScriptInterface<IPubnubLoggerInterface>> UPubnubLogManager::GetLoggers() const
{
	TArray<TScriptInterface<IPubnubLoggerInterface>> Result;
	Result.Reserve(LoggerObjects.Num());

	for (const TObjectPtr<UObject>& LoggerEntry : LoggerObjects)
	{
		UObject* LoggerObject = LoggerEntry.Get();
		if (!LoggerObject || !LoggerObject->GetClass()->ImplementsInterface(UPubnubLoggerInterface::StaticClass()))
		{
			continue;
		}

		TScriptInterface<IPubnubLoggerInterface> LoggerInterface;
		LoggerInterface.SetObject(LoggerObject);
		LoggerInterface.SetInterface(Cast<IPubnubLoggerInterface>(LoggerObject));
		Result.Add(LoggerInterface);
	}

	return Result;
}

void UPubnubLogManager::SetUESdkEmitterID(const FString& InEmitterID)
{
	UESdkEmitterID = InEmitterID.IsEmpty() ? TEXT("PubNub-unknown") : InEmitterID;
}

void UPubnubLogManager::Log(EPubnubLogLevel Level, EPubnubLogSource Source, const FString& Message, const FString& Callsite)
{
	if (Level == EPubnubLogLevel::PLL_None)
	{
		return;
	}

	FPubnubLogMessage PubnubLogMessage;
	PubnubLogMessage.LogLevel = Level;
	PubnubLogMessage.Source = Source;
	PubnubLogMessage.Message = Message;
	PubnubLogMessage.TimestampUtc = FDateTime::UtcNow();
	PubnubLogMessage.Callsite = Callsite;
	PubnubLogMessage.PubnubInstanceID = Source == EPubnubLogSource::PLS_CCore ? TEXT("PubNub-unknown") : UESdkEmitterID;
	DispatchMessage(PubnubLogMessage);
}

void UPubnubLogManager::DispatchMessage(const FPubnubLogMessage& Message)
{
	for (int32 Index = LoggerObjects.Num() - 1; Index >= 0; --Index)
	{
		UObject* LoggerObject = LoggerObjects[Index].Get();
		if (!LoggerObject
			|| !IsValid(LoggerObject)
			|| LoggerObject->IsUnreachable()
			|| LoggerObject->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed)
			|| !LoggerObject->GetClass()->ImplementsInterface(UPubnubLoggerInterface::StaticClass()))
		{
			LoggerObjects.RemoveAt(Index);
			continue;
		}

		const EPubnubLogLevel MinimumLevel = Message.Source == EPubnubLogSource::PLS_CCore
			? IPubnubLoggerInterface::Execute_GetMinimumCCoreLogLevel(LoggerObject)
			: IPubnubLoggerInterface::Execute_GetMinimumLogLevel(LoggerObject);

		if (!IsLevelEnabled(Message.LogLevel, MinimumLevel))
		{
			continue;
		}

		DispatchLevelSpecific(LoggerObject, Message.LogLevel, Message);
	}
}

void UPubnubLogManager::HandleCCoreLog(const pubnub_log_entry_t* Entry)
{
	if (!Entry)
	{
		return;
	}

	const FString MessageText = BuildCCoreLogMessage(Entry);
	if (ShouldCCoreLogBeSkipped(MessageText))
	{
		return;
	}

	FPubnubLogMessage PubnubLogMessage;
	PubnubLogMessage.LogLevel = ConvertCCoreLogLevel(Entry->level);
	PubnubLogMessage.Source = EPubnubLogSource::PLS_CCore;
	PubnubLogMessage.Message = MessageText;
	PubnubLogMessage.Callsite = BuildCCoreCallsite(Entry);
	PubnubLogMessage.PubnubInstanceID = NormalizeCCoreEmitterID(Entry->context_id);

	if (Entry->timestamp_ms > 0)
	{
		PubnubLogMessage.TimestampUtc = FDateTime::FromUnixTimestamp(static_cast<int64>(Entry->timestamp_ms / 1000))
			+ FTimespan::FromMilliseconds(static_cast<int64>(Entry->timestamp_ms % 1000));
	}
	else
	{
		PubnubLogMessage.TimestampUtc = FDateTime::UtcNow();
	}

	DispatchMessage(PubnubLogMessage);
}

pubnub_logger_provider_t* UPubnubLogManager::GetCCoreLoggerProvider()
{
	if (!CCoreLoggerBridge)
	{
		CCoreLoggerBridge = new FPubnubCCoreLoggerBridge();
		CCoreLoggerBridge->Base.log = &ForwardCCoreLog;
		CCoreLoggerBridge->Base.set_level = nullptr;
		CCoreLoggerBridge->Manager = this;
	}

	return &CCoreLoggerBridge->Base;
}

bool UPubnubLogManager::IsLevelEnabled(EPubnubLogLevel MessageLevel, EPubnubLogLevel MinimumLevel)
{
	if (MinimumLevel == EPubnubLogLevel::PLL_None)
	{
		return false;
	}

	return static_cast<uint8>(MessageLevel) >= static_cast<uint8>(MinimumLevel);
}

void UPubnubLogManager::DispatchLevelSpecific(UObject* LoggerObject, EPubnubLogLevel Level, const FPubnubLogMessage& Message)
{
	if (!LoggerObject
		|| !IsValid(LoggerObject)
		|| LoggerObject->IsUnreachable()
		|| LoggerObject->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
	{
		return;
	}

	switch (Level)
	{
	case EPubnubLogLevel::PLL_Trace:
		IPubnubLoggerInterface::Execute_LogTrace(LoggerObject, Message);
		break;
	case EPubnubLogLevel::PLL_Debug:
		IPubnubLoggerInterface::Execute_LogDebug(LoggerObject, Message);
		break;
	case EPubnubLogLevel::PLL_Info:
		IPubnubLoggerInterface::Execute_LogInfo(LoggerObject, Message);
		break;
	case EPubnubLogLevel::PLL_Warning:
		IPubnubLoggerInterface::Execute_LogWarning(LoggerObject, Message);
		break;
	case EPubnubLogLevel::PLL_Error:
		IPubnubLoggerInterface::Execute_LogError(LoggerObject, Message);
		break;
	case EPubnubLogLevel::PLL_None:
	default:
		break;
	}
}
