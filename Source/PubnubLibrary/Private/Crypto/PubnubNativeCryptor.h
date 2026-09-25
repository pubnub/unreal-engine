// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubnubStructLibrary.h"

struct pubnub_crypto_provider;

namespace PubnubNativeCryptor
{
	/** Encrypts UTF-8 plaintext with a materialized C-Core cryptor. Metadata is empty for legacy. */
	bool EncryptUtf8(pubnub_crypto_provider* Cryptor, const FString& PlainText, FPubnubEncryptedData& OutData);

	/** Decrypts Base64 fields produced by EncryptUtf8. */
	bool DecryptUtf8(pubnub_crypto_provider* Cryptor, const FPubnubEncryptedData& Encrypted, FString& OutPlainText);

	TArray<uint8> CopyIdentifier(const pubnub_crypto_provider* Cryptor);
}
