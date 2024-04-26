// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatSystem/PubnubChatChannel.h"
#include "ChatSystem/PubnubChatSystem.h"


void UPubnubChatChannel::Initialize(UPubnubChatSystem* InChatSystem, FString InChannelID, FString InChannelData)
{
	ChatSystem = InChatSystem;
	ChannelID = InChannelID;
	ChannelData = InChannelData;
	IsInitialized = true;
}

void UPubnubChatChannel::Connect()
{
	ChatSystem->SubscribeToChannel(ChannelID);
}
