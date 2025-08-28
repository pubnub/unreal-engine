// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubChannelGroupEntity.h"


UPubnubChannelGroupEntity::UPubnubChannelGroupEntity()
{
	EntityType = EPubnubEntityType::PEnT_ChannelGroup;
}

void UPubnubChannelGroupEntity::AddChannelToGroup(FString Channel, FOnAddChannelToGroupResponse OnAddChannelToGroupResponse)
{
	PubnubSubsystem->AddChannelToGroup(Channel, EntityID, OnAddChannelToGroupResponse);
}

void UPubnubChannelGroupEntity::AddChannelToGroup(FString Channel, FOnAddChannelToGroupResponseNative NativeCallback)
{
	PubnubSubsystem->AddChannelToGroup(Channel, EntityID, NativeCallback);
}

void UPubnubChannelGroupEntity::RemoveChannelFromGroup(FString Channel, FOnRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse)
{
	PubnubSubsystem->RemoveChannelFromGroup(Channel, EntityID, OnRemoveChannelFromGroupResponse);
}

void UPubnubChannelGroupEntity::RemoveChannelFromGroup(FString Channel, FOnRemoveChannelFromGroupResponseNative NativeCallback)
{
	PubnubSubsystem->RemoveChannelFromGroup(Channel, EntityID, NativeCallback);
}

void UPubnubChannelGroupEntity::ListChannelsFromGroup(FOnListChannelsFromGroupResponse OnListChannelsResponse)
{
	PubnubSubsystem->ListChannelsFromGroup(EntityID, OnListChannelsResponse);
}

void UPubnubChannelGroupEntity::ListChannelsFromGroup(FOnListChannelsFromGroupResponseNative NativeCallback)
{
	PubnubSubsystem->ListChannelsFromGroup(EntityID, NativeCallback);
}

void UPubnubChannelGroupEntity::RemoveChannelGroup(FOnRemoveChannelGroupResponse OnRemoveChannelGroupResponse)
{
	PubnubSubsystem->RemoveChannelGroup(EntityID, OnRemoveChannelGroupResponse);
}

void UPubnubChannelGroupEntity::RemoveChannelGroup(FOnRemoveChannelGroupResponseNative NativeCallback)
{
	PubnubSubsystem->RemoveChannelGroup(EntityID, NativeCallback);
}

