// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubClient.h"

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
	// blueprint.w-qs5s7j
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void ListUsersFromChannelSample();

	UFUNCTION()
	void OnListUsersFromChannelResponse_Simple(FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper Data);

	// snippet.list_users_from_channel_with_settings
	// blueprint._b89ktgr
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void ListUsersFromChannelWithSettingsSample();

	UFUNCTION()
	void OnListUsersFromChannelResponse_WithSettings(FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper Data);

	// snippet.list_users_from_channel_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void ListUsersFromChannelWithLambdaSample();

	// snippet.list_user_subscribed_channels
	// blueprint.lvjys815
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void ListUserSubscribedChannelsSample();

	UFUNCTION()
	void OnListUserSubscribedChannelsResponse(FPubnubOperationResult Result, const TArray<FString>& Channels);
	
	// snippet.list_user_subscribed_channels_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void ListUserSubscribedChannelsWithLambdaSample();

	// snippet.set_state
	// blueprint.n0gg8yqg
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void SetStateSample();

	// snippet.set_state_with_settings
	// blueprint.js86cm8b
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void SetStateWithSettingsSample();

	// snippet.set_state_with_result
	// blueprint.8l84j8-9
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void SetStateWithResultSample();

	UFUNCTION()
	void OnSetStateResponse(FPubnubOperationResult Result);

	// snippet.set_state_with_result_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void SetStateWithResultLambdaSample();

	// snippet.get_state
	// blueprint.msg62_si
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void GetStateSample();

	UFUNCTION()
	void OnGetStateResponse_Simple(FPubnubOperationResult Result, FString StateResponse);
	
	// snippet.get_state_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Presence")
	void GetStateWithLambdaSample();
	
	// snippet.end
	
private:
	UPubnubClient* GetPubnubClient();
};


