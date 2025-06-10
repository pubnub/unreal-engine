// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "Samples/PubnubSampleBase.h"


// Called when the game starts or when spawned
void APubnubSampleBase::BeginPlay()
{
	Super::BeginPlay();

	if(RunSamplesAutomatically)
	{
		RunSamples();
	}
}

void APubnubSampleBase::RunSamples()
{
	UE_LOG(LogTemp, Warning, TEXT("Starting samples for group: %s"), *SamplesName)
}