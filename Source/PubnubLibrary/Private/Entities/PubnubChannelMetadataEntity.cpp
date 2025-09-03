// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubChannelMetadataEntity.h"


UPubnubChannelMetadataEntity::UPubnubChannelMetadataEntity()
{
	EntityType = EPubnubEntityType::PEnT_ChannelMetadata;
}

void UPubnubChannelMetadataEntity::SetChannelMetadata(FPubnubChannelData ChannelMetadata, FOnSetChannelMetadataResponse OnSetChannelMetadataResponse, FPubnubGetMetadataInclude Include)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set channel metadata - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->SetChannelMetadata(EntityID, ChannelMetadata, OnSetChannelMetadataResponse, Include);
}

void UPubnubChannelMetadataEntity::SetChannelMetadata(FPubnubChannelData ChannelMetadata, FOnSetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set channel metadata - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->SetChannelMetadata(EntityID, ChannelMetadata, NativeCallback, Include);
}

void UPubnubChannelMetadataEntity::GetChannelMetadata(FOnGetChannelMetadataResponse OnGetChannelMetadataResponse, FPubnubGetMetadataInclude Include)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get channel metadata - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->GetChannelMetadata(EntityID, OnGetChannelMetadataResponse, Include);
}

void UPubnubChannelMetadataEntity::GetChannelMetadata(FOnGetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get channel metadata - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->GetChannelMetadata(EntityID, NativeCallback, Include);
}

void UPubnubChannelMetadataEntity::RemoveChannelMetadata(FOnRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel metadata - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->RemoveChannelMetadata(EntityID, OnRemoveChannelMetadataResponse);
}

void UPubnubChannelMetadataEntity::RemoveChannelMetadata(FOnRemoveChannelMetadataResponseNative NativeCallback)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel metadata - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->RemoveChannelMetadata(EntityID, NativeCallback);
}

