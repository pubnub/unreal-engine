// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubChannelMetadataEntity.h"
#include "PubnubSubsystem.h"


UPubnubChannelMetadataEntity::UPubnubChannelMetadataEntity()
{
	EntityType = EPubnubEntityType::PEnT_ChannelMetadata;
}

void UPubnubChannelMetadataEntity::SetChannelMetadata(FPubnubChannelData ChannelMetadata, FOnPubnubSetChannelMetadataResponse OnSetChannelMetadataResponse, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set channel metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->SetChannelMetadata(EntityID, ChannelMetadata, OnSetChannelMetadataResponse, Include);
}

void UPubnubChannelMetadataEntity::SetChannelMetadata(FPubnubChannelData ChannelMetadata, FOnPubnubSetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set channel metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->SetChannelMetadata(EntityID, ChannelMetadata, NativeCallback, Include);
}

void UPubnubChannelMetadataEntity::GetChannelMetadata(FOnPubnubGetChannelMetadataResponse OnGetChannelMetadataResponse, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get channel metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->GetChannelMetadata(EntityID, OnGetChannelMetadataResponse, Include);
}

void UPubnubChannelMetadataEntity::GetChannelMetadata(FOnPubnubGetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get channel metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->GetChannelMetadata(EntityID, NativeCallback, Include);
}

void UPubnubChannelMetadataEntity::RemoveChannelMetadata(FOnPubnubRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveChannelMetadata(EntityID, OnRemoveChannelMetadataResponse);
}

void UPubnubChannelMetadataEntity::RemoveChannelMetadata(FOnPubnubRemoveChannelMetadataResponseNative NativeCallback)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveChannelMetadata(EntityID, NativeCallback);
}

