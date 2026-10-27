// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Crypto/PubnubCryptorInterface.h"
#include "PubnubCryptoModule.generated.h"

struct pubnub_crypto_module;

/**
 * Holds the cryptors used for payload encryption.
 *
 * Assign an initialized module to FPubnubConfig::CryptoModule before UPubnubSubsystem::CreatePubnubClient.
 * C-Core reads the module only while creating the context, and it owns PNED header routing.
 *
 * The default cryptor encrypts outbound payloads. Additional cryptors are used only when decrypting
 * a payload whose 4-byte id matches them. At most 4 additional cryptors are accepted.
 *
 * Only UPubnubAesCryptor and UPubnubLegacyCryptor can be registered. Custom IPubnubCryptorInterface
 * objects are rejected until C-Core honors the cryptor's metadata length on encrypt.
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubCryptoModule : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Registers cryptors and builds the C-Core module.
	 * Each cryptor must already have its cipher key set.
	 * @return true when the C-Core module was created.
	 */
	UFUNCTION(BlueprintCallable, Category = "PubNub|Crypto")
	bool InitCryptoModule(const TScriptInterface<IPubnubCryptorInterface>& InDefaultCryptor, const TArray<TScriptInterface<IPubnubCryptorInterface>>& InAdditionalCryptors);

	virtual void BeginDestroy() override;

	/** Borrowed module passed to pubnub_create. Null until InitCryptoModule succeeds. */
	pubnub_crypto_module* GetCCoreModule() const { return CCoreModule; }

private:
	void DestroyCCoreModule();

	UPROPERTY()
	TScriptInterface<IPubnubCryptorInterface> DefaultCryptor;

	UPROPERTY()
	TArray<TScriptInterface<IPubnubCryptorInterface>> AdditionalCryptors;

	pubnub_crypto_module* CCoreModule = nullptr;
};
