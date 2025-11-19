// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubnubStructLibrary.h"
#include "PubnubEnumLibrary.h"
#include "PubnubClient.h"
#include "PubnubBaseEntity.generated.h"


class UPubnubSubscription;

/**
 * Base class for all PubNub entity types in Unreal Engine.
 * 
 * This class serves as the foundation for all PubNub entities, providing common functionality
 * such as subscription creation and entity identification. All PubNub entities inherit from
 * this class and are designed to work seamlessly within the Unreal Engine ecosystem with 
 * full Blueprint support and UObject lifecycle management.
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubBaseEntity : public UObject
{
	GENERATED_BODY()
	
	friend class UPubnubClient;

	
public:
	
	/**
	 * Creates a subscription object for this entity.
	 * 
	 * The returned subscription object is responsible for subscribing and unsubscribing
	 * to this entity, as well as adding and managing event listeners for receiving
	 * real-time messages, signals, and other PubNub events.
	 * 
	 * @note The subscription object lives independently from this entity and requires
	 * separate lifecycle management. The subscription will remain valid and functional
	 * even if this entity goes out of scope or is destroyed. Make sure to properly
	 * manage the subscription's lifetime according to your application's needs.
	 * 
	 * @param SubscribeSettings Optional settings for the subscription configuration.
	 * @return A new subscription object configured for this entity.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	UPubnubSubscription* CreateSubscription(FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());

	FString EntityID = "";
	EPubnubEntityType EntityType = EPubnubEntityType::PEnT_Channel;

protected:
	
	UPROPERTY()
	TObjectPtr<UPubnubClient> PubnubClient = nullptr;

	void InitEntity(UPubnubClient* InPubnubClient);
	
};
