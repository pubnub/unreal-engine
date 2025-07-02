// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_AppContext.h"
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
void ASample_AppContext::RunSamples()
{
	Super::RunSamples();
	
	SetUserMetadataSample();
	SetUserMetadataWithResultSample();
	SetUserMetadataWithLambdaSample();
	SetUserMetadataRawSample();
	GetAllUserMetadataSample();
	GetAllUserMetadataWithSettingsSample();
	GetAllUserMetadataWithAllIncludesSample();
	GetAllUserMetadataWithLambdaSample();
	GetAllUserMetadataRawSample();
	GetUserMetadataSample();
	GetUserMetadataWithAllIncludesSample();
	GetUserMetadataWithLambdaSample();
	GetUserMetadataRawSample();
	RemoveUserMetadataSample();
	RemoveUserMetadataWithResultSample();
	RemoveUserMetadataWithResultLambdaSample();
	SetChannelMetadataSample();
	SetChannelMetadataWithResultSample();
	SetChannelMetadataWithLambdaSample();
	SetChannelMetadataRawSample();
	GetAllChannelMetadataSample();
	GetAllChannelMetadataWithSettingsSample();
	GetAllChannelMetadataWithAllIncludesSample();
	GetAllChannelMetadataWithLambdaSample();
	GetAllChannelMetadataRawSample();
	GetChannelMetadataSample();
	GetChannelMetadataWithAllIncludesSample();
	GetChannelMetadataWithLambdaSample();
	GetChannelMetadataRawSample();
	RemoveChannelMetadataSample();
	RemoveChannelMetadataWithResultSample();
	RemoveChannelMetadataWithResultLambdaSample();
}
//Internal function, don't copy it with the samples
ASample_AppContext::ASample_AppContext()
{
	SamplesName = "App Context";
}


/* SAMPLE FUNCTIONS */

// snippet.set_user_metadata
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::SetUserMetadataSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create user metadata object
	FPubnubUserData UserMetadata;
	UserMetadata.UserName = "Player One";
	UserMetadata.Email = "player.one@pubnub.com";
	UserMetadata.Custom = "{\"level\": 5, \"rank\": \"gold\"}";
	
	// Set user metadata
	PubnubSubsystem->SetUserMetadata(UserID, UserMetadata);
}

// snippet.set_user_metadata_with_result
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::SetUserMetadataWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_002");
	PubnubSubsystem->SetUserID(UserID);
	
	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnSetUserMetadataResponse OnSetUserMetadataResponse;
	OnSetUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnSetUserMetadataResponse);

	// Create user metadata object
	FPubnubUserData UserMetadata;
	UserMetadata.UserName = "Player Two";
	UserMetadata.Status = "active";
	UserMetadata.Custom = "{\"inventory_slots\": 20, \"guild_id\": \"G2\"}";

	// Set user metadata with all available data included in response
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	PubnubSubsystem->SetUserMetadata(UserID, UserMetadata, OnSetUserMetadataResponse, Include);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnSetUserMetadataResponse(FPubnubOperationResult Result, FPubnubUserData UserData)
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

// snippet.set_user_metadata_with_lambda
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::SetUserMetadataWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_003");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnSetUserMetadataResponseNative OnSetUserMetadataResponse;
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
	FPubnubUserData UserMetadata;
	UserMetadata.UserName = "Player Three";
	UserMetadata.Status = "inactive";
	UserMetadata.Custom = "{\"last_seen\": \"2024-01-01\"}";

	// Set user metadata with all available data included in response
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	PubnubSubsystem->SetUserMetadata(UserID, UserMetadata, OnSetUserMetadataResponse, Include);
}

// snippet.set_user_metadata_raw
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::SetUserMetadataRawSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_004");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate	
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnSetUserMetadataResponse OnSetUserMetadataResponse;
	OnSetUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnSetUserMetadataRawResponse);

	// Create user metadata object as a raw JSON string
	FString UserMetadataJson = R"({"name": "Player Four", "custom": {"class": "mage", "mana": 150}})";
	
	// Set user metadata with a raw include string
	FString Include = TEXT("custom");
	PubnubSubsystem->SetUserMetadataRaw(UserID, UserMetadataJson, OnSetUserMetadataResponse, Include);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnSetUserMetadataRawResponse(FPubnubOperationResult Result, FPubnubUserData UserData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set user metadata (raw). Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully set user metadata (raw). UserID: %s, Name: %s, Custom: %s"), *UserData.UserID, *UserData.UserName, *UserData.Custom);
	}
}

// snippet.get_all_user_metadata
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllUserMetadataSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse;
	OnGetAllUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllUserMetadataResponse_Simple);

	// Get all user metadata with a limit of 5
	PubnubSubsystem->GetAllUserMetadata(OnGetAllUserMetadataResponse, FPubnubGetAllInclude(), 5);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllUserMetadataResponse_Simple(FPubnubOperationResult Result, const TArray<FPubnubUserData>& UsersData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get all user metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got all user metadata. Number of users: %d"), UsersData.Num());
		// Print all user metadata
		for (const FPubnubUserData& User : UsersData)
		{
			UE_LOG(LogTemp, Log, TEXT("- UserID: %s, Name: %s"), *User.UserID, *User.UserName);
		}
		if (!PageNext.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *PageNext);
		if (!PagePrev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *PagePrev);
	}
}

// snippet.get_all_user_metadata_with_settings
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllUserMetadataWithSettingsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse;
	OnGetAllUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllUserMetadataResponse_WithSettings);

	// Create settings with includes and a filter
	FPubnubGetAllInclude Include;
	Include.IncludeCustom = true;
	Include.IncludeStatus = true;
	FString Filter = TEXT("status=='active'");
	int Limit = 10;
	
	// Get all user metadata with custom settings
	PubnubSubsystem->GetAllUserMetadata(OnGetAllUserMetadataResponse, Include, Limit, Filter);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllUserMetadataResponse_WithSettings(FPubnubOperationResult Result, const TArray<FPubnubUserData>& UsersData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get all user metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got all user metadata with settings. Number of users: %d"), UsersData.Num());
		// Print all user metadata
		for (const FPubnubUserData& User : UsersData)
		{
			UE_LOG(LogTemp, Log, TEXT("- UserID: %s, Name: %s, Custom: %s, Status: %s"), *User.UserID, *User.UserName, *User.Custom, *User.Status);
		}
		if (!PageNext.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *PageNext);
		if (!PagePrev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *PagePrev);
	}
}

// snippet.get_all_user_metadata_with_all_includes
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllUserMetadataWithAllIncludesSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse;
	OnGetAllUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllUserMetadataResponse_WithAllIncludes);

	// Get all user metadata with all available data included
	FPubnubGetAllInclude Include = FPubnubGetAllInclude::FromValue(true);
	FString Filter = TEXT("status=='active' || status=='inactive'");
	FPubnubGetAllSort Sort;
	Sort.GetAllSort.Add(FPubnubGetAllSingleSort{EPubnubGetAllSortType::PGAST_Name, true});
	FString NextPage = "Mg"; // Page should be taken from previous response to get next or previous part of the objects

	PubnubSubsystem->GetAllUserMetadata(OnGetAllUserMetadataResponse, Include, 100, Filter, Sort, NextPage);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllUserMetadataResponse_WithAllIncludes(FPubnubOperationResult Result, const TArray<FPubnubUserData>& UsersData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get all user metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got all user metadata with all includes. Number of users: %d"), UsersData.Num());
		// Print all user metadata
		for (const FPubnubUserData& User : UsersData)
		{
			UE_LOG(LogTemp, Log, TEXT("- UserID: %s, Name: %s, Custom: %s, Status: %s"), *User.UserID, *User.UserName, *User.Custom, *User.Status);
		}
		if (!PageNext.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *PageNext);
		if (!PagePrev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *PagePrev);
	}
}

// snippet.get_all_user_metadata_with_lambda
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllUserMetadataWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetAllUserMetadataResponseNative OnGetAllUserMetadataResponse;
	OnGetAllUserMetadataResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubUserData>& UsersData, FString PageNext, FString PagePrev)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get all user metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully got all user metadata. Number of users: %d"), UsersData.Num());
			// Print all user metadata
			for (const FPubnubUserData& User : UsersData)
			{
				UE_LOG(LogTemp, Log, TEXT("- UserID: %s, Name: %s"), *User.UserID, *User.UserName);
			}
			if (!PageNext.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *PageNext);
			if (!PagePrev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *PagePrev);
		}
	});
	
	// Get all user metadata with a limit of 5
	PubnubSubsystem->GetAllUserMetadata(OnGetAllUserMetadataResponse, FPubnubGetAllInclude(), 5);
}

// snippet.get_all_user_metadata_raw
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllUserMetadataRawSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse;
	OnGetAllUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllUserMetadataRawResponse);

	// Create settings with includes and a filter
	FString Include = TEXT("custom,status");
	FString Filter = TEXT("status=='active'");
	FString Sort = TEXT("name:desc,status");
	int Limit = 10;
	
	// Get all user metadata with custom settings
	PubnubSubsystem->GetAllUserMetadataRaw(OnGetAllUserMetadataResponse, Include, Limit, Filter, Sort);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllUserMetadataRawResponse(FPubnubOperationResult Result, const TArray<FPubnubUserData>& UsersData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get all user metadata (raw). Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got all user metadata (raw). Number of users: %d"), UsersData.Num());
		// Print all user metadata
		for (const FPubnubUserData& User : UsersData)
		{
			UE_LOG(LogTemp, Log, TEXT("- UserID: %s, Name: %s, Custom: %s, Status: %s"), *User.UserID, *User.UserName, *User.Custom, *User.Status);
		}
		if (!PageNext.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *PageNext);
		if (!PagePrev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *PagePrev);
	}
}

// snippet.get_user_metadata
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetUserMetadataSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetUserMetadataResponse OnGetUserMetadataResponse;
	OnGetUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetUserMetadataResponse_Simple);

	// Get user metadata
	PubnubSubsystem->GetUserMetadata(UserID, OnGetUserMetadataResponse);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetUserMetadataResponse_Simple(FPubnubOperationResult Result, FPubnubUserData UserData)
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

// snippet.get_user_metadata_with_all_includes
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetUserMetadataWithAllIncludesSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_002");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetUserMetadataResponse OnGetUserMetadataResponse;
	OnGetUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetUserMetadataResponse_WithAllIncludes);

	// Get user metadata with all available data included
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	PubnubSubsystem->GetUserMetadata(UserID, OnGetUserMetadataResponse, Include);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetUserMetadataResponse_WithAllIncludes(FPubnubOperationResult Result, FPubnubUserData UserData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get user metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got user metadata with all includes. UserID: %s, Name: %s, Custom: %s, Status: %s"), *UserData.UserID, *UserData.UserName, *UserData.Custom, *UserData.Status);
	}
}

// snippet.get_user_metadata_with_lambda
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetUserMetadataWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_003");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetUserMetadataResponseNative OnGetUserMetadataResponse;
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
	
	// Get user metadata
	PubnubSubsystem->GetUserMetadata(UserID, OnGetUserMetadataResponse);
}

// snippet.get_user_metadata_raw
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetUserMetadataRawSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_004");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetUserMetadataResponse OnGetUserMetadataResponse;
	OnGetUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetUserMetadataRawResponse);

	// Get user metadata with raw include string
	FString Include = TEXT("custom,status");
	PubnubSubsystem->GetUserMetadataRaw(UserID, OnGetUserMetadataResponse, Include);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetUserMetadataRawResponse(FPubnubOperationResult Result, FPubnubUserData UserData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get user metadata (raw). Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got user metadata (raw). UserID: %s, Name: %s, Custom: %s, Status: %s"), *UserData.UserID, *UserData.UserName, *UserData.Custom, *UserData.Status);
	}
}

// snippet.remove_user_metadata
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::RemoveUserMetadataSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);
	
	// Remove user metadata
	FString UserToRemove = TEXT("Player_001");
	PubnubSubsystem->RemoveUserMetadata(UserToRemove);
}

// snippet.remove_user_metadata_with_result
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::RemoveUserMetadataWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnRemoveUserMetadataResponse OnRemoveUserMetadataResponse;
	OnRemoveUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnRemoveUserMetadataResponse);

	// Remove user metadata
	FString UserToRemove = TEXT("Player_002");
	PubnubSubsystem->RemoveUserMetadata(UserToRemove, OnRemoveUserMetadataResponse);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnRemoveUserMetadataResponse(FPubnubOperationResult Result)
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

// snippet.remove_user_metadata_with_result_lambda
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::RemoveUserMetadataWithResultLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnRemoveUserMetadataResponseNative OnRemoveUserMetadataResponse;
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
	
	// Remove user metadata
	FString UserToRemove = TEXT("Player_003");
	PubnubSubsystem->RemoveUserMetadata(UserToRemove, OnRemoveUserMetadataResponse);
}

// snippet.set_channel_metadata
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::SetChannelMetadataSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create channel metadata object
	FPubnubChannelData ChannelMetadata;
	ChannelMetadata.ChannelName = "General Chat";
	ChannelMetadata.Description = "Channel for all players to chat.";
	ChannelMetadata.Custom = "{\"topic\": \"welcomes\"}";
	
	// Set channel metadata
	FString Channel = "general-chat-channel";
	PubnubSubsystem->SetChannelMetadata(Channel, ChannelMetadata);
}

// snippet.set_channel_metadata_with_result
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::SetChannelMetadataWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);
	
	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnSetChannelMetadataResponse OnSetChannelMetadataResponse;
	OnSetChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnSetChannelMetadataResponse);

	// Create channel metadata object
	FPubnubChannelData ChannelMetadata;
	ChannelMetadata.ChannelName = "Trade Chat";
	ChannelMetadata.Status = "active";
	ChannelMetadata.Custom = "{\"rules\": \"wts_wtt_only\"}";

	// Set channel metadata with all available data included in response
	FString Channel = "trade-chat-channel";
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	PubnubSubsystem->SetChannelMetadata(Channel, ChannelMetadata, OnSetChannelMetadataResponse, Include);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnSetChannelMetadataResponse(FPubnubOperationResult Result, FPubnubChannelData ChannelData)
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

// snippet.set_channel_metadata_with_lambda
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::SetChannelMetadataWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnSetChannelMetadataResponseNative OnSetChannelMetadataResponse;
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

	// Set channel metadata with all available data included in response
	FString Channel = "guild-hall-channel";
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	PubnubSubsystem->SetChannelMetadata(Channel, ChannelMetadata, OnSetChannelMetadataResponse, Include);
}

// snippet.set_channel_metadata_raw
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::SetChannelMetadataRawSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnSetChannelMetadataResponse OnSetChannelMetadataResponse;
	OnSetChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnSetChannelMetadataRawResponse);

	// Create channel metadata object as a raw JSON string
	FString ChannelMetadataJson = R"({"name": "Secret Lair", "custom": {"max_players": 4, "password_protected": true}})";
	
	// Set channel metadata with a raw include string
	FString Channel = "secret-lair-channel";
	FString Include = TEXT("custom");
	PubnubSubsystem->SetChannelMetadataRaw(Channel, ChannelMetadataJson, OnSetChannelMetadataResponse, Include);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnSetChannelMetadataRawResponse(FPubnubOperationResult Result, FPubnubChannelData ChannelData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set channel metadata (raw). Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully set channel metadata (raw). ChannelID: %s, Name: %s, Custom: %s"), *ChannelData.ChannelID, *ChannelData.ChannelName, *ChannelData.Custom);
	}
}


// snippet.get_all_channel_metadata
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllChannelMetadataSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse;
	OnGetAllChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllChannelMetadataResponse_Simple);

	// Get all channel metadata with a limit of 5
	PubnubSubsystem->GetAllChannelMetadata(OnGetAllChannelMetadataResponse, FPubnubGetAllInclude(), 5);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllChannelMetadataResponse_Simple(FPubnubOperationResult Result, const TArray<FPubnubChannelData>& ChannelsData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get all channel metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got all channel metadata. Number of channels: %d"), ChannelsData.Num());
		// Print all channel metadata
		for (const FPubnubChannelData& Channel : ChannelsData)
		{
			UE_LOG(LogTemp, Log, TEXT("- ChannelID: %s, Name: %s"), *Channel.ChannelID, *Channel.ChannelName);
		}
		if (!PageNext.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *PageNext);
		if (!PagePrev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *PagePrev);
	}
}

// snippet.get_all_channel_metadata_with_settings
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllChannelMetadataWithSettingsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse;
	OnGetAllChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllChannelMetadataResponse_WithSettings);

	// Create settings with includes and a filter
	FPubnubGetAllInclude Include;
	Include.IncludeCustom = true;
	Include.IncludeStatus = true;
	FString Filter = TEXT("status=='active'");
	int Limit = 10;
	
	// Get all channel metadata with custom settings
	PubnubSubsystem->GetAllChannelMetadata(OnGetAllChannelMetadataResponse, Include, Limit, Filter);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllChannelMetadataResponse_WithSettings(FPubnubOperationResult Result, const TArray<FPubnubChannelData>& ChannelsData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get all channel metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got all channel metadata with settings. Number of channels: %d"), ChannelsData.Num());
		// Print all channel metadata
		for (const FPubnubChannelData& Channel : ChannelsData)
		{
			UE_LOG(LogTemp, Log, TEXT("- ChannelID: %s, Name: %s, Custom: %s, Status: %s"), *Channel.ChannelID, *Channel.ChannelName, *Channel.Custom, *Channel.Status);
		}
		if (!PageNext.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *PageNext);
		if (!PagePrev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *PagePrev);
	}
}

// snippet.get_all_channel_metadata_with_all_includes
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllChannelMetadataWithAllIncludesSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse;
	OnGetAllChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllChannelMetadataResponse_WithAllIncludes);

	// Get all channel metadata with all available data included
	FPubnubGetAllInclude Include = FPubnubGetAllInclude::FromValue(true);
	FString Filter = TEXT("status=='active' || status=='inactive'");
	FPubnubGetAllSort Sort;
	Sort.GetAllSort.Add(FPubnubGetAllSingleSort{EPubnubGetAllSortType::PGAST_Name, true});
	FString NextPage = "Mg"; // Page should be taken from previous response to get next or previous part of the objects

	PubnubSubsystem->GetAllChannelMetadata(OnGetAllChannelMetadataResponse, Include, 100, Filter, Sort, NextPage);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllChannelMetadataResponse_WithAllIncludes(FPubnubOperationResult Result, const TArray<FPubnubChannelData>& ChannelsData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get all channel metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got all channel metadata with all includes. Number of channels: %d"), ChannelsData.Num());
		// Print all channel metadata
		for (const FPubnubChannelData& Channel : ChannelsData)
		{
			UE_LOG(LogTemp, Log, TEXT("- ChannelID: %s, Name: %s, Custom: %s, Status: %s"), *Channel.ChannelID, *Channel.ChannelName, *Channel.Custom, *Channel.Status);
		}
		if (!PageNext.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *PageNext);
		if (!PagePrev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *PagePrev);
	}
}

// snippet.get_all_channel_metadata_with_lambda
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllChannelMetadataWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnGetAllChannelMetadataResponseNative OnGetAllChannelMetadataResponse;
	OnGetAllChannelMetadataResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubChannelData>& ChannelsData, FString PageNext, FString PagePrev)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get all channel metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully got all channel metadata. Number of channels: %d"), ChannelsData.Num());
			for (const FPubnubChannelData& Channel : ChannelsData)
			{
				UE_LOG(LogTemp, Log, TEXT("- ChannelID: %s, Name: %s"), *Channel.ChannelID, *Channel.ChannelName);
			}
			if (!PageNext.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *PageNext);
			if (!PagePrev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *PagePrev);
		}
	});
	
	// Get all channel metadata with a limit of 5
	PubnubSubsystem->GetAllChannelMetadata(OnGetAllChannelMetadataResponse, FPubnubGetAllInclude(), 5);
}

// snippet.get_all_channel_metadata_raw
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllChannelMetadataRawSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse;
	OnGetAllChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllChannelMetadataRawResponse);

	// Create settings with includes and a filter
	FString Include = TEXT("custom,status");
	FString Filter = TEXT("status=='active'");
	FString Sort = TEXT("name:desc,status");
	int Limit = 10;
	
	// Get all channel metadata with custom settings
	PubnubSubsystem->GetAllChannelMetadataRaw(OnGetAllChannelMetadataResponse, Include, Limit, Filter, Sort);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllChannelMetadataRawResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelData>& ChannelsData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get all channel metadata (raw). Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got all channel metadata (raw). Number of channels: %d"), ChannelsData.Num());
		// Print all channel metadata
		for (const FPubnubChannelData& Channel : ChannelsData)
		{
			UE_LOG(LogTemp, Log, TEXT("- ChannelID: %s, Name: %s, Custom: %s, Status: %s"), *Channel.ChannelID, *Channel.ChannelName, *Channel.Custom, *Channel.Status);
		}
		if (!PageNext.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *PageNext);
		if (!PagePrev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *PagePrev);
	}
}

// snippet.get_channel_metadata
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetChannelMetadataSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetChannelMetadataResponse OnGetChannelMetadataResponse;
	OnGetChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetChannelMetadataResponse_Simple);

	// Get channel metadata
	FString Channel = TEXT("general-chat-channel");
	PubnubSubsystem->GetChannelMetadata(Channel, OnGetChannelMetadataResponse);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetChannelMetadataResponse_Simple(FPubnubOperationResult Result, FPubnubChannelData ChannelData)
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

// snippet.get_channel_metadata_with_all_includes
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetChannelMetadataWithAllIncludesSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetChannelMetadataResponse OnGetChannelMetadataResponse;
	OnGetChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetChannelMetadataResponse_WithAllIncludes);

	// Get channel metadata with all available data included
	FString Channel = TEXT("trade-chat-channel");
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	PubnubSubsystem->GetChannelMetadata(Channel, OnGetChannelMetadataResponse, Include);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetChannelMetadataResponse_WithAllIncludes(FPubnubOperationResult Result, FPubnubChannelData ChannelData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get channel metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got channel metadata with all includes. ChannelID: %s, Name: %s, Custom: %s, Status: %s"), *ChannelData.ChannelID, *ChannelData.ChannelName, *ChannelData.Custom, *ChannelData.Status);
	}
}

// snippet.get_channel_metadata_with_lambda
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetChannelMetadataWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnGetChannelMetadataResponseNative OnGetChannelMetadataResponse;
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
	
	// Get channel metadata
	FString Channel = TEXT("guild-hall-channel");
	PubnubSubsystem->GetChannelMetadata(Channel, OnGetChannelMetadataResponse);
}

// snippet.get_channel_metadata_raw
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetChannelMetadataRawSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnGetChannelMetadataResponse OnGetChannelMetadataResponse;
	OnGetChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetChannelMetadataRawResponse);

	// Get channel metadata with raw include string
	FString Channel = TEXT("trade-chat-channel");
	FString Include = TEXT("custom,status");
	PubnubSubsystem->GetChannelMetadataRaw(Channel, OnGetChannelMetadataResponse, Include);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetChannelMetadataRawResponse(FPubnubOperationResult Result, FPubnubChannelData ChannelData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get channel metadata (raw). Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got channel metadata (raw). ChannelID: %s, Name: %s, Custom: %s, Status: %s"), *ChannelData.ChannelID, *ChannelData.ChannelName, *ChannelData.Custom, *ChannelData.Status);
	}
}

// snippet.remove_channel_metadata
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::RemoveChannelMetadataSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);
	
	// Remove channel metadata
	FString Channel = TEXT("general-chat-channel");
	PubnubSubsystem->RemoveChannelMetadata(Channel);
}

// snippet.remove_channel_metadata_with_result
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::RemoveChannelMetadataWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse;
	OnRemoveChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnRemoveChannelMetadataResponse);

	// Remove channel metadata
	FString Channel = TEXT("trade-chat-channel");
	PubnubSubsystem->RemoveChannelMetadata(Channel, OnRemoveChannelMetadataResponse);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnRemoveChannelMetadataResponse(FPubnubOperationResult Result)
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

// snippet.remove_channel_metadata_with_result_lambda
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::RemoveChannelMetadataWithResultLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnRemoveChannelMetadataResponseNative OnRemoveChannelMetadataResponse;
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
	
	// Remove channel metadata
	FString Channel = TEXT("guild-hall-channel");
	PubnubSubsystem->RemoveChannelMetadata(Channel, OnRemoveChannelMetadataResponse);
}

// snippet.end
