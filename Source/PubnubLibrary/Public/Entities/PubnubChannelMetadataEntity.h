// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entities/PubnubBaseEntity.h"
#include "PubnubChannelMetadataEntity.generated.h"

class UPubnubSubsystem;


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
	 * Sets metadata for a specified Channel in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param ChannelMetadata Channel Metadata object to set.
	 * @param OnSetChannelMetadataResponse (Optional) Delegate to listen for the operation result.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnSetChannelMetadataResponse"))
	void SetChannelMetadata(FPubnubChannelData ChannelMetadata, FOnSetChannelMetadataResponse OnSetChannelMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

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
	void SetChannelMetadata(FPubnubChannelData ChannelMetadata, FOnSetChannelMetadataResponseNative NativeCallback = nullptr, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());
	
	/**
	 * Retrieves metadata for a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param OnGetChannelMetadataResponse The callback function used to handle the result.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void GetChannelMetadata(FOnGetChannelMetadataResponse OnGetChannelMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Retrieves metadata for a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	void GetChannelMetadata(FOnGetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());
	
	/**
	 * Removes all metadata associated with a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param OnRemoveChannelMetadataResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnRemoveChannelMetadataResponse"))
	void RemoveChannelMetadata(FOnRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse);

	/**
	 * Removes all metadata associated with a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 */
	void RemoveChannelMetadata(FOnRemoveChannelMetadataResponseNative NativeCallback = nullptr);
	
};
