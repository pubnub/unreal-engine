// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubChannelGroupEntity.h"


UPubnubChannelGroupEntity::UPubnubChannelGroupEntity()
{
	EntityType = EPubnubEntityType::PEnT_ChannelGroup;
}

void UPubnubChannelGroupEntity::AddChannelToGroup(FString Channel, FOnAddChannelToGroupResponse OnAddChannelToGroupResponse)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot add channel to group - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->AddChannelToGroup(Channel, EntityID, OnAddChannelToGroupResponse);
}

void UPubnubChannelGroupEntity::AddChannelToGroup(FString Channel, FOnAddChannelToGroupResponseNative NativeCallback)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot add channel to group - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->AddChannelToGroup(Channel, EntityID, NativeCallback);
}

void UPubnubChannelGroupEntity::RemoveChannelFromGroup(FString Channel, FOnRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel from group - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->RemoveChannelFromGroup(Channel, EntityID, OnRemoveChannelFromGroupResponse);
}

void UPubnubChannelGroupEntity::RemoveChannelFromGroup(FString Channel, FOnRemoveChannelFromGroupResponseNative NativeCallback)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel from group - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->RemoveChannelFromGroup(Channel, EntityID, NativeCallback);
}

void UPubnubChannelGroupEntity::ListChannelsFromGroup(FOnListChannelsFromGroupResponse OnListChannelsResponse)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list channels from group - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->ListChannelsFromGroup(EntityID, OnListChannelsResponse);
}

void UPubnubChannelGroupEntity::ListChannelsFromGroup(FOnListChannelsFromGroupResponseNative NativeCallback)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list channels from group - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->ListChannelsFromGroup(EntityID, NativeCallback);
}

void UPubnubChannelGroupEntity::RemoveChannelGroup(FOnRemoveChannelGroupResponse OnRemoveChannelGroupResponse)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel group - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->RemoveChannelGroup(EntityID, OnRemoveChannelGroupResponse);
}

void UPubnubChannelGroupEntity::RemoveChannelGroup(FOnRemoveChannelGroupResponseNative NativeCallback)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel group - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->RemoveChannelGroup(EntityID, NativeCallback);
}

