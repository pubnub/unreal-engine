// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PubnubSampleBase.generated.h"

/**
 * Base class for all PubNub UE SDK Samples
 */
UCLASS()
class PUBNUBLIBRARYTESTS_API APubnubSampleBase : public AActor
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	//Runs all samples.
	virtual void RunSamples();
	
	/** If true, all samples will be run on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pubnub|Samples")
	bool RunSamplesAutomatically = true;

	/** Name of the samples group, just for logging */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pubnub|Samples")
	FString SamplesName = "";
	

};
