// Copyright 2026 PubNub Inc. All Rights Reserved.

// snippet.full_groups_example

#include "Samples/Sample_GroupsFull.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "PubnubSubsystem.h"
#include "PubnubClient.h"


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
	
	//Create Pubnub Client using Pubnub Subsystem
	FPubnubConfig Config;
	Config.PublishKey = TEXT("demo");   //replace with your Publish Key from Admin Portal
	Config.SubscribeKey = TEXT("demo"); //replace with your Subscribe Key from Admin Portal
	Config.UserID = TEXT("Player_001");
	PubnubClient = PubnubSubsystem->CreatePubnubClient(Config);

	UE_LOG(LogTemp, Log, TEXT("Channel Groups example: Pubnub Client is created"));
	
	
	// Bind delegate for AddChannelToGroup result
	FOnPubnubAddChannelToGroupResponse OnAddChannelToGroupResponse;
	OnAddChannelToGroupResponse.BindDynamic(this, &ASample_GroupsFull::OnAddChannelToGroupResponse);

	//Add channel to the group
	PubnubClient->AddChannelToGroupAsync(Channel, ChannelGroup, OnAddChannelToGroupResponse);
}

void ASample_GroupsFull::OnAddChannelToGroupResponse(FPubnubOperationResult Result)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Groups example, failed to add channel to group. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Channel Groups example: channel successfully added to group."));
	
	//Subscribe to the group
	PubnubClient->OnMessageReceived.AddDynamic(this, &ASample_GroupsFull::OnPubnubMessageReceived);
	PubnubClient->SubscribeToGroupAsync(ChannelGroup);

	// NOTE: Subscribing to a group or channel may take a few seconds to complete.
	// This sleep is used to simulate the waiting period in an actual application.
	FPlatformProcess::Sleep(3);
	
	UE_LOG(LogTemp, Log, TEXT("Channel Groups example: subscribed to group: %s"), *ChannelGroup);
		
	//Publish a message to the channel (which is in the subscribed group)
	FOnPubnubPublishMessageResponse OnPublishMessageResponse;
	OnPublishMessageResponse.BindDynamic(this, &ASample_GroupsFull::OnPublishResult);
	
	FString Message = R"({"message": "Welcome to the 'all-chats' group!"})";
	PubnubClient->PublishMessageAsync(Channel, Message, OnPublishMessageResponse);

	UE_LOG(LogTemp, Log, TEXT("Channel Groups example: message published to channel: %s"), *Channel);
}

void ASample_GroupsFull::OnPubnubMessageReceived(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("Channel Groups example: message received on Channel: %s, via Group: %s, Message Content: %s"), *Message.Channel, *Message.MatchOrGroup, *Message.Message);

	PubnubClient->UnsubscribeFromGroupAsync(ChannelGroup);
	
	// NOTE: Unsubscribing from a group or channel may take a few seconds to complete.
	// This sleep is used to simulate the waiting period in an actual application.
	FPlatformProcess::Sleep(3);

	UE_LOG(LogTemp, Log, TEXT("Channel Groups example: unsubscribed from group: %s"), *ChannelGroup);

	//Remove channel from the group
	PubnubClient->RemoveChannelFromGroupAsync(Channel, ChannelGroup);

	UE_LOG(LogTemp, Log, TEXT("Channel Groups example: channel removed from group"));
}

void ASample_GroupsFull::OnPublishResult(FPubnubOperationResult Result, FPubnubMessageData Message)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Channel Groups example: failed to publish message. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Channel Groups example: message published successfully. Published message timetoken: %s"), *Message.Timetoken);
	}
}

// snippet.end