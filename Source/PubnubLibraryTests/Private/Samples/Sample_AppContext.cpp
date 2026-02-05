// Copyright 2025 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_AppContext.h"
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
void ASample_AppContext::RunSamples()
{
	Super::RunSamples();
	
	SetUserMetadataSample();
	SetUserMetadataWithResultSample();
	SetUserMetadataWithLambdaSample();
	SetUserMetadataRawSample();
	UpdateUserMetadataIterativelySample();
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
	UpdateChannelMetadataIterativelySample();
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Create user metadata object
	FPubnubUserData UserMetadata;
	UserMetadata.UserName = "Player One";
	UserMetadata.Email = "player.one@pubnub.com";
	UserMetadata.Custom = "{\"level\": 5, \"rank\": \"gold\"}";
	
	// Set user metadata
	FString UserID = TEXT("Player_001");
	FPubnubUserInputData UserInputData = FPubnubUserInputData::FromPubnubUserData(UserMetadata);
	PubnubClient->SetUserMetadataAsync(UserID, UserInputData);
}

// snippet.set_user_metadata_with_result
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::SetUserMetadataWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse;
	OnSetUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnSetUserMetadataResponse);

	// Create user metadata object
	FString UserID = TEXT("Player_002");
	FPubnubUserData UserMetadata;
	UserMetadata.UserName = "Player Two";
	UserMetadata.Status = "active";
	UserMetadata.Custom = "{\"inventory_slots\": 20, \"guild_id\": \"G2\"}";

	// Set user metadata with all available data included in response
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	FPubnubUserInputData UserInputData = FPubnubUserInputData::FromPubnubUserData(UserMetadata);
	PubnubClient->SetUserMetadataAsync(UserID, UserInputData, OnSetUserMetadataResponse, Include);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

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
	FString UserID = TEXT("Player_003");
	FPubnubUserData UserMetadata;
	UserMetadata.UserName = "Player Three";
	UserMetadata.Status = "inactive";
	UserMetadata.Custom = "{\"last_seen\": \"2024-01-01\"}";

	// Set user metadata with all available data included in response
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	FPubnubUserInputData UserInputData = FPubnubUserInputData::FromPubnubUserData(UserMetadata);
	PubnubClient->SetUserMetadataAsync(UserID, UserInputData, OnSetUserMetadataResponse, Include);
}

// snippet.set_user_metadata_raw
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::SetUserMetadataRawSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate	
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse;
	OnSetUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnSetUserMetadataRawResponse);

	// Create user metadata object as a raw JSON string
	FString UserID = TEXT("Player_004");
	FString UserMetadataJson = R"({"name": "Player Four", "custom": {"class": "mage", "mana": 150}})";
	
	// Set user metadata with a raw include string
	FString Include = TEXT("custom");
	PubnubClient->SetUserMetadataRawAsync(UserID, UserMetadataJson, OnSetUserMetadataResponse, Include);
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

// snippet.update_user_metadata_iteratively
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::UpdateUserMetadataIterativelySample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	// Create initial user metadata object
	FString UserID = TEXT("Player_005");
	FPubnubUserInputData UserInputData;
	UserInputData.UserName = "Player Two";
	UserInputData.Status = "active";
	UserInputData.Custom = "{\"inventory_slots\": 20, \"guild_id\": \"G2\"}";

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse;
	OnSetUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnInitialSetUserMetadataResponse);

	// Set initial user metadata
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	PubnubClient->SetUserMetadataAsync(UserID, UserInputData, OnSetUserMetadataResponse, Include);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnInitialSetUserMetadataResponse(FPubnubOperationResult Result, FPubnubUserData UserData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set initial user metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		return;
	}
	
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show

	FString UserID = TEXT("Player_005");
	
	// Create updated version of user metadata object - change status to "inactive"
	FPubnubUserData UpdatedUserMetadata = UserData;
	UpdatedUserMetadata.Status = "inactive";

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse;
	OnSetUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnUpdateUserMetadataResponse);

	// Update user metadata
	FPubnubGetMetadataInclude Include;
	Include.IncludeStatus = true;
	FPubnubUserInputData UserInputData = FPubnubUserInputData::FromPubnubUserData(UpdatedUserMetadata);
	PubnubClient->SetUserMetadataAsync(UserID, UserInputData, OnSetUserMetadataResponse, Include);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnUpdateUserMetadataResponse(FPubnubOperationResult Result, FPubnubUserData UserData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to update user metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully updated user metadata. New status is: %s"), *UserData.Status);
	}
}

// snippet.get_all_user_metadata
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllUserMetadataSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubGetAllUserMetadataResponse OnGetAllUserMetadataResponse;
	OnGetAllUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllUserMetadataResponse_Simple);

	// Get all user metadata with a limit of 5
	PubnubClient->GetAllUserMetadataAsync(OnGetAllUserMetadataResponse, FPubnubGetAllInclude(), 5);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllUserMetadataResponse_Simple(FPubnubOperationResult Result, const TArray<FPubnubUserData>& UsersData, FPubnubPage Page, int TotalCount)
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
		if (!Page.Next.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *Page.Next);
		if (!Page.Prev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *Page.Prev);
	}
}

// snippet.get_all_user_metadata_with_settings
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllUserMetadataWithSettingsSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubGetAllUserMetadataResponse OnGetAllUserMetadataResponse;
	OnGetAllUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllUserMetadataResponse_WithSettings);

	// Create settings with includes and a filter
	FPubnubGetAllInclude Include;
	Include.IncludeCustom = true;
	Include.IncludeStatus = true;
	FString Filter = TEXT("status=='active'");
	int Limit = 10;
	
	// Get all user metadata with custom settings
	PubnubClient->GetAllUserMetadataAsync(OnGetAllUserMetadataResponse, Include, Limit, Filter);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllUserMetadataResponse_WithSettings(FPubnubOperationResult Result, const TArray<FPubnubUserData>& UsersData, FPubnubPage Page, int TotalCount)
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
		if (!Page.Next.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *Page.Next);
		if (!Page.Prev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *Page.Prev);
	}
}

// snippet.get_all_user_metadata_with_all_includes
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllUserMetadataWithAllIncludesSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubGetAllUserMetadataResponse OnGetAllUserMetadataResponse;
	OnGetAllUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllUserMetadataResponse_WithAllIncludes);

	// Get all user metadata with all available data included
	FPubnubGetAllInclude Include = FPubnubGetAllInclude::FromValue(true);
	FString Filter = TEXT("status=='active' || status=='inactive'");
	FPubnubGetAllSort Sort;
	Sort.GetAllSort.Add(FPubnubGetAllSingleSort{EPubnubGetAllSortType::PGAST_Name, true});
	FString NextPage = "Mg"; // Page should be taken from previous response to get next or previous part of the objects
	FPubnubPage Page;
	Page.Next = NextPage;

	PubnubClient->GetAllUserMetadataAsync(OnGetAllUserMetadataResponse, Include, 100, Filter, Sort, Page);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllUserMetadataResponse_WithAllIncludes(FPubnubOperationResult Result, const TArray<FPubnubUserData>& UsersData, FPubnubPage Page, int TotalCount)
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
		if (!Page.Next.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *Page.Next);
		if (!Page.Prev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *Page.Prev);
	}
}

// snippet.get_all_user_metadata_with_lambda
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllUserMetadataWithLambdaSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubGetAllUserMetadataResponseNative OnGetAllUserMetadataResponse;
	OnGetAllUserMetadataResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubUserData>& UsersData, FPubnubPage Page, int TotalCount)
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
			if (!Page.Next.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *Page.Next);
			if (!Page.Prev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *Page.Prev);
		}
	});
	
	// Get all user metadata with a limit of 5
	PubnubClient->GetAllUserMetadataAsync(OnGetAllUserMetadataResponse, FPubnubGetAllInclude(), 5);
}

// snippet.get_all_user_metadata_raw
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllUserMetadataRawSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubGetAllUserMetadataResponse OnGetAllUserMetadataResponse;
	OnGetAllUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllUserMetadataRawResponse);

	// Create settings with includes and a filter
	FString Include = TEXT("custom,status");
	FString Filter = TEXT("status=='active'");
	FString Sort = TEXT("name:desc,status");
	int Limit = 10;
	
	// Get all user metadata with custom settings
	PubnubClient->GetAllUserMetadataRawAsync(OnGetAllUserMetadataResponse, Include, Limit, Filter, Sort);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllUserMetadataRawResponse(FPubnubOperationResult Result, const TArray<FPubnubUserData>& UsersData, FPubnubPage Page, int TotalCount)
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
		if (!Page.Next.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *Page.Next);
		if (!Page.Prev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *Page.Prev);
	}
}

// snippet.get_user_metadata
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetUserMetadataSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubGetUserMetadataResponse OnGetUserMetadataResponse;
	OnGetUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetUserMetadataResponse_Simple);

	// Get user metadata
	FString UserID = TEXT("Player_001");
	PubnubClient->GetUserMetadataAsync(UserID, OnGetUserMetadataResponse);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubGetUserMetadataResponse OnGetUserMetadataResponse;
	OnGetUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetUserMetadataResponse_WithAllIncludes);

	// Get user metadata with all available data included
	FString UserID = TEXT("Player_002");
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	PubnubClient->GetUserMetadataAsync(UserID, OnGetUserMetadataResponse, Include);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
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
	
	// Get user metadata
	FString UserID = TEXT("Player_003");
	PubnubClient->GetUserMetadataAsync(UserID, OnGetUserMetadataResponse);
}

// snippet.get_user_metadata_raw
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetUserMetadataRawSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubGetUserMetadataResponse OnGetUserMetadataResponse;
	OnGetUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetUserMetadataRawResponse);

	// Get user metadata with raw include string
	FString UserID = TEXT("Player_004");
	FString Include = TEXT("custom,status");
	PubnubClient->GetUserMetadataRawAsync(UserID, OnGetUserMetadataResponse, Include);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	// Remove user metadata
	FString UserToRemove = TEXT("Player_001");
	PubnubClient->RemoveUserMetadataAsync(UserToRemove);
}

// snippet.remove_user_metadata_with_result
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::RemoveUserMetadataWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubRemoveUserMetadataResponse OnRemoveUserMetadataResponse;
	OnRemoveUserMetadataResponse.BindDynamic(this, &ASample_AppContext::OnRemoveUserMetadataResponse);

	// Remove user metadata
	FString UserToRemove = TEXT("Player_002");
	PubnubClient->RemoveUserMetadataAsync(UserToRemove, OnRemoveUserMetadataResponse);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

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
	
	// Remove user metadata
	FString UserToRemove = TEXT("Player_003");
	PubnubClient->RemoveUserMetadataAsync(UserToRemove, OnRemoveUserMetadataResponse);
}

// snippet.set_channel_metadata
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::SetChannelMetadataSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Create channel metadata object
	FPubnubChannelData ChannelMetadata;
	ChannelMetadata.ChannelName = "General Chat";
	ChannelMetadata.Description = "Channel for all players to chat.";
	ChannelMetadata.Custom = "{\"topic\": \"welcomes\"}";
	
	// Set channel metadata
	FString Channel = "general-chat-channel";
	FPubnubChannelInputData ChannelInputData = FPubnubChannelInputData::FromPubnubChannelData(ChannelMetadata);
	PubnubClient->SetChannelMetadataAsync(Channel, ChannelInputData);
}

// snippet.set_channel_metadata_with_result
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::SetChannelMetadataWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubSetChannelMetadataResponse OnSetChannelMetadataResponse;
	OnSetChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnSetChannelMetadataResponse);

	// Create channel metadata object
	FPubnubChannelInputData ChannelInputData;
	ChannelInputData.ChannelName = "Trade Chat";
	ChannelInputData.Status = "active";
	ChannelInputData.Custom = "{\"rules\": \"wts_wtt_only\"}";

	// Set channel metadata with all available data included in response
	FString Channel = "trade-chat-channel";
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	PubnubClient->SetChannelMetadataAsync(Channel, ChannelInputData, OnSetChannelMetadataResponse, Include);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

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

	// Set channel metadata with all available data included in response
	FString Channel = "guild-hall-channel";
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	FPubnubChannelInputData ChannelInputData = FPubnubChannelInputData({ChannelMetadata.ChannelName, ChannelMetadata.Description, ChannelMetadata.Custom, ChannelMetadata.Status, ChannelMetadata.Type});
	PubnubClient->SetChannelMetadataAsync(Channel, ChannelInputData, OnSetChannelMetadataResponse, Include);
}

// snippet.set_channel_metadata_raw
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::SetChannelMetadataRawSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubSetChannelMetadataResponse OnSetChannelMetadataResponse;
	OnSetChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnSetChannelMetadataRawResponse);

	// Create channel metadata object as a raw JSON string
	FString ChannelMetadataJson = R"({"name": "Secret Lair", "custom": {"max_players": 4, "password_protected": true}})";
	
	// Set channel metadata with a raw include string
	FString Channel = "secret-lair-channel";
	FString Include = TEXT("custom");
	PubnubClient->SetChannelMetadataRawAsync(Channel, ChannelMetadataJson, OnSetChannelMetadataResponse, Include);
}

// snippet.update_channel_metadata_iteratively
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::UpdateChannelMetadataIterativelySample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	// Create initial channel metadata object
	FString ChannelID = TEXT("iterative-channel-update-test");
	FPubnubChannelData ChannelMetadata;
	ChannelMetadata.ChannelName = "Channel For Iterative Update";
	ChannelMetadata.Description = "This is the initial description.";
	ChannelMetadata.Custom = "{\"topic\": \"initial_topic\"}";

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubSetChannelMetadataResponse OnSetChannelMetadataResponse;
	OnSetChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnInitialSetChannelMetadataResponse);

	// Set initial channel metadata 
	FPubnubChannelInputData ChannelInputData = FPubnubChannelInputData::FromPubnubChannelData(ChannelMetadata);
	PubnubClient->SetChannelMetadataAsync(ChannelID, ChannelInputData, OnSetChannelMetadataResponse);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnInitialSetChannelMetadataResponse(FPubnubOperationResult Result, FPubnubChannelData ChannelData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set initial channel metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("Successfully set initial channel metadata. Description: %s"), *ChannelData.Description);

	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show

	// Use the ChannelID from the response
	FString ChannelID = ChannelData.ChannelID;
	
	// Create updated version of channel metadata object - change description only
	FPubnubChannelData UpdatedChannelMetadata;
	UpdatedChannelMetadata.Description = "This is the updated description!";

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubSetChannelMetadataResponse OnSetChannelMetadataResponse;
	OnSetChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnUpdateChannelMetadataResponse);

	// Update channel metadata 
	FPubnubChannelInputData ChannelInputData = FPubnubChannelInputData::FromPubnubChannelData(UpdatedChannelMetadata);
	PubnubClient->SetChannelMetadataAsync(ChannelID, ChannelInputData, OnSetChannelMetadataResponse);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnUpdateChannelMetadataResponse(FPubnubOperationResult Result, FPubnubChannelData ChannelData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to update channel metadata. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully updated channel metadata. New description is: '%s' and channel name is still: '%s'"), *ChannelData.Description, *ChannelData.ChannelName);
	}
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse;
	OnGetAllChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllChannelMetadataResponse_Simple);

	// Get all channel metadata with a limit of 5
	PubnubClient->GetAllChannelMetadataAsync(OnGetAllChannelMetadataResponse, FPubnubGetAllInclude(), 5);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllChannelMetadataResponse_Simple(FPubnubOperationResult Result, const TArray<FPubnubChannelData>& ChannelsData, FPubnubPage Page, int TotalCount)
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
		if (!Page.Next.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *Page.Next);
		if (!Page.Prev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *Page.Prev);
	}
}

// snippet.get_all_channel_metadata_with_settings
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllChannelMetadataWithSettingsSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse;
	OnGetAllChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllChannelMetadataResponse_WithSettings);

	// Create settings with includes and a filter
	FPubnubGetAllInclude Include;
	Include.IncludeCustom = true;
	Include.IncludeStatus = true;
	FString Filter = TEXT("status=='active'");
	int Limit = 10;
	
	// Get all channel metadata with custom settings
	PubnubClient->GetAllChannelMetadataAsync(OnGetAllChannelMetadataResponse, Include, Limit, Filter);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllChannelMetadataResponse_WithSettings(FPubnubOperationResult Result, const TArray<FPubnubChannelData>& ChannelsData, FPubnubPage Page, int TotalCount)
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
		if (!Page.Next.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *Page.Next);
		if (!Page.Prev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *Page.Prev);
	}
}

// snippet.get_all_channel_metadata_with_all_includes
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllChannelMetadataWithAllIncludesSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse;
	OnGetAllChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllChannelMetadataResponse_WithAllIncludes);

	// Get all channel metadata with all available data included
	FPubnubGetAllInclude Include = FPubnubGetAllInclude::FromValue(true);
	FString Filter = TEXT("status=='active' || status=='inactive'");
	FPubnubGetAllSort Sort;
	Sort.GetAllSort.Add(FPubnubGetAllSingleSort{EPubnubGetAllSortType::PGAST_Name, true});
	FString NextPage = "Mg"; // Page should be taken from previous response to get next or previous part of the objects
	FPubnubPage Page;
	Page.Next = NextPage;

	PubnubClient->GetAllChannelMetadataAsync(OnGetAllChannelMetadataResponse, Include, 100, Filter, Sort, Page);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllChannelMetadataResponse_WithAllIncludes(FPubnubOperationResult Result, const TArray<FPubnubChannelData>& ChannelsData, FPubnubPage Page, int TotalCount)
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
		if (!Page.Next.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *Page.Next);
		if (!Page.Prev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *Page.Prev);
	}
}

// snippet.get_all_channel_metadata_with_lambda
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllChannelMetadataWithLambdaSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubGetAllChannelMetadataResponseNative OnGetAllChannelMetadataResponse;
	OnGetAllChannelMetadataResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubChannelData>& ChannelsData, FPubnubPage Page, int TotalCount)
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
			if (!Page.Next.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *Page.Next);
			if (!Page.Prev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *Page.Prev);
		}
	});
	
	// Get all channel metadata with a limit of 5
	PubnubClient->GetAllChannelMetadataAsync(OnGetAllChannelMetadataResponse, FPubnubGetAllInclude(), 5);
}

// snippet.get_all_channel_metadata_raw
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetAllChannelMetadataRawSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse;
	OnGetAllChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetAllChannelMetadataRawResponse);

	// Create settings with includes and a filter
	FString Include = TEXT("custom,status");
	FString Filter = TEXT("status=='active'");
	FString Sort = TEXT("name:desc,status");
	int Limit = 10;
	
	// Get all channel metadata with custom settings
	PubnubClient->GetAllChannelMetadataRawAsync(OnGetAllChannelMetadataResponse, Include, Limit, Filter, Sort);
}

// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::OnGetAllChannelMetadataRawResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelData>& ChannelsData, FPubnubPage Page, int TotalCount)
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
		if (!Page.Next.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Next Page: %s"), *Page.Next);
		if (!Page.Prev.IsEmpty()) UE_LOG(LogTemp, Log, TEXT("Previous Page: %s"), *Page.Prev);
	}
}

// snippet.get_channel_metadata
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetChannelMetadataSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubGetChannelMetadataResponse OnGetChannelMetadataResponse;
	OnGetChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetChannelMetadataResponse_Simple);

	// Get channel metadata
	FString Channel = TEXT("general-chat-channel");
	PubnubClient->GetChannelMetadataAsync(Channel, OnGetChannelMetadataResponse);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubGetChannelMetadataResponse OnGetChannelMetadataResponse;
	OnGetChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetChannelMetadataResponse_WithAllIncludes);

	// Get channel metadata with all available data included
	FString Channel = TEXT("trade-chat-channel");
	FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	PubnubClient->GetChannelMetadataAsync(Channel, OnGetChannelMetadataResponse, Include);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

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
	
	// Get channel metadata
	FString Channel = TEXT("guild-hall-channel");
	PubnubClient->GetChannelMetadataAsync(Channel, OnGetChannelMetadataResponse);
}

// snippet.get_channel_metadata_raw
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::GetChannelMetadataRawSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubGetChannelMetadataResponse OnGetChannelMetadataResponse;
	OnGetChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnGetChannelMetadataRawResponse);

	// Get channel metadata with raw include string
	FString Channel = TEXT("trade-chat-channel");
	FString Include = TEXT("custom,status");
	PubnubClient->GetChannelMetadataRawAsync(Channel, OnGetChannelMetadataResponse, Include);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	// Remove channel metadata
	FString Channel = TEXT("general-chat-channel");
	PubnubClient->RemoveChannelMetadataAsync(Channel);
}

// snippet.remove_channel_metadata_with_result
// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
void ASample_AppContext::RemoveChannelMetadataWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AppContext with name of your Actor class
	FOnPubnubRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse;
	OnRemoveChannelMetadataResponse.BindDynamic(this, &ASample_AppContext::OnRemoveChannelMetadataResponse);

	// Remove channel metadata
	FString Channel = TEXT("trade-chat-channel");
	PubnubClient->RemoveChannelMetadataAsync(Channel, OnRemoveChannelMetadataResponse);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

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
	
	// Remove channel metadata
	FString Channel = TEXT("guild-hall-channel");
	PubnubClient->RemoveChannelMetadataAsync(Channel, OnRemoveChannelMetadataResponse);
}

// snippet.end

UPubnubClient* ASample_AppContext::GetPubnubClient()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	
	//Get default PubnubClient - created automatically if PluginSettings are set to do so
	UPubnubClient* PubnubClient = PubnubSubsystem->GetPubnubClient(0);
	
	PubnubClient->SetUserID(TEXT("player_001"));
	return PubnubClient;
}
