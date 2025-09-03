// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubChannelEntity.h"


UPubnubChannelEntity::UPubnubChannelEntity()
{
	EntityType = EPubnubEntityType::PEnT_Channel;
}

void UPubnubChannelEntity::PublishMessage(FString Message, FOnPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot publish message - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	if (EntityID.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot publish message - EntityID is empty."));
		return;
	}
	PubnubSubsystem->PublishMessage(EntityID, Message, OnPublishMessageResponse, PublishSettings);
}

void UPubnubChannelEntity::PublishMessage(FString Message, FOnPublishMessageResponseNative NativeCallback, FPubnubPublishSettings PublishSettings)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot publish message - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->PublishMessage(EntityID, Message, NativeCallback, PublishSettings);
}

void UPubnubChannelEntity::PublishMessage(FString Message, FPubnubPublishSettings PublishSettings)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot publish message - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->PublishMessage(EntityID, Message, PublishSettings);
}

void UPubnubChannelEntity::Signal(FString Message, FOnSignalResponse OnSignalResponse, FPubnubSignalSettings SignalSettings)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot send signal - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->Signal(EntityID, Message, OnSignalResponse, SignalSettings);
}

void UPubnubChannelEntity::Signal(FString Message, FOnSignalResponseNative NativeCallback, FPubnubSignalSettings SignalSettings)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot send signal - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->Signal(EntityID, Message, NativeCallback, SignalSettings);
}

void UPubnubChannelEntity::Signal(FString Message, FPubnubSignalSettings SignalSettings)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot send signal - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->Signal(EntityID, Message, SignalSettings);
}

void UPubnubChannelEntity::ListUsersFromChannel(FOnListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list users from channel - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->ListUsersFromChannel(EntityID, ListUsersFromChannelResponse, ListUsersFromChannelSettings);
}

void UPubnubChannelEntity::ListUsersFromChannel(FOnListUsersFromChannelResponseNative NativeCallback, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	if (!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot list users from channel - PubnubSubsystem is null. Entity not properly initialized."));
		return;
	}
	PubnubSubsystem->ListUsersFromChannel(EntityID, NativeCallback, ListUsersFromChannelSettings);
}
