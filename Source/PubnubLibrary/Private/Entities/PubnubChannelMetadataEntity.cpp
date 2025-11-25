// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubChannelMetadataEntity.h"
#include "PubnubSubsystem.h"


UPubnubChannelMetadataEntity::UPubnubChannelMetadataEntity()
{
	EntityType = EPubnubEntityType::PEnT_ChannelMetadata;
}

void UPubnubChannelMetadataEntity::SetChannelMetadataAsync(FPubnubChannelData ChannelMetadata, FOnPubnubSetChannelMetadataResponse OnSetChannelMetadataResponse, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set channel metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->SetChannelMetadataAsync(EntityID, ChannelMetadata, OnSetChannelMetadataResponse, Include);
}

void UPubnubChannelMetadataEntity::SetChannelMetadataAsync(FPubnubChannelData ChannelMetadata, FOnPubnubSetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set channel metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->SetChannelMetadataAsync(EntityID, ChannelMetadata, NativeCallback, Include);
}

void UPubnubChannelMetadataEntity::GetChannelMetadataAsync(FOnPubnubGetChannelMetadataResponse OnGetChannelMetadataResponse, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get channel metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->GetChannelMetadataAsync(EntityID, OnGetChannelMetadataResponse, Include);
}

void UPubnubChannelMetadataEntity::GetChannelMetadataAsync(FOnPubnubGetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get channel metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->GetChannelMetadataAsync(EntityID, NativeCallback, Include);
}

void UPubnubChannelMetadataEntity::RemoveChannelMetadataAsync(FOnPubnubRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveChannelMetadataAsync(EntityID, OnRemoveChannelMetadataResponse);
}

void UPubnubChannelMetadataEntity::RemoveChannelMetadataAsync(FOnPubnubRemoveChannelMetadataResponseNative NativeCallback)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveChannelMetadataAsync(EntityID, NativeCallback);
}

