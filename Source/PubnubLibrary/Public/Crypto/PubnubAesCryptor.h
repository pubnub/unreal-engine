// Copyright 2025 PubNub Inc. All Rights Reserved.

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
 * 
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
