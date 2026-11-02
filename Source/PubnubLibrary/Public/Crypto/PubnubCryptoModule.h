// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Crypto/PubnubCryptorInterface.h"
#include "PubnubCryptoModule.generated.h"

struct pubnub_crypto_module;

class UPubnubClient;

/**
 * Holds the cryptors used for payload encryption.
 *
 * Assign an initialized module to FPubnubConfig::CryptoModule before UPubnubSubsystem::CreatePubnubClient.
 * The client keeps that module for publish, subscribe, history, and files.
 * InitCryptoModule refuses to replace it while any client still holds it.
 *
 * The default cryptor encrypts outbound payloads. Additional cryptors are used only when decrypting
 * a payload whose 4-byte id matches them. At most 4 additional cryptors are accepted.
 *
 * Only UPubnubAesCryptor and UPubnubLegacyCryptor can be registered. Custom cryptors are rejected.
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubCryptoModule : public UObject
{
	GENERATED_BODY()

	friend class UPubnubClient;

public:
	/**
	 * Registers cryptors and prepares this module for client creation.
	 * Each cryptor must already have its cipher key set.
	 * Fails when a live client still holds this module.
	 * @return true when the module is ready.
	 */
	UFUNCTION(BlueprintCallable, Category = "PubNub|Crypto")
	bool InitCryptoModule(const TScriptInterface<IPubnubCryptorInterface>& InDefaultCryptor, const TArray<TScriptInterface<IPubnubCryptorInterface>>& InAdditionalCryptors);

	virtual void BeginDestroy() override;

	/** Null until InitCryptoModule succeeds. */
	pubnub_crypto_module* GetCCoreModule() const { return CCoreModule; }

private:
	void AddClientBorrow();
	void RemoveClientBorrow();
	void DestroyCCoreModule();

	UPROPERTY()
	TScriptInterface<IPubnubCryptorInterface> DefaultCryptor;

	UPROPERTY()
	TArray<TScriptInterface<IPubnubCryptorInterface>> AdditionalCryptors;

	pubnub_crypto_module* CCoreModule = nullptr;

	/** Live contexts that borrowed CCoreModule. pubnub_create does not copy it. */
	int32 ClientBorrowCount = 0;
};
