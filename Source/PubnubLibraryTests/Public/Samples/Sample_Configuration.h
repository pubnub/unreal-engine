// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubSubsystem.h"

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
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void MessageListenerSample();

	UFUNCTION()
	void OnPubnubMessageReceived(FPubnubMessageData Message);

	// snippet.message_listener_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void MessageListenerLambdaSample();

	// snippet.error_listener
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void ErrorListenerSample();

	UFUNCTION()
	void OnPubnubErrorReceived(FString ErrorMessage, EPubnubErrorType ErrorType);

	// snippet.error_listener_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void ErrorListenerLambdaSample();

	// snippet.subscription_status_listener
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void SubscriptionStatusListenerSample();
	
	UFUNCTION()
	void OnSubscriptionStatusChanged(EPubnubSubscriptionStatus Status, FPubnubSubscriptionStatusData StatusData);

	// snippet.subscription_status_listener_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void SubscriptionStatusListenerLambdaSample();

	// snippet.callback_function
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void ListUsersFromChannelSample();

	UFUNCTION()
	void OnListUsersFromChannelResponse(FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper Data);

	// snippet.callback_function_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void ListUsersFromChannelLambdaSample();

	// snippet.init_with_config
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Configuration")
	void InitWithConfigSample();
	
	// snippet.end
};


