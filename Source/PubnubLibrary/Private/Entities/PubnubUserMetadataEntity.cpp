// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubUserMetadataEntity.h"
#include "PubnubSubsystem.h"


UPubnubUserMetadataEntity::UPubnubUserMetadataEntity()
{
	EntityType = EPubnubEntityType::PEnT_UserMetadata;
}

void UPubnubUserMetadataEntity::SetUserMetadata(FPubnubUserData UserMetadata, FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->SetUserMetadata(EntityID, UserMetadata, OnSetUserMetadataResponse, Include);
}

void UPubnubUserMetadataEntity::SetUserMetadata(FPubnubUserData UserMetadata, FOnPubnubSetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->SetUserMetadata(EntityID, UserMetadata, NativeCallback, Include);
}

void UPubnubUserMetadataEntity::GetUserMetadata(FOnPubnubGetUserMetadataResponse OnGetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->GetUserMetadata(EntityID, OnGetUserMetadataResponse, Include);
}

void UPubnubUserMetadataEntity::GetUserMetadata(FOnPubnubGetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->GetUserMetadata(EntityID, NativeCallback, Include);
}

void UPubnubUserMetadataEntity::RemoveUserMetadata(FOnPubnubRemoveUserMetadataResponse OnRemoveUserMetadataResponse)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveUserMetadata(EntityID, OnRemoveUserMetadataResponse);
}

void UPubnubUserMetadataEntity::RemoveUserMetadata(FOnPubnubRemoveUserMetadataResponseNative NativeCallback)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveUserMetadata(EntityID, NativeCallback);
}

