// Copyright 2025 PubNub Inc. All Rights Reserved.

// snippet.full_crypto_example
#pragma once

#include "PubnubClient.h"

#include "CoreMinimal.h"
#include "Sample_CryptoFull.generated.h"

// ACTION REQUIRED: Replace PUBNUBLIBRARYTESTS_API with your project's module API macro (usually ProjectName_API)
UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_CryptoFull : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|FullExamples|Crypto")
	void RunCryptoFullExample();

	UFUNCTION()
	void OnPubnubMessageReceived(FPubnubMessageData Message);

	UFUNCTION()
	void OnPublishResult(FPubnubOperationResult Result, FPubnubMessageData Message);

private:
	UPROPERTY()
	UPubnubClient* PubnubClient = nullptr;
};

// snippet.end