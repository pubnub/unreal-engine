// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubClient.h"
#include "Entities/PubnubChannelMetadataEntity.h"
#include "Entities/PubnubSubscription.h"

// snippet.end

#include "CoreMinimal.h"
#include "Samples/PubnubSampleBase.h"
#include "Sample_ChannelMetadataEntity.generated.h"


UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_ChannelMetadataEntity : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_ChannelMetadataEntity();

	
	/* CHANNEL METADATA ENTITY SAMPLE FUNCTIONS */
	
    // snippet.create_channel_metadata_entity
    // blueprint.5pbi727k
    UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelMetadataEntity")
    void CreateChannelMetadataEntitySample();

	// snippet.subscribe_with_channel_metadata_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelMetadataEntity")
	void SubscribeWithChannelMetadataEntitySample();

	UFUNCTION()
	void OnObjectEvent_ChannelMetadataEntitySample(FPubnubMessageData Message);

	// snippet.channel_metadata_entity_set_metadata
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelMetadataEntity")
	void ChannelMetadataEntitySetMetadataSample();

	UFUNCTION()
	void OnSetChannelMetadataResult_Sample(FPubnubOperationResult Result, FPubnubChannelData ChannelData);

	// snippet.set_channel_metadata_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelMetadataEntity")
	void SetChannelMetadataSample();

	// snippet.set_channel_metadata_with_result_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelMetadataEntity")
	void SetChannelMetadataWithResultSample();

	UFUNCTION()
	void OnSetChannelMetadataResponse(FPubnubOperationResult Result, FPubnubChannelData ChannelData);

	// snippet.set_channel_metadata_with_lambda_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelMetadataEntity")
	void SetChannelMetadataWithLambdaSample();

	// snippet.get_channel_metadata_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelMetadataEntity")
	void GetChannelMetadataSample();

	UFUNCTION()
	void OnGetChannelMetadataResponse_Simple(FPubnubOperationResult Result, FPubnubChannelData ChannelData);

	// snippet.get_channel_metadata_with_lambda_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelMetadataEntity")
	void GetChannelMetadataWithLambdaSample();

	// snippet.remove_channel_metadata_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelMetadataEntity")
	void RemoveChannelMetadataSample();

	// snippet.remove_channel_metadata_with_result_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelMetadataEntity")
	void RemoveChannelMetadataWithResultSample();

	UFUNCTION()
	void OnRemoveChannelMetadataResponse(FPubnubOperationResult Result);

	// snippet.remove_channel_metadata_with_result_lambda_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelMetadataEntity")
	void RemoveChannelMetadataWithResultLambdaSample();

	
	// snippet.end
	
private:
	UPubnubClient* GetPubnubClient();
};
