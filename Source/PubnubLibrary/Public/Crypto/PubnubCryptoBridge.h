// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubNub.h"
#include "Crypto/PubnubCryptorInterface.h"

#include "PubnubCryptoBridge.generated.h"

/**
 * This class is a Bridge between C-Core and Unreal SDK crypto module.
 * It creates and keeps alive pubnub_crypto_provider_t which is C-Core's crypto module.
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubCryptoBridge : public UObject
{
	GENERATED_BODY()
	
public:
	// Initialize bridge with a UE crypto module (provider), build a C-Core provider that calls UE module
	void InitCryptoBridge(const TScriptInterface<IPubnubCryptoProviderInterface>& InCryptoModule);
	// Clean up all allocated resources, has to be called if InitCryptoBridge were used - otherwise there will be memory leak
	void CleanUpCryptoBridge();

	TScriptInterface<IPubnubCryptoProviderInterface> GetUECryptoModule() {return  CryptoModule;}

	// Returns C-Core crypto provider
	pubnub_crypto_provider_t* GetProvider() const { return CCoreCryptoProvider; }

private:
	// These functions are bound to CCoreCryptoProvider encrypt and decrypt. They will be called inside C-Core.
	// They will call ProviderEncrypt / ProviderDecrypt from IPubnubCryptoProviderInterface
	static pubnub_bymebl_t CCoreProviderEncrypt(const pubnub_crypto_provider_t* provider, pubnub_bymebl_t to_encrypt);
	static pubnub_bymebl_t CCoreProviderDecrypt(const pubnub_crypto_provider_t* provider, pubnub_bymebl_t to_decrypt);

	// Unreal crypto module
	UPROPERTY()
	TScriptInterface<IPubnubCryptoProviderInterface> CryptoModule;

	// Provider (crypto module) from C-Core
	pubnub_crypto_provider_t* CCoreCryptoProvider = nullptr;
	
};

