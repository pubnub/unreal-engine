// Copyright 2024 PubNub Inc. All Rights Reserved.

// snippet.full_presence_example
#pragma once

#include "PubnubSubsystem.h"

#include "CoreMinimal.h"
#include "PubnubSampleBase.h"
#include "Sample_PresenceFull.generated.h"

// ACTION REQUIRED: Replace PUBNUBLIBRARYTESTS_API with your project's module API macro (usually ProjectName_API)
UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_PresenceFull : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|FullExamples|Presence")
	void RunPresenceFullExample();

	UFUNCTION()
	void OnSetStateResponse(FPubnubOperationResult Result);

	UFUNCTION()
	void OnListUsersFromChannelResponse(FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper Data);

	UFUNCTION()
	void OnGetStateResponse(FPubnubOperationResult Result, FString StateResponse);
	
};

// snippet.end