// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubChannelMetadataEntity.h"


UPubnubChannelMetadataEntity::UPubnubChannelMetadataEntity()
{
	EntityType = EPubnubEntityType::PEnT_ChannelMetadata;
}

void UPubnubChannelMetadataEntity::SetChannelMetadata(FPubnubChannelData ChannelMetadata, FOnSetChannelMetadataResponse OnSetChannelMetadataResponse, FPubnubGetMetadataInclude Include)
{
	PubnubSubsystem->SetChannelMetadata(EntityID, ChannelMetadata, OnSetChannelMetadataResponse, Include);
}

void UPubnubChannelMetadataEntity::SetChannelMetadata(FPubnubChannelData ChannelMetadata, FOnSetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	PubnubSubsystem->SetChannelMetadata(EntityID, ChannelMetadata, NativeCallback, Include);
}

void UPubnubChannelMetadataEntity::GetChannelMetadata(FOnGetChannelMetadataResponse OnGetChannelMetadataResponse, FPubnubGetMetadataInclude Include)
{
	PubnubSubsystem->GetChannelMetadata(EntityID, OnGetChannelMetadataResponse, Include);
}

void UPubnubChannelMetadataEntity::GetChannelMetadata(FOnGetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	PubnubSubsystem->GetChannelMetadata(EntityID, NativeCallback, Include);
}

void UPubnubChannelMetadataEntity::RemoveChannelMetadata(FOnRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse)
{
	PubnubSubsystem->RemoveChannelMetadata(EntityID, OnRemoveChannelMetadataResponse);
}

void UPubnubChannelMetadataEntity::RemoveChannelMetadata(FOnRemoveChannelMetadataResponseNative NativeCallback)
{
	PubnubSubsystem->RemoveChannelMetadata(EntityID, NativeCallback);
}

