// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PubnubStructLibrary.h"
#include "PubnubCryptorInterface.generated.h"



/**
 * Interface for providing custom encryption/decryption to PubNub UE SDK.
 *
 * How it’s used:
 * - You implement this interface (in Blueprint or C++) to provide your own cryptor.
 * - The bridge calls Encrypt before publish and Decrypt on incoming messages.
 * - We ship AES and Legacy cryptors as examples; you can plug in additional ones.
 *
 * Requirements:
 * - The 4-byte Identifier must uniquely mark your algorithm and be stable.
 * - Encrypt must return Base64-encoded ciphertext and optional Base64 metadata (e.g., IV).
 * - Decrypt must reverse Encrypt using the same key/params.
 * - Keep the API contract: return empty string on decryption failure.
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
	/** Returns a stable 4-byte identifier for this cryptor.
	 *
	 * - Used by PubNub SDK to route decryption to the correct algorithm.
	 * - Must be exactly 4 bytes. Choose a unique ID to avoid collisions.
	 * - Examples: AES CBC used "ACRH"; Legacy uses {0,0,0,0}.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Pubnub|Crypto")
	TArray<uint8> GetIdentifier();

	/** Encrypts plaintext into Base64-encoded ciphertext with optional metadata.
	 *
	 * Input:
	 * - Data: plaintext (interpreted as UTF-8)
	 *
	 * Output:
	 * - FPubnubEncryptedData:
	 *   - EncryptedData: Base64 ciphertext (required)
	 *   - Metadata: Base64 metadata (optional; e.g., 16-byte IV). Leave empty if unused.
	 *
	 * Notes:
	 * - Must be compatible with your Decrypt implementation.
	 * - Keep metadata small; it may be embedded in the message header.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Pubnub|Crypto")
	FPubnubEncryptedData Encrypt(const FString& Data);

	/** Decrypts data produced by this cryptor back to plaintext.
	 *
	 * Input:
	 * - Data:
	 *   - EncryptedData: Base64 ciphertext (required)
	 *   - Metadata: Base64 metadata (optional; may be empty)
	 *
	 * Output:
	 * - Plaintext as FString (UTF-8). Return empty string on failure.
	 *
	 * Notes:
	 * - Must accept exactly what your Encrypt produces (same key/IV/padding).
	 * - The bridge treats an empty return as a decryption error.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Pubnub|Crypto")
	FString Decrypt(const FPubnubEncryptedData& Data);
};



/**
 * Advanced provider-level crypto interface for PubNub.
 *
 * Use this to override the entire encryption/decryption pipeline at the provider level.
 * NOT RECOMMENDED for most cases. Prefer implementing cryptors via IPubnubCryptorInterface
 * (AES/Legacy or your own) to retain compatibility with PubNub defaults and mixed modules.
 *
 * When you implement this interface, your code fully controls:
 * - How the data is encrypted/decrypted
 * - Whether/which header/metadata format is used
 * - Which (if any) cryptor IDs are honored
 *
 * Important:
 * - This bypasses the default (header-based) multi-cryptor routing.
 * - You are responsible for interoperability with other clients/modules.
 */
UINTERFACE(Blueprintable)
class PUBNUBLIBRARY_API UPubnubCryptoProviderInterface : public UInterface
{
	GENERATED_BODY()
};

class PUBNUBLIBRARY_API IPubnubCryptoProviderInterface
{
	GENERATED_BODY()

public:
	/**
	 * Encrypt the given plaintext at the provider level.
	 *
	 * Input:
	 * - Data: plaintext (treated as UTF-8)
	 *
	 * Output:
	 * - Return a Base64 string of the complete encrypted payload your provider expects to send.
	 *   If you want compatibility with PubNub’s header-based flow, return:
	 *     Base64( header + metadata(optional) + ciphertext )
	 *   Otherwise, you can return any format that your matching ProviderDecrypt understands.
	 *
	 * Notes:
	 * - Return an empty string on failure (the bridge will treat it as an error).
	 * - Prefer using IPubnubCryptorInterface for custom algorithms unless you intentionally
	 *   need to replace the whole provider behavior.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Pubnub|Crypto")
	FString ProviderEncrypt(const FString& Data);

	/**
	 * Decrypt the previously encrypted provider-level payload.
	 *
	 * Input:
	 * - Data: Base64-encoded encrypted payload (whatever your ProviderEncrypt produced).
	 *   If you follow PubNub’s header-based format, this is:
	 *     Base64( header + metadata(optional) + ciphertext )
	 *
	 * Output:
	 * - Plaintext as FString (UTF-8). Return an empty string on failure.
	 *
	 * Notes:
	 * - This is the inverse of ProviderEncrypt. Keep formats aligned.
	 * - Prefer IPubnubCryptorInterface for adding custom cryptors while keeping
	 *   default PubNub encryption and mixed-module support.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Pubnub|Crypto")
	FString ProviderDecrypt(const FString& Data);
};