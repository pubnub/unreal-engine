// Copyright 2025 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_Memberships.h"
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
void ASample_Memberships::RunSamples()
{
	Super::RunSamples();
	
	SetMembershipsSample();
	SetMembershipsWithResultSample();
	SetMembershipsWithLambdaSample();
	SetMembershipsRawSample();
	GetMembershipsSample();
	GetMembershipsWithSettingsSample();
	GetMembershipsWithLambdaSample();
	GetMembershipsRawSample();
	RemoveMembershipsSample();
	RemoveMembershipsWithResultSample();
	RemoveMembershipsWithLambdaSample();
	RemoveMembershipsRawSample();
	SetChannelMembersSample();
	SetChannelMembersWithResultSample();
	SetChannelMembersWithLambdaSample();
	SetChannelMembersRawSample();
	GetChannelMembersSample();
	GetChannelMembersWithSettingsSample();
	GetChannelMembersWithLambdaSample();
	GetChannelMembersRawSample();
	RemoveChannelMembersSample();
	RemoveChannelMembersWithResultSample();
	RemoveChannelMembersWithLambdaSample();
	RemoveChannelMembersRawSample();
}
//Internal function, don't copy it with the samples
ASample_Memberships::ASample_Memberships()
{
	SamplesName = "App Context - Memberships";
}


/* SAMPLE FUNCTIONS */

// snippet.set_memberships
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::SetMembershipsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create memberships data
	TArray<FPubnubMembershipInputData> MembershipsToSet;
	FPubnubMembershipInputData Membership1;
	Membership1.Channel = "general-chat-channel";
	Membership1.Type = "standard";
	MembershipsToSet.Add(Membership1);
	
	FPubnubMembershipInputData Membership2;
	Membership2.Channel = "trade-chat-channel";
	Membership2.Custom = "{\"is_moderator\": true}";
	Membership2.Status = "active";
	MembershipsToSet.Add(Membership2);
	
	// Set memberships for the user
	PubnubSubsystem->SetMemberships(UserID, MembershipsToSet);
}

// snippet.set_memberships_with_result
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::SetMembershipsWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_002");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnSetMembershipsResponse OnSetMembershipsResponse;
	OnSetMembershipsResponse.BindDynamic(this, &ASample_Memberships::OnSetMembershipsResponse);
	
	// Create memberships data
	TArray<FPubnubMembershipInputData> MembershipsToSet;
	FPubnubMembershipInputData Membership1;
	Membership1.Channel = "guild-hall-channel";
	Membership1.Status = "pending";
	MembershipsToSet.Add(Membership1);

	// Set memberships and include extra data in the response
	FPubnubMembershipInclude Include = FPubnubMembershipInclude::FromValue(true);
	PubnubSubsystem->SetMemberships(UserID, MembershipsToSet, OnSetMembershipsResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnSetMembershipsResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set memberships. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully set memberships. Count: %d"), MembershipsData.Num());
		for (const auto& Membership : MembershipsData)
		{
			UE_LOG(LogTemp, Log, TEXT("- Channel: %s, Type: %s, Status: %s, Custom: %s"), *Membership.Channel.ChannelID, *Membership.Type, *Membership.Status, *Membership.Custom);
		}
	}
}

// snippet.set_memberships_with_lambda
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::SetMembershipsWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_003");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnSetMembershipsResponseNative OnSetMembershipsResponse;
	OnSetMembershipsResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to set memberships. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully set memberships. Count: %d"), MembershipsData.Num());
			for (const auto& Membership : MembershipsData)
			{
				UE_LOG(LogTemp, Log, TEXT("- Channel: %s, Type: %s, Status: %s, Custom: %s"), *Membership.Channel.ChannelID, *Membership.Type, *Membership.Status, *Membership.Custom);
			}
		}
	});

	// Create memberships data
	TArray<FPubnubMembershipInputData> MembershipsToSet;
	FPubnubMembershipInputData Membership1;
	Membership1.Channel = "private-lounge-channel";
	Membership1.Type = "premium";
	MembershipsToSet.Add(Membership1);
	
	// Set memberships and include extra data in the response
	FPubnubMembershipInclude Include = FPubnubMembershipInclude::FromValue(true);
	PubnubSubsystem->SetMemberships(UserID, MembershipsToSet, OnSetMembershipsResponse, Include);
}

// snippet.set_memberships_raw
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::SetMembershipsRawSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_004");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnSetMembershipsResponse OnSetMembershipsResponse;
	OnSetMembershipsResponse.BindDynamic(this, &ASample_Memberships::OnSetMembershipsRawResponse);
	
	// Create memberships data as a raw JSON string
	FString MembershipsJson = R"([{"channel": {"id": "arena-channel"}, "custom": {"rank": "diamond"}, "status": "active"}, {"channel": {"id": "spectator-channel"}, "type": "viewer"}])";
	
	// Set memberships with raw JSON and includes
	FString Include = "custom,status,type,channel.custom";
	PubnubSubsystem->SetMembershipsRaw(UserID, MembershipsJson, OnSetMembershipsResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnSetMembershipsRawResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set memberships. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully set memberships. Count: %d"), MembershipsData.Num());
		for (const auto& Membership : MembershipsData)
		{
			UE_LOG(LogTemp, Log, TEXT("- Channel: %s, Type: %s, Status: %s, Custom: %s"), *Membership.Channel.ChannelID, *Membership.Type, *Membership.Status, *Membership.Custom);
		}
	}
}

// snippet.get_memberships
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::GetMembershipsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnGetMembershipsResponse OnGetMembershipsResponse;
	OnGetMembershipsResponse.BindDynamic(this, &ASample_Memberships::OnGetMembershipsResponse_Simple);

	// Get memberships for a user
	PubnubSubsystem->GetMemberships(UserID, OnGetMembershipsResponse);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnGetMembershipsResponse_Simple(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get memberships. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got memberships. Count: %d"), MembershipsData.Num());
		for (const auto& Membership : MembershipsData)
		{
			UE_LOG(LogTemp, Log, TEXT("- Channel: %s"), *Membership.Channel.ChannelID);
		}
	}
}

// snippet.get_memberships_with_settings
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::GetMembershipsWithSettingsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_002");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnGetMembershipsResponse OnGetMembershipsResponse;
	OnGetMembershipsResponse.BindDynamic(this, &ASample_Memberships::OnGetMembershipsResponse_WithSettings);

	// Create settings
	FPubnubMembershipInclude Include = FPubnubMembershipInclude::FromValue(true);
	FString Filter = TEXT("status=='active'");
	FPubnubMembershipSort Sort;
	Sort.MembershipSort.Add(FPubnubMembershipSingleSort{EPubnubMembershipSortType::PMST_ChannelID, true});
	
	// Get memberships with custom settings
	PubnubSubsystem->GetMemberships(UserID, OnGetMembershipsResponse, Include, 100, Filter, Sort);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnGetMembershipsResponse_WithSettings(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get memberships. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got memberships with settings. Count: %d"), MembershipsData.Num());
		for (const auto& Membership : MembershipsData)
		{
			UE_LOG(LogTemp, Log, TEXT("- Channel: %s, Custom: %s"), *Membership.Channel.ChannelID, *Membership.Custom);
		}
	}
}

// snippet.get_memberships_with_lambda
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::GetMembershipsWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_003");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnGetMembershipsResponseNative OnGetMembershipsResponse;
	OnGetMembershipsResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get memberships. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully got memberships. Count: %d"), MembershipsData.Num());
			for (const auto& Membership : MembershipsData)
			{
				UE_LOG(LogTemp, Log, TEXT("- Channel: %s"), *Membership.Channel.ChannelID);
			}
		}
	});
	
	// Get memberships for a user
	PubnubSubsystem->GetMemberships(UserID, OnGetMembershipsResponse);
}

// snippet.get_memberships_raw
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::GetMembershipsRawSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_004");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnGetMembershipsResponse OnGetMembershipsResponse;
	OnGetMembershipsResponse.BindDynamic(this, &ASample_Memberships::OnGetMembershipsRawResponse);
	
	// Create settings as raw strings
	FString Include = "custom,channel.custom";
	FString Filter = TEXT("status=='active'");
	FString Sort = "channel.id:desc";
	
	// Get memberships with raw settings
	PubnubSubsystem->GetMembershipsRaw(UserID, OnGetMembershipsResponse, Include, 100, Filter, Sort);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnGetMembershipsRawResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get memberships (raw). Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got memberships (raw). Count: %d"), MembershipsData.Num());
		for (const auto& Membership : MembershipsData)
		{
			UE_LOG(LogTemp, Log, TEXT("- Channel: %s, Custom: %s"), *Membership.Channel.ChannelID, *Membership.Custom);
		}
	}
}

// snippet.remove_memberships
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::RemoveMembershipsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a list of channel IDs to remove memberships from
	TArray<FString> ChannelsToRemove;
	ChannelsToRemove.Add("general-chat-channel");
	ChannelsToRemove.Add("trade-chat-channel");
	
	// Remove memberships for the user
	PubnubSubsystem->RemoveMemberships(UserID, ChannelsToRemove);
}

// snippet.remove_memberships_with_result
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::RemoveMembershipsWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_002");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnRemoveMembershipsResponse OnRemoveMembershipsResponse;
	OnRemoveMembershipsResponse.BindDynamic(this, &ASample_Memberships::OnRemoveMembershipsResponse);
	
	// Create a list of channel IDs to remove memberships from
	TArray<FString> ChannelsToRemove;
	ChannelsToRemove.Add("guild-hall-channel");

	// Remove memberships and include extra data in the response
	FPubnubMembershipInclude Include = FPubnubMembershipInclude::FromValue(true);
	PubnubSubsystem->RemoveMemberships(UserID, ChannelsToRemove, OnRemoveMembershipsResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnRemoveMembershipsResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to remove memberships. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully removed memberships. Count: %d"), MembershipsData.Num());
	}
}

// snippet.remove_memberships_with_lambda
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::RemoveMembershipsWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_003");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnRemoveMembershipsResponseNative OnRemoveMembershipsResponse;
	OnRemoveMembershipsResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to remove memberships. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully removed memberships. Count: %d"), MembershipsData.Num());
		}
	});

	// Create a list of channel IDs to remove memberships from
	TArray<FString> ChannelsToRemove;
	ChannelsToRemove.Add("private-lounge-channel");
	
	// Remove memberships and include extra data in the response
	FPubnubMembershipInclude Include = FPubnubMembershipInclude::FromValue(true);
	PubnubSubsystem->RemoveMemberships(UserID, ChannelsToRemove, OnRemoveMembershipsResponse, Include);
}

// snippet.remove_memberships_raw
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::RemoveMembershipsRawSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_004");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnRemoveMembershipsResponse OnRemoveMembershipsResponse;
	OnRemoveMembershipsResponse.BindDynamic(this, &ASample_Memberships::OnRemoveMembershipsRawResponse);
	
	// Create memberships data as a raw JSON string
	FString MembershipsJson = R"([{"channel": {"id": "arena-channel"}}, {"channel": {"id": "spectator-channel"}}])";
	
	// Remove memberships with raw JSON and includes
	FString Include = "custom,channel.custom";
	PubnubSubsystem->RemoveMembershipsRaw(UserID, MembershipsJson, OnRemoveMembershipsResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnRemoveMembershipsRawResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to remove memberships. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully removed memberships. Count: %d"), MembershipsData.Num());
	}
}

// snippet.set_channel_members
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::SetChannelMembersSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create members data
	TArray<FPubnubChannelMemberInputData> MembersToSet;
	FPubnubChannelMemberInputData Member1;
	Member1.User = "User_001";
	Member1.Type = "standard";
	MembersToSet.Add(Member1);
	
	FPubnubChannelMemberInputData Member2;
	Member2.User = "User_002";
	Member2.Custom = "{\"is_moderator\": true}";
	Member2.Status = "active";
	MembersToSet.Add(Member2);
	
	// Set members for the channel
	PubnubSubsystem->SetChannelMembers("general-chat-channel", MembersToSet);
}

// snippet.set_channel_members_with_result
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::SetChannelMembersWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_002");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnSetChannelMembersResponse OnSetChannelMembersResponse;
	OnSetChannelMembersResponse.BindDynamic(this, &ASample_Memberships::OnSetChannelMembersResponse);
	
	// Create members data
	TArray<FPubnubChannelMemberInputData> MembersToSet;
	FPubnubChannelMemberInputData Member1;
	Member1.User = "User_003";
	Member1.Status = "pending";
	MembersToSet.Add(Member1);

	// Set members and include extra data in the response
	FPubnubMemberInclude Include = FPubnubMemberInclude::FromValue(true);
	PubnubSubsystem->SetChannelMembers("guild-hall-channel", MembersToSet, OnSetChannelMembersResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnSetChannelMembersResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set channel members. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully set channel members. Count: %d"), MembersData.Num());
		for (const auto& Member : MembersData)
		{
			UE_LOG(LogTemp, Log, TEXT("- User: %s, Type: %s, Status: %s, Custom: %s"), *Member.User.UserID, *Member.Type, *Member.Status, *Member.Custom);
		}
	}
}

// snippet.set_channel_members_with_lambda
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::SetChannelMembersWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_003");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnSetChannelMembersResponseNative OnSetChannelMembersResponse;
	OnSetChannelMembersResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to set channel members. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully set channel members. Count: %d"), MembersData.Num());
			for (const auto& Member : MembersData)
			{
				UE_LOG(LogTemp, Log, TEXT("- User: %s, Type: %s, Status: %s, Custom: %s"), *Member.User.UserID, *Member.Type, *Member.Status, *Member.Custom);
			}
		}
	});

	// Create members data
	TArray<FPubnubChannelMemberInputData> MembersToSet;
	FPubnubChannelMemberInputData Member1;
	Member1.User = "User_004";
	Member1.Type = "premium";
	MembersToSet.Add(Member1);
	
	// Set members and include extra data in the response
	FPubnubMemberInclude Include = FPubnubMemberInclude::FromValue(true);
	PubnubSubsystem->SetChannelMembers("private-lounge-channel", MembersToSet, OnSetChannelMembersResponse, Include);
}

// snippet.set_channel_members_raw
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::SetChannelMembersRawSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_004");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnSetChannelMembersResponse OnSetChannelMembersResponse;
	OnSetChannelMembersResponse.BindDynamic(this, &ASample_Memberships::OnSetChannelMembersRawResponse);
	
	// Create members data as a raw JSON string
	FString MembersJson = R"([{"user": {"id": "User_005"}, "custom": {"rank": "diamond"}, "status": "active"}, {"user": {"id": "User_006"}, "type": "viewer"}])";
	
	// Set members with raw JSON and includes
	FString Include = "custom,status,type,user.custom";
	PubnubSubsystem->SetChannelMembersRaw("arena-channel", MembersJson, OnSetChannelMembersResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnSetChannelMembersRawResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set channel members. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully set channel members. Count: %d"), MembersData.Num());
		for (const auto& Member : MembersData)
		{
			UE_LOG(LogTemp, Log, TEXT("- User: %s, Type: %s, Status: %s, Custom: %s"), *Member.User.UserID, *Member.Type, *Member.Status, *Member.Custom);
		}
	}
}

// snippet.get_channel_members
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::GetChannelMembersSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnGetChannelMembersResponse OnGetChannelMembersResponse;
	OnGetChannelMembersResponse.BindDynamic(this, &ASample_Memberships::OnGetChannelMembersResponse_Simple);

	// Get members for a channel
	PubnubSubsystem->GetChannelMembers("general-chat-channel", OnGetChannelMembersResponse);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnGetChannelMembersResponse_Simple(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get channel members. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got channel members. Count: %d"), MembersData.Num());
		for (const auto& Member : MembersData)
		{
			UE_LOG(LogTemp, Log, TEXT("- User: %s"), *Member.User.UserID);
		}
	}
}

// snippet.get_channel_members_with_settings
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::GetChannelMembersWithSettingsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_002");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnGetChannelMembersResponse OnGetChannelMembersResponse;
	OnGetChannelMembersResponse.BindDynamic(this, &ASample_Memberships::OnGetChannelMembersResponse_WithSettings);

	// Create settings
	FPubnubMemberInclude Include = FPubnubMemberInclude::FromValue(true);
	FString Filter = TEXT("status=='active'");
	FPubnubMemberSort Sort;
	Sort.MemberSort.Add(FPubnubMemberSingleSort{EPubnubMemberSortType::PMeST_UserID, true});
	
	// Get members with custom settings
	PubnubSubsystem->GetChannelMembers("guild-hall-channel", OnGetChannelMembersResponse, Include, 100, Filter, Sort);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnGetChannelMembersResponse_WithSettings(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get channel members. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got channel members with settings. Count: %d"), MembersData.Num());
		for (const auto& Member : MembersData)
		{
			UE_LOG(LogTemp, Log, TEXT("- User: %s, Custom: %s"), *Member.User.UserID, *Member.Custom);
		}
	}
}

// snippet.get_channel_members_with_lambda
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::GetChannelMembersWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_003");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnGetChannelMembersResponseNative OnGetChannelMembersResponse;
	OnGetChannelMembersResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get channel members. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully got channel members. Count: %d"), MembersData.Num());
			for (const auto& Member : MembersData)
			{
				UE_LOG(LogTemp, Log, TEXT("- User: %s"), *Member.User.UserID);
			}
		}
	});
	
	// Get members for a channel
	PubnubSubsystem->GetChannelMembers("private-lounge-channel", OnGetChannelMembersResponse);
}

// snippet.get_channel_members_raw
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::GetChannelMembersRawSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_004");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnGetChannelMembersResponse OnGetChannelMembersResponse;
	OnGetChannelMembersResponse.BindDynamic(this, &ASample_Memberships::OnGetChannelMembersRawResponse);
	
	// Create settings as raw strings
	FString Include = "custom,user.custom";
	FString Filter = TEXT("status=='active'");
	FString Sort = "user.id:desc";
	
	// Get members with raw settings
	PubnubSubsystem->GetChannelMembersRaw("arena-channel", OnGetChannelMembersResponse, Include, 100, Filter, Sort);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnGetChannelMembersRawResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get channel members (raw). Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully got channel members (raw). Count: %d"), MembersData.Num());
		for (const auto& Member : MembersData)
		{
			UE_LOG(LogTemp, Log, TEXT("- User: %s, Custom: %s"), *Member.User.UserID, *Member.Custom);
		}
	}
}

// snippet.remove_channel_members
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::RemoveChannelMembersSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a list of user IDs to remove from the channel
	TArray<FString> UsersToRemove;
	UsersToRemove.Add("User_001");
	UsersToRemove.Add("User_002");
	
	// Remove members from the channel
	PubnubSubsystem->RemoveChannelMembers("general-chat-channel", UsersToRemove);
}

// snippet.remove_channel_members_with_result
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::RemoveChannelMembersWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_002");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnRemoveChannelMembersResponse OnRemoveChannelMembersResponse;
	OnRemoveChannelMembersResponse.BindDynamic(this, &ASample_Memberships::OnRemoveChannelMembersResponse);
	
	// Create a list of user IDs to remove
	TArray<FString> UsersToRemove;
	UsersToRemove.Add("User_003");

	// Remove members and include extra data in the response
	FPubnubMemberInclude Include = FPubnubMemberInclude::FromValue(true);
	PubnubSubsystem->RemoveChannelMembers("guild-hall-channel", UsersToRemove, OnRemoveChannelMembersResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnRemoveChannelMembersResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to remove channel members. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully removed channel members. Count: %d"), MembersData.Num());
	}
}

// snippet.remove_channel_members_with_lambda
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::RemoveChannelMembersWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_003");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnRemoveChannelMembersResponseNative OnRemoveChannelMembersResponse;
	OnRemoveChannelMembersResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to remove channel members. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully removed channel members. Count: %d"), MembersData.Num());
		}
	});

	// Create a list of user IDs to remove
	TArray<FString> UsersToRemove;
	UsersToRemove.Add("User_004");
	
	// Remove members and include extra data in the response
	FPubnubMemberInclude Include = FPubnubMemberInclude::FromValue(true);
	PubnubSubsystem->RemoveChannelMembers("private-lounge-channel", UsersToRemove, OnRemoveChannelMembersResponse, Include);
}

// snippet.remove_channel_members_raw
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::RemoveChannelMembersRawSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_004");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnRemoveChannelMembersResponse OnRemoveChannelMembersResponse;
	OnRemoveChannelMembersResponse.BindDynamic(this, &ASample_Memberships::OnRemoveChannelMembersRawResponse);
	
	// Create members data as a raw JSON string
	FString MembersJson = R"([{"user": {"id": "User_005"}}, {"user": {"id": "User_006"}}])";
	
	// Remove members with raw JSON and includes
	FString Include = "custom,user.custom";
	PubnubSubsystem->RemoveChannelMembersRaw("arena-channel", MembersJson, OnRemoveChannelMembersResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnRemoveChannelMembersRawResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to remove channel members. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully removed channel members. Count: %d"), MembersData.Num());
	}
}

// snippet.end
