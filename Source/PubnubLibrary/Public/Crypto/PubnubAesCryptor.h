// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubnubCryptorInterface.h"
#include "UObject/NoExportTypes.h"
#include "PubnubAesCryptor.generated.h"


struct pubnub_crypto_provider_t;

struct FPubnubEncryptedDataInternal
{
	TArray<uint8> Data;
	TArray<uint8> Metadata; // Contains the IV
};

/**
 * AES-based cryptor implementation for PubNub message encryption and decryption.
 * 
 * This class provides AES (Advanced Encryption Standard) encryption/decryption functionality
 * for PubNub messages. It implements the IPubnubCryptorInterface to provide a standardized
 * encryption interface that can be used with PubNub's crypto module.
 * 
 * Features:
 * - AES encryption with 256-bit keys
 * - Random IV generation for each encryption operation
 * - SHA-256 key hashing for enhanced security
 * - Blueprint-compatible interface for Unreal Engine projects
 * 
 * Usage:
 * 1. Create an instance of UPubnubAesCryptor
 * 2. Set the cipher key using SetCipherKey()
 * 3. Use the cryptor with PubNub's crypto module for automatic message encryption/decryption
 * 
 * @note The cipher key must be set before performing any encryption/decryption operations.
 * @note Each encryption operation generates a unique random IV for security.
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubAesCryptor : public UObject, public IPubnubCryptorInterface
{
	GENERATED_BODY()

public:

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

	//Constants
	static constexpr int32 AesBlockSize = 16;
	static constexpr int32 AesIvSize    = 16;
	static constexpr int32 Sha256Len    = 32;

	static bool EncryptData(const FString& CipherKey, const TArray<uint8>& DataToEncrypt, FPubnubEncryptedDataInternal& OutResult);
	static bool DecryptData(const FString& CipherKey, const FPubnubEncryptedDataInternal& EncryptedData, TArray<uint8>& OutResult);
	
	static void GenerateRandomIV(uint8* IV, int32 Size);
	static bool HashKeySHA256(const FString& Key, uint8* OutHash);
    
	// Data conversion helpers
	FPubnubEncryptedData ConvertToInterface(const FPubnubEncryptedDataInternal& InternalData);
	FPubnubEncryptedDataInternal ConvertFromInterface(const FPubnubEncryptedData& InterfaceData);
	
};
