// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_Crypto.h"
// snippet.includes
#include "Crypto/PubnubCryptoModule.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

// snippet.end

/**
 * NOTE: Each sample is designed to be fully self-contained and portable. 
 * You can copy-paste any individual sample into a new project, and it should compile and run without errors 
 * — as long as you also include the necessary `#include` statements.
 *
 * To ensure independence, each sample retrieves the PubnubSubsystem and explicitly calls `SetUserID()` 
 * before performing any PubNub operations.
 *
 * In a real project, however, you only need to call `SetUserID()` once — typically during initialization 
 * (e.g., in GameInstance or at login) before making your first PubNub request.
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
	EncryptSample();
	DecryptSample();
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
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	UPubnubCryptoModule* CryptoModule = NewObject<UPubnubCryptoModule>(this);
	CryptoModule->Initialize("enigma");
	PubnubSubsystem->SetCryptoModule(CryptoModule);
}

// snippet.encrypt
// ACTION REQUIRED: Replace ASample_Crypto with name of your Actor class
void ASample_Crypto::EncryptSample()
{
	FString Message = TEXT("my_message");

	UPubnubCryptoModule* CryptoModule = NewObject<UPubnubCryptoModule>(this);
	CryptoModule->Initialize("enigma");

	FString EcyptedMessage = CryptoModule->Encrypt(Message);
}

// snippet.decrypt
// ACTION REQUIRED: Replace ASample_Crypto with name of your Actor class
void ASample_Crypto::DecryptSample()
{
	FString EncryptedMessage = TEXT("dsadsdasda");

	UPubnubCryptoModule* CryptoModule = NewObject<UPubnubCryptoModule>(this);
	CryptoModule->Initialize("enigma");

	FString DecryptedMessage = CryptoModule->Decrypt(EncryptedMessage);
}


// snippet.end
