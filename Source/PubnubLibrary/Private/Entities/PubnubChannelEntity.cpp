// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubChannelEntity.h"
#include "PubnubSubsystem.h"


UPubnubChannelEntity::UPubnubChannelEntity()
{
	EntityType = EPubnubEntityType::PEnT_Channel;
}

void UPubnubChannelEntity::PublishMessage(FString Message, FOnPubnubPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings)
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
	PubnubClient->PublishMessage(EntityID, Message, OnPublishMessageResponse, PublishSettings);
}

void UPubnubChannelEntity::PublishMessage(FString Message, FOnPubnubPublishMessageResponseNative NativeCallback, FPubnubPublishSettings PublishSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot publish message - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->PublishMessage(EntityID, Message, NativeCallback, PublishSettings);
}

void UPubnubChannelEntity::PublishMessage(FString Message, FPubnubPublishSettings PublishSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot publish message - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->PublishMessage(EntityID, Message, PublishSettings);
}

void UPubnubChannelEntity::Signal(FString Message, FOnPubnubSignalResponse OnSignalResponse, FPubnubSignalSettings SignalSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot send signal - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->Signal(EntityID, Message, OnSignalResponse, SignalSettings);
}

void UPubnubChannelEntity::Signal(FString Message, FOnPubnubSignalResponseNative NativeCallback, FPubnubSignalSettings SignalSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot send signal - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->Signal(EntityID, Message, NativeCallback, SignalSettings);
}

void UPubnubChannelEntity::Signal(FString Message, FPubnubSignalSettings SignalSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot send signal - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->Signal(EntityID, Message, SignalSettings);
}

void UPubnubChannelEntity::ListUsersFromChannel(FOnPubnubListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list users from channel - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->ListUsersFromChannel(EntityID, ListUsersFromChannelResponse, ListUsersFromChannelSettings);
}

void UPubnubChannelEntity::ListUsersFromChannel(FOnPubnubListUsersFromChannelResponseNative NativeCallback, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list users from channel - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->ListUsersFromChannel(EntityID, NativeCallback, ListUsersFromChannelSettings);
}
