// Copyright 2026 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_Crypto.h"
#include "Crypto/PubnubAesCryptor.h"
#include "Crypto/PubnubCryptoModule.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "PubnubSubsystem.h"

/**
 * NOTE: Each sample is designed to be fully self-contained and portable. 
 * You can copy-paste any individual sample into a new project, and it should compile and run without errors 
 * — as long as you also include the necessary `#include` statements.
 *
 * The samples assume that in Pubnub SDK settings sections in ProjectSettings following fields are set:
 * PublishKey and SubscribeKey have correct keys, InitializeAutomatically is true.
 */

// NOTE: Comments marked with `ACTION REQUIRED` indicate lines you must change/adjust.


//Internal function, don't copy it with the samples
void ASample_Crypto::RunSamples()
{
	Super::RunSamples();
	
	SetCryptoModuleSample();
	SetCryptoModuleWithLegacySample();
	ProviderEncryptSample();
	ProviderDecryptSample();
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	// Crypto is assigned on FPubnubConfig before the client is created.
	UPubnubAesCryptor* AesCryptor = NewObject<UPubnubAesCryptor>(this);
	AesCryptor->SetCipherKey("enigma");

	UPubnubCryptoModule* CryptoModule = NewObject<UPubnubCryptoModule>(this);
	CryptoModule->InitCryptoModule(AesCryptor, {});

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	FPubnubConfig Config;
	Config.PublishKey = TEXT("demo");
	Config.SubscribeKey = TEXT("demo");
	Config.UserID = TEXT("player_001");
	Config.CryptoModule = CryptoModule;
	PubnubClient = PubnubSubsystem->CreatePubnubClient(Config);
}

// snippet.set_crypto_module_with_legacy
#include "Crypto/PubnubLegacyCryptor.h"

// ACTION REQUIRED: Replace ASample_Crypto with name of your Actor class
void ASample_Crypto::SetCryptoModuleWithLegacySample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	// Crypto is assigned on FPubnubConfig before the client is created.
	UPubnubAesCryptor* AesCryptor = NewObject<UPubnubAesCryptor>(this);
	AesCryptor->SetCipherKey("enigma");

	// Legacy Cryptor is only needed for compatibility with SDKs that already use Legacy encryption.
	UPubnubLegacyCryptor* LegacyCryptor = NewObject<UPubnubLegacyCryptor>(this);
	LegacyCryptor->SetCipherKey("enigma");

	UPubnubCryptoModule* CryptoModule = NewObject<UPubnubCryptoModule>(this);
	CryptoModule->InitCryptoModule(AesCryptor, {LegacyCryptor});

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	FPubnubConfig Config;
	Config.PublishKey = TEXT("demo");
	Config.SubscribeKey = TEXT("demo");
	Config.UserID = TEXT("player_001");
	Config.CryptoModule = CryptoModule;
	PubnubClient = PubnubSubsystem->CreatePubnubClient(Config);
}

// snippet.provider_encrypt
// ACTION REQUIRED: Replace ASample_Crypto with name of your Actor class
void ASample_Crypto::ProviderEncryptSample()
{
	UPubnubAesCryptor* AesCryptor = NewObject<UPubnubAesCryptor>(this);
	AesCryptor->SetCipherKey("enigma");

	FString MessageToEncrypt = TEXT("Ready for action!");
	FPubnubEncryptedData EncryptedMessage = IPubnubCryptorInterface::Execute_Encrypt(AesCryptor, MessageToEncrypt);
	UE_LOG(LogTemp, Log, TEXT("Encrypted data: %s"), *EncryptedMessage.EncryptedData);
}

// snippet.provider_decrypt
// ACTION REQUIRED: Replace ASample_Crypto with name of your Actor class
void ASample_Crypto::ProviderDecryptSample()
{
	UPubnubAesCryptor* AesCryptor = NewObject<UPubnubAesCryptor>(this);
	AesCryptor->SetCipherKey("enigma");

	FPubnubEncryptedData EncryptedMessage;
	EncryptedMessage.EncryptedData = TEXT("UE5FRAFBQ1JIEAiPzR+6d0U+p/7iTcrvsBuoiJEjvqP90rLD8iC1NKLr7AQJFUv7NiI1pIRZKmtFWQ==");
	FString DecryptedMessage = IPubnubCryptorInterface::Execute_Decrypt(AesCryptor, EncryptedMessage);
	UE_LOG(LogTemp, Log, TEXT("Decrypted data: %s"), *DecryptedMessage);
}

// snippet.end

UPubnubClient* ASample_Crypto::GetPubnubClient()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	
	//Get default PubnubClient - created automatically if PluginSettings are set to do so
	UPubnubClient* PubnubClient = PubnubSubsystem->GetPubnubClient(0);
	
	PubnubClient->SetUserID(TEXT("player_001"));
	return PubnubClient;
}
