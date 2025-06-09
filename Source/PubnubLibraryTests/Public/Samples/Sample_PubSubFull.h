// Copyright 2024 PubNub Inc. All Rights Reserved.

// snippet.full_pubsub_example
#pragma once

#include "PubnubSubsystem.h"

#include "CoreMinimal.h"
#include "PubnubSampleBase.h"
#include "Sample_PubSubFull.generated.h"

// Replace PUBNUBLIBRARYTESTS_API with your project's module API macro (usually ProjectName_API)
UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_PubSubFull : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|FullExamples|PubSub")
	void RunPubSubFullExample();

	UFUNCTION(BlueprintCallable, Category = "Pubnub|FullExamples|PubSub")
	void OnPubnubMessageReceived(FPubnubMessageData Message);
};

// snippet.end