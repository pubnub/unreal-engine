// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "PubnubDefaultLogger.h"

#include "FunctionLibraries/PubnubLogUtilities.h"
#include "PubnubSubsystem.h"

static FString ToLevelString(EPubnubLogLevel InLevel)
{
	switch (InLevel)
	{
	case EPubnubLogLevel::PLL_Trace:
		return TEXT("TRACE");
	case EPubnubLogLevel::PLL_Debug:
		return TEXT("DEBUG");
	case EPubnubLogLevel::PLL_Info:
		return TEXT("INFO");
	case EPubnubLogLevel::PLL_Warning:
		return TEXT("WARNING");
	case EPubnubLogLevel::PLL_Error:
		return TEXT("ERROR");
	case EPubnubLogLevel::PLL_None:
	default:
		return TEXT("NONE");
	}
}

void UPubnubDefaultLogger::Log_Implementation(const FPubnubLogMessage& LogMessage)
{
	const FString Timestamp = UPubnubLogUtilities::FormatIso8601UtcWithMilliseconds(LogMessage.TimestampUtc);
	const FString Level = ToLevelString(LogMessage.LogLevel);
	FString FinalText;
	if (LogMessage.Source == EPubnubLogSource::PLS_CCore)
	{
		const FString Emitter = LogMessage.PubnubInstanceID.IsEmpty() ? TEXT("PubNub-unknown") : LogMessage.PubnubInstanceID;
		FinalText = FString::Printf(TEXT("%s [C-Core] %s %s %s %s"), *Timestamp, *Emitter, *Level, *LogMessage.Callsite, *LogMessage.Message);
	}
	else
	{
		const FString LocationSuffix = LogMessage.Callsite.IsEmpty() ? TEXT("") : FString::Printf(TEXT(" [%s]"), *LogMessage.Callsite);
		const FString Emitter = LogMessage.PubnubInstanceID.IsEmpty() ? TEXT("PubNub-unknown") : LogMessage.PubnubInstanceID;
		FinalText = FString::Printf(TEXT("%s [UE-SDK] %s %s %s %s"), *Timestamp, *Emitter, *Level, *LocationSuffix, *LogMessage.Message);
	}

	switch (LogMessage.LogLevel)
	{
	case EPubnubLogLevel::PLL_Trace:
	case EPubnubLogLevel::PLL_Debug:
	case EPubnubLogLevel::PLL_Info:
		UE_LOG(PubnubLog, Log, TEXT("%s"), *FinalText);
		break;
	case EPubnubLogLevel::PLL_Warning:
		UE_LOG(PubnubLog, Warning, TEXT("%s"), *FinalText);
		break;
	case EPubnubLogLevel::PLL_Error:
		UE_LOG(PubnubLog, Error, TEXT("%s"), *FinalText);
		break;
	case EPubnubLogLevel::PLL_None:
	default:
		break;
	}
}
