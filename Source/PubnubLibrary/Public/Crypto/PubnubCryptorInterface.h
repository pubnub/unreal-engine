// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PubnubStructLibrary.h"
#include "PubnubCryptorInterface.generated.h"

/**
 * Cryptor used by UPubnubCryptoModule.
 *
 * Encrypt returns Base64 ciphertext and optional Base64 metadata (for AES-CBC, the 16-byte IV).
 * Decrypt reverses Encrypt. Return an empty string on failure.
 * GetIdentifier must return a stable 4-byte id. AES-CBC uses "ACRH". Legacy uses {0,0,0,0}.
 *
 * UPubnubCryptoModule currently accepts only UPubnubAesCryptor and UPubnubLegacyCryptor.
 * Custom implementations of this interface are kept source-compatible, but cannot be registered
 * until C-Core writes the cryptor's real metadata length into the PNED header.
 */
UINTERFACE(Blueprintable)
class PUBNUBLIBRARY_API UPubnubCryptorInterface : public UInterface
{
	GENERATED_BODY()
};

class PUBNUBLIBRARY_API IPubnubCryptorInterface
{
	GENERATED_BODY()

public:
	/** Stable 4-byte algorithm id. The first 4 bytes are used. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Pubnub|Crypto")
	TArray<uint8> GetIdentifier();

	/**
	 * Encrypt plaintext (UTF-8) to Base64 ciphertext.
	 * Metadata is Base64 and may be empty (legacy). AES-CBC returns a 16-byte IV here.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Pubnub|Crypto")
	FPubnubEncryptedData Encrypt(const FString& Data);

	/** Decrypt data produced by Encrypt. Empty string means failure. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Pubnub|Crypto")
	FString Decrypt(const FPubnubEncryptedData& Data);
};
