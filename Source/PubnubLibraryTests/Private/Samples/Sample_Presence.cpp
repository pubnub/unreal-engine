// Copyright 2025 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_Presence.h"
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
void ASample_Presence::RunSamples()
{
	Super::RunSamples();
	ListUsersFromChannelSample();
	ListUsersFromChannelWithSettingsSample();
	ListUsersFromChannelWithLambdaSample();
	ListUserSubscribedChannelsSample();
	ListUserSubscribedChannelsWithLambdaSample();
	SetStateSample();
	SetStateWithSettingsSample();
	SetStateWithResultSample();
	SetStateWithResultLambdaSample();
	GetStateSample();
	GetStateWithLambdaSample();
}
//Internal function, don't copy it with the samples
ASample_Presence::ASample_Presence()
{
	SamplesName = "Presence";
}


/* SAMPLE FUNCTIONS */

// snippet.list_users_from_channel
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::ListUsersFromChannelSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
	FOnListUsersFromChannelResponse OnListUsersFromChannelResponse;
	OnListUsersFromChannelResponse.BindDynamic(this, &ASample_Presence::OnListUsersFromChannelResponse_Simple);

	//List users from a channel
	FString Channel = TEXT("guild-channel");
	PubnubSubsystem->ListUsersFromChannel(Channel, OnListUsersFromChannelResponse);
}

// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::OnListUsersFromChannelResponse_Simple(FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper Data)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to list users from channel. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Users successfully listed from channel. Occupancy: %d"), Data.Occupancy);
	}
}

// snippet.list_users_from_channel_with_settings
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::ListUsersFromChannelWithSettingsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
	FOnListUsersFromChannelResponse OnListUsersFromChannelResponse;
	OnListUsersFromChannelResponse.BindDynamic(this, &ASample_Presence::OnListUsersFromChannelResponse_WithSettings);

	// Create additional settings
	FPubnubListUsersFromChannelSettings Settings;
	Settings.State = true;
	Settings.DisableUserID = false;

	//List users from a channel
	FString Channel = TEXT("guild-channel");
	PubnubSubsystem->ListUsersFromChannel(Channel, OnListUsersFromChannelResponse, Settings);
}

// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::OnListUsersFromChannelResponse_WithSettings(FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper Data)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to list users from channel. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Users successfully listed from channel. Total occupancy: %d"), Data.Occupancy);
		//List all users with theirs states
		for (auto const& [UserID, UserState] : Data.UsersState)
		{
			UE_LOG(LogTemp, Log, TEXT("UserID: %s, User State: %d"), *UserID, *UserState);
		}
	}
}

// snippet.list_users_from_channel_with_lambda
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::ListUsersFromChannelWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnListUsersFromChannelResponseNative OnListUsersFromChannelResponse;
	OnListUsersFromChannelResponse.BindLambda([](const FPubnubOperationResult& Result, const FPubnubListUsersFromChannelWrapper& Data)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to list users from channel. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Users successfully listed from channel. Occupancy: %d"), Data.Occupancy);
		}
	});
	
	//List users from a channel
	FString Channel = TEXT("guild-channel");
	PubnubSubsystem->ListUsersFromChannel(Channel, OnListUsersFromChannelResponse);
}

// snippet.list_user_subscribed_channels
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::ListUserSubscribedChannelsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
	FOnListUsersSubscribedChannelsResponse OnListUsersSubscribedChannelsResponse;
	OnListUsersSubscribedChannelsResponse.BindDynamic(this, &ASample_Presence::OnListUserSubscribedChannelsResponse);

	//List channels the user is subscribed to
	PubnubSubsystem->ListUserSubscribedChannels(UserID, OnListUsersSubscribedChannelsResponse);
}

// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::OnListUserSubscribedChannelsResponse(FPubnubOperationResult Result, const TArray<FString>& Channels)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to list user subscribed channels. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully listed user subscribed channels:"));
		//List all channels
		for (const FString& Channel : Channels)
		{
			UE_LOG(LogTemp, Log, TEXT("- %s"), *Channel);
		}
	}
}

// snippet.list_user_subscribed_channels_with_lambda
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::ListUserSubscribedChannelsWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnListUsersSubscribedChannelsResponseNative OnListUsersSubscribedChannelsResponse;
	OnListUsersSubscribedChannelsResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to list user subscribed channels. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully listed user subscribed channels:"));
			//List all channels
			for (const FString& Channel : Channels)
			{
				UE_LOG(LogTemp, Log, TEXT("- %s"), *Channel);
			}
		}
	});
	
	//List channels the user is subscribed to
	PubnubSubsystem->ListUserSubscribedChannels(UserID, OnListUsersSubscribedChannelsResponse);
}

// snippet.set_state
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::SetStateSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Set state for the user on a channel
	FString Channel = TEXT("presence-channel");
	FString StateJson = R"({"health": 100, "status": "active"})";
	PubnubSubsystem->SetState(Channel, StateJson);
}

// snippet.set_state_with_settings
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::SetStateWithSettingsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Set state for the user on a channel
	FString Channel = TEXT("presence-channel");
	FString StateJson = R"({"health": 100, "status": "active"})";

	// Create additional settings
	FPubnubSetStateSettings Settings;
	Settings.ChannelGroup = TEXT("all-presence-channels");
	Settings.UserID = TEXT("Player_005");
	
	PubnubSubsystem->SetState(Channel, StateJson, Settings);
}

// snippet.set_state_with_result
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::SetStateWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
	FOnSetStateResponse OnSetStateResponse;
	OnSetStateResponse.BindDynamic(this, &ASample_Presence::OnSetStateResponse);

	//Set state for the user on a channel
	FString Channel = TEXT("presence-channel");
	FString StateJson = R"({"health": 100, "status": "active"})";
	PubnubSubsystem->SetState(Channel, StateJson, OnSetStateResponse);
}

// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::OnSetStateResponse(FPubnubOperationResult Result)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set state. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully set state."));
	}
}

// snippet.set_state_with_result_lambda
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::SetStateWithResultLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnSetStateResponseNative OnSetStateResponse;
	OnSetStateResponse.BindLambda([](const FPubnubOperationResult& Result)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to set state. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully set state."));
		}
	});
	
	//Set state for the user on a channel
	FString Channel = TEXT("presence-channel");
	FString StateJson = R"({"health": 100, "status": "active"})";
	PubnubSubsystem->SetState(Channel, StateJson, OnSetStateResponse);
}

// snippet.get_state
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::GetStateSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
	FOnGetStateResponse OnGetStateResponse;
	OnGetStateResponse.BindDynamic(this, &ASample_Presence::OnGetStateResponse_Simple);

	//Get state for the user on a channel
	FString Channel = TEXT("presence-channel");
	FString ChannelGroup = TEXT("");
	PubnubSubsystem->GetState(Channel, ChannelGroup, UserID, OnGetStateResponse);
}

// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::OnGetStateResponse_Simple(FPubnubOperationResult Result, FString StateResponse)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get state. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got state: %s"), *StateResponse);
	}
}

// snippet.get_state_with_lambda
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::GetStateWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnGetStateResponseNative OnGetStateResponse;
	OnGetStateResponse.BindLambda([](const FPubnubOperationResult& Result, FString StateResponse)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get state. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully got state: %s"), *StateResponse);
		}
	});
	
	//Get state for the user on a channel
	FString Channel = TEXT("presence-channel");
	FString ChannelGroup = TEXT("");
	PubnubSubsystem->GetState(Channel, ChannelGroup, UserID, OnGetStateResponse);
}

// snippet.end