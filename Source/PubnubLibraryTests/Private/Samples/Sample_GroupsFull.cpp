// Copyright 2024 PubNub Inc. All Rights Reserved.

// snippet.full_groups_example

#include "Samples/Sample_GroupsFull.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "PubnubSubsystem.h"


void ASample_GroupsFull::BeginPlay()
{
	Super::BeginPlay();

	//Run the example on BeginPlay
	RunGroupsFullExample();
}

void ASample_GroupsFull::RunGroupsFullExample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	UE_LOG(LogTemp, Log, TEXT("Channel Groups example, User ID is set"));

	// Define channel and group names
	FString Channel = TEXT("global_chat");
	FString ChannelGroup = TEXT("all-chats");
	
	// Bind delegate for AddChannelToGroup result
	FOnAddChannelToGroupResponse OnAddChannelToGroupResponse;
	OnAddChannelToGroupResponse.BindDynamic(this, &ASample_GroupsFull::OnAddChannelToGroupResponse);

	//Add channel to the group
	PubnubSubsystem->AddChannelToGroup(Channel, ChannelGroup, OnAddChannelToGroupResponse);
	
	//Subscribe to the group
	PubnubSubsystem->OnMessageReceived.AddDynamic(this, &ASample_GroupsFull::OnPubnubMessageReceived);
	PubnubSubsystem->SubscribeToGroup(ChannelGroup);

	// NOTE: Subscribing to a group or channel may take a few seconds to complete.
	// This sleep is used to simulate the waiting period in an actual application.
	FPlatformProcess::Sleep(3);
	
	UE_LOG(LogTemp, Log, TEXT("Channel Groups example, subscribed to group: %s"), *ChannelGroup);
		
	//Publish a message to the channel (which is in the subscribed group)
	FOnPublishMessageResponse OnPublishMessageResponse;
	OnPublishMessageResponse.BindDynamic(this, &ASample_GroupsFull::OnPublishResult);
	
	FString Message = R"({"message": "Welcome to the 'all-chats' group!"})";
	PubnubSubsystem->PublishMessage(Channel, Message, OnPublishMessageResponse);

	UE_LOG(LogTemp, Log, TEXT("Channel Groups example, message published to channel: %s"), *Channel);

	//Remove channel from the group
	PubnubSubsystem->RemoveChannelFromGroup(Channel, ChannelGroup);

	UE_LOG(LogTemp, Log, TEXT("Channel Groups example, channel removed from group"));
}

void ASample_GroupsFull::OnAddChannelToGroupResponse(FPubnubOperationResult Result)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Groups example, failed to add channel to group. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Channel Groups example, channel successfully added to group."));
	}
}

void ASample_GroupsFull::OnPubnubMessageReceived(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("Channel Groups example, message received on Channel: %s, via Group: %s, Message Content: %s"), *Message.Channel, *Message.MatchOrGroup, *Message.Message);
}

void ASample_GroupsFull::OnPublishResult(FPubnubOperationResult Result, FPubnubMessageData Message)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Channel Groups example, failed to publish message. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Channel Groups example, message published successfully. Published message timetoken: %s"), *Message.Timetoken);
	}
}

// snippet.end