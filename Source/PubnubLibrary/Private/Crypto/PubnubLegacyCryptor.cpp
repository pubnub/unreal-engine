// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "Crypto/PubnubLegacyCryptor.h"
#include "Crypto/PubnubNativeCryptor.h"
#include "PubnubSubsystem.h"
#include "PubNub.h"

void UPubnubLegacyCryptor::SetCipherKey(const FString& NewCipherKey)
{
	if (bSealed)
	{
		UE_LOG(PubnubLog, Warning, TEXT("Legacy cryptor cipher key is sealed by a crypto module and was not changed."));
		return;
	}

	if (CipherKey == NewCipherKey)
	{
		return;
	}

	DestroyNativeCryptor();
	CipherKey = NewCipherKey;
}

void UPubnubLegacyCryptor::BeginDestroy()
{
	DestroyNativeCryptor();
	Super::BeginDestroy();
}

pubnub_crypto_provider_t* UPubnubLegacyCryptor::GetNativeCryptor()
{
	if (NativeCryptor && bNativeUsesRandomIV == UseRandomIV)
	{
		return NativeCryptor;
	}
	if (bSealed && NativeCryptor)
	{
		return NativeCryptor;
	}
	if (CipherKey.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Legacy cryptor cipher key is empty. Call SetCipherKey first."));
		return nullptr;
	}

	DestroyNativeCryptor();

	const FTCHARToUTF8 KeyUtf8(*CipherKey);
	NativeCryptor = pubnub_cryptor_legacy_create(KeyUtf8.Get(), UseRandomIV ? 1 : 0, nullptr);
	if (!NativeCryptor)
	{
		UE_LOG(PubnubLog, Error, TEXT("pubnub_cryptor_legacy_create failed."));
		return nullptr;
	}

	bNativeUsesRandomIV = UseRandomIV;
	return NativeCryptor;
}

void UPubnubLegacyCryptor::Seal()
{
	bSealed = true;
}

TArray<uint8> UPubnubLegacyCryptor::GetIdentifier_Implementation()
{
	if (pubnub_crypto_provider_t* Cryptor = GetNativeCryptor())
	{
		return PubnubNativeCryptor::CopyIdentifier(Cryptor);
	}
	return TArray<uint8>();
}

FPubnubEncryptedData UPubnubLegacyCryptor::Encrypt_Implementation(const FString& Data)
{
	FPubnubEncryptedData Encrypted;
	pubnub_crypto_provider_t* Cryptor = GetNativeCryptor();
	if (!Cryptor || !PubnubNativeCryptor::EncryptUtf8(Cryptor, Data, Encrypted))
	{
		return FPubnubEncryptedData();
	}
	return Encrypted;
}

FString UPubnubLegacyCryptor::Decrypt_Implementation(const FPubnubEncryptedData& Data)
{
	FString Plain;
	pubnub_crypto_provider_t* Cryptor = GetNativeCryptor();
	if (!Cryptor || !PubnubNativeCryptor::DecryptUtf8(Cryptor, Data, Plain))
	{
		return FString();
	}
	return Plain;
}

void UPubnubLegacyCryptor::DestroyNativeCryptor()
{
	if (NativeCryptor)
	{
		pubnub_cryptor_destroy(NativeCryptor);
		NativeCryptor = nullptr;
	}
}
