// Copyright 2024 PubNub Inc. All Rights Reserved.

// snippet.full_app_context_example
#pragma once

#include "PubnubSubsystem.h"

#include "CoreMinimal.h"
#include "PubnubSampleBase.h"
#include "Sample_AppContextFull.generated.h"

// ACTION REQUIRED: Replace PUBNUBLIBRARYTESTS_API with your project's module API macro (usually ProjectName_API)
UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_AppContextFull : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|FullExamples|App Context")
	void RunAppContextFullExample();
	
private:

	UPROPERTY()
	UPubnubSubsystem* PubnubSubsystem = nullptr;

	FString ExampleUserID = "DragonSlayer_91";
	FString ExampleChannelID = "quest-the-dark-forest";

	UFUNCTION()
	void OnSetUserMetadataResponse(FPubnubOperationResult Result, FPubnubUserData UserData);

	UFUNCTION()
	void OnSetChannelMetadataResponse(FPubnubOperationResult Result, FPubnubChannelData ChannelData);

	UFUNCTION()
	void OnSetMembershipsResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);

	UFUNCTION()
	void OnGetMembershipsResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);

	UFUNCTION()
	void OnGetChannelMembersResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);
    
	UFUNCTION()
	void OnRemoveMembershipsResponse(FPubnubOperationResult Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);

	UFUNCTION()
	void OnRemoveUserMetadataResponse(FPubnubOperationResult Result);

	UFUNCTION()
	void OnRemoveChannelMetadataResponse(FPubnubOperationResult Result);
};

// snippet.end