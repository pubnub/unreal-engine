// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubClient.h"
#include "Entities/PubnubChannelEntity.h"
#include "Entities/PubnubSubscription.h"

// snippet.end

#include "CoreMinimal.h"
#include "Samples/PubnubSampleBase.h"
#include "Sample_ChannelEntity.generated.h"

UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_ChannelEntity : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_ChannelEntity();

	
	/* CHANNEL ENTITY SAMPLE FUNCTIONS */

    // snippet.create_channel_entity
    // blueprint.e1-eg4ch
    UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelEntity")
    void CreateChannelEntitySample();

	// snippet.publish_simple_entity
	// blueprint.q927jwux
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelEntity")
	void PublishSimpleSample();

	// snippet.publish_with_settings_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelEntity")
	void PublishWithSettingsSample();

	// snippet.publish_with_result_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelEntity")
	void PublishWithResultSample();
	
	UFUNCTION()
	void PublishedMessageResponse(FPubnubOperationResult Result, FPubnubMessageData Message);

	// snippet.publish_with_result_lambda_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelEntity")
	void PublishWithResultLambdaSample();

	// snippet.simple_signal_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelEntity")
	void SimpleSignalSample();

	// snippet.signal_with_settings_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelEntity")
	void SignalWithSettingsSample();

	// snippet.signal_with_result_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelEntity")
	void SignalWithResultSample();

	UFUNCTION()
	void SignalMessageResponse(FPubnubOperationResult Result, FPubnubMessageData Message);

	// snippet.signal_with_result_lambda_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelEntity")
	void SignalWithResultLambdaSample();

	// snippet.list_users_from_channel_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelEntity")
	void ListUsersFromChannelSample();

	UFUNCTION()
	void OnListUsersFromChannelResponse_Simple(FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper Data);

	// snippet.list_users_from_channel_with_settings_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelEntity")
	void ListUsersFromChannelWithSettingsSample();

	UFUNCTION()
	void OnListUsersFromChannelResponse_WithSettings(FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper Data);

	// snippet.list_users_from_channel_with_lambda_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelEntity")
	void ListUsersFromChannelWithLambdaSample();

	// snippet.subscribe_with_channel_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelEntity")
	void SubscribeWithChannelEntitySample();

	UFUNCTION()
	void OnMessageReceived_ChannelEntitySample(FPubnubMessageData Message);

	// snippet.channel_entity_all_listeners
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelEntity")
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
	// blueprint.86kccdrk
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelEntity")
	void ChannelEntityPublishMessageSample();

	UFUNCTION()
	void OnPublishResult_ChannelEntitySample(FPubnubOperationResult Result, FPubnubMessageData Message);

	
	// snippet.end
	
private:
	UPubnubClient* GetPubnubClient();
};
