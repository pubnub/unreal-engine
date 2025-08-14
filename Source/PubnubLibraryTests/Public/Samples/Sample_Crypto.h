// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubSubsystem.h"

// snippet.end

#include "CoreMinimal.h"
#include "PubnubSampleBase.h"
#include "Sample_Crypto.generated.h"


UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_Crypto : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_Crypto();

	
	/* SAMPLE FUNCTIONS */
	
	// snippet.set_crypto_module
	/*UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Crypto")
	void SetCryptoModuleSample();

	// snippet.encrypt
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Crypto")
	void EncryptSample();

	// snippet.decrypt
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Crypto")
	void DecryptSample();*/
	
	// snippet.end
};


