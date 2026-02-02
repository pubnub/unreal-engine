// Copyright 2025 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_Groups.h"
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
void ASample_Groups::RunSamples()
{
	Super::RunSamples();
	
	AddChannelToGroupSample();
	AddChannelToGroupWithResultSample();
	AddChannelToGroupWithResultLambdaSample();
	ListChannelsFromGroupSample();
	ListChannelsFromGroupWithLambdaSample();
	RemoveChannelFromGroupSample();
	RemoveChannelFromGroupWithResultSample();
	RemoveChannelFromGroupWithResultLambdaSample();
	RemoveChannelGroupSample();
	RemoveChannelGroupWithResultSample();
	RemoveChannelGroupWithResultLambdaSample();
}
//Internal function, don't copy it with the samples
ASample_Groups::ASample_Groups()
{
	SamplesName = "Channel Groups";
}


/* SAMPLE FUNCTIONS */

// snippet.add_channel_to_group
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::AddChannelToGroupSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Add channel to a channel group
	FString Channel = TEXT("global_chat");
	FString ChannelGroup = TEXT("all-chats");
	PubnubClient->AddChannelToGroupAsync(Channel, ChannelGroup);
}

// snippet.add_channel_to_group_with_result
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::AddChannelToGroupWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
	FOnPubnubAddChannelToGroupResponse OnAddChannelToGroupResponse;
	OnAddChannelToGroupResponse.BindDynamic(this, &ASample_Groups::OnAddChannelToGroupResponse);

	//Add channel to a channel group
	FString Channel = TEXT("trade_chat");
	FString ChannelGroup = TEXT("all-chats");
	PubnubClient->AddChannelToGroupAsync(Channel, ChannelGroup, OnAddChannelToGroupResponse);
}

// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::OnAddChannelToGroupResponse(FPubnubOperationResult Result)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to add channel to group. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Channel successfully added to group."));
	}
}

// snippet.add_channel_to_group_with_result_lambda
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::AddChannelToGroupWithResultLambdaSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubAddChannelToGroupResponseNative OnAddChannelToGroupResponse;
	OnAddChannelToGroupResponse.BindLambda([](const FPubnubOperationResult& Result)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to add channel to group. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Channel successfully added to group."));
		}
	});
	
	//Add channel to a channel group
	FString Channel = TEXT("guild_chat");
	FString ChannelGroup = TEXT("all-chats");
	PubnubClient->AddChannelToGroupAsync(Channel, ChannelGroup, OnAddChannelToGroupResponse);
}

// snippet.list_channels_from_group
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::ListChannelsFromGroupSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
	FOnPubnubListChannelsFromGroupResponse OnListChannelsFromGroupResponse;
	OnListChannelsFromGroupResponse.BindDynamic(this, &ASample_Groups::OnListChannelsFromGroupResponse);

	//List channels from a channel group
	FString ChannelGroup = TEXT("all-chats");
	PubnubClient->ListChannelsFromGroupAsync(ChannelGroup, OnListChannelsFromGroupResponse);
}

// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::OnListChannelsFromGroupResponse(FPubnubOperationResult Result, const TArray<FString>& Channels)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to list channels from group. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Channels successfully listed from group. Listed channels:"));
		for (const FString& Channel : Channels)
		{
			UE_LOG(LogTemp, Log, TEXT("- %s"), *Channel);
		}
	}
}

// snippet.list_channels_from_group_with_lambda
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::ListChannelsFromGroupWithLambdaSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubListChannelsFromGroupResponseNative OnListChannelsFromGroupResponse;
	OnListChannelsFromGroupResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to list channels from group. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Channels successfully listed from group. Listed channels:"));
			for (const FString& Channel : Channels)
			{
				UE_LOG(LogTemp, Log, TEXT("- %s"), *Channel);
			}
		}
	});
	
	//List channels from a channel group
	FString ChannelGroup = TEXT("all-chats");
	PubnubClient->ListChannelsFromGroupAsync(ChannelGroup, OnListChannelsFromGroupResponse);
}

// snippet.remove_channel_from_group
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::RemoveChannelFromGroupSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Remove channel from a channel group
	FString Channel = TEXT("global_chat");
	FString ChannelGroup = TEXT("all-chats");
	PubnubClient->RemoveChannelFromGroupAsync(Channel, ChannelGroup);
}

// snippet.remove_channel_from_group_with_result
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::RemoveChannelFromGroupWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
	FOnPubnubRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse;
	OnRemoveChannelFromGroupResponse.BindDynamic(this, &ASample_Groups::OnRemoveChannelFromGroupResponse);

	//Remove channel from a channel group
	FString Channel = TEXT("trade_chat");
	FString ChannelGroup = TEXT("all-chats");
	PubnubClient->RemoveChannelFromGroupAsync(Channel, ChannelGroup, OnRemoveChannelFromGroupResponse);
}

// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::OnRemoveChannelFromGroupResponse(FPubnubOperationResult Result)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to remove channel from group. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Channel successfully removed from group."));
	}
}

// snippet.remove_channel_from_group_with_result_lambda
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::RemoveChannelFromGroupWithResultLambdaSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubRemoveChannelFromGroupResponseNative OnRemoveChannelFromGroupResponse;
	OnRemoveChannelFromGroupResponse.BindLambda([](const FPubnubOperationResult& Result)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to remove channel from group. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Channel successfully removed from group."));
		}
	});
	
	//Remove channel from a channel group
	FString Channel = TEXT("guild_chat");
	FString ChannelGroup = TEXT("all-chats");
	PubnubClient->RemoveChannelFromGroupAsync(Channel, ChannelGroup, OnRemoveChannelFromGroupResponse);
}

// snippet.remove_channel_group
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::RemoveChannelGroupSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Remove a channel group
	FString ChannelGroup = TEXT("all-chats");
	PubnubClient->RemoveChannelGroupAsync(ChannelGroup);
}

// snippet.remove_channel_group_with_result
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::RemoveChannelGroupWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
	FOnPubnubRemoveChannelGroupResponse OnRemoveChannelGroupResponse;
	OnRemoveChannelGroupResponse.BindDynamic(this, &ASample_Groups::OnRemoveChannelGroupResponse);

	//Remove a channel group
	FString ChannelGroup = TEXT("all-chats");
	PubnubClient->RemoveChannelGroupAsync(ChannelGroup, OnRemoveChannelGroupResponse);
}

// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::OnRemoveChannelGroupResponse(FPubnubOperationResult Result)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to remove channel group. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Channel group successfully removed."));
	}
}

// snippet.remove_channel_group_with_result_lambda
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::RemoveChannelGroupWithResultLambdaSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubRemoveChannelGroupResponseNative OnRemoveChannelGroupResponse;
	OnRemoveChannelGroupResponse.BindLambda([](const FPubnubOperationResult& Result)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to remove channel group. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Channel group successfully removed."));
		}
	});
	
	//Remove a channel group
	FString ChannelGroup = TEXT("all-chats");
	PubnubClient->RemoveChannelGroupAsync(ChannelGroup, OnRemoveChannelGroupResponse);
}

// snippet.end

UPubnubClient* ASample_Groups::GetPubnubClient()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	
	//Get default PubnubClient - created automatically if PluginSettings are set to do so
	UPubnubClient* PubnubClient = PubnubSubsystem->GetPubnubClient(0);
	
	PubnubClient->SetUserID(TEXT("player_001"));
	return PubnubClient;
}
