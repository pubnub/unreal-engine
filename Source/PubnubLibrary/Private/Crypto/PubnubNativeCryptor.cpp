// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "Crypto/PubnubNativeCryptor.h"
#include "FunctionLibraries/PubnubCryptoUtilities.h"
#include "PubnubSubsystem.h"
#include "PubNub.h"

namespace PubnubNativeCryptor
{
	namespace
	{
		bool IsLegacyIdentifier(const pubnub_crypto_provider_t* Cryptor)
		{
			return Cryptor
				&& Cryptor->identifier[0] == 0
				&& Cryptor->identifier[1] == 0
				&& Cryptor->identifier[2] == 0
				&& Cryptor->identifier[3] == 0;
		}
	}

	bool EncryptUtf8(pubnub_crypto_provider_t* Cryptor, const FString& PlainText, FPubnubEncryptedData& OutData)
	{
		OutData = FPubnubEncryptedData();
		if (!Cryptor || !Cryptor->encrypt || !Cryptor->encrypt_size)
		{
			return false;
		}

		FTCHARToUTF8 Utf8(*PlainText);
		const int32 PlainLen = Utf8.Length();
		const uint8 EmptyByte = 0;
		const uint8* PlainBytes = PlainLen > 0 ? reinterpret_cast<const uint8*>(Utf8.Get()) : &EmptyByte;

		const size_t CipherCap = Cryptor->encrypt_size(Cryptor, static_cast<size_t>(PlainLen));
		if (CipherCap == 0)
		{
			return false;
		}

		TArray<uint8> Cipher;
		Cipher.SetNumUninitialized(static_cast<int32>(CipherCap));

		uint8 MetadataBuf[16];
		const bool bLegacy = IsLegacyIdentifier(Cryptor);

		pubnub_encrypted_data_t Encrypted = {};
		Encrypted.data = Cipher.GetData();
		Encrypted.data_len = CipherCap;
		Encrypted.metadata = bLegacy ? nullptr : MetadataBuf;
		Encrypted.metadata_len = bLegacy ? 0 : sizeof(MetadataBuf);

		const pubnub_res_t Result = Cryptor->encrypt(Cryptor, PlainBytes, static_cast<size_t>(PlainLen), &Encrypted);
		if (Result != PUBNUB_OK || Encrypted.data_len == 0 || Encrypted.data_len > CipherCap)
		{
			UE_LOG(PubnubLog, Error, TEXT("C-Core cryptor encrypt failed (%d)."), static_cast<int32>(Result));
			return false;
		}

		OutData.EncryptedData = UPubnubCryptoUtilities::Base64Encode(Cipher.GetData(), Encrypted.data_len);
		if (!bLegacy && Encrypted.metadata_len > 0)
		{
			OutData.Metadata = UPubnubCryptoUtilities::Base64Encode(MetadataBuf, Encrypted.metadata_len);
		}
		return !OutData.EncryptedData.IsEmpty();
	}

	bool DecryptUtf8(pubnub_crypto_provider_t* Cryptor, const FPubnubEncryptedData& Encrypted, FString& OutPlainText)
	{
		OutPlainText.Reset();
		if (!Cryptor || !Cryptor->decrypt || Encrypted.EncryptedData.IsEmpty())
		{
			return false;
		}

		TArray<uint8> Cipher;
		if (!UPubnubCryptoUtilities::Base64Decode(Encrypted.EncryptedData, Cipher) || Cipher.Num() == 0)
		{
			return false;
		}

		TArray<uint8> Metadata;
		if (!Encrypted.Metadata.IsEmpty() && !UPubnubCryptoUtilities::Base64Decode(Encrypted.Metadata, Metadata))
		{
			return false;
		}

		TArray<uint8> Plain;
		Plain.SetNumUninitialized(Cipher.Num());
		size_t PlainLen = static_cast<size_t>(Plain.Num());

		pubnub_encrypted_data_t Input = {};
		Input.data = Cipher.GetData();
		Input.data_len = static_cast<size_t>(Cipher.Num());
		Input.metadata = Metadata.Num() > 0 ? Metadata.GetData() : nullptr;
		Input.metadata_len = static_cast<size_t>(Metadata.Num());

		const pubnub_res_t Result = Cryptor->decrypt(Cryptor, &Input, Plain.GetData(), &PlainLen);
		if (Result != PUBNUB_OK || PlainLen > static_cast<size_t>(Plain.Num()))
		{
			UE_LOG(PubnubLog, Error, TEXT("C-Core cryptor decrypt failed (%d)."), static_cast<int32>(Result));
			return false;
		}

		OutPlainText = UPubnubCryptoUtilities::ConvertBytesToString(Plain.GetData(), PlainLen);
		return true;
	}

	TArray<uint8> CopyIdentifier(const pubnub_crypto_provider_t* Cryptor)
	{
		TArray<uint8> Identifier;
		if (!Cryptor)
		{
			return Identifier;
		}
		Identifier.Append(Cryptor->identifier, 4);
		return Identifier;
	}
}
