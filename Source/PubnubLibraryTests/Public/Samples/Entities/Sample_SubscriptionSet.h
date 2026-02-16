// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubClient.h"
#include "Entities/PubnubChannelEntity.h"
#include "Entities/PubnubChannelGroupEntity.h"
#include "Entities/PubnubChannelMetadataEntity.h"
#include "Entities/PubnubUserMetadataEntity.h"
#include "Entities/PubnubSubscription.h"

// snippet.end

#include "CoreMinimal.h"
#include "Samples/PubnubSampleBase.h"
#include "Sample_SubscriptionSet.generated.h"


UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_SubscriptionSet : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_SubscriptionSet();

	
	/* SUBSCRIPTION SET SAMPLE FUNCTIONS */
	
	// snippet.create_subscription_set_from_names
	// blueprint.9d9mt0u6
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|SubscriptionSet")
	void CreateSubscriptionSetFromNamesSample();

	UFUNCTION()
	void OnMessage_SubscriptionSetFromNamesSample(FPubnubMessageData Message);

	// snippet.create_subscription_set_from_entities
	// blueprint.alzawuju
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|SubscriptionSet")
	void CreateSubscriptionSetFromEntitiesSample();

	UFUNCTION()
	void OnMessage_SubscriptionSetFromEntitiesSample(FPubnubMessageData Message);

	// snippet.subscription_set_add_remove_subscriptions
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|SubscriptionSet")
	void SubscriptionSetAddRemoveSubscriptionsSample();

	// snippet.subscription_set_merge_operations
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|SubscriptionSet")
	void SubscriptionSetMergeOperationsSample();

	
	// snippet.end
	
private:
	UPubnubClient* GetPubnubClient();
};
