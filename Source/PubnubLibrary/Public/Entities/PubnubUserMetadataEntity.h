// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entities/PubnubBaseEntity.h"
#include "PubnubUserMetadataEntity.generated.h"


/**
 * Represents a PubNub User Metadata entity in Unreal Engine.
 * 
 * This class provides a complete interface for managing PubNub App Context user metadata
 * within Unreal Engine. It enables operations for setting, retrieving, and removing metadata
 * associated with specific users through Blueprint-compatible functions and delegates.
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubUserMetadataEntity : public UPubnubBaseEntity
{
	GENERATED_BODY()
	
public:
	
	UPubnubUserMetadataEntity();

		
	/**
	 * Sets metadata for this User in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param UserMetadata User Metadata object to set.
	 * @param OnSetUserMetadataResponse (Optional) Delegate to listen for the operation result.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnSetUserMetadataResponse"))
	void SetUserMetadataAsync(FPubnubUserData UserMetadata, FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Sets metadata for this User in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param UserMetadata User Metadata object to set.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	void SetUserMetadataAsync(FPubnubUserData UserMetadata, FOnPubnubSetUserMetadataResponseNative NativeCallback = nullptr, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Retrieves metadata for this User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param OnGetUserMetadataResponse The callback function used to handle the result.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void GetUserMetadataAsync(FOnPubnubGetUserMetadataResponse OnGetUserMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Retrieves metadata for this User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	void GetUserMetadataAsync(FOnPubnubGetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());
	

	/**
	 * Removes all metadata associated with this User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param OnRemoveUserMetadataResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnRemoveUserMetadataResponse"))
	void RemoveUserMetadataAsync(FOnPubnubRemoveUserMetadataResponse OnRemoveUserMetadataResponse);

	/**
	 * Removes all metadata associated with this User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 */
	void RemoveUserMetadataAsync(FOnPubnubRemoveUserMetadataResponseNative NativeCallback = nullptr);
	
};
