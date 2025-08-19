// Copyright 2025 PubNub Inc. All Rights Reserved.

// snippet.full_crypto_example

#include "Samples/Sample_CryptoFull.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Crypto/PubnubAesCryptor.h"
#include "Crypto/PubnubCryptoModule.h"
#include "PubnubSubsystem.h"


void ASample_CryptoFull::BeginPlay()
{
	Super::BeginPlay();

	//Run the example on BeginPlay
	RunCryptoFullExample();
}

void ASample_CryptoFull::RunCryptoFullExample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	UE_LOG(LogTemp, Log, TEXT("Crypto example, User ID is set"));
	
	// Set Crypto Module With Aes Cryptor
	// Setting crypto module automatically encrypts all published messages and decrypts received messages
	// Messages fetched from history are alsodecrypted automatically
	UPubnubAesCryptor* AesCryptor = NewObject<UPubnubAesCryptor>(this);
	AesCryptor->SetCipherKey("enigma");
	UPubnubCryptoModule* CryptoModule = NewObject<UPubnubCryptoModule>(this);
	CryptoModule->InitCryptoModule(AesCryptor, {});
	PubnubSubsystem->SetCryptoModule(CryptoModule);

	UE_LOG(LogTemp, Log, TEXT("Crypto example, Crypto Module is set"));

	//Add Listener/Delegate that will broadcast whenever message is received on any subscribed channel or group
	PubnubSubsystem->OnMessageReceived.AddDynamic(this, &ASample_CryptoFull::OnPubnubMessageReceived);

	//Subscribe to the Channel
	FString Channel = TEXT("secret_guild_chat");
	PubnubSubsystem->SubscribeToChannel(Channel);

	// NOTE: Subscribing to a group or channel may take a few seconds to complete.
	// This sleep is used to simulate the waiting period in an actual application.
	FPlatformProcess::Sleep(3);
	
	UE_LOG(LogTemp, Log, TEXT("Crypto example, subscribed to channel: %s"), *Channel);
		
	//Bind delegate to the publish result
	FOnPublishMessageResponse OnPublishMessageResponse;
	OnPublishMessageResponse.BindDynamic(this, &ASample_CryptoFull::OnPublishResult);
	
	//Publish message to the subscribed channel - this message will be encrypted automatically because crypto module is set
	FString Message = R"({"event": "PowerUpUsed", "powerup": "Invisibility Cloak", "duration": 10})";
	PubnubSubsystem->PublishMessage(Channel, Message, OnPublishMessageResponse);

	// NOTE: Give some time to receive message before unsubscribing
	// This sleep is used only to simulate the waiting period in an actual application.
	FPlatformProcess::Sleep(3);
	
	//Unsubscribe from previously subscribed channel
	PubnubSubsystem->UnsubscribeFromChannel(Channel);

	UE_LOG(LogTemp, Log, TEXT("Crypto example, message published"));
}

void ASample_CryptoFull::OnPubnubMessageReceived(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("Crypto example, message received on Channel: %s, Message Content: %s"), *Message.Channel, *Message.Message);
}

void ASample_CryptoFull::OnPublishResult(FPubnubOperationResult Result, FPubnubMessageData Message)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Crypto example, failed to publish message. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Crypto example, message published successfully. Published message timetoken: %s"), *Message.Timetoken);
	}
}

// snippet.end