// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubChannelGroupEntity.h"
#include "PubnubSubsystem.h"


UPubnubChannelGroupEntity::UPubnubChannelGroupEntity()
{
	EntityType = EPubnubEntityType::PEnT_ChannelGroup;
}

FPubnubOperationResult UPubnubChannelGroupEntity::AddChannelToGroup(FString Channel)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot add channel to group - PubnubClient is null. Entity not properly initialized."));
		return FPubnubOperationResult();
	}
	if (EntityID.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot add channel to group - EntityID is empty."));
		return FPubnubOperationResult();
	}
	return PubnubClient->AddChannelToGroup(Channel, EntityID);
}

void UPubnubChannelGroupEntity::AddChannelToGroupAsync(FString Channel, FOnPubnubAddChannelToGroupResponse OnAddChannelToGroupResponse)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot add channel to group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->AddChannelToGroupAsync(Channel, EntityID, OnAddChannelToGroupResponse);
}

void UPubnubChannelGroupEntity::AddChannelToGroupAsync(FString Channel, FOnPubnubAddChannelToGroupResponseNative NativeCallback)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot add channel to group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->AddChannelToGroupAsync(Channel, EntityID, NativeCallback);
}

FPubnubOperationResult UPubnubChannelGroupEntity::RemoveChannelFromGroup(FString Channel)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel from group - PubnubClient is null. Entity not properly initialized."));
		return FPubnubOperationResult();
	}
	if (EntityID.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel from group - EntityID is empty."));
		return FPubnubOperationResult();
	}
	return PubnubClient->RemoveChannelFromGroup(Channel, EntityID);
}

void UPubnubChannelGroupEntity::RemoveChannelFromGroupAsync(FString Channel, FOnPubnubRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel from group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveChannelFromGroupAsync(Channel, EntityID, OnRemoveChannelFromGroupResponse);
}

void UPubnubChannelGroupEntity::RemoveChannelFromGroupAsync(FString Channel, FOnPubnubRemoveChannelFromGroupResponseNative NativeCallback)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel from group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveChannelFromGroupAsync(Channel, EntityID, NativeCallback);
}

FPubnubListChannelsFromGroupResult UPubnubChannelGroupEntity::ListChannelsFromGroup()
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list channels from group - PubnubClient is null. Entity not properly initialized."));
		return FPubnubListChannelsFromGroupResult();
	}
	if (EntityID.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list channels from group - EntityID is empty."));
		return FPubnubListChannelsFromGroupResult();
	}
	return PubnubClient->ListChannelsFromGroup(EntityID);
}

void UPubnubChannelGroupEntity::ListChannelsFromGroupAsync(FOnPubnubListChannelsFromGroupResponse OnListChannelsResponse)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list channels from group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->ListChannelsFromGroupAsync(EntityID, OnListChannelsResponse);
}

void UPubnubChannelGroupEntity::ListChannelsFromGroupAsync(FOnPubnubListChannelsFromGroupResponseNative NativeCallback)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list channels from group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->ListChannelsFromGroupAsync(EntityID, NativeCallback);
}

FPubnubOperationResult UPubnubChannelGroupEntity::RemoveChannelGroup()
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel group - PubnubClient is null. Entity not properly initialized."));
		return FPubnubOperationResult();
	}
	if (EntityID.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel group - EntityID is empty."));
		return FPubnubOperationResult();
	}
	return PubnubClient->RemoveChannelGroup(EntityID);
}

void UPubnubChannelGroupEntity::RemoveChannelGroupAsync(FOnPubnubRemoveChannelGroupResponse OnRemoveChannelGroupResponse)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveChannelGroupAsync(EntityID, OnRemoveChannelGroupResponse);
}

void UPubnubChannelGroupEntity::RemoveChannelGroupAsync(FOnPubnubRemoveChannelGroupResponseNative NativeCallback)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove channel group - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveChannelGroupAsync(EntityID, NativeCallback);
}

