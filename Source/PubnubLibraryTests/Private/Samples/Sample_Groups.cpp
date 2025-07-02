// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_Groups.h"
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
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Add channel to a channel group
	FString Channel = TEXT("global_chat");
	FString ChannelGroup = TEXT("all-chats");
	PubnubSubsystem->AddChannelToGroup(Channel, ChannelGroup);
}

// snippet.add_channel_to_group_with_result
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::AddChannelToGroupWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
	FOnAddChannelToGroupResponse OnAddChannelToGroupResponse;
	OnAddChannelToGroupResponse.BindDynamic(this, &ASample_Groups::OnAddChannelToGroupResponse);

	//Add channel to a channel group
	FString Channel = TEXT("trade_chat");
	FString ChannelGroup = TEXT("all-chats");
	PubnubSubsystem->AddChannelToGroup(Channel, ChannelGroup, OnAddChannelToGroupResponse);
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
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnAddChannelToGroupResponseNative OnAddChannelToGroupResponse;
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
	PubnubSubsystem->AddChannelToGroup(Channel, ChannelGroup, OnAddChannelToGroupResponse);
}

// snippet.list_channels_from_group
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::ListChannelsFromGroupSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
	FOnListChannelsFromGroupResponse OnListChannelsFromGroupResponse;
	OnListChannelsFromGroupResponse.BindDynamic(this, &ASample_Groups::OnListChannelsFromGroupResponse);

	//List channels from a channel group
	FString ChannelGroup = TEXT("all-chats");
	PubnubSubsystem->ListChannelsFromGroup(ChannelGroup, OnListChannelsFromGroupResponse);
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
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnListChannelsFromGroupResponseNative OnListChannelsFromGroupResponse;
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
	PubnubSubsystem->ListChannelsFromGroup(ChannelGroup, OnListChannelsFromGroupResponse);
}

// snippet.remove_channel_from_group
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::RemoveChannelFromGroupSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Remove channel from a channel group
	FString Channel = TEXT("global_chat");
	FString ChannelGroup = TEXT("all-chats");
	PubnubSubsystem->RemoveChannelFromGroup(Channel, ChannelGroup);
}

// snippet.remove_channel_from_group_with_result
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::RemoveChannelFromGroupWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
	FOnRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse;
	OnRemoveChannelFromGroupResponse.BindDynamic(this, &ASample_Groups::OnRemoveChannelFromGroupResponse);

	//Remove channel from a channel group
	FString Channel = TEXT("trade_chat");
	FString ChannelGroup = TEXT("all-chats");
	PubnubSubsystem->RemoveChannelFromGroup(Channel, ChannelGroup, OnRemoveChannelFromGroupResponse);
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
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnRemoveChannelFromGroupResponseNative OnRemoveChannelFromGroupResponse;
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
	PubnubSubsystem->RemoveChannelFromGroup(Channel, ChannelGroup, OnRemoveChannelFromGroupResponse);
}

// snippet.remove_channel_group
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::RemoveChannelGroupSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Remove a channel group
	FString ChannelGroup = TEXT("all-chats");
	PubnubSubsystem->RemoveChannelGroup(ChannelGroup);
}

// snippet.remove_channel_group_with_result
// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
void ASample_Groups::RemoveChannelGroupWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Groups with name of your Actor class
	FOnRemoveChannelGroupResponse OnRemoveChannelGroupResponse;
	OnRemoveChannelGroupResponse.BindDynamic(this, &ASample_Groups::OnRemoveChannelGroupResponse);

	//Remove a channel group
	FString ChannelGroup = TEXT("all-chats");
	PubnubSubsystem->RemoveChannelGroup(ChannelGroup, OnRemoveChannelGroupResponse);
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
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnRemoveChannelGroupResponseNative OnRemoveChannelGroupResponse;
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
	PubnubSubsystem->RemoveChannelGroup(ChannelGroup, OnRemoveChannelGroupResponse);
}

// snippet.end
