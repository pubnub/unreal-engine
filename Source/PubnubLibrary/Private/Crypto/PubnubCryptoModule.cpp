// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "Crypto/PubnubCryptoModule.h"
#include "Crypto/PubnubAesCryptor.h"
#include "Crypto/PubnubLegacyCryptor.h"
#include "PubnubSubsystem.h"
#include "PubNub.h"

namespace
{
	constexpr int32 MaxAdditionalCryptors = PUBNUB_CFG_CRYPTO_MAX_FALLBACK_CRYPTORS;

	pubnub_crypto_provider_t* MaterializeBuiltin(UObject* CryptorObject)
	{
		if (!CryptorObject)
		{
			return nullptr;
		}
		if (UPubnubAesCryptor* Aes = Cast<UPubnubAesCryptor>(CryptorObject))
		{
			return Aes->GetNativeCryptor();
		}
		if (UPubnubLegacyCryptor* Legacy = Cast<UPubnubLegacyCryptor>(CryptorObject))
		{
			return Legacy->GetNativeCryptor();
		}

		UE_LOG(PubnubLog, Error, TEXT("Crypto module accepts only UPubnubAesCryptor and UPubnubLegacyCryptor. Custom cryptors are not registered until C-Core uses the cryptor metadata length."));
		return nullptr;
	}
}

bool UPubnubCryptoModule::InitCryptoModule(const TScriptInterface<IPubnubCryptorInterface>& InDefaultCryptor, const TArray<TScriptInterface<IPubnubCryptorInterface>>& InAdditionalCryptors)
{
	pubnub_crypto_provider_t* DefaultNative = MaterializeBuiltin(InDefaultCryptor.GetObject());
	if (!DefaultNative)
	{
		UE_LOG(PubnubLog, Error, TEXT("InitCryptoModule failed. Default cryptor is missing or is not a built-in AES/Legacy cryptor with a cipher key."));
		return false;
	}

	pubnub_crypto_provider_t* Others[MaxAdditionalCryptors] = {};
	int32 OthersCount = 0;
	for (const TScriptInterface<IPubnubCryptorInterface>& Candidate : InAdditionalCryptors)
	{
		if (!Candidate.GetObject())
		{
			UE_LOG(PubnubLog, Warning, TEXT("InitCryptoModule skipped an empty additional cryptor."));
			continue;
		}
		if (OthersCount >= MaxAdditionalCryptors)
		{
			UE_LOG(PubnubLog, Error, TEXT("InitCryptoModule accepts at most %d additional cryptors."), MaxAdditionalCryptors);
			return false;
		}

		pubnub_crypto_provider_t* Native = MaterializeBuiltin(Candidate.GetObject());
		if (!Native)
		{
			UE_LOG(PubnubLog, Error, TEXT("InitCryptoModule failed on an additional cryptor."));
			return false;
		}
		Others[OthersCount++] = Native;
	}

	pubnub_crypto_provider_t** OthersArg = OthersCount > 0 ? Others : nullptr;
	pubnub_crypto_module_t* NewModule = pubnub_crypto_module_create(DefaultNative, OthersArg, static_cast<size_t>(OthersCount), nullptr);
	if (!NewModule)
	{
		UE_LOG(PubnubLog, Error, TEXT("pubnub_crypto_module_create failed."));
		return false;
	}

	if (CCoreModule)
	{
		UE_LOG(PubnubLog, Warning, TEXT("InitCryptoModule replaced an existing C-Core module. Create the client after this call."));
	}
	DestroyCCoreModule();
	CCoreModule = NewModule;

	auto SealCryptor = [](UObject* CryptorObject)
	{
		if (UPubnubAesCryptor* Aes = Cast<UPubnubAesCryptor>(CryptorObject))
		{
			Aes->Seal();
		}
		else if (UPubnubLegacyCryptor* Legacy = Cast<UPubnubLegacyCryptor>(CryptorObject))
		{
			Legacy->Seal();
		}
	};

	SealCryptor(InDefaultCryptor.GetObject());
	DefaultCryptor = InDefaultCryptor;
	AdditionalCryptors.Reset();
	for (const TScriptInterface<IPubnubCryptorInterface>& Candidate : InAdditionalCryptors)
	{
		if (Candidate.GetObject())
		{
			SealCryptor(Candidate.GetObject());
			AdditionalCryptors.Add(Candidate);
		}
	}
	return true;
}

void UPubnubCryptoModule::BeginDestroy()
{
	DestroyCCoreModule();
	Super::BeginDestroy();
}

void UPubnubCryptoModule::DestroyCCoreModule()
{
	if (CCoreModule)
	{
		pubnub_crypto_module_destroy(CCoreModule);
		CCoreModule = nullptr;
	}
}
