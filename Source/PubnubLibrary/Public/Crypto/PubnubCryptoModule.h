// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubNub.h"
#include "PubnubCryptorInterface.h"
#include "UObject/NoExportTypes.h"
#include "PubnubCryptoModule.generated.h"


/**
 * Crypto module implementation for PubNub message encryption and decryption.
 * 
 * This class provides a modular system for managing multiple encryption/decryption methods
 * within a single PubNub instance. It implements the IPubnubCryptoProviderInterface to
 * provide a unified interface for encrypting/decrypting messages using different cryptors.
 * 
 * Key Features:
 * - Supports multiple encryption/decryption methods (cryptors)
 * - Automatic routing based on cryptor identifier in header
 * - Blueprint-compatible interface for Unreal Engine projects
 * - Supports legacy encryption methods for backward compatibility
 * 
 * Usage:
 * 1. Create an instance of UPubnubCryptoModule
 * 2. Initialize it with a default cryptor and optional additional cryptors
 * 3. Use the module with PubNub's crypto module for message encryption/decryption
 * 
 * @note In Blueprints, the cryptors must be cast to IPubnubCryptorInterface before being
 *       provided to InitCryptoModule. Don't plug them directly as UObjects, as this will
 *       compile, but empty Crypto object will be provided.
 * @note The module stores strong UPROPERTY references to the cryptors to prevent GC during use.
 * @note The module is designed to be used with PubNub's crypto module for automatic message
 *       encryption/decryption.
 */
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubCryptoModule : public UObject, public IPubnubCryptoProviderInterface
{
	GENERATED_BODY()

public:

	/** Initializes this crypto module with a default cryptor and optional additional cryptors.
	 *
	 * Purpose:
	 * - Registers the UE cryptors that this module will use to encrypt/decrypt data.
	 * - The default cryptor is used for outbound encryption.
	 * - On inbound decryption, the module routes to the correct cryptor by matching its 4‑byte identifier.
	 *
	 * Requirements:
	 * - Each cryptor must implement IPubnubCryptorInterface and return a stable 4‑byte identifier
	 *   from GetIdentifier() (first 4 bytes are used for routing).
	 * - Call this before SetCryptoModule, so it's fully initialized on time.
	 *
	 * Lifetime/GC:
	 * - The module stores strong UPROPERTY references to these cryptors to prevent GC during use.
	 *
	 * @param InDefaultCryptor: The primary cryptor used for encryption by default.
	 * @param InAdditionalCryptors: Optional list of alternative cryptors; used only when their identifier
	 *   appears in the incoming header (mixed/legacy + custom scenarios).
	 *
	 * @note: In Blueprints InAdditionalCryptors has to be Casted to IPubnubCryptoProviderInterface. Don't plug them directly as UObjects,
	 *		  as this will compile, but empty Crypto object will be provided.
	 */
	UFUNCTION(BlueprintCallable, Category = "PubNub|Crypto")
	void InitCryptoModule(const TScriptInterface<IPubnubCryptorInterface>& InDefaultCryptor, const TArray<TScriptInterface<IPubnubCryptorInterface>>& InAdditionalCryptors);

private:

	//Constants
	static constexpr char Sentinel[] = "PNED";
	static constexpr size_t SentinelLen = 4;
	static constexpr uint8  HeaderVer = 1;
	static constexpr size_t IdentLen  = 4;
	inline static const uint8 LegacyId[4] = {0,0,0,0};
	
	//IPubnubCryptoProviderInterface
	virtual FString ProviderEncrypt_Implementation(const FString& Data) override;
	virtual FString ProviderDecrypt_Implementation(const FString& Data) override;

	//Cryptors provided during Init
    UPROPERTY()
    TScriptInterface<IPubnubCryptorInterface> DefaultCryptor;
    UPROPERTY()
    TArray<TScriptInterface<IPubnubCryptorInterface>> AdditionalCryptors;
	
	
    // Utilities for header format compatible with C-Core provider
    bool IdEquals(const uint8 a[4], const uint8 b[4]);
    size_t ComputeHeaderSize(size_t metadataSize);
    size_t WriteHeader(uint8* dst, size_t headerSize, const uint8 ident[4], size_t metadataSize);
    bool ParseHeader(const uint8* buf, size_t bufSize, uint8 outIdent[4], size_t& outHeaderSize, size_t& outMetaSize, size_t& outMetaOffset);
	
	bool ProviderEncrypt_Internal(UPubnubCryptoModule* Self, const TArray<uint8>& PlainUTF8, TArray<uint8>& OutHeaderPlusCipher);
	bool ProviderDecrypt_Internal(UPubnubCryptoModule* Self, const TArray<uint8>& InHeaderPlusCipher, TArray<uint8>& OutPlainUTF8);
	
    // Select Unreal cryptor by 4-byte identifier. Returns null interface if not found
    TScriptInterface<IPubnubCryptorInterface> FindUECryptorById(const uint8 ident[4]) const;
};
