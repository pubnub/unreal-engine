// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "Samples/Entities/Sample_UserMetadataEntity.h"
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
 * 
 * USER METADATA ENTITY SAMPLES demonstrate working with PubNub user metadata through the entity-based approach.
 */

// NOTE: Comments marked with `ACTION REQUIRED` indicate lines you must change/adjust.

ASample_UserMetadataEntity::ASample_UserMetadataEntity()
{
	SamplesName = "UserMetadataEntity";
}

void ASample_UserMetadataEntity::RunSamples()
{
	Super::RunSamples();
	
	CreateUserMetadataEntitySample();
	SubscribeWithUserMetadataEntitySample();
	UserMetadataEntitySetMetadataSample();
	SetUserMetadataSample();
	SetUserMetadataWithResultSample();
	SetUserMetadataWithLambdaSample();
	GetUserMetadataSample();
	GetUserMetadataWithLambdaSample();
	RemoveUserMetadataSample();
	RemoveUserMetadataWithResultSample();
	RemoveUserMetadataWithResultLambdaSample();
}


/* USER METADATA ENTITY SAMPLE FUNCTIONS */

// snippet.create_user_metadata_entity
void ASample_UserMetadataEntity::CreateUserMetadataEntitySample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Create a user metadata entity for the user you want to monitor metadata changes
	FString UserToMonitor = TEXT("Player_002");
	UPubnubUserMetadataEntity* UserMetadataEntity = PubnubClient->CreateUserMetadataEntity(UserToMonitor);
}

// snippet.subscribe_with_user_metadata_entity
// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
void ASample_UserMetadataEntity::SubscribeWithUserMetadataEntitySample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Create a user metadata entity for the user you want to monitor metadata changes
	FString UserToMonitor = TEXT("Player_002");
	UPubnubUserMetadataEntity* UserMetadataEntity = PubnubClient->CreateUserMetadataEntity(UserToMonitor);

	// Create a subscription from the user metadata entity
	UPubnubSubscription* UserMetadataSubscription = UserMetadataEntity->CreateSubscription();

	// Add object event listener to receive App Context user metadata change notifications
	// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
	UserMetadataSubscription->OnPubnubObjectEvent.AddDynamic(this, &ASample_UserMetadataEntity::OnObjectEvent_UserMetadataEntitySample);

	// Subscribe to start receiving user metadata change events
	UserMetadataSubscription->SubscribeAsync();
}

// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
void ASample_UserMetadataEntity::OnObjectEvent_UserMetadataEntitySample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("User Metadata Entity - Object event received: %s"), *Message.Message);
}

// snippet.user_metadata_entity_set_metadata
// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
void ASample_UserMetadataEntity::UserMetadataEntitySetMetadataSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Create a user metadata entity for updating player profile
	FString PlayerToUpdate = TEXT("Champion_Alex");
	UPubnubUserMetadataEntity* PlayerMetadataEntity = PubnubClient->CreateUserMetadataEntity(PlayerToUpdate);

	// Set up callback to handle metadata set result
	// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
	FOnPubnubSetUserMetadataResponse OnSetPlayerInfoResult;
	OnSetPlayerInfoResult.BindDynamic(this, &ASample_UserMetadataEntity::OnSetUserMetadataResult_Sample);

	// Set champion player profile and statistics
	FPubnubUserInputData ChampionPlayerProfile;
	ChampionPlayerProfile.UserName = "Alex 'Lightning' Rodriguez";
	ChampionPlayerProfile.Email = "alex.lightning@esports.com";
	ChampionPlayerProfile.Custom = "{\"rank\":\"Champion\",\"wins\":127,\"losses\":23,\"favorite_weapon\":\"Plasma Rifle\",\"achievements\":[\"First Blood\",\"Triple Kill Master\"]}";

	PlayerMetadataEntity->SetUserMetadataAsync(ChampionPlayerProfile, OnSetPlayerInfoResult);
}

// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
void ASample_UserMetadataEntity::OnSetUserMetadataResult_Sample(FPubnubOperationResult Result, FPubnubUserData UserData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to update champion player profile. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Champion player profile updated successfully: %s (%s)"), *UserData.UserName, *UserData.UserID);
	}
}

// snippet.set_user_metadata_entity
// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
void ASample_UserMetadataEntity::SetUserMetadataSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	FString UserID = TEXT("Player_001");
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Create a user metadata entity
	UPubnubUserMetadataEntity* UserMetadataEntity = PubnubClient->CreateUserMetadataEntity(UserID);

	// Create user metadata object
	FPubnubUserInputData UserMetadata;
	UserMetadata.UserName = "Player One";
	UserMetadata.Email = "player.one@pubnub.com";
	UserMetadata.Custom = "{\"level\": 5, \"rank\": \"gold\"}";
	
	// Set user metadata using the user metadata entity
	UserMetadataEntity->SetUserMetadataAsync(UserMetadata);
}

// snippet.set_user_metadata_with_result_entity
// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
void ASample_UserMetadataEntity::SetUserMetadataWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	FString UserID = TEXT("Player_002");
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Create a user metadata entity
	UPubnubUserMetadataEntity* UserMetadataEntity = PubnubClient->CreateUserMetadataEntity(UserID);
	
	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
	FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse;
	OnSetUserMetadataResponse.BindDynamic(this, &ASample_UserMetadataEntity::OnSetUserMetadataResponse);

	// Create user metadata object
	FPubnubUserInputData UserMetadata;
	UserMetadata.UserName = "Player Two";
	UserMetadata.Status = "active";
	UserMetadata.Custom = "{\"inventory_slots\": 20, \"guild_id\": \"G2\"}";

	// Set user metadata with all available data included in response using the user metadata entity
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	UserMetadataEntity->SetUserMetadataAsync(UserMetadata, OnSetUserMetadataResponse, Include);
}

// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
void ASample_UserMetadataEntity::OnSetUserMetadataResponse(FPubnubOperationResult Result, FPubnubUserData UserData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set user metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully set user metadata. UserID: %s, Name: %s, Custom: %s, Status: %s"), *UserData.UserID, *UserData.UserName, *UserData.Custom, *UserData.Status);
	}
}

// snippet.set_user_metadata_with_lambda_entity
// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
void ASample_UserMetadataEntity::SetUserMetadataWithLambdaSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	FString UserID = TEXT("Player_003");
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Create a user metadata entity
	UPubnubUserMetadataEntity* UserMetadataEntity = PubnubClient->CreateUserMetadataEntity(UserID);

	// Bind lambda to response delegate
	FOnPubnubSetUserMetadataResponseNative OnSetUserMetadataResponse;
	OnSetUserMetadataResponse.BindLambda([](const FPubnubOperationResult& Result, const FPubnubUserData& UserData)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to set user metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully set user metadata. UserID: %s, Name: %s, Custom: %s, Status: %s"), *UserData.UserID, *UserData.UserName, *UserData.Custom, *UserData.Status);
		}
	});
	
	// Create user metadata object
	FPubnubUserInputData UserMetadata;
	UserMetadata.UserName = "Player Three";
	UserMetadata.Status = "inactive";
	UserMetadata.Custom = "{\"last_seen\": \"2024-01-01\"}";

	// Set user metadata with all available data included in response using the user metadata entity
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	UserMetadataEntity->SetUserMetadataAsync(UserMetadata, OnSetUserMetadataResponse, Include);
}

// snippet.get_user_metadata_entity
// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
void ASample_UserMetadataEntity::GetUserMetadataSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	FString UserID = TEXT("Player_001");
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Create a user metadata entity
	UPubnubUserMetadataEntity* UserMetadataEntity = PubnubClient->CreateUserMetadataEntity(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
	FOnPubnubGetUserMetadataResponse OnGetUserMetadataResponse;
	OnGetUserMetadataResponse.BindDynamic(this, &ASample_UserMetadataEntity::OnGetUserMetadataResponse_Simple);

	// Get user metadata using the user metadata entity
	UserMetadataEntity->GetUserMetadataAsync(OnGetUserMetadataResponse);
}

// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
void ASample_UserMetadataEntity::OnGetUserMetadataResponse_Simple(FPubnubOperationResult Result, FPubnubUserData UserData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get user metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got user metadata. UserID: %s, Name: %s"), *UserData.UserID, *UserData.UserName);
	}
}

// snippet.get_user_metadata_with_lambda_entity
// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
void ASample_UserMetadataEntity::GetUserMetadataWithLambdaSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	FString UserID = TEXT("Player_003");
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Create a user metadata entity
	UPubnubUserMetadataEntity* UserMetadataEntity = PubnubClient->CreateUserMetadataEntity(UserID);

	// Bind lambda to response delegate
	FOnPubnubGetUserMetadataResponseNative OnGetUserMetadataResponse;
	OnGetUserMetadataResponse.BindLambda([](const FPubnubOperationResult& Result, const FPubnubUserData& UserData)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get user metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully got user metadata. UserID: %s, Name: %s"), *UserData.UserID, *UserData.UserName);
		}
	});
	
	// Get user metadata using the user metadata entity
	UserMetadataEntity->GetUserMetadataAsync(OnGetUserMetadataResponse);
}

// snippet.remove_user_metadata_entity
// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
void ASample_UserMetadataEntity::RemoveUserMetadataSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	FString UserID = TEXT("Player_001");
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Create a user metadata entity
	UPubnubUserMetadataEntity* UserMetadataEntity = PubnubClient->CreateUserMetadataEntity(UserID);
	
	// Remove user metadata using the user metadata entity
	UserMetadataEntity->RemoveUserMetadataAsync();
}

// snippet.remove_user_metadata_with_result_entity
// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
void ASample_UserMetadataEntity::RemoveUserMetadataWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Create a user metadata entity
	FString UserToRemove = TEXT("Player_002");
	UPubnubUserMetadataEntity* UserMetadataEntity = PubnubClient->CreateUserMetadataEntity(UserToRemove);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
	FOnPubnubRemoveUserMetadataResponse OnRemoveUserMetadataResponse;
	OnRemoveUserMetadataResponse.BindDynamic(this, &ASample_UserMetadataEntity::OnRemoveUserMetadataResponse);

	// Remove user metadata using the user metadata entity
	UserMetadataEntity->RemoveUserMetadataAsync(OnRemoveUserMetadataResponse);
}

// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
void ASample_UserMetadataEntity::OnRemoveUserMetadataResponse(FPubnubOperationResult Result)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to remove user metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully removed user metadata."));
	}
}

// snippet.remove_user_metadata_with_result_lambda_entity
// ACTION REQUIRED: Replace ASample_UserMetadataEntity with name of your Actor class
void ASample_UserMetadataEntity::RemoveUserMetadataWithResultLambdaSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Create a user metadata entity
	FString UserToRemove = TEXT("Player_003");
	UPubnubUserMetadataEntity* UserMetadataEntity = PubnubClient->CreateUserMetadataEntity(UserToRemove);

	// Bind lambda to response delegate
	FOnPubnubRemoveUserMetadataResponseNative OnRemoveUserMetadataResponse;
	OnRemoveUserMetadataResponse.BindLambda([](const FPubnubOperationResult& Result)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to remove user metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully removed user metadata."));
		}
	});
	
	// Remove user metadata using the user metadata entity
	UserMetadataEntity->RemoveUserMetadataAsync(OnRemoveUserMetadataResponse);
}

// snippet.end

UPubnubClient* ASample_UserMetadataEntity::GetPubnubClient()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	
	//Get default PubnubClient - created automatically if PluginSettings are set to do so
	UPubnubClient* PubnubClient = PubnubSubsystem->GetPubnubClient(0);
	
	PubnubClient->SetUserID(TEXT("player_001"));
	return PubnubClient;
}
