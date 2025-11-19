// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubChannelGroupEntity.h"
#include "PubnubSubsystem.h"


UPubnubChannelGroupEntity::UPubnubChannelGroupEntity()
{
	EntityType = EPubnubEntityType::PEnT_ChannelGroup;
}

void UPubnubChannelGroupEntity::AddChannelToGroup(FString Channel, FOnPubnubAddChannelToGroupResponse OnAddChannelToGroupResponse)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot add channel to group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->AddChannelToGroup(Channel, EntityID, OnAddChannelToGroupResponse);
}

void UPubnubChannelGroupEntity::AddChannelToGroup(FString Channel, FOnPubnubAddChannelToGroupResponseNative NativeCallback)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot add channel to group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->AddChannelToGroup(Channel, EntityID, NativeCallback);
}

void UPubnubChannelGroupEntity::RemoveChannelFromGroup(FString Channel, FOnPubnubRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel from group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveChannelFromGroup(Channel, EntityID, OnRemoveChannelFromGroupResponse);
}

void UPubnubChannelGroupEntity::RemoveChannelFromGroup(FString Channel, FOnPubnubRemoveChannelFromGroupResponseNative NativeCallback)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel from group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveChannelFromGroup(Channel, EntityID, NativeCallback);
}

void UPubnubChannelGroupEntity::ListChannelsFromGroup(FOnPubnubListChannelsFromGroupResponse OnListChannelsResponse)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list channels from group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->ListChannelsFromGroup(EntityID, OnListChannelsResponse);
}

void UPubnubChannelGroupEntity::ListChannelsFromGroup(FOnPubnubListChannelsFromGroupResponseNative NativeCallback)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list channels from group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->ListChannelsFromGroup(EntityID, NativeCallback);
}

void UPubnubChannelGroupEntity::RemoveChannelGroup(FOnPubnubRemoveChannelGroupResponse OnRemoveChannelGroupResponse)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveChannelGroup(EntityID, OnRemoveChannelGroupResponse);
}

void UPubnubChannelGroupEntity::RemoveChannelGroup(FOnPubnubRemoveChannelGroupResponseNative NativeCallback)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveChannelGroup(EntityID, NativeCallback);
}

