// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubChannelEntity.h"


UPubnubChannelEntity::UPubnubChannelEntity()
{
	EntityType = EPubnubEntityType::PEnT_Channel;
}

void UPubnubChannelEntity::PublishMessage(FString Message, FOnPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings)
{
	PubnubSubsystem->PublishMessage(EntityID, Message, OnPublishMessageResponse, PublishSettings);
}

void UPubnubChannelEntity::PublishMessage(FString Message, FOnPublishMessageResponseNative NativeCallback, FPubnubPublishSettings PublishSettings)
{
	PubnubSubsystem->PublishMessage(EntityID, Message, NativeCallback, PublishSettings);
}

void UPubnubChannelEntity::PublishMessage(FString Message, FPubnubPublishSettings PublishSettings)
{
	PubnubSubsystem->PublishMessage(EntityID, Message, PublishSettings);
}

void UPubnubChannelEntity::Signal(FString Message, FOnSignalResponse OnSignalResponse, FPubnubSignalSettings SignalSettings)
{
	PubnubSubsystem->Signal(EntityID, Message, OnSignalResponse, SignalSettings);
}

void UPubnubChannelEntity::Signal(FString Message, FOnSignalResponseNative NativeCallback, FPubnubSignalSettings SignalSettings)
{
	PubnubSubsystem->Signal(EntityID, Message, NativeCallback, SignalSettings);
}

void UPubnubChannelEntity::Signal(FString Message, FPubnubSignalSettings SignalSettings)
{
	PubnubSubsystem->Signal(EntityID, Message, SignalSettings);
}

void UPubnubChannelEntity::ListUsersFromChannel(FOnListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	PubnubSubsystem->ListUsersFromChannel(EntityID, ListUsersFromChannelResponse, ListUsersFromChannelSettings);
}

void UPubnubChannelEntity::ListUsersFromChannel(FOnListUsersFromChannelResponseNative NativeCallback, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	PubnubSubsystem->ListUsersFromChannel(EntityID, NativeCallback, ListUsersFromChannelSettings);
}
