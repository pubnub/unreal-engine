// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubBaseEntity.h"
#include "Entities/PubnubSubscription.h"
#include "PubnubSubsystem.h"

UPubnubSubscription* UPubnubBaseEntity::CreateSubscription(FPubnubSubscribeSettings SubscribeSettings)
{
	UPubnubSubscription* Subscription = NewObject<UPubnubSubscription>(this);

	Subscription->InitSubscription(PubnubSubsystem, this, SubscribeSettings);

	return Subscription;
}

void UPubnubBaseEntity::InitEntity(UPubnubSubsystem* InPubnubSubsystem)
{
	if(!InPubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Init Entity failed, PubnubSubsystem is invalid"));
	}

	PubnubSubsystem = InPubnubSubsystem;
}
