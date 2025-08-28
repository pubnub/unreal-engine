// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entities/PubnubBaseEntity.h"
#include "PubnubUserMetadataEntity.generated.h"

class UPubnubSubsystem;


/**
 *
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
	void SetUserMetadata(FPubnubUserData UserMetadata, FOnSetUserMetadataResponse OnSetUserMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

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
	void SetUserMetadata(FPubnubUserData UserMetadata, FOnSetUserMetadataResponseNative NativeCallback = nullptr, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Retrieves metadata for this User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param OnGetUserMetadataResponse The callback function used to handle the result.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void GetUserMetadata(FOnGetUserMetadataResponse OnGetUserMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Retrieves metadata for this User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	void GetUserMetadata(FOnGetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());
	

	/**
	 * Removes all metadata associated with this User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param OnRemoveUserMetadataResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnRemoveUserMetadataResponse"))
	void RemoveUserMetadata(FOnRemoveUserMetadataResponse OnRemoveUserMetadataResponse);

	/**
	 * Removes all metadata associated with this User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 */
	void RemoveUserMetadata(FOnRemoveUserMetadataResponseNative NativeCallback = nullptr);
	
};
