// Copyright 2025 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_MessageActions.h"
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
void ASample_MessageActions::RunSamples()
{
	Super::RunSamples();

	AddMessageActionSample();
	AddMessageActionWithResultSample();
	AddMessageActionWithResultLambdaSample();
	GetMessageActionsSample();
	GetMessageActionsWithSettingsSample();
	GetMessageActionsWithLambdaSample();
	RemoveMessageActionSample();
	RemoveMessageActionWithResultSample();
	RemoveMessageActionWithResultLambdaSample();
}
//Internal function, don't copy it with the samples
ASample_MessageActions::ASample_MessageActions()
{
	SamplesName = "Message Actions";
}


/* SAMPLE FUNCTIONS */


// snippet.add_message_action
// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::AddMessageActionSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);
	
	// You should get this from a previously published message.
	FString MessageTimetoken = TEXT("17298418370000000");

	//Add a message action to a message
	FString Channel = TEXT("message-actions-channel");
	FString ActionType = TEXT("reaction");
	FString ActionValue = TEXT("smiley_face");
	PubnubSubsystem->AddMessageAction(Channel, MessageTimetoken, ActionType, ActionValue);
}

// snippet.add_message_action_with_result
// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::AddMessageActionWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);
	
	// You should get this from a previously published message.
	FString MessageTimetoken = TEXT("17298418370000000");
	
	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
	FOnAddMessageActionResponse OnAddMessageActionResponse;
	OnAddMessageActionResponse.BindDynamic(this, &ASample_MessageActions::OnAddMessageActionResponse);

	//Add a message action to a message
	FString Channel = TEXT("message-actions-channel");
	FString ActionType = TEXT("reaction");
	FString ActionValue = TEXT("heart");
	PubnubSubsystem->AddMessageAction(Channel, MessageTimetoken, ActionType, ActionValue, OnAddMessageActionResponse);
}

// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::OnAddMessageActionResponse(FPubnubOperationResult Result, FPubnubMessageActionData MessageActionData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to add message action. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully added message action. Type: '%s', Value: '%s', Timetoken: %s"), *MessageActionData.Type, *MessageActionData.Value, *MessageActionData.ActionTimetoken);
	}
}

// snippet.add_message_action_with_result_lambda
// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::AddMessageActionWithResultLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);
	
	// You should get this from a previously published message.
	FString MessageTimetoken = TEXT("17298418370000000");

	// Bind lambda to response delegate
	FOnAddMessageActionResponseNative OnAddMessageActionResponse;
	OnAddMessageActionResponse.BindLambda([](const FPubnubOperationResult& Result, const FPubnubMessageActionData& MessageActionData)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to add message action. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully added message action. Type: '%s', Value: '%s', Timetoken: %s"), *MessageActionData.Type, *MessageActionData.Value, *MessageActionData.ActionTimetoken);
		}
	});
	
	//Add a message action to a message
	FString Channel = TEXT("message-actions-channel");
	FString ActionType = TEXT("reaction");
	FString ActionValue = TEXT("thumbs_up");
	PubnubSubsystem->AddMessageAction(Channel, MessageTimetoken, ActionType, ActionValue, OnAddMessageActionResponse);
}

// snippet.get_message_actions
// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::GetMessageActionsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);
	
	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
	FOnGetMessageActionsResponse OnGetMessageActionsResponse;
	OnGetMessageActionsResponse.BindDynamic(this, &ASample_MessageActions::OnGetMessageActionsResponse);

	//Get message actions from a channel
	FString Channel = TEXT("message-actions-channel");
	PubnubSubsystem->GetMessageActions(Channel, OnGetMessageActionsResponse);
}

// snippet.get_message_actions_with_settings
// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::GetMessageActionsWithSettingsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);
	
	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
	FOnGetMessageActionsResponse OnGetMessageActionsResponse;
	OnGetMessageActionsResponse.BindDynamic(this, &ASample_MessageActions::OnGetMessageActionsResponse);

	//Get message actions from a channel with a specific time window and limit
	FString Channel = TEXT("message-actions-channel");
	FString StartTimetoken = TEXT("17298418380000000"); // Newer timetoken
	FString EndTimetoken = TEXT("17298418360000000");   // Older timetoken
	int Limit = 5;
	PubnubSubsystem->GetMessageActions(Channel, OnGetMessageActionsResponse, StartTimetoken, EndTimetoken, Limit);
}

// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::OnGetMessageActionsResponse(FPubnubOperationResult Result, const TArray<FPubnubMessageActionData>& MessageActions)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get message actions. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got message actions. Number of actions: %d"), MessageActions.Num());
		//List all received message actions
		for (const FPubnubMessageActionData& Action : MessageActions)
		{
			UE_LOG(LogTemp, Log, TEXT("- Type: '%s', Value: '%s', UserID: %s, Action Timetoken: %s"), *Action.Type, *Action.Value, *Action.UserID, *Action.ActionTimetoken);
		}
	}
}

// snippet.get_message_actions_with_lambda
// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::GetMessageActionsWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnGetMessageActionsResponseNative OnGetMessageActionsResponse;
	OnGetMessageActionsResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubMessageActionData>& MessageActions)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get message actions. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully got message actions. Number of actions: %d"), MessageActions.Num());
			//List all received message actions
			for (const FPubnubMessageActionData& Action : MessageActions)
			{
				UE_LOG(LogTemp, Log, TEXT("- Type: '%s', Value: '%s', UserID: %s, Action Timetoken: %s"), *Action.Type, *Action.Value, *Action.UserID, *Action.ActionTimetoken);
			}
		}
	});
	
	//Get message actions from a channel
	FString Channel = TEXT("message-actions-channel");
	PubnubSubsystem->GetMessageActions(Channel, OnGetMessageActionsResponse);
}

// snippet.remove_message_action
// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::RemoveMessageActionSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);
	
	// ACTION REQUIRED: Replace with real Message timetoken that has an action added.
	FString MessageTimetoken = TEXT("17298418370000000");
	// ACTION REQUIRED: Replace with real Message Action timetoken.
	FString ActionTimetoken = TEXT("17298418390000000");

	//Remove a message action from a message
	FString Channel = TEXT("message-actions-channel");
	PubnubSubsystem->RemoveMessageAction(Channel, MessageTimetoken, ActionTimetoken);
}

// snippet.remove_message_action_with_result
// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::RemoveMessageActionWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);
	
	// ACTION REQUIRED: Replace with real Message timetoken that has an action added.
	FString MessageTimetoken = TEXT("17298418370000000");
	// ACTION REQUIRED: Replace with real Message Action timetoken.
	FString ActionTimetoken = TEXT("17298418390000000");
	
	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
	FOnRemoveMessageActionResponse OnRemoveMessageActionResponse;
	OnRemoveMessageActionResponse.BindDynamic(this, &ASample_MessageActions::OnRemoveMessageActionResponse);

	//Remove a message action from a message
	FString Channel = TEXT("message-actions-channel");
	PubnubSubsystem->RemoveMessageAction(Channel, MessageTimetoken, ActionTimetoken, OnRemoveMessageActionResponse);
}

// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::OnRemoveMessageActionResponse(FPubnubOperationResult Result)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to remove message action. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully removed message action."));
	}
}

// snippet.remove_message_action_with_result_lambda
// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::RemoveMessageActionWithResultLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// ACTION REQUIRED: Replace with real Message timetoken that has an action added.
	FString MessageTimetoken = TEXT("17298418370000000");
	// ACTION REQUIRED: Replace with real Message Action timetoken.
	FString ActionTimetoken = TEXT("17298418390000000");

	// Bind lambda to response delegate
	FOnRemoveMessageActionResponseNative OnRemoveMessageActionResponse;
	OnRemoveMessageActionResponse.BindLambda([](const FPubnubOperationResult& Result)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to remove message action. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully removed message action."));
		}
	});
	
	//Remove a message action from a message
	FString Channel = TEXT("message-actions-channel");
	PubnubSubsystem->RemoveMessageAction(Channel, MessageTimetoken, ActionTimetoken, OnRemoveMessageActionResponse);
}


// snippet.end
