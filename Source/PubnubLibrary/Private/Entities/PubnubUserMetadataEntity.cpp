// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubUserMetadataEntity.h"
#include "PubnubSubsystem.h"


UPubnubUserMetadataEntity::UPubnubUserMetadataEntity()
{
	EntityType = EPubnubEntityType::PEnT_UserMetadata;
}

void UPubnubUserMetadataEntity::SetUserMetadataAsync(FPubnubUserData UserMetadata, FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->SetUserMetadataAsync(EntityID, UserMetadata, OnSetUserMetadataResponse, Include);
}

void UPubnubUserMetadataEntity::SetUserMetadataAsync(FPubnubUserData UserMetadata, FOnPubnubSetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->SetUserMetadataAsync(EntityID, UserMetadata, NativeCallback, Include);
}

void UPubnubUserMetadataEntity::GetUserMetadataAsync(FOnPubnubGetUserMetadataResponse OnGetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->GetUserMetadataAsync(EntityID, OnGetUserMetadataResponse, Include);
}

void UPubnubUserMetadataEntity::GetUserMetadataAsync(FOnPubnubGetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->GetUserMetadataAsync(EntityID, NativeCallback, Include);
}

void UPubnubUserMetadataEntity::RemoveUserMetadataAsync(FOnPubnubRemoveUserMetadataResponse OnRemoveUserMetadataResponse)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveUserMetadataAsync(EntityID, OnRemoveUserMetadataResponse);
}

void UPubnubUserMetadataEntity::RemoveUserMetadataAsync(FOnPubnubRemoveUserMetadataResponseNative NativeCallback)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveUserMetadataAsync(EntityID, NativeCallback);
}

