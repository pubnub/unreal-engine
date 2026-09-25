// Copyright 2026 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_Presence.h"
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
	FOnPubnubListUsersFromChannelResponse OnListUsersFromChannelResponse;
	OnListUsersFromChannelResponse.BindDynamic(this, &ASample_Presence::OnListUsersFromChannelResponse_Simple);

	//List users from a channel
	FString Channel = TEXT("guild-channel");
	PubnubClient->ListUsersFromChannelAsync(Channel, OnListUsersFromChannelResponse);
}

// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::OnListUsersFromChannelResponse_Simple(FPubnubOperationResult Result, int TotalOccupancy, int TotalChannels, const TArray<FPubnubUsersFromChannel>& Channels)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to list users from channel. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Users successfully listed from channel. Occupancy: %d"), TotalOccupancy);
	}
}

// snippet.list_users_from_channel_with_settings
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::ListUsersFromChannelWithSettingsSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
	FOnPubnubListUsersFromChannelResponse OnListUsersFromChannelResponse;
	OnListUsersFromChannelResponse.BindDynamic(this, &ASample_Presence::OnListUsersFromChannelResponse_WithSettings);

	// Create additional settings
	FPubnubListUsersFromChannelSettings Settings;
	Settings.State = true;
	Settings.DisableUserID = false;

	//List users from a channel
	FString Channel = TEXT("guild-channel");
	PubnubClient->ListUsersFromChannelAsync(Channel, OnListUsersFromChannelResponse, Settings);
}

// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::OnListUsersFromChannelResponse_WithSettings(FPubnubOperationResult Result, int TotalOccupancy, int TotalChannels, const TArray<FPubnubUsersFromChannel>& Channels)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to list users from channel. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Users successfully listed from channel. Total occupancy: %d"), TotalOccupancy);
		//List all users with theirs states
		for (const FPubnubUsersFromChannel& ChannelUsers : Channels)
		{
			for (const FPubnubUserFromChannel& User : ChannelUsers.Users)
			{
				UE_LOG(LogTemp, Log, TEXT("UserID: %s, User State: %s"), *User.UserID, *User.State);
			}
		}
	}
}

// snippet.list_users_from_channel_with_lambda
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::ListUsersFromChannelWithLambdaSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubListUsersFromChannelResponseNative OnListUsersFromChannelResponse;
	OnListUsersFromChannelResponse.BindLambda([](const FPubnubOperationResult& Result, int TotalOccupancy, int TotalChannels, const TArray<FPubnubUsersFromChannel>& Channels)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to list users from channel. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Users successfully listed from channel. Occupancy: %d"), TotalOccupancy);
		}
	});
	
	//List users from a channel
	FString Channel = TEXT("guild-channel");
	PubnubClient->ListUsersFromChannelAsync(Channel, OnListUsersFromChannelResponse);
}

// snippet.list_user_subscribed_channels
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::ListUserSubscribedChannelsSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	FString UserID = TEXT("Player_001");
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
	FOnPubnubListUsersSubscribedChannelsResponse OnListUsersSubscribedChannelsResponse;
	OnListUsersSubscribedChannelsResponse.BindDynamic(this, &ASample_Presence::OnListUserSubscribedChannelsResponse);

	//List channels the user is subscribed to
	PubnubClient->ListUserSubscribedChannelsAsync(UserID, OnListUsersSubscribedChannelsResponse);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	FString UserID = TEXT("Player_001");
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubListUsersSubscribedChannelsResponseNative OnListUsersSubscribedChannelsResponse;
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
	PubnubClient->ListUserSubscribedChannelsAsync(UserID, OnListUsersSubscribedChannelsResponse);
}

// snippet.set_state
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::SetStateSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Set state for the user on a channel
	FString Channel = TEXT("presence-channel");
	FString StateJson = R"({"health": 100, "status": "active"})";
	PubnubClient->SetStateAsync(Channel, StateJson);
}

// snippet.set_state_with_settings
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::SetStateWithSettingsSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Set state for the user on a channel
	FString Channel = TEXT("presence-channel");
	FString StateJson = R"({"health": 100, "status": "active"})";

	// Create additional settings
	FPubnubSetStateSettings Settings;
	Settings.ChannelGroup = TEXT("all-presence-channels");
	
	PubnubClient->SetStateAsync(Channel, StateJson, Settings);
}

// snippet.set_state_with_result
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::SetStateWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
	FOnPubnubSetStateResponse OnSetStateResponse;
	OnSetStateResponse.BindDynamic(this, &ASample_Presence::OnSetStateResponse);

	//Set state for the user on a channel
	FString Channel = TEXT("presence-channel");
	FString StateJson = R"({"health": 100, "status": "active"})";
	PubnubClient->SetStateAsync(Channel, StateJson, OnSetStateResponse);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubSetStateResponseNative OnSetStateResponse;
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
	PubnubClient->SetStateAsync(Channel, StateJson, OnSetStateResponse);
}

// snippet.get_state
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::GetStateSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	FString UserID = TEXT("Player_001");
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
	FOnPubnubGetStateResponse OnGetStateResponse;
	OnGetStateResponse.BindDynamic(this, &ASample_Presence::OnGetStateResponse_Simple);

	//Get state for the user on a channel
	FString Channel = TEXT("presence-channel");
	FString ChannelGroup = TEXT("");
	PubnubClient->GetStateAsync(Channel, ChannelGroup, UserID, OnGetStateResponse);
}

// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::OnGetStateResponse_Simple(FPubnubOperationResult Result, const TArray<FPubnubUserStateOnChannel>& States)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get state. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got state: %s"), *(States.Num() > 0 ? States[0].State : FString()));
	}
}

// snippet.get_state_with_lambda
// ACTION REQUIRED: Replace ASample_Presence with name of your Actor class
void ASample_Presence::GetStateWithLambdaSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	FString UserID = TEXT("Player_001");
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubGetStateResponseNative OnGetStateResponse;
	OnGetStateResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubUserStateOnChannel>& States)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get state. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully got state: %s"), *(States.Num() > 0 ? States[0].State : FString()));
		}
	});
	
	//Get state for the user on a channel
	FString Channel = TEXT("presence-channel");
	FString ChannelGroup = TEXT("");
	PubnubClient->GetStateAsync(Channel, ChannelGroup, UserID, OnGetStateResponse);
}

// snippet.end

UPubnubClient* ASample_Presence::GetPubnubClient()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	
	//Get default PubnubClient - created automatically if PluginSettings are set to do so
	UPubnubClient* PubnubClient = PubnubSubsystem->GetPubnubClient(0);
	
	PubnubClient->SetUserID(TEXT("player_001"));
	return PubnubClient;
}