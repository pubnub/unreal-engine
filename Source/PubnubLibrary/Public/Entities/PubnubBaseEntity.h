// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubnubSubsystem.h"
#include "PubnubBaseEntity.generated.h"


class UPubnubSubscription;

/**
 *
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubBaseEntity : public UObject
{
	GENERATED_BODY()

	friend class UPubnubSubsystem;

	
public:
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	UPubnubSubscription* CreateSubscription(FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());

	FString EntityID = "";
	EPubnubEntityType EntityType = EPubnubEntityType::PEnT_Channel;

protected:
	UPROPERTY()
	UPubnubSubsystem* PubnubSubsystem = nullptr;

	void InitEntity(UPubnubSubsystem* InPubnubSubsystem);
	
};
