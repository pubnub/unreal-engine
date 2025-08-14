// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubnubCryptorInterface.h"
#include "UObject/NoExportTypes.h"
#include "PubnubLegacyCryptor.generated.h"


struct pubnub_crypto_provider_t;
struct FPubnubEncryptedDataInternal;

/**
 * 
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubLegacyCryptor : public UObject, public IPubnubCryptorInterface
{
	GENERATED_BODY()

public:

	/**
	 * When true, uses a random 16-byte IV (RAND_bytes/SecRandom) and prefixes it to the ciphertext.
	 * When false, uses the fixed IV "0123456789012345" and does NOT prefix it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Legacy")
	bool UseRandomIV = true;

	/** Sets the cipher key. It's required to SetCipherKey before any encryption/decryption operations. */
	UFUNCTION(BlueprintCallable, Category = "PubNub|Crypto")
	void SetCipherKey(const FString& NewCipherKey) { CipherKey = NewCipherKey; }

	/** Returns the current cipher key. */
	UFUNCTION(BlueprintCallable, Category = "PubNub|Crypto")
	FString GetCipherKey() const { return CipherKey; }

protected:
	UPROPERTY()
	FString CipherKey = "";

private:

	// IPubnubCryptoInterface
	virtual TArray<uint8> GetIdentifier_Implementation() override;
	virtual FPubnubEncryptedData Encrypt_Implementation(const FString& Data) override;
	virtual FString Decrypt_Implementation(const FPubnubEncryptedData& Data) override;

	
	bool EncryptDataLegacy(const TArray<uint8>& Plain, TArray<uint8>& OutCipher);
	bool DecryptDataLegacy(const TArray<uint8>& Cipher, TArray<uint8>& OutPlain);
	
	void MakeLegacyKeyAsciiHex32(TArray<uint8_t>& OutKeyAsciiHex32) const;
	static void GetFixedIV16(uint8 OutIV[16]);
	static bool Aes256CbcEncrypt(const TArray<uint8>& Key, const uint8* IV, const TArray<uint8>& In, TArray<uint8>& Out);
	static bool Aes256CbcDecrypt(const TArray<uint8>& Key, const uint8* IV, const uint8* Data, int32 Len, TArray<uint8>& Out);
};
