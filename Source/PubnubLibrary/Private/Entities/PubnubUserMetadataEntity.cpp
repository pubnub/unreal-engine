// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubUserMetadataEntity.h"


UPubnubUserMetadataEntity::UPubnubUserMetadataEntity()
{
	EntityType = EPubnubEntityType::PEnT_UserMetadata;
}

void UPubnubUserMetadataEntity::SetUserMetadata(FPubnubUserData UserMetadata, FOnSetUserMetadataResponse OnSetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	PubnubSubsystem->SetUserMetadata(EntityID, UserMetadata, OnSetUserMetadataResponse, Include);
}

void UPubnubUserMetadataEntity::SetUserMetadata(FPubnubUserData UserMetadata, FOnSetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	PubnubSubsystem->SetUserMetadata(EntityID, UserMetadata, NativeCallback, Include);
}

void UPubnubUserMetadataEntity::GetUserMetadata(FOnGetUserMetadataResponse OnGetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	PubnubSubsystem->GetUserMetadata(EntityID, OnGetUserMetadataResponse, Include);
}

void UPubnubUserMetadataEntity::GetUserMetadata(FOnGetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	PubnubSubsystem->GetUserMetadata(EntityID, NativeCallback, Include);
}

void UPubnubUserMetadataEntity::RemoveUserMetadata(FOnRemoveUserMetadataResponse OnRemoveUserMetadataResponse)
{
	PubnubSubsystem->RemoveUserMetadata(EntityID, OnRemoveUserMetadataResponse);
}

void UPubnubUserMetadataEntity::RemoveUserMetadata(FOnRemoveUserMetadataResponseNative NativeCallback)
{
	PubnubSubsystem->RemoveUserMetadata(EntityID, NativeCallback);
}

