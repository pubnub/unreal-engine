// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubClient.h"
#include "Entities/PubnubChannelGroupEntity.h"
#include "Entities/PubnubSubscription.h"

// snippet.end

#include "CoreMinimal.h"
#include "Samples/PubnubSampleBase.h"
#include "Sample_ChannelGroupEntity.generated.h"


UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_ChannelGroupEntity : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_ChannelGroupEntity();

	
	/* CHANNEL GROUP ENTITY SAMPLE FUNCTIONS */
	
    // snippet.create_channel_group_entity
    UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelGroupEntity")
    void CreateChannelGroupEntitySample();

	// snippet.subscribe_with_channel_group_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelGroupEntity")
	void SubscribeWithChannelGroupEntitySample();

	UFUNCTION()
	void OnMessageReceived_ChannelGroupEntitySample(FPubnubMessageData Message);

	// snippet.add_channel_to_group_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelGroupEntity")
	void AddChannelToGroupSample();

	// snippet.add_channel_to_group_with_result_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelGroupEntity")
	void AddChannelToGroupWithResultSample();

	UFUNCTION()
	void OnAddChannelToGroupResponse(FPubnubOperationResult Result);

	// snippet.add_channel_to_group_with_result_lambda_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelGroupEntity")
	void AddChannelToGroupWithResultLambdaSample();

	// snippet.list_channels_from_group_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelGroupEntity")
	void ListChannelsFromGroupSample();
	
	UFUNCTION()
	void OnListChannelsFromGroupResponse(FPubnubOperationResult Result, const TArray<FString>& Channels);

	// snippet.list_channels_from_group_with_lambda_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelGroupEntity")
	void ListChannelsFromGroupWithLambdaSample();

	// snippet.remove_channel_from_group_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelGroupEntity")
	void RemoveChannelFromGroupSample();

	// snippet.remove_channel_from_group_with_result_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelGroupEntity")
	void RemoveChannelFromGroupWithResultSample();

	UFUNCTION()
	void OnRemoveChannelFromGroupResponse(FPubnubOperationResult Result);

	// snippet.remove_channel_from_group_with_result_lambda_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelGroupEntity")
	void RemoveChannelFromGroupWithResultLambdaSample();
	
	// snippet.remove_channel_group_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelGroupEntity")
	void RemoveChannelGroupSample();

	// snippet.remove_channel_group_with_result_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelGroupEntity")
	void RemoveChannelGroupWithResultSample();

	UFUNCTION()
	void OnRemoveChannelGroupResponse(FPubnubOperationResult Result);

	// snippet.remove_channel_group_with_result_lambda_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|ChannelGroupEntity")
	void RemoveChannelGroupWithResultLambdaSample();

	
	// snippet.end
	
private:
	UPubnubClient* GetPubnubClient();
};
