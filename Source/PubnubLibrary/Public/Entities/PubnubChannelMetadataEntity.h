// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entities/PubnubBaseEntity.h"
#include "PubnubChannelMetadataEntity.generated.h"


/**
 * Represents a PubNub Channel Metadata entity in Unreal Engine.
 * 
 * This class provides a complete interface for managing PubNub App Context channel metadata
 * within Unreal Engine. It enables operations for setting, retrieving, and removing metadata
 * associated with specific channels through Blueprint-compatible functions and delegates.
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubChannelMetadataEntity : public UPubnubBaseEntity
{
	GENERATED_BODY()
	
public:
	
	UPubnubChannelMetadataEntity();


	/**
	 * Sets metadata for this Channel in the PubNub App Context (blocking).
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param ChannelMetadata Channel Metadata object to set.
	 * @param Include (Optional) List of property names to include in the response.
	 * @return Result structure containing operation status and updated channel metadata.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	FPubnubChannelMetadataResult SetChannelMetadata(FPubnubChannelInputData ChannelMetadata, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Sets metadata for a specified Channel in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param ChannelMetadata Channel Metadata object to set.
	 * @param OnSetChannelMetadataResponse (Optional) Delegate to listen for the operation result.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnSetChannelMetadataResponse"))
	void SetChannelMetadataAsync(FPubnubChannelInputData ChannelMetadata, FOnPubnubSetChannelMetadataResponse OnSetChannelMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Sets metadata for a specified Channel in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param ChannelMetadata Channel Metadata object to set.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	void SetChannelMetadataAsync(FPubnubChannelInputData ChannelMetadata, FOnPubnubSetChannelMetadataResponseNative NativeCallback = nullptr, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());
	
	/**
	 * Retrieves metadata for this Channel from the PubNub App Context (blocking).
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Include (Optional) List of property names to include in the response.
	 * @return Result structure containing operation status and channel metadata.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	FPubnubChannelMetadataResult GetChannelMetadata(FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Retrieves metadata for a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param OnGetChannelMetadataResponse The callback function used to handle the result.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void GetChannelMetadataAsync(FOnPubnubGetChannelMetadataResponse OnGetChannelMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Retrieves metadata for a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	void GetChannelMetadataAsync(FOnPubnubGetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());
	
	/**
	 * Removes all metadata associated with this Channel from the PubNub App Context (blocking).
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @return Result structure containing operation status.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	FPubnubOperationResult RemoveChannelMetadata();

	/**
	 * Removes all metadata associated with a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param OnRemoveChannelMetadataResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnRemoveChannelMetadataResponse"))
	void RemoveChannelMetadataAsync(FOnPubnubRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse);

	/**
	 * Removes all metadata associated with a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 */
	void RemoveChannelMetadataAsync(FOnPubnubRemoveChannelMetadataResponseNative NativeCallback = nullptr);
	
};
