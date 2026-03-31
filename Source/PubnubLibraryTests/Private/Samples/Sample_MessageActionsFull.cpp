// Copyright 2026 PubNub Inc. All Rights Reserved.

// snippet.full_message_actions_example

#include "Samples/Sample_MessageActionsFull.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "PubnubSubsystem.h"
#include "PubnubClient.h"


void ASample_MessageActionsFull::BeginPlay()
{
	Super::BeginPlay();

	//Run the example on BeginPlay
	RunMessageActionsFullExample();
}

void ASample_MessageActionsFull::RunMessageActionsFullExample()
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

	UE_LOG(LogTemp, Log, TEXT("Message Actions example: Pubnub Client is created"));

	// 1. Publish a message to get a timetoken
	FOnPubnubPublishMessageResponse OnPublishResponse;
	OnPublishResponse.BindDynamic(this, &ASample_MessageActionsFull::OnPublishResponse);
	PubnubClient->PublishMessageAsync(TestChannel, TEXT("{\"message\":\"What a great play!\"}"), OnPublishResponse);
}

void ASample_MessageActionsFull::OnPublishResponse(FPubnubOperationResult Result, FPubnubMessageData Message)
{
	if (!Result.Error)
	{
		UE_LOG(LogTemp, Log, TEXT("Message Actions example: message published successfully. Timetoken: %s"), *Message.Timetoken);
		TestMessageTimetoken = Message.Timetoken;
		AddFirstMessageAction();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Message Actions example: failed to publish message. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

void ASample_MessageActionsFull::AddFirstMessageAction()
{
	// 2. Add a "like" action
	UE_LOG(LogTemp, Log, TEXT("Message Actions example: adding 'like' action..."));
	FOnPubnubAddMessageActionResponse OnAddMessageActionResponse;
	OnAddMessageActionResponse.BindDynamic(this, &ASample_MessageActionsFull::OnAddFirstMessageActionResponse);
	PubnubClient->AddMessageActionAsync(TestChannel, TestMessageTimetoken, TEXT("reaction"), TEXT("like"), OnAddMessageActionResponse);
}

void ASample_MessageActionsFull::OnAddFirstMessageActionResponse(FPubnubOperationResult Result, FPubnubMessageActionData MessageActionData)
{
	if (!Result.Error)
	{
		UE_LOG(LogTemp, Log, TEXT("Message Actions example: 'like' action added. Action Timetoken: %s"), *MessageActionData.ActionTimetoken);
		LikeActionTimetoken = MessageActionData.ActionTimetoken;
		AddSecondMessageAction();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Message Actions example: failed to add 'like' action. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

void ASample_MessageActionsFull::AddSecondMessageAction()
{
	// 3. Add a "smiley" action
	UE_LOG(LogTemp, Log, TEXT("Message Actions example: adding 'smiley_face' action..."));
	FOnPubnubAddMessageActionResponse OnAddMessageActionResponse;
	OnAddMessageActionResponse.BindDynamic(this, &ASample_MessageActionsFull::OnAddSecondMessageActionResponse);
	PubnubClient->AddMessageActionAsync(TestChannel, TestMessageTimetoken, TEXT("reaction"), TEXT("smiley_face"), OnAddMessageActionResponse);
}

void ASample_MessageActionsFull::OnAddSecondMessageActionResponse(FPubnubOperationResult Result, FPubnubMessageActionData MessageActionData)
{
	if (!Result.Error)
	{
		UE_LOG(LogTemp, Log, TEXT("Message Actions example: 'smiley_face' action added."));
		GetAllMessageActions();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Message Actions example: failed to add 'smiley_face' action. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

void ASample_MessageActionsFull::GetAllMessageActions()
{
	// 4. Get all actions for the channel
	UE_LOG(LogTemp, Log, TEXT("Message Actions example: getting all actions..."));
	FOnPubnubGetMessageActionsResponse OnGetMessageActionsResponse;
	OnGetMessageActionsResponse.BindDynamic(this, &ASample_MessageActionsFull::OnGetMessageActionsResponse);
	FString StartTimetoken = TEXT("18000000000000000"); // Newer timetoken
	FString EndTimetoken = TEXT("17000000000000000");   // Older timetoken
	PubnubClient->GetMessageActionsAsync(TestChannel, OnGetMessageActionsResponse, StartTimetoken, EndTimetoken);
}

void ASample_MessageActionsFull::OnGetMessageActionsResponse(FPubnubOperationResult Result, const TArray<FPubnubMessageActionData>& MessageActions)
{
	if (!Result.Error)
	{
		UE_LOG(LogTemp, Log, TEXT("Message Actions example: successfully got message actions. Found %d action(s)."), MessageActions.Num());
		for (const FPubnubMessageActionData& Action : MessageActions)
		{
			UE_LOG(LogTemp, Log, TEXT("- Type: '%s', Value: '%s', UserID: %s, Action Timetoken: %s"), *Action.Type, *Action.Value, *Action.UserID, *Action.ActionTimetoken);
		}
		RemoveFirstMessageAction();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Message Actions example: failed to get message actions. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

void ASample_MessageActionsFull::RemoveFirstMessageAction()
{
	// 5. Remove the "like" action
	UE_LOG(LogTemp, Log, TEXT("Message Actions example: removing 'like' action..."));
	FOnPubnubRemoveMessageActionResponse OnRemoveMessageActionResponse;
	OnRemoveMessageActionResponse.BindDynamic(this, &ASample_MessageActionsFull::OnRemoveMessageActionResponse);
	PubnubClient->RemoveMessageActionAsync(TestChannel, TestMessageTimetoken, LikeActionTimetoken, OnRemoveMessageActionResponse);
}

void ASample_MessageActionsFull::OnRemoveMessageActionResponse(FPubnubOperationResult Result)
{
	if (!Result.Error)
	{
		UE_LOG(LogTemp, Log, TEXT("Message Actions example: successfully removed 'like' action."));
		UE_LOG(LogTemp, Log, TEXT("Message Actions example finished."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Message Actions example: failed to remove 'like' action. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

// snippet.end