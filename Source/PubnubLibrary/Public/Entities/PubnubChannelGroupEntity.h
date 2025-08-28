// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entities/PubnubBaseEntity.h"
#include "PubnubChannelGroupEntity.generated.h"

class UPubnubSubsystem;


/**
 *
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubChannelGroupEntity : public UPubnubBaseEntity
{
	GENERATED_BODY()
	
public:
	
	UPubnubChannelGroupEntity();

	/**
	 * Adds a channel to this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to add to the channel group.
	 * @param OnAddChannelToGroupResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups", meta = (AutoCreateRefTerm = "OnAddChannelToGroupResponse"))
	void AddChannelToGroup(FString Channel, FOnAddChannelToGroupResponse OnAddChannelToGroupResponse);

	/**
	 * Adds a channel to this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to add to the channel group.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if operation result is not needed.
	 */
	void AddChannelToGroup(FString Channel, FOnAddChannelToGroupResponseNative NativeCallback = nullptr);

	/**
	 * Removes a channel from this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to remove from the channel group.
	 * @param OnRemoveChannelFromGroupResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups", meta = (AutoCreateRefTerm = "OnRemoveChannelFromGroupResponse"))
	void RemoveChannelFromGroup(FString Channel, FOnRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse);

	/**
	 * Removes a channel from this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to remove from the channel group.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if operation result is not needed.
	 */
	void RemoveChannelFromGroup(FString Channel, FOnRemoveChannelFromGroupResponseNative NativeCallback = nullptr);

	/**
	 * Lists the channels that belong to this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param OnListChannelsResponse The callback function used to handle the result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups")
	void ListChannelsFromGroup(FOnListChannelsFromGroupResponse OnListChannelsResponse);

	/**
	 * Lists the channels that belong to this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 */
	void ListChannelsFromGroup(FOnListChannelsFromGroupResponseNative NativeCallback);

	/**
	 * Removes this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param OnRemoveChannelGroupResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups", meta = (AutoCreateRefTerm = "OnRemoveChannelGroupResponse"))
	void RemoveChannelGroup(FOnRemoveChannelGroupResponse OnRemoveChannelGroupResponse);

	/**
	 * Removes this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if operation result is not needed.
	 */
	void RemoveChannelGroup(FOnRemoveChannelGroupResponseNative NativeCallback = nullptr);
	
};
