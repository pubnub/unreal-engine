// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_Crypto.h"
// snippet.includes
#include "Crypto/PubnubAesCryptor.h"
#include "Crypto/PubnubCryptoModule.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

// snippet.end

/**
 * NOTE: Each sample is designed to be fully self-contained and portable. 
 * You can copy-paste any individual sample into a new project, and it should compile and run without errors 
 * — as long as you also include the necessary `#include` statements.
 *
 * To ensure independence, each sample retrieves the PubnubSubsystem before performing any PubNub operations.
 * 
 * The samples assume that in Pubnub SDK settings sections in ProjectSettings following fields are set:
 * PublishKey and SubscribeKey have correct keys, InitializeAutomatically is true.
 */

// NOTE: Comments marked with `ACTION REQUIRED` indicate lines you must change.


//Internal function, don't copy it with the samples
void ASample_Crypto::RunSamples()
{
	Super::RunSamples();
	
	SetCryptoModuleSample();
	SetCryptoModuleWithLegacySample();
	GetCryptoModuleSample();
	EncryptSample();
	DecryptSample();
	EncryptUsingAlreadySetModuleSample();
	DecryptUsingAlreadySetModuleSample();
}
//Internal function, don't copy it with the samples
ASample_Crypto::ASample_Crypto()
{
	SamplesName = "Crypto";
	
}


/* SAMPLE FUNCTIONS */

// snippet.set_crypto_module
// ACTION REQUIRED: Replace ASample_Crypto with name of your Actor class
void ASample_Crypto::SetCryptoModuleSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Create Aes Cryptor - our main encryption algorithm
	UPubnubAesCryptor* AesCryptor = NewObject<UPubnubAesCryptor>(this);
	AesCryptor->SetCipherKey("enigma");

	// Create CryptoModule and Init it with Aes Cryptor
	UPubnubCryptoModule* CryptoModule = NewObject<UPubnubCryptoModule>(this);
	CryptoModule->InitCryptoModule(AesCryptor, {});

	// Set CryptoModule to Pubnub Subsystem
	PubnubSubsystem->SetCryptoModule(CryptoModule);
}

// snippet.set_crypto_module_with_legacy
// ACTION REQUIRED: Replace ASample_Crypto with name of your Actor class

#include "Crypto/PubnubLegacyCryptor.h"

void ASample_Crypto::SetCryptoModuleWithLegacySample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Create Aes Cryptor - our main encryption algorithm
	UPubnubAesCryptor* AesCryptor = NewObject<UPubnubAesCryptor>(this);
	AesCryptor->SetCipherKey("enigma");

	// Create Legacy Cryptor
	// Legacy Cryptor is only needed if you need compatibility with other PubNub SDKs that already use Legacy encryption
	UPubnubLegacyCryptor* LegacyCryptor = NewObject<UPubnubLegacyCryptor>(this);
	AesCryptor->SetCipherKey("enigma");

	// Create CryptoModule and Init it with Aes Cryptor as the default Cryptor and Legacy Cryptor as an additional one
	UPubnubCryptoModule* CryptoModule = NewObject<UPubnubCryptoModule>(this);
	CryptoModule->InitCryptoModule(AesCryptor, {LegacyCryptor});

	// Set CryptoModule to Pubnub Subsystem
	PubnubSubsystem->SetCryptoModule(CryptoModule);
}

// snippet.get_crypto_module
void ASample_Crypto::GetCryptoModuleSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Get Crypto Module - assuming CryptoModule was already set
	TScriptInterface<IPubnubCryptoProviderInterface> CryptoModuleInterface = PubnubSubsystem->GetCryptoModule();

	//Get Crypto Module as Object
	UObject* CryptoModuleObject = CryptoModuleInterface.GetObject();

	// CryptoModule needs to be validated before use
	if(CryptoModuleObject)
	{
		//Use your CryptoModule
	}
}


// snippet.provider_encrypt
// ACTION REQUIRED: Replace ASample_Crypto with name of your Actor class
void ASample_Crypto::ProviderEncryptSample()
{
	// Create Aes Cryptor - our main encryption algorithm
	UPubnubAesCryptor* AesCryptor = NewObject<UPubnubAesCryptor>(this);
	AesCryptor->SetCipherKey("enigma");

	// Create CryptoModule and Init it with Aes Cryptor
	UPubnubCryptoModule* CryptoModule = NewObject<UPubnubCryptoModule>(this);
	CryptoModule->InitCryptoModule(AesCryptor, {});

	FString MessageToEncrypt = TEXT("Ready for action!");
	FString EncryptedMessage = IPubnubCryptoProviderInterface::Execute_ProviderEncrypt(CryptoModule, MessageToEncrypt);
}

// snippet.provider_decrypt
// ACTION REQUIRED: Replace ASample_Crypto with name of your Actor class
void ASample_Crypto::ProviderDecryptSample()
{
	// Create Aes Cryptor - our main encryption algorithm
	UPubnubAesCryptor* AesCryptor = NewObject<UPubnubAesCryptor>(this);
	AesCryptor->SetCipherKey("enigma");

	// Create CryptoModule and Init it with Aes Cryptor
	UPubnubCryptoModule* CryptoModule = NewObject<UPubnubCryptoModule>(this);
	CryptoModule->InitCryptoModule(AesCryptor, {});
	
	FString EncryptedMessage = TEXT("UE5FRAFBQ1JIEAiPzR+6d0U+p/7iTcrvsBuoiJEjvqP90rLD8iC1NKLr7AQJFUv7NiI1pIRZKmtFWQ==");
	FString DecryptedMessage = IPubnubCryptoProviderInterface::Execute_ProviderDecrypt(CryptoModule, EncryptedMessage);
}

// snippet.provider_encrypt_from_module
// ACTION REQUIRED: Replace ASample_Crypto with name of your Actor class
void ASample_Crypto::ProviderEncryptUsingAlreadySetModuleSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Get Crypto Module - assuming CryptoModule was already set
	TScriptInterface<IPubnubCryptoProviderInterface> CryptoModule = PubnubSubsystem->GetCryptoModule();

	UObject* CryptoModuleObject = CryptoModule.GetObject();

	if(CryptoModuleObject)
	{
		FString MessageToEncrypt = TEXT("Ready for action!");
		FString EncryptedMessage = IPubnubCryptoProviderInterface::Execute_ProviderEncrypt(CryptoModuleObject, MessageToEncrypt);
	}
}

// snippet.provider_decrypt_from_module
// ACTION REQUIRED: Replace ASample_Crypto with name of your Actor class
void ASample_Crypto::ProviderDecryptUsingAlreadySetModuleSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Get Crypto Module - assuming CryptoModule was already set
	TScriptInterface<IPubnubCryptoProviderInterface> CryptoModuleInterface = PubnubSubsystem->GetCryptoModule();

	UObject* CryptoModuleObject = CryptoModuleInterface.GetObject();

	if(CryptoModuleObject)
	{
		FString EncryptedMessage = TEXT("UE5FRAFBQ1JIEAiPzR+6d0U+p/7iTcrvsBuoiJEjvqP90rLD8iC1NKLr7AQJFUv7NiI1pIRZKmtFWQ==");
		FString DecryptedMessage = IPubnubCryptoProviderInterface::Execute_ProviderDecrypt(CryptoModuleObject, EncryptedMessage);
	}
}

// snippet.end
