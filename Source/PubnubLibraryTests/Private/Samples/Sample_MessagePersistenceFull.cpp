// Copyright 2025 PubNub Inc. All Rights Reserved.

// snippet.full_message_persistence_example

#include "Samples/Sample_MessagePersistenceFull.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "PubnubSubsystem.h"
#include "PubnubClient.h"


void ASample_MessagePersistenceFull::BeginPlay()
{
	Super::BeginPlay();

	//Run the example on BeginPlay
	RunMessagePersistenceFullExample();
}

void ASample_MessagePersistenceFull::RunMessagePersistenceFullExample()
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

	UE_LOG(LogTemp, Log, TEXT("Message Persistence exampl: Pubnub Client is created"));
	
	// 1. Publish two messages. The second one will trigger the next step.
	PubnubClient->PublishMessageAsync(TestChannel, TEXT("{\"message\":\"Hello from the past!\"}"));

	FOnPubnubPublishMessageResponse OnPublishResponse;
	OnPublishResponse.BindDynamic(this, &ASample_MessagePersistenceFull::OnPublishResponse);
	PubnubClient->PublishMessageAsync(TestChannel, TEXT("{\"message\":\"This is the second message.\"}"), OnPublishResponse);
	
	UE_LOG(LogTemp, Log, TEXT("Message Persistence exampl: published two messages to channel: %s"), *TestChannel);
}

void ASample_MessagePersistenceFull::OnPublishResponse(FPubnubOperationResult Result, FPubnubMessageData Message)
{
	if (!Result.Error)
	{
		UE_LOG(LogTemp, Log, TEXT("Message Persistence exampl: second message published successfully."));
		FetchHistory();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Message Persistence exampl: failed to publish second message. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

void ASample_MessagePersistenceFull::FetchHistory()
{
	// 2. Fetch history now that we know a message has been published.
	FOnPubnubFetchHistoryResponse OnFetchHistoryResponse;
	OnFetchHistoryResponse.BindDynamic(this, &ASample_MessagePersistenceFull::OnFetchHistoryResponse);
	FPubnubFetchHistorySettings Settings;
	Settings.MaxPerChannel = 2;
	PubnubClient->FetchHistoryAsync(TestChannel, OnFetchHistoryResponse, Settings);
}

void ASample_MessagePersistenceFull::OnFetchHistoryResponse(FPubnubOperationResult Result, const TArray<FPubnubHistoryMessageData>& Messages)
{
	if (!Result.Error && !Messages.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("Message Persistence exampl: successfully fetched history. Found %d messages."), Messages.Num());
		
		FString Timetoken = Messages[0].Timetoken;
		GetMessageCounts(Timetoken);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Message Persistence exampl: failed to fetch history. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

void ASample_MessagePersistenceFull::GetMessageCounts(const FString& Timetoken)
{
	// 3. Get message counts using the timetoken of the first message.
	FOnPubnubMessageCountsResponse OnMessageCountsResponse;
	OnMessageCountsResponse.BindDynamic(this, &ASample_MessagePersistenceFull::OnMessageCountsResponse);
	// NOTE:: Timetoken param in MessageCounts is exclusive, so the message with this timetoken won't be calculated
	PubnubClient->MessageCountsAsync(TestChannel, Timetoken, OnMessageCountsResponse);
}

void ASample_MessagePersistenceFull::OnMessageCountsResponse(FPubnubOperationResult Result, int MessageCounts)
{
	if (!Result.Error)
	{
		UE_LOG(LogTemp, Log, TEXT("Message Persistence exampl: successfully got message counts: %d"), MessageCounts);
		DeleteMessages();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Message Persistence exampl: failed to get message counts. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

void ASample_MessagePersistenceFull::DeleteMessages()
{
	// 4. Delete all messages from the channel.
	FOnPubnubDeleteMessagesResponse OnDeleteMessagesResponse;
	OnDeleteMessagesResponse.BindDynamic(this, &ASample_MessagePersistenceFull::OnDeleteMessagesResponse);
	PubnubClient->DeleteMessagesAsync(TestChannel, OnDeleteMessagesResponse);
}

void ASample_MessagePersistenceFull::OnDeleteMessagesResponse(FPubnubOperationResult Result)
{
	if (!Result.Error)
	{
		UE_LOG(LogTemp, Log, TEXT("Message Persistence exampl: successfully deleted messages."));
		UE_LOG(LogTemp, Log, TEXT("Message Persistence example finished."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Message Persistence exampl: failed to delete messages. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

// snippet.end