// Copyright 2025 PubNub Inc. All Rights Reserved.

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
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Crypto")
	void SetCryptoModuleSample();

	// snippet.set_crypto_module_with_legacy
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Crypto")
	void SetCryptoModuleWithLegacySample();
	
	// snippet.get_crypto_module
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Crypto")
	void GetCryptoModuleSample();

	// snippet.provider_encrypt
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Crypto")
	void ProviderEncryptSample();

	// snippet.provider_decrypt
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Crypto")
	void ProviderDecryptSample();

	// snippet.provider_encrypt_from_module
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Crypto")
	void ProviderEncryptUsingAlreadySetModuleSample();

	// snippet.provider_decrypt_from_module
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Crypto")
	void ProviderDecryptUsingAlreadySetModuleSample();
	
	// snippet.end
};


