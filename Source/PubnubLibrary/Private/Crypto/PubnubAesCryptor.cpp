// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "Crypto/PubnubAesCryptor.h"
#include "Crypto/PubnubNativeCryptor.h"
#include "PubnubSubsystem.h"
#include "PubNub.h"

void UPubnubAesCryptor::SetCipherKey(const FString& NewCipherKey)
{
	if (bSealed)
	{
		UE_LOG(PubnubLog, Warning, TEXT("AES cryptor cipher key is sealed by a crypto module and was not changed."));
		return;
	}

	if (CipherKey == NewCipherKey)
	{
		return;
	}

	DestroyNativeCryptor();
	CipherKey = NewCipherKey;
}

void UPubnubAesCryptor::BeginDestroy()
{
	DestroyNativeCryptor();
	Super::BeginDestroy();
}

pubnub_crypto_provider_t* UPubnubAesCryptor::GetNativeCryptor()
{
	if (NativeCryptor)
	{
		return NativeCryptor;
	}
	if (CipherKey.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("AES cryptor cipher key is empty. Call SetCipherKey first."));
		return nullptr;
	}

	const FTCHARToUTF8 KeyUtf8(*CipherKey);
	NativeCryptor = pubnub_cryptor_aes_cbc_create(KeyUtf8.Get(), nullptr);
	if (!NativeCryptor)
	{
		UE_LOG(PubnubLog, Error, TEXT("pubnub_cryptor_aes_cbc_create failed."));
	}
	return NativeCryptor;
}

void UPubnubAesCryptor::Seal()
{
	bSealed = true;
}

TArray<uint8> UPubnubAesCryptor::GetIdentifier_Implementation()
{
	if (pubnub_crypto_provider_t* Cryptor = GetNativeCryptor())
	{
		return PubnubNativeCryptor::CopyIdentifier(Cryptor);
	}
	return TArray<uint8>();
}

FPubnubEncryptedData UPubnubAesCryptor::Encrypt_Implementation(const FString& Data)
{
	FPubnubEncryptedData Encrypted;
	pubnub_crypto_provider_t* Cryptor = GetNativeCryptor();
	if (!Cryptor || !PubnubNativeCryptor::EncryptUtf8(Cryptor, Data, Encrypted))
	{
		return FPubnubEncryptedData();
	}
	return Encrypted;
}

FString UPubnubAesCryptor::Decrypt_Implementation(const FPubnubEncryptedData& Data)
{
	FString Plain;
	pubnub_crypto_provider_t* Cryptor = GetNativeCryptor();
	if (!Cryptor || !PubnubNativeCryptor::DecryptUtf8(Cryptor, Data, Plain))
	{
		return FString();
	}
	return Plain;
}

void UPubnubAesCryptor::DestroyNativeCryptor()
{
	if (NativeCryptor)
	{
		pubnub_cryptor_destroy(NativeCryptor);
		NativeCryptor = nullptr;
	}
}
