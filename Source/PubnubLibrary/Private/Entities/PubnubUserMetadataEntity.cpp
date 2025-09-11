// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubUserMetadataEntity.h"


UPubnubUserMetadataEntity::UPubnubUserMetadataEntity()
{
	EntityType = EPubnubEntityType::PEnT_UserMetadata;
}

void UPubnubUserMetadataEntity::SetUserMetadata(FPubnubUserData UserMetadata, FOnSetUserMetadataResponse OnSetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set user metadata - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->SetUserMetadata(EntityID, UserMetadata, OnSetUserMetadataResponse, Include);
}

void UPubnubUserMetadataEntity::SetUserMetadata(FPubnubUserData UserMetadata, FOnSetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set user metadata - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->SetUserMetadata(EntityID, UserMetadata, NativeCallback, Include);
}

void UPubnubUserMetadataEntity::GetUserMetadata(FOnGetUserMetadataResponse OnGetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get user metadata - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->GetUserMetadata(EntityID, OnGetUserMetadataResponse, Include);
}

void UPubnubUserMetadataEntity::GetUserMetadata(FOnGetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get user metadata - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->GetUserMetadata(EntityID, NativeCallback, Include);
}

void UPubnubUserMetadataEntity::RemoveUserMetadata(FOnRemoveUserMetadataResponse OnRemoveUserMetadataResponse)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove user metadata - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->RemoveUserMetadata(EntityID, OnRemoveUserMetadataResponse);
}

void UPubnubUserMetadataEntity::RemoveUserMetadata(FOnRemoveUserMetadataResponseNative NativeCallback)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove user metadata - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->RemoveUserMetadata(EntityID, NativeCallback);
}

