// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_MessagePersistence.h"
// snippet.includes
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

// snippet.end

/**
 * NOTE: Each sample is designed to be fully self-contained and portable. 
 * You can copy-paste any individual sample into a new project, and it should compile and run without errors 
 * — as long as you also include the necessary `#include` statements.
 *
 * To ensure independence, each sample retrieves the PubnubSubsystem and explicitly calls `SetUserID()` 
 * before performing any PubNub operations.
 *
 * In a real project, however, you only need to call `SetUserID()` once — typically during initialization 
 * (e.g., in GameInstance or at login) before making your first PubNub request.
 * 
 * The samples assume that in Pubnub SDK settings sections in ProjectSettings following fields are set:
 * PublishKey and SubscribeKey have correct keys, InitializeAutomatically is true.
 */

// NOTE: Comments marked with `ACTION REQUIRED` indicate lines you must change.


//Internal function, don't copy it with the samples
void ASample_MessagePersistence::RunSamples()
{
	Super::RunSamples();
	FetchHistorySample();
	FetchHistoryWithLambdaSample();
	FetchHistoryWithTimeWindowSample();
	FetchHistoryWithAllIncludesSample();
	DeleteMessagesSample();
	DeleteMessagesWithSettingsSample();
	DeleteMessagesWithResultSample();
	DeleteMessagesWithResultLambdaSample();
	MessageCountsSample();
	MessageCountsWithLambdaSample();
}
//Internal function, don't copy it with the samples
ASample_MessagePersistence::ASample_MessagePersistence()
{
	SamplesName = "Message Persistence";
}


/* SAMPLE FUNCTIONS */

// snippet.fetch_history
// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
void ASample_MessagePersistence::FetchHistorySample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
	FOnFetchHistoryResponse OnFetchHistoryResponse;
	OnFetchHistoryResponse.BindDynamic(this, &ASample_MessagePersistence::OnFetchHistoryResponse_Simple);

	//Fetch history for a channel
	FString Channel = TEXT("history-channel");
	PubnubSubsystem->FetchHistory(Channel, OnFetchHistoryResponse);
}

// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
void ASample_MessagePersistence::OnFetchHistoryResponse_Simple(FPubnubOperationResult Result, const TArray<FPubnubHistoryMessageData>& Messages)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to fetch history. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully fetched history. Number of messages: %d"), Messages.Num());
		//List all received messages
		for (const FPubnubHistoryMessageData& Message : Messages)
		{
			UE_LOG(LogTemp, Log, TEXT("- Message: %s, Timetoken: %s"), *Message.Message, *Message.Timetoken);
		}
	}
}

// snippet.fetch_history_with_lambda
// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
void ASample_MessagePersistence::FetchHistoryWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnFetchHistoryResponseNative OnFetchHistoryResponse;
	OnFetchHistoryResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubHistoryMessageData>& Messages)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to fetch history. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully fetched history. Number of messages: %d"), Messages.Num());
			//List all received messages
			for (const FPubnubHistoryMessageData& Message : Messages)
			{
				UE_LOG(LogTemp, Log, TEXT("- Message: %s, Timetoken: %s"), *Message.Message, *Message.Timetoken);
			}
		}
	});
	
	//Fetch history for a channel
	FString Channel = TEXT("history-channel");
	PubnubSubsystem->FetchHistory(Channel, OnFetchHistoryResponse);
}

// snippet.fetch_history_with_time_window
// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
void ASample_MessagePersistence::FetchHistoryWithTimeWindowSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create settings with a specific time window
	FPubnubFetchHistorySettings Settings;
	Settings.Start = TEXT("17298418360000000"); // Newer timetoken (exclusive)
	Settings.End = TEXT("17292370360000000");   // Older timetoken (inclusive)

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
	FOnFetchHistoryResponse OnFetchHistoryResponse;
	OnFetchHistoryResponse.BindDynamic(this, &ASample_MessagePersistence::OnFetchHistoryResponse_WithTimeWindow);

	//Fetch history for a channel
	FString Channel = TEXT("history-channel");
	PubnubSubsystem->FetchHistory(Channel, OnFetchHistoryResponse, Settings);
}

// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
void ASample_MessagePersistence::OnFetchHistoryResponse_WithTimeWindow(FPubnubOperationResult Result, const TArray<FPubnubHistoryMessageData>& Messages)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to fetch history. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully fetched history with time window. Number of messages: %d"), Messages.Num());
		//List all received messages
		for (const FPubnubHistoryMessageData& Message : Messages)
		{
			UE_LOG(LogTemp, Log, TEXT("- Message: %s, Timetoken: %s"), *Message.Message, *Message.Timetoken);
		}
	}
}

// snippet.fetch_history_with_all_includes
// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
void ASample_MessagePersistence::FetchHistoryWithAllIncludesSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create settings to include all extra data
	FPubnubFetchHistorySettings Settings;
	Settings.IncludeMeta = true;
	Settings.IncludeMessageType = true;
	Settings.IncludeUserID = true;
	Settings.IncludeMessageActions = true;
	Settings.IncludeCustomMessageType = true;
	
	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
	FOnFetchHistoryResponse OnFetchHistoryResponse;
	OnFetchHistoryResponse.BindDynamic(this, &ASample_MessagePersistence::OnFetchHistoryResponse_WithAllIncludes);

	//Fetch history for a channel
	FString Channel = TEXT("history-channel");
	PubnubSubsystem->FetchHistory(Channel, OnFetchHistoryResponse, Settings);
}

// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
void ASample_MessagePersistence::OnFetchHistoryResponse_WithAllIncludes(FPubnubOperationResult Result, const TArray<FPubnubHistoryMessageData>& Messages)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to fetch history. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully fetched history with all includes. Number of messages: %d"), Messages.Num());
		//List all received messages
		for (const FPubnubHistoryMessageData& Message : Messages)
		{
			UE_LOG(LogTemp, Log, TEXT("- Message: %s, Timetoken: %s, Meta: %s, MessageType: %s, UserID: %s, CustomMessageType: %s"), 
				*Message.Message, *Message.Timetoken, *Message.Meta, *Message.MessageType, *Message.UserID, *Message.CustomMessageType);
		}
	}
}

// snippet.delete_messages
// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
void ASample_MessagePersistence::DeleteMessagesSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Delete all messages from a channel
	FString Channel = TEXT("history-channel");
	PubnubSubsystem->DeleteMessages(Channel);
}

// snippet.delete_messages_with_settings
// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
void ASample_MessagePersistence::DeleteMessagesWithSettingsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create settings with a specific time window to delete
	FPubnubDeleteMessagesSettings Settings;
	Settings.Start = TEXT("17298418360000000"); // Newer timetoken (exclusive)
	Settings.End = TEXT("17292370360000000");   // Older timetoken (inclusive)
	
	//Delete messages from a channel within the specified time window
	FString Channel = TEXT("history-channel");
	PubnubSubsystem->DeleteMessages(Channel, Settings);
}

// snippet.delete_messages_with_result
// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
void ASample_MessagePersistence::DeleteMessagesWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
	FOnDeleteMessagesResponse OnDeleteMessagesResponse;
	OnDeleteMessagesResponse.BindDynamic(this, &ASample_MessagePersistence::OnDeleteMessagesResponse);

	//Delete all messages from a channel
	FString Channel = TEXT("history-channel");
	PubnubSubsystem->DeleteMessages(Channel, OnDeleteMessagesResponse);
}

// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
void ASample_MessagePersistence::OnDeleteMessagesResponse(FPubnubOperationResult Result)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to delete messages. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully deleted messages."));
	}
}

// snippet.delete_messages_with_result_lambda
// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
void ASample_MessagePersistence::DeleteMessagesWithResultLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnDeleteMessagesResponseNative OnDeleteMessagesResponse;
	OnDeleteMessagesResponse.BindLambda([](const FPubnubOperationResult& Result)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to delete messages. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully deleted messages."));
		}
	});
	
	//Delete all messages from a channel
	FString Channel = TEXT("history-channel");
	PubnubSubsystem->DeleteMessages(Channel, OnDeleteMessagesResponse);
}

// snippet.message_counts
// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
void ASample_MessagePersistence::MessageCountsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
	FOnMessageCountsResponse OnMessageCountsResponse;
	OnMessageCountsResponse.BindDynamic(this, &ASample_MessagePersistence::OnMessageCountsResponse);

	//Get message counts for a channel from a specific timetoken
	FString Channel = TEXT("history-channel");
	FString Timetoken = TEXT("17292370360000000");
	PubnubSubsystem->MessageCounts(Channel, Timetoken, OnMessageCountsResponse);
}

// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
void ASample_MessagePersistence::OnMessageCountsResponse(FPubnubOperationResult Result, int MessageCounts)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get message counts. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got message counts: %d"), MessageCounts);
	}
}

// snippet.message_counts_with_lambda
// ACTION REQUIRED: Replace ASample_MessagePersistence with name of your Actor class
void ASample_MessagePersistence::MessageCountsWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnMessageCountsResponseNative OnMessageCountsResponse;
	OnMessageCountsResponse.BindLambda([](const FPubnubOperationResult& Result, int MessageCounts)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get message counts. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully got message counts: %d"), MessageCounts);
		}
	});
	
	//Get message counts for a channel from a specific timetoken
	FString Channel = TEXT("history-channel");
	FString Timetoken = TEXT("17292370360000000");
	PubnubSubsystem->MessageCounts(Channel, Timetoken, OnMessageCountsResponse);
}

// snippet.end
