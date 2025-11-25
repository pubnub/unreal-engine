// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Samples/Entities/Sample_ChannelGroupEntity.h"
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
 * 
 * CHANNEL GROUP ENTITY SAMPLES demonstrate working with PubNub channel groups through the entity-based approach.
 */

// NOTE: Comments marked with `ACTION REQUIRED` indicate lines you must change.

ASample_ChannelGroupEntity::ASample_ChannelGroupEntity()
{
	SamplesName = "ChannelGroupEntity";
}

void ASample_ChannelGroupEntity::RunSamples()
{
	Super::RunSamples();
	
	CreateChannelGroupEntitySample();
	SubscribeWithChannelGroupEntitySample();
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


/* CHANNEL GROUP ENTITY SAMPLE FUNCTIONS */

// snippet.create_channel_group_entity
void ASample_ChannelGroupEntity::CreateChannelGroupEntitySample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel group entity for the group you want to work with
	FString ChannelGroupName = TEXT("game_rooms");
	UPubnubChannelGroupEntity* ChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(ChannelGroupName);
}

// snippet.subscribe_with_channel_group_entity
// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::SubscribeWithChannelGroupEntitySample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel group entity for the group you want to work with
	FString ChannelGroupName = TEXT("game_rooms");
	UPubnubChannelGroupEntity* ChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(ChannelGroupName);

	// Create a subscription from the channel group entity
	UPubnubSubscription* GroupSubscription = ChannelGroupEntity->CreateSubscription();

	// Add message listener to the subscription
	// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
	GroupSubscription->OnPubnubMessage.AddDynamic(this, &ASample_ChannelGroupEntity::OnMessageReceived_ChannelGroupEntitySample);

	// Subscribe to start receiving messages from all channels in the group
	GroupSubscription->Subscribe();
}

// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::OnMessageReceived_ChannelGroupEntitySample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("Channel Group Entity - Message received: %s on channel: %s"), *Message.Message, *Message.Channel);
}

// snippet.add_channel_to_group_entity
// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::AddChannelToGroupSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel group entity for the group you want to work with
	FString ChannelGroupName = TEXT("all-chats");
	UPubnubChannelGroupEntity* ChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(ChannelGroupName);

	// Add channel to the channel group using the channel group entity
	FString Channel = TEXT("global_chat");
	ChannelGroupEntity->AddChannelToGroupAsync(Channel);
}

// snippet.add_channel_to_group_with_result_entity
// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::AddChannelToGroupWithResultSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel group entity for the group you want to work with
	FString ChannelGroupName = TEXT("all-chats");
	UPubnubChannelGroupEntity* ChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(ChannelGroupName);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
	FOnPubnubAddChannelToGroupResponse OnAddChannelToGroupResponse;
	OnAddChannelToGroupResponse.BindDynamic(this, &ASample_ChannelGroupEntity::OnAddChannelToGroupResponse);

	// Add channel to the channel group using the channel group entity
	FString Channel = TEXT("trade_chat");
	ChannelGroupEntity->AddChannelToGroupAsync(Channel, OnAddChannelToGroupResponse);
}

// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::OnAddChannelToGroupResponse(FPubnubOperationResult Result)
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

// snippet.add_channel_to_group_with_result_lambda_entity
// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::AddChannelToGroupWithResultLambdaSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel group entity for the group you want to work with
	FString ChannelGroupName = TEXT("all-chats");
	UPubnubChannelGroupEntity* ChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(ChannelGroupName);

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
	
	// Add channel to the channel group using the channel group entity
	FString Channel = TEXT("guild_chat");
	ChannelGroupEntity->AddChannelToGroupAsync(Channel, OnAddChannelToGroupResponse);
}

// snippet.list_channels_from_group_entity
// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::ListChannelsFromGroupSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel group entity for the group you want to work with
	FString ChannelGroupName = TEXT("all-chats");
	UPubnubChannelGroupEntity* ChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(ChannelGroupName);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
	FOnPubnubListChannelsFromGroupResponse OnListChannelsFromGroupResponse;
	OnListChannelsFromGroupResponse.BindDynamic(this, &ASample_ChannelGroupEntity::OnListChannelsFromGroupResponse);

	// List channels from the channel group using the channel group entity
	ChannelGroupEntity->ListChannelsFromGroupAsync(OnListChannelsFromGroupResponse);
}

// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::OnListChannelsFromGroupResponse(FPubnubOperationResult Result, const TArray<FString>& Channels)
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

// snippet.list_channels_from_group_with_lambda_entity
// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::ListChannelsFromGroupWithLambdaSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel group entity for the group you want to work with
	FString ChannelGroupName = TEXT("all-chats");
	UPubnubChannelGroupEntity* ChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(ChannelGroupName);

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
	
	// List channels from the channel group using the channel group entity
	ChannelGroupEntity->ListChannelsFromGroupAsync(OnListChannelsFromGroupResponse);
}

// snippet.remove_channel_from_group_entity
// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::RemoveChannelFromGroupSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel group entity for the group you want to work with
	FString ChannelGroupName = TEXT("all-chats");
	UPubnubChannelGroupEntity* ChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(ChannelGroupName);

	// Remove channel from the channel group using the channel group entity
	FString Channel = TEXT("global_chat");
	ChannelGroupEntity->RemoveChannelFromGroupAsync(Channel);
}

// snippet.remove_channel_from_group_with_result_entity
// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::RemoveChannelFromGroupWithResultSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel group entity for the group you want to work with
	FString ChannelGroupName = TEXT("all-chats");
	UPubnubChannelGroupEntity* ChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(ChannelGroupName);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
	FOnPubnubRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse;
	OnRemoveChannelFromGroupResponse.BindDynamic(this, &ASample_ChannelGroupEntity::OnRemoveChannelFromGroupResponse);

	// Remove channel from the channel group using the channel group entity
	FString Channel = TEXT("trade_chat");
	ChannelGroupEntity->RemoveChannelFromGroupAsync(Channel, OnRemoveChannelFromGroupResponse);
}

// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::OnRemoveChannelFromGroupResponse(FPubnubOperationResult Result)
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

// snippet.remove_channel_from_group_with_result_lambda_entity
// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::RemoveChannelFromGroupWithResultLambdaSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel group entity for the group you want to work with
	FString ChannelGroupName = TEXT("all-chats");
	UPubnubChannelGroupEntity* ChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(ChannelGroupName);

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
	
	// Remove channel from the channel group using the channel group entity
	FString Channel = TEXT("guild_chat");
	ChannelGroupEntity->RemoveChannelFromGroupAsync(Channel, OnRemoveChannelFromGroupResponse);
}

// snippet.remove_channel_group_entity
// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::RemoveChannelGroupSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel group entity for the group you want to work with
	FString ChannelGroupName = TEXT("all-chats");
	UPubnubChannelGroupEntity* ChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(ChannelGroupName);

	// Remove the entire channel group using the channel group entity
	ChannelGroupEntity->RemoveChannelGroupAsync();
}

// snippet.remove_channel_group_with_result_entity
// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::RemoveChannelGroupWithResultSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel group entity for the group you want to work with
	FString ChannelGroupName = TEXT("all-chats");
	UPubnubChannelGroupEntity* ChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(ChannelGroupName);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
	FOnPubnubRemoveChannelGroupResponse OnRemoveChannelGroupResponse;
	OnRemoveChannelGroupResponse.BindDynamic(this, &ASample_ChannelGroupEntity::OnRemoveChannelGroupResponse);

	// Remove the entire channel group using the channel group entity
	ChannelGroupEntity->RemoveChannelGroupAsync(OnRemoveChannelGroupResponse);
}

// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::OnRemoveChannelGroupResponse(FPubnubOperationResult Result)
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

// snippet.remove_channel_group_with_result_lambda_entity
// ACTION REQUIRED: Replace ASample_ChannelGroupEntity with name of your Actor class
void ASample_ChannelGroupEntity::RemoveChannelGroupWithResultLambdaSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel group entity for the group you want to work with
	FString ChannelGroupName = TEXT("all-chats");
	UPubnubChannelGroupEntity* ChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(ChannelGroupName);

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
	
	// Remove the entire channel group using the channel group entity
	ChannelGroupEntity->RemoveChannelGroupAsync(OnRemoveChannelGroupResponse);
}

// snippet.end
