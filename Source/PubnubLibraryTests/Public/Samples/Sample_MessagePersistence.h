// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubClient.h"

// snippet.end

#include "CoreMinimal.h"
#include "PubnubSampleBase.h"
#include "Sample_MessagePersistence.generated.h"


UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_MessagePersistence : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_MessagePersistence();

	
	/* SAMPLE FUNCTIONS */
	
	// snippet.fetch_history
	// blueprint.yy21j44m
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Persistence")
	void FetchHistorySample();
	
	UFUNCTION()
	void OnFetchHistoryResponse_Simple(FPubnubOperationResult Result, const TArray<FPubnubHistoryMessageData>& Messages);
	
	// snippet.fetch_history_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Persistence")
	void FetchHistoryWithLambdaSample();
	
	// snippet.fetch_history_with_time_window
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Persistence")
	void FetchHistoryWithTimeWindowSample();

	UFUNCTION()
	void OnFetchHistoryResponse_WithTimeWindow(FPubnubOperationResult Result, const TArray<FPubnubHistoryMessageData>& Messages);

	// snippet.fetch_history_with_all_includes
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Persistence")
	void FetchHistoryWithAllIncludesSample();

	UFUNCTION()
	void OnFetchHistoryResponse_WithAllIncludes(FPubnubOperationResult Result, const TArray<FPubnubHistoryMessageData>& Messages);
	
	// snippet.delete_messages
	// blueprint.6nhkck5i
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Persistence")
	void DeleteMessagesSample();

	// snippet.delete_messages_with_settings
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Persistence")
	void DeleteMessagesWithSettingsSample();

	// snippet.delete_messages_with_result
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Persistence")
	void DeleteMessagesWithResultSample();

	UFUNCTION()
	void OnDeleteMessagesResponse(FPubnubOperationResult Result);

	// snippet.delete_messages_with_result_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Persistence")
	void DeleteMessagesWithResultLambdaSample();

	// snippet.message_counts
	// blueprint.cw7px-_q
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Persistence")
	void MessageCountsSample();

	UFUNCTION()
	void OnMessageCountsResponse(FPubnubOperationResult Result, int MessageCounts);

	// snippet.message_counts_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Message Persistence")
	void MessageCountsWithLambdaSample();
	
	// snippet.end
	
private:
	UPubnubClient* GetPubnubClient();
};


