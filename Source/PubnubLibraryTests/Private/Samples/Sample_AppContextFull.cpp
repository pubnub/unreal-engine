// Copyright 2026 PubNub Inc. All Rights Reserved.

// snippet.full_app_context_example

#include "Samples/Sample_AppContextFull.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "PubnubSubsystem.h"
#include "PubnubClient.h"


void ASample_AppContextFull::BeginPlay()
{
	Super::BeginPlay();

	//Run the example on BeginPlay
	RunAppContextFullExample();
}

void ASample_AppContextFull::RunAppContextFullExample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	
	//Create Pubnub Client using Pubnub Subsystem
	FPubnubConfig Config;
	Config.PublishKey = TEXT("demo");   //replace with your Publish Key from Admin Portal
	Config.SubscribeKey = TEXT("demo"); //replace with your Subscribe Key from Admin Portal
	Config.UserID = ExampleUserID;
	PubnubClient = PubnubSubsystem->CreatePubnubClient(Config);

	UE_LOG(LogTemp, Log, TEXT("App Context Example: Pubnub Client is created"));
	
	// Start the example by setting user metadata
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Running..."));
	
	// 1. Set User Metadata
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 1: Setting User Metadata for user '%s'"), *ExampleUserID);

	FPubnubUserData UserMetadata;
	UserMetadata.UserName = "Dragon Slayer 91";
	UserMetadata.Email = "dragon.slayer91@email.com";
	UserMetadata.Custom = "{\"class\": \"Warrior\", \"level\": 42, \"guild\": \"TheSilverSwords\"}";

	FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse;
	OnSetUserMetadataResponse.BindDynamic(this, &ASample_AppContextFull::OnSetUserMetadataResponse);
	
	FPubnubUserInputData UserInputData = FPubnubUserInputData::FromPubnubUserData(UserMetadata);
	PubnubClient->SetUserMetadataAsync(ExampleUserID, UserInputData, OnSetUserMetadataResponse);
}

void ASample_AppContextFull::OnSetUserMetadataResponse(FPubnubOperationResult Result, FPubnubUserData UserData)
{
	if (Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("App Context Example: Example failed at Step 1 (SetUserMetadata). Reason: %s"), *Result.ErrorMessage);
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 1 successful. User metadata set for '%s'."), *UserData.UserID);
	
	// 2. Set Channel Metadata
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 2: Setting Channel Metadata for channel '%s'"), *ExampleChannelID);

	FPubnubChannelData ChannelMetadata;
	ChannelMetadata.ChannelName = "The Dark Forest Quest";
	ChannelMetadata.Description = "A dangerous quest to retrieve the Sunstone.";
	ChannelMetadata.Custom = "{\"difficulty\": \"Hard\", \"min_level\": 30}";

	FOnPubnubSetChannelMetadataResponse OnSetChannelMetadataResponse;
	OnSetChannelMetadataResponse.BindDynamic(this, &ASample_AppContextFull::OnSetChannelMetadataResponse);
	
	FPubnubChannelInputData ChannelInputData = FPubnubChannelInputData::FromPubnubChannelData(ChannelMetadata);
	PubnubClient->SetChannelMetadataAsync(ExampleChannelID, ChannelInputData, OnSetChannelMetadataResponse);
}

void ASample_AppContextFull::OnSetChannelMetadataResponse(FPubnubOperationResult Result, FPubnubChannelData ChannelData)
{
	if (Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("App Context Example: Example failed at Step 2 (SetChannelMetadata). Reason: %s"), *Result.ErrorMessage);
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 2 successful. Channel metadata set for '%s'."), *ChannelData.ChannelID);
	
	// 3. Set Membership for the user in the channel
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 3: Setting Membership for user '%s' in channel '%s'"), *ExampleUserID, *ExampleChannelID);

	TArray<FPubnubMembershipInputData> MembershipsToSet;
	FPubnubMembershipInputData Membership;
	Membership.Channel = ExampleChannelID;
	Membership.Custom = "{\"role\": \"QuestLeader\", \"party_invite_pending\": false}";
	MembershipsToSet.Add(Membership);

	FOnPubnubSetMembershipsResponse OnSetMembershipsResponse;
	OnSetMembershipsResponse.BindDynamic(this, &ASample_AppContextFull::OnSetMembershipsResponse);

	PubnubClient->SetMembershipsAsync(ExampleUserID, MembershipsToSet, OnSetMembershipsResponse);
}

void ASample_AppContextFull::OnSetMembershipsResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
{
	if (Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("App Context Example: Example failed at Step 3 (SetMemberships). Reason: %s"), *Result.ErrorMessage);
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 3 successful. Membership set."));
	
	// 4a. Verify by getting the user's memberships
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 4a: Verifying by getting user's memberships..."));
	
	FOnPubnubGetMembershipsResponse OnGetMembershipsResponse;
	OnGetMembershipsResponse.BindDynamic(this, &ASample_AppContextFull::OnGetMembershipsResponse);

	PubnubClient->GetMembershipsAsync(ExampleUserID, OnGetMembershipsResponse);
}

void ASample_AppContextFull::OnGetMembershipsResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
{
	if (Result.Error || MembershipsData.Num() == 0 || MembershipsData[0].Channel.ChannelID != ExampleChannelID)
	{
		UE_LOG(LogTemp, Error, TEXT("App Context Example: Example failed at Step 4a (GetMemberships). Reason: %s"), *Result.ErrorMessage);
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 4a successful. Verified user is in channel '%s'."), *ExampleChannelID);
	
	// 4b. Verify by getting the channel's members
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 4b: Verifying by getting channel's members..."));

	FOnPubnubGetChannelMembersResponse OnGetChannelMembersResponse;
	OnGetChannelMembersResponse.BindDynamic(this, &ASample_AppContextFull::OnGetChannelMembersResponse);

	PubnubClient->GetChannelMembersAsync(ExampleChannelID, OnGetChannelMembersResponse);
}

void ASample_AppContextFull::OnGetChannelMembersResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
{
	if (Result.Error || MembersData.Num() == 0 || MembersData[0].User.UserID != ExampleUserID)
	{
		UE_LOG(LogTemp, Error, TEXT("App Context Example: Example failed at Step 4b (GetChannelMembers). Reason: %s"), *Result.ErrorMessage);
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 4b successful. Verified channel has user '%s' as a member."), *ExampleUserID);
	
	// 5. Clean up: Remove the membership
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 5: Cleaning up... Removing membership."));
	
	TArray<FString> ChannelsToRemove;
	ChannelsToRemove.Add(ExampleChannelID);

	FOnPubnubRemoveMembershipsResponse OnRemoveMembershipsResponse;
	OnRemoveMembershipsResponse.BindDynamic(this, &ASample_AppContextFull::OnRemoveMembershipsResponse);

	PubnubClient->RemoveMembershipsAsync(ExampleUserID, ChannelsToRemove, OnRemoveMembershipsResponse);
}

void ASample_AppContextFull::OnRemoveMembershipsResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
{
	if (Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("App Context Example: Example failed at Step 5 (RemoveMemberships). Reason: %s"), *Result.ErrorMessage);
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 5 successful. Membership removed."));
	
	// 6. Clean up: Remove the user metadata
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 6: Cleaning up... Removing user metadata."));

	FOnPubnubRemoveUserMetadataResponse OnRemoveUserMetadataResponse;
	OnRemoveUserMetadataResponse.BindDynamic(this, &ASample_AppContextFull::OnRemoveUserMetadataResponse);

	PubnubClient->RemoveUserMetadataAsync(ExampleUserID, OnRemoveUserMetadataResponse);
}

void ASample_AppContextFull::OnRemoveUserMetadataResponse(FPubnubOperationResult Result)
{
	if (Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("App Context Example: Example failed at Step 6 (RemoveUserMetadata). Reason: %s"), *Result.ErrorMessage);
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 6 successful. User metadata removed."));
	
	// 7. Clean up: Remove the channel metadata
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 7: Cleaning up... Removing channel metadata."));

	FOnPubnubRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse;
	OnRemoveChannelMetadataResponse.BindDynamic(this, &ASample_AppContextFull::OnRemoveChannelMetadataResponse);

	PubnubClient->RemoveChannelMetadataAsync(ExampleChannelID, OnRemoveChannelMetadataResponse);
}

void ASample_AppContextFull::OnRemoveChannelMetadataResponse(FPubnubOperationResult Result)
{
	if (Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("App Context Example: Example failed at Step 7 (RemoveChannelMetadata). Reason: %s"), *Result.ErrorMessage);
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("App Context Example: Step 7 successful. Channel metadata removed."));
	UE_LOG(LogTemp, Log, TEXT("App Context Example: finished successfully!"));
}


// snippet.end