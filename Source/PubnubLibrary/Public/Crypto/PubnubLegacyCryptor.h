// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Crypto/PubnubCryptorInterface.h"
#include "PubnubLegacyCryptor.generated.h"

struct pubnub_crypto_provider;

/**
 * Legacy AES-256-CBC cryptor (identifier {0,0,0,0}).
 *
 * Wraps pubnub_cryptor_legacy_create. The key is the first 32 hex characters of SHA-256(cipher key).
 * UseRandomIV true prepends a random IV to the ciphertext. False uses the static IV "0123456789012345".
 * Legacy ciphertext has no separate metadata field.
 * Set the cipher key and UseRandomIV before UPubnubCryptoModule::InitCryptoModule.
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubLegacyCryptor : public UObject, public IPubnubCryptorInterface
{
	GENERATED_BODY()

public:
	/**
	 * True: random 16-byte IV prefixed to the ciphertext.
	 * False: static IV "0123456789012345", not prefixed. Compatibility only.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pubnub|Crypto")
	bool UseRandomIV = true;

	/** Sets the cipher key. Required before encryption, decryption, or module init. */
	UFUNCTION(BlueprintCallable, Category = "PubNub|Crypto")
	void SetCipherKey(const FString& NewCipherKey);

	/** Returns the cipher key currently stored on this object. */
	UFUNCTION(BlueprintCallable, Category = "PubNub|Crypto")
	FString GetCipherKey() const { return CipherKey; }

	virtual void BeginDestroy() override;

	/** Borrowed C-Core cryptor. Null until the key has been applied. */
	pubnub_crypto_provider* GetNativeCryptor();

	/** Stops further key or IV-mode changes. Called when a crypto module takes this cryptor. */
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
	bool bNativeUsesRandomIV = true;
};
