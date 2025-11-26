// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entities/PubnubBaseEntity.h"
#include "PubnubChannelEntity.generated.h"


/**
 * Represents a PubNub Channel entity in Unreal Engine.
 * 
 * This class provides a complete interface for PubNub channel operations within Unreal Engine.
 * It enables channel-specific operations such as publishing messages, sending signals,
 * and managing presence information through Blueprint-compatible functions and delegates.
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubChannelEntity : public UPubnubBaseEntity
{
	GENERATED_BODY()
	
public:
	
	UPubnubChannelEntity();
	
	/**
	 * Publishes a message to this channel (blocking).
	 * 
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 * @return Result structure containing operation status and published message data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel")
	FPubnubPublishMessageResult PublishMessage(FString Message, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

	/**
	 * Publishes a message to this channel.
	 * 
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param OnPublishMessageResponse Optional delegate to listen for the publish result.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel", meta = (AutoCreateRefTerm = "OnPublishMessageResponse"))
	void PublishMessageAsync(FString Message, FOnPubnubPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

	/**
	 * Publishes a message to this channel.
	 * 
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param NativeCallback Optional delegate to listen for the publish result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if publish result is not needed.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	void PublishMessageAsync(FString Message, FOnPubnubPublishMessageResponseNative NativeCallback = nullptr, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

	/**
	 * Publishes a message to this channel. Overload without delegate to get publish result.
	 * 
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	void PublishMessageAsync(FString Message, FPubnubPublishSettings PublishSettings);

	/**
	 * Sends a signal to this channel (blocking).
	 * 
	 * @param Message The message to send as the signal. This message can be any data type that can be serialized into JSON.
	 * @param SignalSettings Optional settings for the signal operation. See FPubnubSignalSettings for more details.
	 * @return Result structure containing operation status and signal message data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel")
	FPubnubSignalResult Signal(FString Message, FPubnubSignalSettings SignalSettings = FPubnubSignalSettings());

	/**
	 * Sends a signal to a this channel.
	 * 
	 * @param Message The message to send as the signal. This message can be any data type that can be serialized into JSON.
	 * @param OnSignalResponse Optional delegate to listen for the signal result.
	 * @param SignalSettings Optional settings for the signal operation. See FPubnubSignalSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel", meta = (AutoCreateRefTerm = "OnSignalResponse"))
	void SignalAsync(FString Message, FOnPubnubSignalResponse OnSignalResponse, FPubnubSignalSettings SignalSettings = FPubnubSignalSettings());

	/**
	 * Sends a signal to this channel.
	 * 
	 * @param Message The message to send as the signal. This message can be any data type that can be serialized into JSON.
	 * @param NativeCallback Optional delegate to listen for the signal result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if signal result is not needed.
	 * @param SignalSettings Optional settings for the signal operation. See FPubnubSignalSettings for more details.
	 */
	void SignalAsync(FString Message, FOnPubnubSignalResponseNative NativeCallback = nullptr, FPubnubSignalSettings SignalSettings = FPubnubSignalSettings());
	
	/**
	 * Sends a signal to this channel. Overload without delegate to get signal result.
	 * 
	 * @param Message The message to send as the signal. This message can be any data type that can be serialized into JSON.
	 * @param SignalSettings Optional settings for the signal operation. See FPubnubSignalSettings for more details.
	 */
	void SignalAsync(FString Message, FPubnubSignalSettings SignalSettings);

	/**
	 * Lists the users currently present on this channel (blocking).
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param ListUsersFromChannelSettings Optional settings for the list users operation. See FPubnubListUsersFromChannelSettings for more details.
	 * @return Result structure containing operation status and list of users.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	FPubnubListUsersFromChannelResult ListUsersFromChannel(FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());

	/**
	 * Lists the users currently present on this channel.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param ListUsersFromChannelResponse The callback function used to handle the result.
	 * @param ListUsersFromChannelSettings Optional settings for the list users operation. See FPubnubListUsersFromChannelSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void ListUsersFromChannelAsync(FOnPubnubListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());

	/**
	 * Lists the users currently present on this channel.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param ListUsersFromChannelSettings Optional settings for the list users operation. See FPubnubListUsersFromChannelSettings for more details. 
	 */
	void ListUsersFromChannelAsync(FOnPubnubListUsersFromChannelResponseNative NativeCallback, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());
	
};
