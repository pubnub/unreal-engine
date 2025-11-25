// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Samples/Entities/Sample_ChannelMetadataEntity.h"
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
 * CHANNEL METADATA ENTITY SAMPLES demonstrate working with PubNub channel metadata through the entity-based approach.
 */

// NOTE: Comments marked with `ACTION REQUIRED` indicate lines you must change.

ASample_ChannelMetadataEntity::ASample_ChannelMetadataEntity()
{
	SamplesName = "ChannelMetadataEntity";
}

void ASample_ChannelMetadataEntity::RunSamples()
{
	Super::RunSamples();
	
	CreateChannelMetadataEntitySample();
	SubscribeWithChannelMetadataEntitySample();
	ChannelMetadataEntitySetMetadataSample();
	SetChannelMetadataSample();
	SetChannelMetadataWithResultSample();
	SetChannelMetadataWithLambdaSample();
	GetChannelMetadataSample();
	GetChannelMetadataWithLambdaSample();
	RemoveChannelMetadataSample();
	RemoveChannelMetadataWithResultSample();
	RemoveChannelMetadataWithResultLambdaSample();
}


/* CHANNEL METADATA ENTITY SAMPLE FUNCTIONS */

// snippet.create_channel_metadata_entity
void ASample_ChannelMetadataEntity::CreateChannelMetadataEntitySample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel metadata entity for the channel you want to monitor metadata changes
	FString ChannelName = TEXT("lobby_settings");
	UPubnubChannelMetadataEntity* ChannelMetadataEntity = PubnubSubsystem->CreateChannelMetadataEntity(ChannelName);
}

// snippet.subscribe_with_channel_metadata_entity
// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
void ASample_ChannelMetadataEntity::SubscribeWithChannelMetadataEntitySample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel metadata entity for the channel you want to monitor metadata changes
	FString ChannelName = TEXT("lobby_settings");
	UPubnubChannelMetadataEntity* ChannelMetadataEntity = PubnubSubsystem->CreateChannelMetadataEntity(ChannelName);

	// Create a subscription from the channel metadata entity
	UPubnubSubscription* MetadataSubscription = ChannelMetadataEntity->CreateSubscription();

	// Add object event listener to receive App Context metadata change notifications
	// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
	MetadataSubscription->OnPubnubObjectEvent.AddDynamic(this, &ASample_ChannelMetadataEntity::OnObjectEvent_ChannelMetadataEntitySample);

	// Subscribe to start receiving metadata change events
	MetadataSubscription->Subscribe();
}

// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
void ASample_ChannelMetadataEntity::OnObjectEvent_ChannelMetadataEntitySample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("Channel Metadata Entity - Object event received: %s"), *Message.Message);
}

// snippet.channel_metadata_entity_set_metadata
// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
void ASample_ChannelMetadataEntity::ChannelMetadataEntitySetMetadataSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Game_Master");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel metadata entity for a tournament lobby
	FString TournamentLobby = TEXT("tournament_lobby_finals");
	UPubnubChannelMetadataEntity* LobbyMetadataEntity = PubnubSubsystem->CreateChannelMetadataEntity(TournamentLobby);

	// Set up callback to handle metadata set result
	// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
	FOnPubnubSetChannelMetadataResponse OnSetMetadataResult;
	OnSetMetadataResult.BindDynamic(this, &ASample_ChannelMetadataEntity::OnSetChannelMetadataResult_Sample);

	// Set tournament lobby information and rules
	FPubnubChannelData TournamentLobbyInfo;
	TournamentLobbyInfo.ChannelID = TournamentLobby;
	TournamentLobbyInfo.ChannelName = "Championship Finals Lobby";
	TournamentLobbyInfo.Description = "Final tournament matches - best of 5 rounds";
	TournamentLobbyInfo.Custom = "{\"max_players\":10,\"tournament_tier\":\"championship\",\"prize_pool\":\"50000\"}";

	LobbyMetadataEntity->SetChannelMetadataAsync(TournamentLobbyInfo, OnSetMetadataResult);
}

// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
void ASample_ChannelMetadataEntity::OnSetChannelMetadataResult_Sample(FPubnubOperationResult Result, FPubnubChannelData ChannelData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set tournament lobby info. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Tournament lobby info set successfully: %s - %s"), *ChannelData.ChannelName, *ChannelData.Description);
	}
}

// snippet.set_channel_metadata_entity
// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
void ASample_ChannelMetadataEntity::SetChannelMetadataSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel metadata entity
	FString ChannelName = TEXT("general-chat-channel");
	UPubnubChannelMetadataEntity* ChannelMetadataEntity = PubnubSubsystem->CreateChannelMetadataEntity(ChannelName);

	// Create channel metadata object
	FPubnubChannelData ChannelMetadata;
	ChannelMetadata.ChannelName = "General Chat";
	ChannelMetadata.Description = "Channel for all players to chat.";
	ChannelMetadata.Custom = "{\"topic\": \"welcomes\"}";
	
	// Set channel metadata using the channel metadata entity
	ChannelMetadataEntity->SetChannelMetadataAsync(ChannelMetadata);
}

// snippet.set_channel_metadata_with_result_entity
// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
void ASample_ChannelMetadataEntity::SetChannelMetadataWithResultSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel metadata entity
	FString ChannelName = TEXT("trade-chat-channel");
	UPubnubChannelMetadataEntity* ChannelMetadataEntity = PubnubSubsystem->CreateChannelMetadataEntity(ChannelName);
	
	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
	FOnPubnubSetChannelMetadataResponse OnSetChannelMetadataResponse;
	OnSetChannelMetadataResponse.BindDynamic(this, &ASample_ChannelMetadataEntity::OnSetChannelMetadataResponse);

	// Create channel metadata object
	FPubnubChannelData ChannelMetadata;
	ChannelMetadata.ChannelName = "Trade Chat";
	ChannelMetadata.Status = "active";
	ChannelMetadata.Custom = "{\"rules\": \"wts_wtt_only\"}";

	// Set channel metadata with all available data included in response using the channel metadata entity
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	ChannelMetadataEntity->SetChannelMetadataAsync(ChannelMetadata, OnSetChannelMetadataResponse, Include);
}

// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
void ASample_ChannelMetadataEntity::OnSetChannelMetadataResponse(FPubnubOperationResult Result, FPubnubChannelData ChannelData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set channel metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully set channel metadata. ChannelID: %s, Name: %s, Custom: %s, Status: %s"), *ChannelData.ChannelID, *ChannelData.ChannelName, *ChannelData.Custom, *ChannelData.Status);
	}
}

// snippet.set_channel_metadata_with_lambda_entity
// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
void ASample_ChannelMetadataEntity::SetChannelMetadataWithLambdaSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel metadata entity
	FString ChannelName = TEXT("guild-hall-channel");
	UPubnubChannelMetadataEntity* ChannelMetadataEntity = PubnubSubsystem->CreateChannelMetadataEntity(ChannelName);

	// Bind lambda to response delegate
	FOnPubnubSetChannelMetadataResponseNative OnSetChannelMetadataResponse;
	OnSetChannelMetadataResponse.BindLambda([](const FPubnubOperationResult& Result, const FPubnubChannelData& ChannelData)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to set channel metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully set channel metadata. ChannelID: %s, Name: %s, Custom: %s, Status: %s"), *ChannelData.ChannelID, *ChannelData.ChannelName, *ChannelData.Custom, *ChannelData.Status);
		}
	});
	
	// Create channel metadata object
	FPubnubChannelData ChannelMetadata;
	ChannelMetadata.ChannelName = "Guild Hall";
	ChannelMetadata.Status = "archived";
	ChannelMetadata.Custom = "{\"motd\": \"Raid tonight at 8!\"}";

	// Set channel metadata with all available data included in response using the channel metadata entity
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	ChannelMetadataEntity->SetChannelMetadataAsync(ChannelMetadata, OnSetChannelMetadataResponse, Include);
}

// snippet.get_channel_metadata_entity
// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
void ASample_ChannelMetadataEntity::GetChannelMetadataSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel metadata entity
	FString ChannelName = TEXT("general-chat-channel");
	UPubnubChannelMetadataEntity* ChannelMetadataEntity = PubnubSubsystem->CreateChannelMetadataEntity(ChannelName);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
	FOnPubnubGetChannelMetadataResponse OnGetChannelMetadataResponse;
	OnGetChannelMetadataResponse.BindDynamic(this, &ASample_ChannelMetadataEntity::OnGetChannelMetadataResponse_Simple);

	// Get channel metadata using the channel metadata entity
	ChannelMetadataEntity->GetChannelMetadataAsync(OnGetChannelMetadataResponse);
}

// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
void ASample_ChannelMetadataEntity::OnGetChannelMetadataResponse_Simple(FPubnubOperationResult Result, FPubnubChannelData ChannelData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get channel metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got channel metadata. ChannelID: %s, Name: %s"), *ChannelData.ChannelID, *ChannelData.ChannelName);
	}
}

// snippet.get_channel_metadata_with_lambda_entity
// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
void ASample_ChannelMetadataEntity::GetChannelMetadataWithLambdaSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel metadata entity
	FString ChannelName = TEXT("guild-hall-channel");
	UPubnubChannelMetadataEntity* ChannelMetadataEntity = PubnubSubsystem->CreateChannelMetadataEntity(ChannelName);

	// Bind lambda to response delegate
	FOnPubnubGetChannelMetadataResponseNative OnGetChannelMetadataResponse;
	OnGetChannelMetadataResponse.BindLambda([](const FPubnubOperationResult& Result, const FPubnubChannelData& ChannelData)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get channel metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully got channel metadata. ChannelID: %s, Name: %s"), *ChannelData.ChannelID, *ChannelData.ChannelName);
		}
	});
	
	// Get channel metadata using the channel metadata entity
	ChannelMetadataEntity->GetChannelMetadataAsync(OnGetChannelMetadataResponse);
}

// snippet.remove_channel_metadata_entity
// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
void ASample_ChannelMetadataEntity::RemoveChannelMetadataSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel metadata entity
	FString ChannelName = TEXT("general-chat-channel");
	UPubnubChannelMetadataEntity* ChannelMetadataEntity = PubnubSubsystem->CreateChannelMetadataEntity(ChannelName);
	
	// Remove channel metadata using the channel metadata entity
	ChannelMetadataEntity->RemoveChannelMetadataAsync();
}

// snippet.remove_channel_metadata_with_result_entity
// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
void ASample_ChannelMetadataEntity::RemoveChannelMetadataWithResultSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel metadata entity
	FString ChannelName = TEXT("trade-chat-channel");
	UPubnubChannelMetadataEntity* ChannelMetadataEntity = PubnubSubsystem->CreateChannelMetadataEntity(ChannelName);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
	FOnPubnubRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse;
	OnRemoveChannelMetadataResponse.BindDynamic(this, &ASample_ChannelMetadataEntity::OnRemoveChannelMetadataResponse);

	// Remove channel metadata using the channel metadata entity
	ChannelMetadataEntity->RemoveChannelMetadataAsync(OnRemoveChannelMetadataResponse);
}

// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
void ASample_ChannelMetadataEntity::OnRemoveChannelMetadataResponse(FPubnubOperationResult Result)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to remove channel metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully removed channel metadata."));
	}
}

// snippet.remove_channel_metadata_with_result_lambda_entity
// ACTION REQUIRED: Replace ASample_ChannelMetadataEntity with name of your Actor class
void ASample_ChannelMetadataEntity::RemoveChannelMetadataWithResultLambdaSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel metadata entity
	FString ChannelName = TEXT("guild-hall-channel");
	UPubnubChannelMetadataEntity* ChannelMetadataEntity = PubnubSubsystem->CreateChannelMetadataEntity(ChannelName);

	// Bind lambda to response delegate
	FOnPubnubRemoveChannelMetadataResponseNative OnRemoveChannelMetadataResponse;
	OnRemoveChannelMetadataResponse.BindLambda([](const FPubnubOperationResult& Result)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to remove channel metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully removed channel metadata."));
		}
	});
	
	// Remove channel metadata using the channel metadata entity
	ChannelMetadataEntity->RemoveChannelMetadataAsync(OnRemoveChannelMetadataResponse);
}

// snippet.end
