// Copyright 2026 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_MessageActions.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "PubnubSubsystem.h"

/**
 * NOTE: Each sample is designed to be fully self-contained and portable. 
 * You can copy-paste any individual sample into a new project, and it should compile and run without errors 
 * — as long as you also include the necessary `#include` statements.
 *
 * The samples assume that in Pubnub SDK settings sections in ProjectSettings following fields are set:
 * PublishKey and SubscribeKey have correct keys, InitializeAutomatically is true.
 */

// NOTE: Comments marked with `ACTION REQUIRED` indicate lines you must change/adjust.


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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	// You should get this from a previously published message.
	FString MessageTimetoken = TEXT("17298418370000000");

	//Add a message action to a message
	FString Channel = TEXT("message-actions-channel");
	FString ActionType = TEXT("reaction");
	FString ActionValue = TEXT("smiley_face");
	PubnubClient->AddMessageActionAsync(Channel, MessageTimetoken, ActionType, ActionValue);
}

// snippet.add_message_action_with_result
// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::AddMessageActionWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	// You should get this from a previously published message.
	FString MessageTimetoken = TEXT("17298418370000000");
	
	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
	FOnPubnubAddMessageActionResponse OnAddMessageActionResponse;
	OnAddMessageActionResponse.BindDynamic(this, &ASample_MessageActions::OnAddMessageActionResponse);

	//Add a message action to a message
	FString Channel = TEXT("message-actions-channel");
	FString ActionType = TEXT("reaction");
	FString ActionValue = TEXT("heart");
	PubnubClient->AddMessageActionAsync(Channel, MessageTimetoken, ActionType, ActionValue, OnAddMessageActionResponse);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	// You should get this from a previously published message.
	FString MessageTimetoken = TEXT("17298418370000000");

	// Bind lambda to response delegate
	FOnPubnubAddMessageActionResponseNative OnAddMessageActionResponse;
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
	PubnubClient->AddMessageActionAsync(Channel, MessageTimetoken, ActionType, ActionValue, OnAddMessageActionResponse);
}

// snippet.get_message_actions
// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::GetMessageActionsSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
	FOnPubnubGetMessageActionsResponse OnGetMessageActionsResponse;
	OnGetMessageActionsResponse.BindDynamic(this, &ASample_MessageActions::OnGetMessageActionsResponse);

	//Get message actions from a channel
	FString Channel = TEXT("message-actions-channel");
	PubnubClient->GetMessageActionsAsync(Channel, OnGetMessageActionsResponse);
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

// snippet.get_message_actions_with_settings
// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::GetMessageActionsWithSettingsSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
	FOnPubnubGetMessageActionsResponse OnGetMessageActionsResponse;
	OnGetMessageActionsResponse.BindDynamic(this, &ASample_MessageActions::OnGetMessageActionsResponse_WithSettings);

	//Get message actions from a channel with a specific time window and limit
	FString Channel = TEXT("message-actions-channel");
	FString StartTimetoken = TEXT("17298418380000000"); // Newer timetoken
	FString EndTimetoken = TEXT("17298418360000000");   // Older timetoken
	int Limit = 5;
	PubnubClient->GetMessageActionsAsync(Channel, OnGetMessageActionsResponse, StartTimetoken, EndTimetoken, Limit);
}

// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::OnGetMessageActionsResponse_WithSettings(FPubnubOperationResult Result, const TArray<FPubnubMessageActionData>& MessageActions)
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubGetMessageActionsResponseNative OnGetMessageActionsResponse;
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
	PubnubClient->GetMessageActionsAsync(Channel, OnGetMessageActionsResponse);
}

// snippet.remove_message_action
// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::RemoveMessageActionSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	// ACTION REQUIRED: Replace with real Message timetoken that has an action added.
	FString MessageTimetoken = TEXT("17298418370000000");
	// ACTION REQUIRED: Replace with real Message Action timetoken.
	FString ActionTimetoken = TEXT("17298418390000000");

	//Remove a message action from a message
	FString Channel = TEXT("message-actions-channel");
	PubnubClient->RemoveMessageActionAsync(Channel, MessageTimetoken, ActionTimetoken);
}

// snippet.remove_message_action_with_result
// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
void ASample_MessageActions::RemoveMessageActionWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	// ACTION REQUIRED: Replace with real Message timetoken that has an action added.
	FString MessageTimetoken = TEXT("17298418370000000");
	// ACTION REQUIRED: Replace with real Message Action timetoken.
	FString ActionTimetoken = TEXT("17298418390000000");
	
	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_MessageActions with name of your Actor class
	FOnPubnubRemoveMessageActionResponse OnRemoveMessageActionResponse;
	OnRemoveMessageActionResponse.BindDynamic(this, &ASample_MessageActions::OnRemoveMessageActionResponse);

	//Remove a message action from a message
	FString Channel = TEXT("message-actions-channel");
	PubnubClient->RemoveMessageActionAsync(Channel, MessageTimetoken, ActionTimetoken, OnRemoveMessageActionResponse);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// ACTION REQUIRED: Replace with real Message timetoken that has an action added.
	FString MessageTimetoken = TEXT("17298418370000000");
	// ACTION REQUIRED: Replace with real Message Action timetoken.
	FString ActionTimetoken = TEXT("17298418390000000");

	// Bind lambda to response delegate
	FOnPubnubRemoveMessageActionResponseNative OnRemoveMessageActionResponse;
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
	PubnubClient->RemoveMessageActionAsync(Channel, MessageTimetoken, ActionTimetoken, OnRemoveMessageActionResponse);
}


// snippet.end

UPubnubClient* ASample_MessageActions::GetPubnubClient()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	
	//Get default PubnubClient - created automatically if PluginSettings are set to do so
	UPubnubClient* PubnubClient = PubnubSubsystem->GetPubnubClient(0);
	
	PubnubClient->SetUserID(TEXT("player_001"));
	return PubnubClient;
}
