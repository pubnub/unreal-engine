// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubSubsystem.h"

// snippet.end

#include "CoreMinimal.h"
#include "PubnubSampleBase.h"
#include "Sample_Presence.generated.h"


UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_Presence : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_Presence();

	
	/* SAMPLE FUNCTIONS */
	
	// snippet.list_users_from_channel
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void ListUsersFromChannelSample();

	UFUNCTION()
	void OnListUsersFromChannelResponse_Simple(FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper Data);

	// snippet.list_users_from_channel_with_settings
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void ListUsersFromChannelWithSettingsSample();

	UFUNCTION()
	void OnListUsersFromChannelResponse_WithSettings(FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper Data);

	// snippet.list_users_from_channel_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void ListUsersFromChannelWithLambdaSample();

	// snippet.list_user_subscribed_channels
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void ListUserSubscribedChannelsSample();

	UFUNCTION()
	void OnListUserSubscribedChannelsResponse(FPubnubOperationResult Result, const TArray<FString>& Channels);
	
	// snippet.list_user_subscribed_channels_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void ListUserSubscribedChannelsWithLambdaSample();

	// snippet.set_state
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void SetStateSample();

	// snippet.set_state_with_settings
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void SetStateWithSettingsSample();

	// snippet.set_state_with_result
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void SetStateWithResultSample();

	UFUNCTION()
	void OnSetStateResponse(FPubnubOperationResult Result);

	// snippet.set_state_with_result_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void SetStateWithResultLambdaSample();

	// snippet.get_state
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void GetStateSample();

	UFUNCTION()
	void OnGetStateResponse_Simple(FPubnubOperationResult Result, FString StateResponse);
	
	// snippet.get_state_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void GetStateWithLambdaSample();
	
	// snippet.end
};


