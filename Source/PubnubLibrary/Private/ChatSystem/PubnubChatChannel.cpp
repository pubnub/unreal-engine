// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatSystem/PubnubChatChannel.h"
#include "ChatSystem/PubnubChatSystem.h"
#include "ChatSystem/PubnubChatUser.h"
#include "PubnubSubsystem.h"


void UPubnubChatChannel::Initialize(UPubnubChatSystem* InChatSystem, FString InChannelID, FString InChannelData)
{
	ChatSystem = InChatSystem;
	PubnubSubsystem = InChatSystem->GetPubnubSubsystem();
	ChannelID = InChannelID;
	ChannelData = InChannelData;
	IsInitialized = true;

}

void UPubnubChatChannel::Connect()
{
	ChatSystem->SubscribeToChannel(ChannelID);
}

void UPubnubChatChannel::Disconnect()
{
	ChatSystem->UnsubscribeFromChannel(ChannelID);
}

void UPubnubChatChannel::Join(FString AdditionalParams)
{
	//Format all data into the correct Response Json
	FString IncludeString = "totalCount,customFields,channelFields,customChannelFields";
	FString CustomParameterString;
	AdditionalParams.IsEmpty() ? CustomParameterString="{}" : CustomParameterString = AdditionalParams;
	FString SetObjectString = FString::Printf(TEXT("[{\"channel\": {\"id\": \"%s\"}, \"custom\": %s}]"), *ChannelID, *AdditionalParams);

	PubnubSubsystem->SetMemberships(PubnubSubsystem->GetUserID(), IncludeString, SetObjectString);
	Connect();
}

void UPubnubChatChannel::Leave()
{
	FString RemoveObjectString = FString::Printf(TEXT("[{\"channel\": {\"id\": \"%s\"}}]"), *ChannelID);
	PubnubSubsystem->RemoveMemberships(PubnubSubsystem->GetUserID(), "", RemoveObjectString);
	Disconnect();
}

void UPubnubChatChannel::Delete()
{
	ChatSystem->DeleteChannel(ChannelID);
}


