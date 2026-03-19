// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entities/PubnubBaseEntity.h"
#include "PubnubChannelGroupEntity.generated.h"


/**
 * Represents a PubNub Channel Group entity in Unreal Engine.
 * 
 * This class provides a complete interface for managing PubNub channel groups within
 * Unreal Engine. It enables operations for adding/removing channels to/from groups,
 * listing channels in a group, and managing channel group lifecycle through 
 * Blueprint-compatible functions and delegates.
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubChannelGroupEntity : public UPubnubBaseEntity
{
	GENERATED_BODY()
	
public:
	
	UPubnubChannelGroupEntity();

	/**
	 * Adds a channel to this channel group (blocking).
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to add to the channel group.
	 * @return Result structure containing operation status.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups")
	FPubnubOperationResult AddChannelToGroup(FString Channel);

	/**
	 * Adds a channel to this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to add to the channel group.
	 * @param OnAddChannelToGroupResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups", meta = (AutoCreateRefTerm = "OnAddChannelToGroupResponse"))
	void AddChannelToGroupAsync(FString Channel, FOnPubnubAddChannelToGroupResponse OnAddChannelToGroupResponse);

	/**
	 * Adds a channel to this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to add to the channel group.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if operation result is not needed.
	 */
	void AddChannelToGroupAsync(FString Channel, FOnPubnubAddChannelToGroupResponseNative NativeCallback = nullptr);

	/**
	 * Removes a channel from this channel group (blocking).
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to remove from the channel group.
	 * @return Result structure containing operation status.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups")
	FPubnubOperationResult RemoveChannelFromGroup(FString Channel);

	/**
	 * Removes a channel from this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to remove from the channel group.
	 * @param OnRemoveChannelFromGroupResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups", meta = (AutoCreateRefTerm = "OnRemoveChannelFromGroupResponse"))
	void RemoveChannelFromGroupAsync(FString Channel, FOnPubnubRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse);

	/**
	 * Removes a channel from this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to remove from the channel group.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if operation result is not needed.
	 */
	void RemoveChannelFromGroupAsync(FString Channel, FOnPubnubRemoveChannelFromGroupResponseNative NativeCallback = nullptr);

	/**
	 * Lists the channels that belong to this channel group (blocking).
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @return Result structure containing operation status and list of channels.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups")
	FPubnubListChannelsFromGroupResult ListChannelsFromGroup();

	/**
	 * Lists the channels that belong to this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param OnListChannelsResponse The callback function used to handle the result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups")
	void ListChannelsFromGroupAsync(FOnPubnubListChannelsFromGroupResponse OnListChannelsResponse);

	/**
	 * Lists the channels that belong to this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 */
	void ListChannelsFromGroupAsync(FOnPubnubListChannelsFromGroupResponseNative NativeCallback);

	/**
	 * Removes this channel group (blocking).
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @return Result structure containing operation status.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups")
	FPubnubOperationResult RemoveChannelGroup();

	/**
	 * Removes this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param OnRemoveChannelGroupResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups", meta = (AutoCreateRefTerm = "OnRemoveChannelGroupResponse"))
	void RemoveChannelGroupAsync(FOnPubnubRemoveChannelGroupResponse OnRemoveChannelGroupResponse);

	/**
	 * Removes this channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if operation result is not needed.
	 */
	void RemoveChannelGroupAsync(FOnPubnubRemoveChannelGroupResponseNative NativeCallback = nullptr);
	
};
