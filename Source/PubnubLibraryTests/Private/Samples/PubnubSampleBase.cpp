// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "Samples/PubnubSampleBase.h"
#include "PubnubSubsystem.h"
#include "PubnubClient.h"


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

UPubnubClient* APubnubSampleBase::GetOrCreateSampleClient(UPubnubSubsystem* PubnubSubsystem)
{
	if(!PubnubSubsystem)
	{
		return nullptr;
	}

	if(UPubnubClient* ExistingClient = PubnubSubsystem->GetPubnubClient(0))
	{
		return ExistingClient;
	}

	FPubnubConfig PubnubConfig;
	PubnubConfig.PublishKey = TEXT("demo");
	PubnubConfig.SubscribeKey = TEXT("demo");
	PubnubConfig.UserID = TEXT("player_001");
	return PubnubSubsystem->CreatePubnubClient(PubnubConfig);
}
