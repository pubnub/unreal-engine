// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Interfaces/PubnubLoggerInterface.h"
#include "PubnubLogManager.generated.h"

// New C-Core logger provider types (borrowed, no ownership).
struct pubnub_logger_provider;
typedef struct pubnub_logger_provider pubnub_logger_provider_t;
struct pubnub_log_entry;
typedef struct pubnub_log_entry pubnub_log_entry_t;
struct FPubnubCCoreLoggerBridge;

UCLASS()
class PUBNUBLIBRARY_API UPubnubLogManager : public UObject
{
	GENERATED_BODY()

public:
	virtual void BeginDestroy() override;

	void AddLogger(const TScriptInterface<IPubnubLoggerInterface>& Logger);
	void RemoveLogger(const TScriptInterface<IPubnubLoggerInterface>& Logger);
	void ClearLoggers();
	TArray<TScriptInterface<IPubnubLoggerInterface>> GetLoggers() const;
	void SetUESdkEmitterID(const FString& InEmitterID);

	void Log(EPubnubLogLevel Level, EPubnubLogSource Source, const FString& Message, const FString& Callsite = TEXT(""));
	void HandleCCoreLog(const pubnub_log_entry_t* Entry);

	pubnub_logger_provider_t* GetCCoreLoggerProvider();

private:
	static bool IsLevelEnabled(EPubnubLogLevel MessageLevel, EPubnubLogLevel MinimumLevel);
	static void DispatchLevelSpecific(UObject* LoggerObject, EPubnubLogLevel Level, const FPubnubLogMessage& Message);
	void DispatchMessage(const FPubnubLogMessage& Message);

	UPROPERTY()
	TArray<TObjectPtr<UObject>> LoggerObjects;

	FString UESdkEmitterID = TEXT("PubNub-unknown");

	FPubnubCCoreLoggerBridge* CCoreLoggerBridge = nullptr;
};
