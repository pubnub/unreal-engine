// Copyright 2024 PubNub Inc. All Rights Reserved.

// snippet.full_pubsub_example

#include "Samples/Sample_PubSubFull.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "PubnubSubsystem.h"


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

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	UE_LOG(LogTemp, Log, TEXT("PubSub example, User ID is set"));

	//Add Listener/Delegate that will broadcast whenever message is received on any subscribed channel or group
	PubnubSubsystem->OnMessageReceived.AddDynamic(this, &ASample_PubSubFull::OnPubnubMessageReceived);

	//Subscribe to the Channel
	FString Channel = TEXT("guild_chat");
	PubnubSubsystem->SubscribeToChannel(Channel);

	//Wait some time to let the server proceed subscription
	FPlatformProcess::Sleep(3);
	
	UE_LOG(LogTemp, Log, TEXT("PubSub example, dubscribed to channel: %s"), *Channel);

	//Publish message to the subscribed channel
	FString Message = R"({"event": "PowerUpUsed", "powerup": "Invisibility Cloak", "duration": 10})";
	PubnubSubsystem->PublishMessage(Channel, Message);

	UE_LOG(LogTemp, Log, TEXT("PubSub example, message published"));
}

void ASample_PubSubFull::OnPubnubMessageReceived(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("PubSub example, message reveived on Channel: %s, Message Content: %s"), *Message.Channel, *Message.Message);
}

// snippet.end