// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Crypto/PubnubCryptorInterface.h"
#include "PubnubAesCryptor.generated.h"

struct pubnub_crypto_provider;

/**
 * AES-256-CBC cryptor (identifier "ACRH").
 *
 * Wraps pubnub_cryptor_aes_cbc_create. The cipher key is hashed with SHA-256.
 * Each encrypt uses a random 16-byte IV, returned as metadata.
 * Call SetCipherKey before Encrypt, Decrypt, or UPubnubCryptoModule::InitCryptoModule.
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubAesCryptor : public UObject, public IPubnubCryptorInterface
{
	GENERATED_BODY()

public:
	/** Sets the cipher key. Required before encryption, decryption, or module init. */
	UFUNCTION(BlueprintCallable, Category = "PubNub|Crypto")
	void SetCipherKey(const FString& NewCipherKey);

	/** Returns the cipher key currently stored on this object. */
	UFUNCTION(BlueprintCallable, Category = "PubNub|Crypto")
	FString GetCipherKey() const { return CipherKey; }

	virtual void BeginDestroy() override;

	/** Borrowed C-Core cryptor. Null until the key has been applied. */
	pubnub_crypto_provider* GetNativeCryptor();

	/** Stops further key changes. Called when a crypto module takes this cryptor. */
	void Seal();

protected:
	UPROPERTY()
	FString CipherKey;

private:
	virtual TArray<uint8> GetIdentifier_Implementation() override;
	virtual FPubnubEncryptedData Encrypt_Implementation(const FString& Data) override;
	virtual FString Decrypt_Implementation(const FPubnubEncryptedData& Data) override;

	void DestroyNativeCryptor();

	pubnub_crypto_provider* NativeCryptor = nullptr;
	bool bSealed = false;
};
