// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubClient.h"

// snippet.end

#include "CoreMinimal.h"
#include "PubnubSampleBase.h"
#include "Sample_Groups.generated.h"


UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_Groups : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_Groups();

	
	/* SAMPLE FUNCTIONS */
	
	// snippet.add_channel_to_group
	// blueprint.3rwckkal
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Channel Groups")
	void AddChannelToGroupSample();

	// snippet.add_channel_to_group_with_result
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Channel Groups")
	void AddChannelToGroupWithResultSample();

	UFUNCTION()
	void OnAddChannelToGroupResponse(FPubnubOperationResult Result);

	// snippet.add_channel_to_group_with_result_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Channel Groups")
	void AddChannelToGroupWithResultLambdaSample();

	// snippet.list_channels_from_group
	// blueprint.saoacm0a
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Channel Groups")
	void ListChannelsFromGroupSample();
	
	UFUNCTION()
	void OnListChannelsFromGroupResponse(FPubnubOperationResult Result, const TArray<FString>& Channels);

	// snippet.list_channels_from_group_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Channel Groups")
	void ListChannelsFromGroupWithLambdaSample();

	// snippet.remove_channel_from_group
	// blueprint.stmbdo5-
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Channel Groups")
	void RemoveChannelFromGroupSample();

	// snippet.remove_channel_from_group_with_result
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Channel Groups")
	void RemoveChannelFromGroupWithResultSample();

	UFUNCTION()
	void OnRemoveChannelFromGroupResponse(FPubnubOperationResult Result);

	// snippet.remove_channel_from_group_with_result_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Channel Groups")
	void RemoveChannelFromGroupWithResultLambdaSample();
	
	// snippet.remove_channel_group
	// blueprint.ygpybt62
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Channel Groups")
	void RemoveChannelGroupSample();

	// snippet.remove_channel_group_with_result
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Channel Groups")
	void RemoveChannelGroupWithResultSample();

	UFUNCTION()
	void OnRemoveChannelGroupResponse(FPubnubOperationResult Result);

	// snippet.remove_channel_group_with_result_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Channel Groups")
	void RemoveChannelGroupWithResultLambdaSample();
	
	// snippet.end
	
private:
	UPubnubClient* GetPubnubClient();
};


