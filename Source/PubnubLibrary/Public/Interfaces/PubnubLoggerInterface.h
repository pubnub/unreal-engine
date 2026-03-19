// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PubnubStructLibrary.h"
#include "PubnubLoggerInterface.generated.h"

UINTERFACE(Blueprintable)
class PUBNUBLIBRARY_API UPubnubLoggerInterface : public UInterface
{
	GENERATED_BODY()
};

class PUBNUBLIBRARY_API IPubnubLoggerInterface
{
	GENERATED_BODY()

public:
	/** Common entry point for all log messages. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pubnub|Logger")
	void Log(const FPubnubLogMessage& LogMessage);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pubnub|Logger")
	void LogTrace(const FPubnubLogMessage& LogMessage);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pubnub|Logger")
	void LogDebug(const FPubnubLogMessage& LogMessage);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pubnub|Logger")
	void LogInfo(const FPubnubLogMessage& LogMessage);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pubnub|Logger")
	void LogWarning(const FPubnubLogMessage& LogMessage);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pubnub|Logger")
	void LogError(const FPubnubLogMessage& LogMessage);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pubnub|Logger")
	void SetMinimumLogLevel(EPubnubLogLevel InLevel);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pubnub|Logger")
	EPubnubLogLevel GetMinimumLogLevel() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pubnub|Logger")
	void SetMinimumCCoreLogLevel(EPubnubLogLevel InLevel);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pubnub|Logger")
	EPubnubLogLevel GetMinimumCCoreLogLevel() const;
};
