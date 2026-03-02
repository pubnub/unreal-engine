#include "Logging/PubnubLogManager.h"

#include "core/pubnub_logger.h"

namespace
{
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

	static EPubnubLogLevel ConvertCCoreLogLevel(const enum pubnub_log_level InLevel)
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
		{
			bool BoolValue = false;
			return pubnub_log_value_get_bool(Value, &BoolValue) == 0 ? (BoolValue ? TEXT("true") : TEXT("false")) : TEXT("false");
		}
		case PUBNUB_LOG_VALUE_NUMBER:
		{
			double NumberValue = 0.0;
			return pubnub_log_value_get_number(Value, &NumberValue) == 0 ? FString::SanitizeFloat(NumberValue) : TEXT("0");
		}
		case PUBNUB_LOG_VALUE_STRING:
		{
			const char* StringValue = pubnub_log_value_get_string(Value);
			return UTF8_TO_TCHAR(StringValue ? StringValue : "");
		}
		default:
			return TEXT("<complex>");
		}
	}

	static void AppendStructuredLogValueLines(const pubnub_log_value_t* Value, const FString& Indent, FString& OutText)
	{
		if (!Value)
		{
			return;
		}

		const enum pubnub_log_value_type ValueType = pubnub_log_value_type(Value);
		if (ValueType == PUBNUB_LOG_VALUE_MAP)
		{
			for (const pubnub_log_value_t* Element = pubnub_log_value_first(Value); Element != nullptr; Element = pubnub_log_value_next(Element))
			{
				const char* KeyValue = pubnub_log_value_key(Element);
				const FString Key = UTF8_TO_TCHAR(KeyValue ? KeyValue : "");
				const enum pubnub_log_value_type ElementType = pubnub_log_value_type(Element);

				if (ElementType == PUBNUB_LOG_VALUE_MAP || ElementType == PUBNUB_LOG_VALUE_ARRAY)
				{
					OutText += FString::Printf(TEXT("%s%s:\n"), *Indent, *Key);
					AppendStructuredLogValueLines(Element, Indent + TEXT("  "), OutText);
				}
				else
				{
					OutText += FString::Printf(TEXT("%s%s: %s\n"), *Indent, *Key, *PrimitiveLogValueToString(Element));
				}
			}
		}
		else if (ValueType == PUBNUB_LOG_VALUE_ARRAY)
		{
			int32 Index = 0;
			for (const pubnub_log_value_t* Element = pubnub_log_value_first(Value); Element != nullptr; Element = pubnub_log_value_next(Element), ++Index)
			{
				const enum pubnub_log_value_type ElementType = pubnub_log_value_type(Element);
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
	}

	static FString BuildCCoreObjectMessage(const pubnub_log_message_object_t* ObjectMessage)
	{
		const FString Details = UTF8_TO_TCHAR(ObjectMessage->details ? ObjectMessage->details : "");
		FString Output = Details.IsEmpty() ? TEXT("C-Core object log.") : Details;

		if (ObjectMessage->message)
		{
			FString StructuredLines;
			AppendStructuredLogValueLines(ObjectMessage->message, TEXT("  "), StructuredLines);
			StructuredLines = StructuredLines.TrimEnd();
			if (!StructuredLines.IsEmpty())
			{
				Output += TEXT("\n");
				Output += StructuredLines;
			}
		}

		return Output;
	}

	static FString BuildCCoreLogMessage(const pubnub_log_message_t* Message)
	{
		if (!Message)
		{
			return TEXT("Unknown C-Core log message.");
		}

		switch (Message->message_type)
		{
		case PUBNUB_LOG_MESSAGE_TYPE_TEXT:
		{
			const pubnub_log_message_text_t* TextMessage = reinterpret_cast<const pubnub_log_message_text_t*>(Message);
			return UTF8_TO_TCHAR(TextMessage->message ? TextMessage->message : "");
		}
		case PUBNUB_LOG_MESSAGE_TYPE_ERROR:
		{
			const pubnub_log_message_error_t* ErrorMessage = reinterpret_cast<const pubnub_log_message_error_t*>(Message);
			const FString MessageText = UTF8_TO_TCHAR(ErrorMessage->error_message ? ErrorMessage->error_message : "");
			const FString DetailsText = UTF8_TO_TCHAR(ErrorMessage->details ? ErrorMessage->details : "");
			return DetailsText.IsEmpty()
				? FString::Printf(TEXT("Error %d: %s"), ErrorMessage->error_code, *MessageText)
				: FString::Printf(TEXT("Error %d: %s (%s)"), ErrorMessage->error_code, *MessageText, *DetailsText);
		}
		case PUBNUB_LOG_MESSAGE_TYPE_NETWORK_REQUEST:
		{
			const pubnub_log_message_network_request_t* NetworkMessage = reinterpret_cast<const pubnub_log_message_network_request_t*>(Message);
			const FString Url = UTF8_TO_TCHAR(NetworkMessage->url ? NetworkMessage->url : "");
			const FString Details = UTF8_TO_TCHAR(NetworkMessage->details ? NetworkMessage->details : "");
			return Details.IsEmpty()
				? FString::Printf(TEXT("Network request: %s"), *Url)
				: FString::Printf(TEXT("Network request: %s (%s)"), *Url, *Details);
		}
		case PUBNUB_LOG_MESSAGE_TYPE_NETWORK_RESPONSE:
		{
			const pubnub_log_message_network_response_t* NetworkResponse = reinterpret_cast<const pubnub_log_message_network_response_t*>(Message);
			const FString Url = UTF8_TO_TCHAR(NetworkResponse->url ? NetworkResponse->url : "");
			return FString::Printf(TEXT("Network response: %s status=%d"), *Url, NetworkResponse->status_code);
		}
		case PUBNUB_LOG_MESSAGE_TYPE_OBJECT:
		{
			const pubnub_log_message_object_t* ObjectMessage = reinterpret_cast<const pubnub_log_message_object_t*>(Message);
			return BuildCCoreObjectMessage(ObjectMessage);
		}
		default:
			return TEXT("Unknown C-Core log message type.");
		}
	}

	static UPubnubLogManager* GetManagerFromCCoreLogger(const pubnub_logger_t* Logger)
	{
		if (!Logger)
		{
			return nullptr;
		}
		return static_cast<UPubnubLogManager*>(pubnub_logger_user_data(const_cast<pubnub_logger_t*>(Logger)));
	}

	static void ForwardCCoreLog(const pubnub_logger_t* Logger, const pubnub_log_message_t* Message)
	{
		if (UPubnubLogManager* LogManager = GetManagerFromCCoreLogger(Logger))
		{
			LogManager->HandleCCoreLog(Message);
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

void UPubnubLogManager::HandleCCoreLog(const pubnub_log_message_t* Message)
{
	if (!Message)
	{
		return;
	}

	const FString MessageText = BuildCCoreLogMessage(Message);
	if (ShouldCCoreLogBeSkipped(MessageText))
	{
		return;
	}

	FPubnubLogMessage PubnubLogMessage;
	PubnubLogMessage.LogLevel = ConvertCCoreLogLevel(Message->level);
	PubnubLogMessage.Source = EPubnubLogSource::PLS_CCore;
	PubnubLogMessage.Message = MessageText;
	PubnubLogMessage.Callsite = UTF8_TO_TCHAR(Message->location ? Message->location : "");
	PubnubLogMessage.PubnubInstanceID = NormalizeCCoreEmitterID(Message->pubnub_id);

	if (Message->timestamp.seconds > 0)
	{
		PubnubLogMessage.TimestampUtc = FDateTime::FromUnixTimestamp(static_cast<int64>(Message->timestamp.seconds))
			+ FTimespan::FromMilliseconds(static_cast<double>(Message->timestamp.milliseconds));
	}
	else
	{
		PubnubLogMessage.TimestampUtc = FDateTime::UtcNow();
	}

	DispatchMessage(PubnubLogMessage);
}

const pubnub_logger_interface& UPubnubLogManager::GetCCoreLoggerInterface()
{
	static const struct pubnub_logger_interface LoggerInterface = {
		.trace = &ForwardCCoreLog,
		.debug = &ForwardCCoreLog,
		.info = &ForwardCCoreLog,
		.warn = &ForwardCCoreLog,
		.error = &ForwardCCoreLog,
		.destroy = nullptr
	};
	return LoggerInterface;
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
