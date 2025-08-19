// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubSubsystem.h"

// snippet.end

#include "CoreMinimal.h"
#include "PubnubSampleBase.h"
#include "Sample_MessageActions.generated.h"


UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_MessageActions : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_MessageActions();

	
	/* SAMPLE FUNCTIONS */
	
	// snippet.add_message_action
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Actions")
	void AddMessageActionSample();

	// snippet.add_message_action_with_result
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Actions")
	void AddMessageActionWithResultSample();

	UFUNCTION()
	void OnAddMessageActionResponse(FPubnubOperationResult Result, FPubnubMessageActionData MessageActionData);

	// snippet.add_message_action_with_result_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Actions")
	void AddMessageActionWithResultLambdaSample();

	// snippet.get_message_actions
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Actions")
	void GetMessageActionsSample();

	// snippet.get_message_actions_with_settings
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Actions")
	void GetMessageActionsWithSettingsSample();

	UFUNCTION()
	void OnGetMessageActionsResponse(FPubnubOperationResult Result, const TArray<FPubnubMessageActionData>& MessageActions);

	// snippet.get_message_actions_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Actions")
	void GetMessageActionsWithLambdaSample();

	// snippet.remove_message_action
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Actions")
	void RemoveMessageActionSample();

	// snippet.remove_message_action_with_result
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Actions")
	void RemoveMessageActionWithResultSample();

	UFUNCTION()
	void OnRemoveMessageActionResponse(FPubnubOperationResult Result);

	// snippet.remove_message_action_with_result_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Actions")
	void RemoveMessageActionWithResultLambdaSample();

	// snippet.end
};


