// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubClient.h"

// snippet.end

#include "CoreMinimal.h"
#include "PubnubSampleBase.h"
#include "Sample_Configuration.generated.h"


UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_Configuration : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_Configuration();

	
	/* SAMPLE FUNCTIONS */
	
	// snippet.message_listener
	// blueprint.v3uoi8lo
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void MessageListenerSample();

	UFUNCTION()
	void OnPubnubMessageReceived(FPubnubMessageData Message);

	// snippet.message_listener_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void MessageListenerLambdaSample();

	// snippet.error_listener
	// blueprint.fb_9oopg
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void ErrorListenerSample();

	UFUNCTION()
	void OnPubnubErrorReceived(FString ErrorMessage, EPubnubErrorType ErrorType);

	// snippet.error_listener_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void ErrorListenerLambdaSample();

	// snippet.subscription_status_listener
	// blueprint.ld_4o3t8
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void SubscriptionStatusListenerSample();
	
	UFUNCTION()
	void OnSubscriptionStatusChanged(EPubnubSubscriptionStatus Status, FPubnubSubscriptionStatusData StatusData);

	// snippet.subscription_status_listener_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void SubscriptionStatusListenerLambdaSample();

	// snippet.callback_function
	// blueprint.w8o07e-l
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void ListUsersFromChannelSample();

	UFUNCTION()
	void OnListUsersFromChannelResponse(FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper Data);

	// snippet.callback_function_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void ListUsersFromChannelLambdaSample();

	// snippet.create_pubnub_client
	// blueprint._9byc9q1
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void CreatePubnubClient();
	
	// snippet.get_pubnub_client
	// blueprint.poqf2o09
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	UPubnubClient* GetPubnubClient();
	
	// snippet.destroy_pubnub_client
	// blueprint.256-9zfg
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void DestroyPubnubClient();

	// snippet.add_logger
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void AddLoggerSample();

	// snippet.set_log_level_runtime
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void SetLogLevelAtRuntimeSample();

	// snippet.set_log_level_configuration
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void SetLogLevelInConfigurationSample();

	// snippet.get_loggers
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void GetLoggersSample();

	// snippet.remove_logger
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void RemoveLoggerSample();

	// snippet.clear_loggers
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void ClearLoggersSample();
	
	// snippet.end
	
};


