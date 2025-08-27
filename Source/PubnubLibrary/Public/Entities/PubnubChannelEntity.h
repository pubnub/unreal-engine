// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entities/PubnubBaseEntity.h"
#include "PubnubChannelEntity.generated.h"

class UPubnubSubsystem;


/**
 *
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubChannelEntity : public UPubnubBaseEntity
{
	GENERATED_BODY()
	
public:
	
	UPubnubChannelEntity();
	
	/**
	 * Publishes a message to a specified channel.
	 * 
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param OnPublishMessageResponse Optional delegate to listen for the publish result.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel", meta = (AutoCreateRefTerm = "OnPublishMessageResponse"))
	void PublishMessage(FString Message, FOnPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

	/**
	 * Publishes a message to a specified channel.
	 * 
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param NativeCallback Optional delegate to listen for the publish result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if publish result is not needed.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	void PublishMessage(FString Message, FOnPublishMessageResponseNative NativeCallback = nullptr, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

	/**
	 * Publishes a message to a specified channel. Overload without delegate to get publish result.
	 * 
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	void PublishMessage(FString Message, FPubnubPublishSettings PublishSettings);

	/**
	 * Sends a signal to a specified channel.
	 * 
	 * @param Message The message to send as the signal. This message can be any data type that can be serialized into JSON.
	 * @param OnSignalResponse Optional delegate to listen for the signal result.
	 * @param SignalSettings Optional settings for the signal operation. See FPubnubSignalSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel", meta = (AutoCreateRefTerm = "OnSignalResponse"))
	void Signal(FString Message, FOnSignalResponse OnSignalResponse, FPubnubSignalSettings SignalSettings = FPubnubSignalSettings());

	/**
	 * Sends a signal to a specified channel.
	 * 
	 * @param Message The message to send as the signal. This message can be any data type that can be serialized into JSON.
	 * @param NativeCallback Optional delegate to listen for the signal result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if signal result is not needed.
	 * @param SignalSettings Optional settings for the signal operation. See FPubnubSignalSettings for more details.
	 */
	void Signal(FString Message, FOnSignalResponseNative NativeCallback = nullptr, FPubnubSignalSettings SignalSettings = FPubnubSignalSettings());
	
	/**
	 * Sends a signal to a specified channel. Overload without delegate to get signal result.
	 * 
	 * @param Message The message to send as the signal. This message can be any data type that can be serialized into JSON.
	 * @param SignalSettings Optional settings for the signal operation. See FPubnubSignalSettings for more details.
	 */
	void Signal(FString Message, FPubnubSignalSettings SignalSettings);

	//UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel")
	
	
};
