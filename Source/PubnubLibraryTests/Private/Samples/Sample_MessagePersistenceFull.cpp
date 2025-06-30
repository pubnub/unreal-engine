// Copyright 2024 PubNub Inc. All Rights Reserved.

// snippet.full_message_persistence_example

#include "Samples/Sample_MessagePersistenceFull.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "PubnubSubsystem.h"


void ASample_MessagePersistenceFull::BeginPlay()
{
	Super::BeginPlay();

	//Run the example on BeginPlay
	RunMessagePersistenceFullExample();
}

void ASample_MessagePersistenceFull::RunMessagePersistenceFullExample()
{
	//Get PubnubSubsystem from GameInstance and store it
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	UE_LOG(LogTemp, Log, TEXT("Message Persistence example, User ID is set"));
	
	// 1. Publish two messages. The second one will trigger the next step.
	PubnubSubsystem->PublishMessage(TestChannel, TEXT("{\"message\":\"Hello from the past!\"}"));

	FOnPublishMessageResponse OnPublishResponse;
	OnPublishResponse.BindDynamic(this, &ASample_MessagePersistenceFull::OnPublishResponse);
	PubnubSubsystem->PublishMessage(TestChannel, TEXT("{\"message\":\"This is the second message.\"}"), OnPublishResponse);
	
	UE_LOG(LogTemp, Log, TEXT("Message Persistence example, published two messages to channel: %s"), *TestChannel);
}

void ASample_MessagePersistenceFull::OnPublishResponse(FPubnubOperationResult Result, FPubnubMessageData Message)
{
	if (!Result.Error)
	{
		UE_LOG(LogTemp, Log, TEXT("Message Persistence example, second message published successfully."));
		FetchHistory();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Message Persistence example, failed to publish second message. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

void ASample_MessagePersistenceFull::FetchHistory()
{
	// 2. Fetch history now that we know a message has been published.
	FOnFetchHistoryResponse OnFetchHistoryResponse;
	OnFetchHistoryResponse.BindDynamic(this, &ASample_MessagePersistenceFull::OnFetchHistoryResponse);
	FPubnubFetchHistorySettings Settings;
	Settings.MaxPerChannel = 2;
	PubnubSubsystem->FetchHistory(TestChannel, OnFetchHistoryResponse, Settings);
}

void ASample_MessagePersistenceFull::OnFetchHistoryResponse(FPubnubOperationResult Result, const TArray<FPubnubHistoryMessageData>& Messages)
{
	if (!Result.Error && !Messages.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("Message Persistence example, successfully fetched history. Found %d messages."), Messages.Num());
		
		FString Timetoken = Messages[0].Timetoken;
		GetMessageCounts(Timetoken);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Message Persistence example, failed to fetch history. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

void ASample_MessagePersistenceFull::GetMessageCounts(const FString& Timetoken)
{
	// 3. Get message counts using the timetoken of the first message.
	FOnMessageCountsResponse OnMessageCountsResponse;
	OnMessageCountsResponse.BindDynamic(this, &ASample_MessagePersistenceFull::OnMessageCountsResponse);
	// NOTE:: Timetoken param in MessageCounts is exclusive, so the message with this timetoken won't be calculated
	PubnubSubsystem->MessageCounts(TestChannel, Timetoken, OnMessageCountsResponse);
}

void ASample_MessagePersistenceFull::OnMessageCountsResponse(FPubnubOperationResult Result, int MessageCounts)
{
	if (!Result.Error)
	{
		UE_LOG(LogTemp, Log, TEXT("Message Persistence example, successfully got message counts: %d"), MessageCounts);
		DeleteMessages();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Message Persistence example, failed to get message counts. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

void ASample_MessagePersistenceFull::DeleteMessages()
{
	// 4. Delete all messages from the channel.
	FOnDeleteMessagesResponse OnDeleteMessagesResponse;
	OnDeleteMessagesResponse.BindDynamic(this, &ASample_MessagePersistenceFull::OnDeleteMessagesResponse);
	PubnubSubsystem->DeleteMessages(TestChannel, OnDeleteMessagesResponse);
}

void ASample_MessagePersistenceFull::OnDeleteMessagesResponse(FPubnubOperationResult Result)
{
	if (!Result.Error)
	{
		UE_LOG(LogTemp, Log, TEXT("Message Persistence example, successfully deleted messages."));
		UE_LOG(LogTemp, Log, TEXT("Message Persistence example finished."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Message Persistence example, failed to delete messages. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
}

// snippet.end