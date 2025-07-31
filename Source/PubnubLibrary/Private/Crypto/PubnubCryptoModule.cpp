// Fill out your copyright notice in the Description page of Project Settings.


#include "Crypto/PubnubCryptoModule.h"
#include "PubnubSubsystem.h"

void UPubnubCryptoModule::Initialize(FString InCipherKey)
{
	IsInitialized = true;
	CipherKey = InCipherKey;
	
	FTCHARToUTF8 Utf8Key(*CipherKey);

	KeyLength = Utf8Key.Length();
	PersistentKey = (uint8_t*)FMemory::Malloc(KeyLength + 1);
	FMemory::Memcpy(PersistentKey, Utf8Key.Get(), KeyLength);
	PersistentKey[KeyLength] = '\0'; // Null-terminate

	crypto_module = pubnub_crypto_aes_cbc_module_init(PersistentKey);
}

FString UPubnubCryptoModule::Encrypt(FString Data, FString CustomCipherKey)
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Encrypt]: Can't encrypt, CryptoModule is not initialized."));
		return "";
	}
	
	FTCHARToUTF8 Utf8(*Data);

	pubnub_bymebl_t Block;
	Block.ptr = (uint8_t*)Utf8.Get();
	Block.size = Utf8.Length();

	pubnub_bymebl_t Encrypted = crypto_module->encrypt(crypto_module, Block);

	if (!Encrypted.ptr) return "";

	// Base64-encode binary ciphertext for safe storage/transmission
	FString Base64 = FBase64::Encode((const uint8*)Encrypted.ptr, Encrypted.size);
	return Base64;
}

FString UPubnubCryptoModule::Decrypt(FString Data, FString CustomCipherKey)
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Encrypt]: Can't encrypt, CryptoModule is not initialized."));
		return "";
	}
	
	TArray<uint8> Binary;
	if (!FBase64::Decode(Data, Binary))
	{
		UE_LOG(PubnubLog, Error, TEXT("[Decrypt]: Failed to decode Base64 input."));
		return "";
	}

	pubnub_bymebl_t Block;
	Block.ptr = Binary.GetData();
	Block.size = Binary.Num();

	pubnub_bymebl_t Decrypted = crypto_module->decrypt(crypto_module, Block);

	if (!Decrypted.ptr) return "";

	FUTF8ToTCHAR Utf8((const ANSICHAR*)Decrypted.ptr, Decrypted.size);
	return FString(Utf8.Length(), Utf8.Get());
}

void UPubnubCryptoModule::SetCipherKey(FString InCipherKey)
{
	CipherKey = InCipherKey;
}
