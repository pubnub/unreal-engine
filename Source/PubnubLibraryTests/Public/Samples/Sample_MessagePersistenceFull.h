// Copyright 2026 PubNub Inc. All Rights Reserved.

// snippet.full_message_persistence_example
#pragma once

#include "GameFramework/Actor.h"
#include "PubnubClient.h"

#include "CoreMinimal.h"
#include "Sample_MessagePersistenceFull.generated.h"

// ACTION REQUIRED: Replace PUBNUBLIBRARYTESTS_API with your project's module API macro (usually ProjectName_API)
UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_MessagePersistenceFull : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|FullExamples|Message Persistence")
	void RunMessagePersistenceFullExample();
	
	UFUNCTION()
	void OnPublishResponse(FPubnubOperationResult Result, FPubnubMessageData Message);

	UFUNCTION()
	void OnFetchHistoryResponse(FPubnubOperationResult Result, const TArray<FPubnubHistoryMessageData>& Messages);

	UFUNCTION()
	void OnMessageCountsResponse(FPubnubOperationResult Result, int MessageCounts);

	UFUNCTION()
	void OnDeleteMessagesResponse(FPubnubOperationResult Result);

private:
	UPROPERTY()
	UPubnubClient* PubnubClient = nullptr;

	FString TestChannel = TEXT("persistence-channel-full");

	void FetchHistory();
	void GetMessageCounts(const FString& Timetoken);
	void DeleteMessages();
	
};

// snippet.end