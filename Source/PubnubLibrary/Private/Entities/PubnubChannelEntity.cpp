// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubChannelEntity.h"
#include "PubnubSubsystem.h"


UPubnubChannelEntity::UPubnubChannelEntity()
{
	EntityType = EPubnubEntityType::PEnT_Channel;
}

FPubnubPublishMessageResult UPubnubChannelEntity::PublishMessage(FString Message, FPubnubPublishSettings PublishSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot publish message - PubnubClient is null. Entity not properly initialized."));
		return FPubnubPublishMessageResult();
	}
	if (EntityID.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot publish message - EntityID is empty."));
		return FPubnubPublishMessageResult();
	}
	return PubnubClient->PublishMessage(EntityID, Message, PublishSettings);
}

void UPubnubChannelEntity::PublishMessageAsync(FString Message, FOnPubnubPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot publish message - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	if (EntityID.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot publish message - EntityID is empty."));
		return;
	}
	PubnubClient->PublishMessageAsync(EntityID, Message, OnPublishMessageResponse, PublishSettings);
}

void UPubnubChannelEntity::PublishMessageAsync(FString Message, FOnPubnubPublishMessageResponseNative NativeCallback, FPubnubPublishSettings PublishSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot publish message - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->PublishMessageAsync(EntityID, Message, NativeCallback, PublishSettings);
}

void UPubnubChannelEntity::PublishMessageAsync(FString Message, FPubnubPublishSettings PublishSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot publish message - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->PublishMessageAsync(EntityID, Message, PublishSettings);
}

FPubnubSignalResult UPubnubChannelEntity::Signal(FString Message, FPubnubSignalSettings SignalSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot send signal - PubnubClient is null. Entity not properly initialized."));
		return FPubnubSignalResult();
	}
	if (EntityID.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot send signal - EntityID is empty."));
		return FPubnubSignalResult();
	}
	return PubnubClient->Signal(EntityID, Message, SignalSettings);
}

void UPubnubChannelEntity::SignalAsync(FString Message, FOnPubnubSignalResponse OnSignalResponse, FPubnubSignalSettings SignalSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot send signal - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->SignalAsync(EntityID, Message, OnSignalResponse, SignalSettings);
}

void UPubnubChannelEntity::SignalAsync(FString Message, FOnPubnubSignalResponseNative NativeCallback, FPubnubSignalSettings SignalSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot send signal - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->SignalAsync(EntityID, Message, NativeCallback, SignalSettings);
}

void UPubnubChannelEntity::SignalAsync(FString Message, FPubnubSignalSettings SignalSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot send signal - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->SignalAsync(EntityID, Message, SignalSettings);
}

FPubnubListUsersFromChannelResult UPubnubChannelEntity::ListUsersFromChannel(FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list users from channel - PubnubClient is null. Entity not properly initialized."));
		return FPubnubListUsersFromChannelResult();
	}
	if (EntityID.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list users from channel - EntityID is empty."));
		return FPubnubListUsersFromChannelResult();
	}
	return PubnubClient->ListUsersFromChannel(EntityID, ListUsersFromChannelSettings);
}

void UPubnubChannelEntity::ListUsersFromChannelAsync(FOnPubnubListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list users from channel - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->ListUsersFromChannelAsync(EntityID, ListUsersFromChannelResponse, ListUsersFromChannelSettings);
}

void UPubnubChannelEntity::ListUsersFromChannelAsync(FOnPubnubListUsersFromChannelResponseNative NativeCallback, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list users from channel - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->ListUsersFromChannelAsync(EntityID, NativeCallback, ListUsersFromChannelSettings);
}
