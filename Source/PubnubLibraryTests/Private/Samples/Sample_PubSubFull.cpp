// Copyright 2025 PubNub Inc. All Rights Reserved.

// snippet.full_pubsub_example

#include "Samples/Sample_PubSubFull.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "PubnubClient.h"


void ASample_PubSubFull::BeginPlay()
{
	Super::BeginPlay();

	//Run the example on BeginPlay
	RunPubSubFullExample();
}

void ASample_PubSubFull::RunPubSubFullExample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	
	//Create Pubnub Client using Pubnub Subsystem
	FPubnubConfig Config;
	Config.PublishKey = TEXT("demo");   //replace with your Publish Key from Admin Portal
	Config.SecretKey = TEXT("demo");    //replace with your Subscribe Key from Admin Portal
	Config.UserID = TEXT("Player_001");
	UPubnubClient* PubnubClient = PubnubSubsystem->CreatePubnubClient(Config);

	UE_LOG(LogTemp, Log, TEXT("PubSub example, Pubnub Client is created"));

	//Add Listener/Delegate that will broadcast whenever message is received on any subscribed channel or group
	PubnubClient->OnMessageReceived.AddDynamic(this, &ASample_PubSubFull::OnPubnubMessageReceived);

	//Subscribe to the Channel
	FString Channel = TEXT("guild_chat");
	PubnubClient->SubscribeToChannelAsync(Channel);

	// NOTE: Subscribing to a group or channel may take a few seconds to complete.
	// This sleep is used to simulate the waiting period in an actual application.
	FPlatformProcess::Sleep(3);
	
	UE_LOG(LogTemp, Log, TEXT("PubSub example, subscribed to channel: %s"), *Channel);
		
	//Bind delegate to the publish result
	FOnPubnubPublishMessageResponse OnPublishMessageResponse;
	OnPublishMessageResponse.BindDynamic(this, &ASample_PubSubFull::OnPublishResult);
	
	//Publish message to the subscribed channel
	FString Message = R"({"event": "PowerUpUsed", "powerup": "Invisibility Cloak", "duration": 10})";
	PubnubClient->PublishMessageAsync(Channel, Message, OnPublishMessageResponse);

	// NOTE: Give some time to receive message before unsubscribing
	// This sleep is used only to simulate the waiting period in an actual application.
	FPlatformProcess::Sleep(3);
	
	//Unsubscribe from previously subscribed channel
	PubnubClient->UnsubscribeFromChannelAsync(Channel);

	UE_LOG(LogTemp, Log, TEXT("PubSub example, message published"));
}

void ASample_PubSubFull::OnPubnubMessageReceived(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("PubSub example, message received on Channel: %s, Message Content: %s"), *Message.Channel, *Message.Message);
}

void ASample_PubSubFull::OnPublishResult(FPubnubOperationResult Result, FPubnubMessageData Message)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("PubSub example, failed to publish message. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("PubSub example, message published successfully. Published message timetoken: %s"), *Message.Timetoken);
	}
}

// snippet.end