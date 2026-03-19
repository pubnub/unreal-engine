// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Interfaces/PubnubLoggerInterface.h"
#include "PubnubLogManager.generated.h"

struct pubnub_logger;
typedef struct pubnub_logger pubnub_logger_t;
struct pubnub_log_message;
typedef struct pubnub_log_message pubnub_log_message_t;
struct pubnub_logger_interface;

UCLASS()
class PUBNUBLIBRARY_API UPubnubLogManager : public UObject
{
	GENERATED_BODY()

public:
	void AddLogger(const TScriptInterface<IPubnubLoggerInterface>& Logger);
	void RemoveLogger(const TScriptInterface<IPubnubLoggerInterface>& Logger);
	void ClearLoggers();
	TArray<TScriptInterface<IPubnubLoggerInterface>> GetLoggers() const;
	void SetUESdkEmitterID(const FString& InEmitterID);

	void Log(EPubnubLogLevel Level, EPubnubLogSource Source, const FString& Message, const FString& Callsite = TEXT(""));
	void HandleCCoreLog(const pubnub_log_message_t* Message);
	static const pubnub_logger_interface& GetCCoreLoggerInterface();

private:
	static bool IsLevelEnabled(EPubnubLogLevel MessageLevel, EPubnubLogLevel MinimumLevel);
	static void DispatchLevelSpecific(UObject* LoggerObject, EPubnubLogLevel Level, const FPubnubLogMessage& Message);
	void DispatchMessage(const FPubnubLogMessage& Message);

	UPROPERTY()
	TArray<TObjectPtr<UObject>> LoggerObjects;

	FString UESdkEmitterID = TEXT("PubNub-unknown");
};
