// Copyright 2025 PubNub Inc. All Rights Reserved.

// snippet.full_pubsub_example
// blueprint.7s012a54
#pragma once

#include "GameFramework/Actor.h"
#include "CoreMinimal.h"
#include "Sample_PubSubFull.generated.h"

// ACTION REQUIRED: Replace PUBNUBLIBRARYTESTS_API with your project's module API macro (usually ProjectName_API)
UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_PubSubFull : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|FullExamples|PubSub")
	void RunPubSubFullExample();

	UFUNCTION()
	void OnPubnubMessageReceived(FPubnubMessageData Message);

	UFUNCTION()
	void OnPublishResult(FPubnubOperationResult Result, FPubnubMessageData Message);
};

// snippet.end