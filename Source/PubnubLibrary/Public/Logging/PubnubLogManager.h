// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Interfaces/PubnubLoggerInterface.h"
#include "PubnubLogManager.generated.h"

// Library logger provider types. The bridge does not own them.
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

	/** Frees the logger bridge. Call only after the client context is destroyed, because teardown still logs through it. */
	void ReleaseCCoreLoggerBridge();

	void AddLogger(const TScriptInterface<IPubnubLoggerInterface>& Logger);
	void RemoveLogger(const TScriptInterface<IPubnubLoggerInterface>& Logger);
	void ClearLoggers();
	TArray<TScriptInterface<IPubnubLoggerInterface>> GetLoggers() const;
	void SetUESdkEmitterID(const FString& InEmitterID);

	void Log(EPubnubLogLevel Level, EPubnubLogSource Source, const FString& Message, const FString& Callsite = TEXT(""));

	/**
	 * Copies a library log entry and dispatches it on the game thread.
	 * The entry and every pointer it borrows are valid only for this call.
	 * Logger objects are not touched until the game thread runs.
	 */
	void HandleCCoreLog(const pubnub_log_entry_t* Entry);

	pubnub_logger_provider_t* GetCCoreLoggerProvider();

private:
	static bool IsLevelEnabled(EPubnubLogLevel MessageLevel, EPubnubLogLevel MinimumLevel);
	static void DispatchLevelSpecific(UObject* LoggerObject, EPubnubLogLevel Level, const FPubnubLogMessage& Message);
	static void EnqueueLogDispatch(TWeakObjectPtr<UPubnubLogManager> Manager, FPubnubLogMessage&& Message);
	static void ForwardCCoreLog(pubnub_logger_provider_t* Self, const pubnub_log_entry_t* Entry);
	void DispatchMessage(const FPubnubLogMessage& Message);

	UPROPERTY()
	TArray<TObjectPtr<UObject>> LoggerObjects;

	FString UESdkEmitterID = TEXT("PubNub-unknown");

	FPubnubCCoreLoggerBridge* CCoreLoggerBridge = nullptr;
};
