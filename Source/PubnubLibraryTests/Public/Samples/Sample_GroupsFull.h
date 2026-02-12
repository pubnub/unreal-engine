// Copyright 2025 PubNub Inc. All Rights Reserved.

// snippet.full_groups_example
#pragma once

#include "PubnubClient.h"

#include "GameFramework/Actor.h"
#include "CoreMinimal.h"
#include "Sample_GroupsFull.generated.h"

// ACTION REQUIRED: Replace PUBNUBLIBRARYTESTS_API with your project's module API macro (usually ProjectName_API)
UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_GroupsFull : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|FullExamples|Channel Groups")
	void RunGroupsFullExample();

private:
	UPROPERTY()
	UPubnubClient* PubnubClient = nullptr;
	
	FString ChannelGroup = "all-chats";
	FString Channel = "global_chat";

	UFUNCTION()
	void OnAddChannelToGroupResponse(FPubnubOperationResult Result);

	UFUNCTION()
	void OnPubnubMessageReceived(FPubnubMessageData Message);

	UFUNCTION()
	void OnPublishResult(FPubnubOperationResult Result, FPubnubMessageData Message);
	
};

// snippet.end