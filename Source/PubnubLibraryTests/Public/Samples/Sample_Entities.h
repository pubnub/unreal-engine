// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubSubsystem.h"
#include "Entities/PubnubChannelEntity.h"
#include "Entities/PubnubChannelGroupEntity.h"
#include "Entities/PubnubChannelMetadataEntity.h"
#include "Entities/PubnubUserMetadataEntity.h"
#include "Entities/PubnubSubscription.h"

// snippet.end

#include "CoreMinimal.h"
#include "PubnubSampleBase.h"
#include "Sample_Entities.generated.h"


UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_Entities : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_Entities();

	
	/* ENTITY SAMPLE FUNCTIONS */
	
	// snippet.subscribe_with_channel_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Entities")
	void SubscribeWithChannelEntitySample();

	UFUNCTION()
	void OnMessageReceived_ChannelEntitySample(FPubnubMessageData Message);

	// snippet.subscribe_with_channel_group_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Entities")
	void SubscribeWithChannelGroupEntitySample();

	UFUNCTION()
	void OnMessageReceived_ChannelGroupEntitySample(FPubnubMessageData Message);

	// snippet.subscribe_with_channel_metadata_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Entities")
	void SubscribeWithChannelMetadataEntitySample();

	UFUNCTION()
	void OnObjectEvent_ChannelMetadataEntitySample(FPubnubMessageData Message);

	// snippet.subscribe_with_user_metadata_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Entities")
	void SubscribeWithUserMetadataEntitySample();

	UFUNCTION()
	void OnObjectEvent_UserMetadataEntitySample(FPubnubMessageData Message);

	// snippet.channel_entity_all_listeners
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Entities")
	void ChannelEntityAllListenersSample();

	UFUNCTION()
	void OnMessage_AllListenersSample(FPubnubMessageData Message);

	UFUNCTION()
	void OnSignal_AllListenersSample(FPubnubMessageData Message);

	UFUNCTION()
	void OnPresenceEvent_AllListenersSample(FPubnubMessageData Message);

	UFUNCTION()
	void OnObjectEvent_AllListenersSample(FPubnubMessageData Message);

	UFUNCTION()
	void OnMessageAction_AllListenersSample(FPubnubMessageData Message);

	UFUNCTION()
	void OnAnyEvent_AllListenersSample(FPubnubMessageData Message);

	// snippet.channel_entity_publish_message
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Entities")
	void ChannelEntityPublishMessageSample();

	UFUNCTION()
	void OnPublishResult_ChannelEntitySample(FPubnubOperationResult Result, FPubnubMessageData Message);

	// snippet.channel_group_entity_add_channel
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Entities")
	void ChannelGroupEntityAddChannelSample();

	UFUNCTION()
	void OnAddChannelResult_GroupEntitySample(FPubnubOperationResult Result);

	// snippet.channel_metadata_entity_set_metadata
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Entities")
	void ChannelMetadataEntitySetMetadataSample();

	UFUNCTION()
	void OnSetChannelMetadataResult_Sample(FPubnubOperationResult Result, FPubnubChannelData ChannelData);

	// snippet.user_metadata_entity_set_metadata
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Entities")
	void UserMetadataEntitySetMetadataSample();

	UFUNCTION()
	void OnSetUserMetadataResult_Sample(FPubnubOperationResult Result, FPubnubUserData UserData);

	// snippet.create_subscription_set_from_names
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Entities")
	void CreateSubscriptionSetFromNamesSample();

	UFUNCTION()
	void OnMessage_SubscriptionSetFromNamesSample(FPubnubMessageData Message);

	// snippet.create_subscription_set_from_entities
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Entities")
	void CreateSubscriptionSetFromEntitiesSample();

	UFUNCTION()
	void OnMessage_SubscriptionSetFromEntitiesSample(FPubnubMessageData Message);

	// snippet.subscription_set_add_remove_subscriptions
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Entities")
	void SubscriptionSetAddRemoveSubscriptionsSample();

	// snippet.subscription_set_merge_operations
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Entities")
	void SubscriptionSetMergeOperationsSample();

	
	// snippet.end
};
