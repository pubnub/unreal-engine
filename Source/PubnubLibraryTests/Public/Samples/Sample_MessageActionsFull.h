// Copyright 2024 PubNub Inc. All Rights Reserved.

// snippet.full_message_actions_example
#pragma once

#include "PubnubSubsystem.h"

#include "CoreMinimal.h"
#include "PubnubSampleBase.h"
#include "Sample_MessageActionsFull.generated.h"

// ACTION REQUIRED: Replace PUBNUBLIBRARYTESTS_API with your project's module API macro (usually ProjectName_API)
UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_MessageActionsFull : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|FullExamples|MessageActions")
	void RunMessageActionsFullExample();
	
private:
	UPROPERTY()
	TObjectPtr<UPubnubSubsystem> PubnubSubsystem = nullptr;

	FString TestChannel = "message-actions-full-channel";
	FString TestMessageTimetoken = "";
	FString LikeActionTimetoken = "";

	void AddFirstMessageAction();
	void AddSecondMessageAction();
	void GetAllMessageActions();
	void RemoveFirstMessageAction();

	UFUNCTION()
	void OnPublishResponse(FPubnubOperationResult Result, FPubnubMessageData Message);
	
	UFUNCTION()
	void OnAddFirstMessageActionResponse(FPubnubOperationResult Result, FPubnubMessageActionData MessageActionData);

	UFUNCTION()
	void OnAddSecondMessageActionResponse(FPubnubOperationResult Result, FPubnubMessageActionData MessageActionData);
	
	UFUNCTION()
	void OnGetMessageActionsResponse(FPubnubOperationResult Result, const TArray<FPubnubMessageActionData>& MessageActions);

	UFUNCTION()
	void OnRemoveMessageActionResponse(FPubnubOperationResult Result);
	
};

// snippet.end