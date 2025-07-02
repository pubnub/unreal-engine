// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubSubsystem.h"

// snippet.end

#include "CoreMinimal.h"
#include "PubnubSampleBase.h"
#include "Sample_Memberships.generated.h"


UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_Memberships : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_Memberships();
	
	/* SAMPLE FUNCTIONS */
	
	// snippet.set_memberships
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetMembershipsSample();

	// snippet.set_memberships_with_result
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetMembershipsWithResultSample();

	UFUNCTION()
	void OnSetMembershipsResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);

	// snippet.set_memberships_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetMembershipsWithLambdaSample();

	// snippet.set_memberships_raw
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetMembershipsRawSample();
	
	UFUNCTION()
	void OnSetMembershipsRawResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);

	// snippet.get_memberships
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetMembershipsSample();
	
	UFUNCTION()
	void OnGetMembershipsResponse_Simple(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);
	
	// snippet.get_memberships_with_settings
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetMembershipsWithSettingsSample();

	UFUNCTION()
	void OnGetMembershipsResponse_WithSettings(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);

	// snippet.get_memberships_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetMembershipsWithLambdaSample();

	// snippet.get_memberships_raw
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetMembershipsRawSample();

	UFUNCTION()
	void OnGetMembershipsRawResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);

	// snippet.remove_memberships
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void RemoveMembershipsSample();

	// snippet.remove_memberships_with_result
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void RemoveMembershipsWithResultSample();

	UFUNCTION()
	void OnRemoveMembershipsResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);

	// snippet.remove_memberships_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void RemoveMembershipsWithLambdaSample();

	// snippet.remove_memberships_raw
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void RemoveMembershipsRawSample();
	
	UFUNCTION()
	void OnRemoveMembershipsRawResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);

	// snippet.set_channel_members
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetChannelMembersSample();

	// snippet.set_channel_members_with_result
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetChannelMembersWithResultSample();

	UFUNCTION()
	void OnSetChannelMembersResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);

	// snippet.set_channel_members_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetChannelMembersWithLambdaSample();

	// snippet.set_channel_members_raw
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetChannelMembersRawSample();
	
	UFUNCTION()
	void OnSetChannelMembersRawResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);

	// snippet.get_channel_members
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetChannelMembersSample();
	
	UFUNCTION()
	void OnGetChannelMembersResponse_Simple(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);
	
	// snippet.get_channel_members_with_settings
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetChannelMembersWithSettingsSample();

	UFUNCTION()
	void OnGetChannelMembersResponse_WithSettings(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);

	// snippet.get_channel_members_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetChannelMembersWithLambdaSample();

	// snippet.get_channel_members_raw
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetChannelMembersRawSample();

	UFUNCTION()
	void OnGetChannelMembersRawResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);

	// snippet.remove_channel_members
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void RemoveChannelMembersSample();

	// snippet.remove_channel_members_with_result
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void RemoveChannelMembersWithResultSample();

	UFUNCTION()
	void OnRemoveChannelMembersResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);

	// snippet.remove_channel_members_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void RemoveChannelMembersWithLambdaSample();

	// snippet.remove_channel_members_raw
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void RemoveChannelMembersRawSample();
	
	UFUNCTION()
	void OnRemoveChannelMembersRawResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);

	// snippet.end
};


