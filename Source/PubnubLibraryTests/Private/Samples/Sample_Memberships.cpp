// Copyright 2025 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_Memberships.h"
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

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
	FString UserID = TEXT("Player_001");
	PubnubClient->SetMembershipsAsync(UserID, MembershipsToSet);
}

// snippet.set_memberships_with_result
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::SetMembershipsWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnPubnubSetMembershipsResponse OnSetMembershipsResponse;
	OnSetMembershipsResponse.BindDynamic(this, &ASample_Memberships::OnSetMembershipsResponse);
	
	// Create memberships data
	TArray<FPubnubMembershipInputData> MembershipsToSet;
	FPubnubMembershipInputData Membership1;
	Membership1.Channel = "guild-hall-channel";
	Membership1.Status = "pending";
	MembershipsToSet.Add(Membership1);

	// Set memberships and include extra data in the response
	FString UserID = TEXT("Player_002");
	FPubnubMembershipInclude Include = FPubnubMembershipInclude::FromValue(true);
	PubnubClient->SetMembershipsAsync(UserID, MembershipsToSet, OnSetMembershipsResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnSetMembershipsResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubSetMembershipsResponseNative OnSetMembershipsResponse;
	OnSetMembershipsResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
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
	FString UserID = TEXT("Player_003");
	FPubnubMembershipInclude Include = FPubnubMembershipInclude::FromValue(true);
	PubnubClient->SetMembershipsAsync(UserID, MembershipsToSet, OnSetMembershipsResponse, Include);
}

// snippet.set_memberships_raw
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::SetMembershipsRawSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnPubnubSetMembershipsResponse OnSetMembershipsResponse;
	OnSetMembershipsResponse.BindDynamic(this, &ASample_Memberships::OnSetMembershipsRawResponse);
	
	// Create memberships data as a raw JSON string
	FString MembershipsJson = R"([{"channel": {"id": "arena-channel"}, "custom": {"rank": "diamond"}, "status": "active"}, {"channel": {"id": "spectator-channel"}, "type": "viewer"}])";
	
	// Set memberships with raw JSON and includes
	FString UserID = TEXT("Player_004");
	FString Include = "custom,status,type,channel.custom";
	PubnubClient->SetMembershipsRawAsync(UserID, MembershipsJson, OnSetMembershipsResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnSetMembershipsRawResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnPubnubGetMembershipsResponse OnGetMembershipsResponse;
	OnGetMembershipsResponse.BindDynamic(this, &ASample_Memberships::OnGetMembershipsResponse_Simple);

	// Get memberships for a user
	FString UserID = TEXT("Player_001");
	PubnubClient->GetMembershipsAsync(UserID, OnGetMembershipsResponse);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnGetMembershipsResponse_Simple(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnPubnubGetMembershipsResponse OnGetMembershipsResponse;
	OnGetMembershipsResponse.BindDynamic(this, &ASample_Memberships::OnGetMembershipsResponse_WithSettings);

	// Create settings
	FString UserID = TEXT("Player_002");
	FPubnubMembershipInclude Include = FPubnubMembershipInclude::FromValue(true);
	FString Filter = TEXT("status=='active'");
	FPubnubMembershipSort Sort;
	Sort.MembershipSort.Add(FPubnubMembershipSingleSort{EPubnubMembershipSortType::PMST_ChannelID, true});
	
	// Get memberships with custom settings
	PubnubClient->GetMembershipsAsync(UserID, OnGetMembershipsResponse, Include, 100, Filter, Sort);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnGetMembershipsResponse_WithSettings(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubGetMembershipsResponseNative OnGetMembershipsResponse;
	OnGetMembershipsResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
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
	FString UserID = TEXT("Player_003");
	PubnubClient->GetMembershipsAsync(UserID, OnGetMembershipsResponse);
}

// snippet.get_memberships_raw
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::GetMembershipsRawSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnPubnubGetMembershipsResponse OnGetMembershipsResponse;
	OnGetMembershipsResponse.BindDynamic(this, &ASample_Memberships::OnGetMembershipsRawResponse);
	
	// Create settings as raw strings
	FString UserID = TEXT("Player_004");
	FString Include = "custom,channel.custom";
	FString Filter = TEXT("status=='active'");
	FString Sort = "channel.id:desc";
	
	// Get memberships with raw settings
	PubnubClient->GetMembershipsRawAsync(UserID, OnGetMembershipsResponse, Include, 100, Filter, Sort);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnGetMembershipsRawResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Create a list of channel IDs to remove memberships from
	TArray<FString> ChannelsToRemove;
	ChannelsToRemove.Add("general-chat-channel");
	ChannelsToRemove.Add("trade-chat-channel");
	
	// Remove memberships for the user
	FString UserID = TEXT("Player_001");
	PubnubClient->RemoveMembershipsAsync(UserID, ChannelsToRemove);
}

// snippet.remove_memberships_with_result
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::RemoveMembershipsWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnPubnubRemoveMembershipsResponse OnRemoveMembershipsResponse;
	OnRemoveMembershipsResponse.BindDynamic(this, &ASample_Memberships::OnRemoveMembershipsResponse);
	
	// Create a list of channel IDs to remove memberships from
	TArray<FString> ChannelsToRemove;
	ChannelsToRemove.Add("guild-hall-channel");

	// Remove memberships and include extra data in the response
	FString UserID = TEXT("Player_002");
	FPubnubMembershipInclude Include = FPubnubMembershipInclude::FromValue(true);
	PubnubClient->RemoveMembershipsAsync(UserID, ChannelsToRemove, OnRemoveMembershipsResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnRemoveMembershipsResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubRemoveMembershipsResponseNative OnRemoveMembershipsResponse;
	OnRemoveMembershipsResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
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
	FString UserID = TEXT("Player_003");
	FPubnubMembershipInclude Include = FPubnubMembershipInclude::FromValue(true);
	PubnubClient->RemoveMembershipsAsync(UserID, ChannelsToRemove, OnRemoveMembershipsResponse, Include);
}

// snippet.remove_memberships_raw
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::RemoveMembershipsRawSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnPubnubRemoveMembershipsResponse OnRemoveMembershipsResponse;
	OnRemoveMembershipsResponse.BindDynamic(this, &ASample_Memberships::OnRemoveMembershipsRawResponse);
	
	// Create memberships data as a raw JSON string
	FString MembershipsJson = R"([{"channel": {"id": "arena-channel"}}, {"channel": {"id": "spectator-channel"}}])";
	
	// Remove memberships with raw JSON and includes
	FString UserID = TEXT("Player_004");
	FString Include = "custom,channel.custom";
	PubnubClient->RemoveMembershipsRawAsync(UserID, MembershipsJson, OnRemoveMembershipsResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnRemoveMembershipsRawResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

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
	PubnubClient->SetChannelMembersAsync("general-chat-channel", MembersToSet);
}

// snippet.set_channel_members_with_result
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::SetChannelMembersWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnPubnubSetChannelMembersResponse OnSetChannelMembersResponse;
	OnSetChannelMembersResponse.BindDynamic(this, &ASample_Memberships::OnSetChannelMembersResponse);
	
	// Create members data
	TArray<FPubnubChannelMemberInputData> MembersToSet;
	FPubnubChannelMemberInputData Member1;
	Member1.User = "User_003";
	Member1.Status = "pending";
	MembersToSet.Add(Member1);

	// Set members and include extra data in the response
	FPubnubMemberInclude Include = FPubnubMemberInclude::FromValue(true);
	PubnubClient->SetChannelMembersAsync("guild-hall-channel", MembersToSet, OnSetChannelMembersResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnSetChannelMembersResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubSetChannelMembersResponseNative OnSetChannelMembersResponse;
	OnSetChannelMembersResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
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
	PubnubClient->SetChannelMembersAsync("private-lounge-channel", MembersToSet, OnSetChannelMembersResponse, Include);
}

// snippet.set_channel_members_raw
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::SetChannelMembersRawSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnPubnubSetChannelMembersResponse OnSetChannelMembersResponse;
	OnSetChannelMembersResponse.BindDynamic(this, &ASample_Memberships::OnSetChannelMembersRawResponse);
	
	// Create members data as a raw JSON string
	FString MembersJson = R"([{"user": {"id": "User_005"}, "custom": {"rank": "diamond"}, "status": "active"}, {"user": {"id": "User_006"}, "type": "viewer"}])";
	
	// Set members with raw JSON and includes
	FString Include = "custom,status,type,user.custom";
	PubnubClient->SetChannelMembersRawAsync("arena-channel", MembersJson, OnSetChannelMembersResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnSetChannelMembersRawResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnPubnubGetChannelMembersResponse OnGetChannelMembersResponse;
	OnGetChannelMembersResponse.BindDynamic(this, &ASample_Memberships::OnGetChannelMembersResponse_Simple);

	// Get members for a channel
	PubnubClient->GetChannelMembersAsync("general-chat-channel", OnGetChannelMembersResponse);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnGetChannelMembersResponse_Simple(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnPubnubGetChannelMembersResponse OnGetChannelMembersResponse;
	OnGetChannelMembersResponse.BindDynamic(this, &ASample_Memberships::OnGetChannelMembersResponse_WithSettings);

	// Create settings
	FPubnubMemberInclude Include = FPubnubMemberInclude::FromValue(true);
	FString Filter = TEXT("status=='active'");
	FPubnubMemberSort Sort;
	Sort.MemberSort.Add(FPubnubMemberSingleSort{EPubnubMemberSortType::PMeST_UserID, true});
	
	// Get members with custom settings
	PubnubClient->GetChannelMembersAsync("guild-hall-channel", OnGetChannelMembersResponse, Include, 100, Filter, Sort);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnGetChannelMembersResponse_WithSettings(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubGetChannelMembersResponseNative OnGetChannelMembersResponse;
	OnGetChannelMembersResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
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
	PubnubClient->GetChannelMembersAsync("private-lounge-channel", OnGetChannelMembersResponse);
}

// snippet.get_channel_members_raw
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::GetChannelMembersRawSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnPubnubGetChannelMembersResponse OnGetChannelMembersResponse;
	OnGetChannelMembersResponse.BindDynamic(this, &ASample_Memberships::OnGetChannelMembersRawResponse);
	
	// Create settings as raw strings
	FString Include = "custom,user.custom";
	FString Filter = TEXT("status=='active'");
	FString Sort = "user.id:desc";
	
	// Get members with raw settings
	PubnubClient->GetChannelMembersRawAsync("arena-channel", OnGetChannelMembersResponse, Include, 100, Filter, Sort);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnGetChannelMembersRawResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Create a list of user IDs to remove from the channel
	TArray<FString> UsersToRemove;
	UsersToRemove.Add("User_001");
	UsersToRemove.Add("User_002");
	
	// Remove members from the channel
	PubnubClient->RemoveChannelMembersAsync("general-chat-channel", UsersToRemove);
}

// snippet.remove_channel_members_with_result
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::RemoveChannelMembersWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnPubnubRemoveChannelMembersResponse OnRemoveChannelMembersResponse;
	OnRemoveChannelMembersResponse.BindDynamic(this, &ASample_Memberships::OnRemoveChannelMembersResponse);
	
	// Create a list of user IDs to remove
	TArray<FString> UsersToRemove;
	UsersToRemove.Add("User_003");

	// Remove members and include extra data in the response
	FPubnubMemberInclude Include = FPubnubMemberInclude::FromValue(true);
	PubnubClient->RemoveChannelMembersAsync("guild-hall-channel", UsersToRemove, OnRemoveChannelMembersResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnRemoveChannelMembersResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubRemoveChannelMembersResponseNative OnRemoveChannelMembersResponse;
	OnRemoveChannelMembersResponse.BindLambda([](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
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
	PubnubClient->RemoveChannelMembersAsync("private-lounge-channel", UsersToRemove, OnRemoveChannelMembersResponse, Include);
}

// snippet.remove_channel_members_raw
// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::RemoveChannelMembersRawSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
	FOnPubnubRemoveChannelMembersResponse OnRemoveChannelMembersResponse;
	OnRemoveChannelMembersResponse.BindDynamic(this, &ASample_Memberships::OnRemoveChannelMembersRawResponse);
	
	// Create members data as a raw JSON string
	FString MembersJson = R"([{"user": {"id": "User_005"}}, {"user": {"id": "User_006"}}])";
	
	// Remove members with raw JSON and includes
	FString Include = "custom,user.custom";
	PubnubClient->RemoveChannelMembersRawAsync("arena-channel", MembersJson, OnRemoveChannelMembersResponse, Include);
}

// ACTION REQUIRED: Replace ASample_Memberships with name of your Actor class
void ASample_Memberships::OnRemoveChannelMembersRawResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
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

UPubnubClient* ASample_Memberships::GetPubnubClient()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	
	//Get default PubnubClient - created automatically if PluginSettings are set to do so
	UPubnubClient* PubnubClient = PubnubSubsystem->GetPubnubClient(0);
	
	PubnubClient->SetUserID(TEXT("player_001"));
	return PubnubClient;
}
