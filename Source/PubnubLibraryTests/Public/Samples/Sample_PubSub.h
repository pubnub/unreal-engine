// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubSubsystem.h"

// snippet.end

#include "CoreMinimal.h"
#include "PubnubSampleBase.h"
#include "Sample_PubSub.generated.h"


UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_PubSub : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_PubSub();

	
	/* SAMPLE FUNCTIONS */
	
	// snippet.simple_publish
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|PubSub")
	void SimplePublishSample();

	// snippet.advance_publish
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|PubSub")
	void AdvancedPublishSample();

	// snippet.publish_with_ttl
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|PubSub")
	void PublishWithTTLSample();

	// snippet.simple_signal
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|PubSub")
	void SimpleSignalSample();

	// snippet.signal_with_settings
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|PubSub")
	void SignalWithSettingsSample();

	// snippet.subscribe
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|PubSub")
	void SubscribeSample();

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|PubSub")
	void OnMessageReceived_SubscribeSample(FPubnubMessageData Message);
	
	// snippet.subscribe_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|PubSub")
	void SubscribeWithLambdaSample();

	// snippet.subscribe_to_group
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|PubSub")
	void SubscribeToGroupSample();

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|PubSub")
	void OnMessageReceived_SubscribeToGroupSample(FPubnubMessageData Message);

	// snippet.subscribe_with_presence
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|PubSub")
	void SubscribeWithPresenceSample();

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|PubSub")
	void OnMessageReceived_SubscribeWithPresenceSample(FPubnubMessageData Message);

	// snippet.unsubscribe_from_channel
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|PubSub")
	void UnsubscribeFromChannelSample();
	
	// snippet.unsubscribe_from_channel
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|PubSub")
	void UnsubscribeFromGroupSample();

	// snippet.unsubscribe_from_all
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|PubSub")
	void UnsubscribeFromAllSample();
	
	// snippet.end
	
};


