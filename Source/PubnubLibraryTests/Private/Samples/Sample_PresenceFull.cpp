// Copyright 2025 PubNub Inc. All Rights Reserved.

// snippet.full_presence_example

#include "Samples/Sample_PresenceFull.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "PubnubSubsystem.h"


void ASample_PresenceFull::BeginPlay()
{
	Super::BeginPlay();

	//Run the example on BeginPlay
	RunPresenceFullExample();
}

void ASample_PresenceFull::RunPresenceFullExample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	UE_LOG(LogTemp, Log, TEXT("Presence example: User ID is set"));
	
	//Subscribe to a channel with presence events enabled.
	FPubnubSubscribeSettings SubscribeSettings;
	SubscribeSettings.ReceivePresenceEvents = true;
	FString Channel = TEXT("presence-channel-full");
	PubnubSubsystem->SubscribeToChannel(Channel, SubscribeSettings);

	// NOTE: Subscribing to a group or channel may take a few seconds to complete.
	// This sleep is used to simulate the waiting period in an actual application.
	FPlatformProcess::Sleep(3);
	UE_LOG(LogTemp, Log, TEXT("Presence example: subscribed to channel: %s"), *Channel);

	//Set the user's state on the channel.
	FOnSetStateResponse OnSetStateResponse;
	OnSetStateResponse.BindDynamic(this, &ASample_PresenceFull::OnSetStateResponse);
	FString StateJson = R"({"health": 100, "status": "ready"})";
	PubnubSubsystem->SetState(Channel, StateJson, OnSetStateResponse);

	//List the users on the channel.
	FOnListUsersFromChannelResponse OnListUsersFromChannelResponse;
	OnListUsersFromChannelResponse.BindDynamic(this, &ASample_PresenceFull::OnListUsersFromChannelResponse);
	PubnubSubsystem->ListUsersFromChannel(Channel, OnListUsersFromChannelResponse);

	//Get the state for our user.
	FOnGetStateResponse OnGetStateResponse;
	OnGetStateResponse.BindDynamic(this, &ASample_PresenceFull::OnGetStateResponse);
	PubnubSubsystem->GetState(Channel, "", UserID, OnGetStateResponse);
}



void ASample_PresenceFull::OnSetStateResponse(FPubnubOperationResult Result)
{
	if (!Result.Error)
	{
		UE_LOG(LogTemp, Log, TEXT("Presence example: state successfully set."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Presence example: failed to set state. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

void ASample_PresenceFull::OnListUsersFromChannelResponse(FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper Data)
{
	if (!Result.Error)
	{
		UE_LOG(LogTemp, Log, TEXT("Presence example: successfully listed users. Occupancy: %d"), Data.Occupancy);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Presence example: failed to list users. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

void ASample_PresenceFull::OnGetStateResponse(FPubnubOperationResult Result, FString StateResponse)
{
	if (!Result.Error)
	{
		UE_LOG(LogTemp, Log, TEXT("Presence example: successfully got state: %s"), *StateResponse);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Presence example: failed to get state. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

// snippet.end