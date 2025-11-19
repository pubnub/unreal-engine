// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubBaseEntity.h"
#include "Entities/PubnubSubscription.h"
#include "PubnubSubsystem.h"
#include "PubnubClient.h"

UPubnubSubscription* UPubnubBaseEntity::CreateSubscription(FPubnubSubscribeSettings SubscribeSettings)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot create subscription - PubnubClient is invalid. Entity not properly initialized."));
		return nullptr;
	}

	UPubnubSubscription* Subscription = NewObject<UPubnubSubscription>(this);

	Subscription->InitSubscription(PubnubClient, this, SubscribeSettings);

	return Subscription;
}

void UPubnubBaseEntity::InitEntity(UPubnubClient* InPubnubClient)
{
	if(!InPubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Init Entity failed, PubnubSubsystem is invalid"));
		return;
	}

	PubnubClient = InPubnubClient;
}
