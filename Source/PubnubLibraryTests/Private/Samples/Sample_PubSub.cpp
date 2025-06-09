// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_PubSub.h"
// snippet.includes
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
 */


//Internal function, don't copy it with the samples
void ASample_PubSub::RunSamples()
{
	Super::RunSamples();
	
	SimplePublishSample();
	AdvancedPublishSample();
	PublishWithTTLSample();
	SimpleSignalSample();
	SignalWithSettingsSample();
	SubscribeSample();
	SubscribeToGroupSample();
	SubscribeWitPresenceSample();
}
//Internal function, don't copy it with the samples
ASample_PubSub::ASample_PubSub()
{
	SamplesName = "PubSub";
}


/* SAMPLE FUNCTIONS */

// snippet.simple_publish
void ASample_PubSub::SimplePublishSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Publish simple text message to provided channel
	FString Channel = TEXT("global_chat");
	FString SimpleMessage = TEXT("Ready to start the mission!");
	PubnubSubsystem->PublishMessage(Channel, SimpleMessage);

	UE_LOG(LogTemp, Log, TEXT("SimplePublishSample Finished successfully"));
}

// snippet.advance_publish
void ASample_PubSub::AdvancedPublishSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Set Channel and Message to Publish
	FString Channel = TEXT("global_chat");
	FString JsonMessage = R"({
        "event": "PowerUpUsed",
        "powerup": "Invisibility Cloak",
        "duration": 10
    })";

	//Create additional PublishSettings
	FPubnubPublishSettings PublishSettings;
	PublishSettings.MetaData = R"({
        "map": "DesertStrike",
        "match_id": "MATCH-42"
    })";
	PublishSettings.CustomMessageType = "game-event";

	//Publish message with settings
	PubnubSubsystem->PublishMessage(Channel, JsonMessage, PublishSettings);

	UE_LOG(LogTemp, Log, TEXT("AdvancedPublishSample Finished successfully"));
}

// snippet.publish_with_ttl
void ASample_PubSub::PublishWithTTLSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Set Channel and Message to Publish
	FString Channel = TEXT("trade_chat");
	FString Message = R"({
        "item": "sword",
        "price": 10
    })";
	
	//Create additional PublishSettings
	FPubnubPublishSettings PublishSettings;
	PublishSettings.Ttl = 10;

	//Publish message with settings
	PubnubSubsystem->PublishMessage(Channel, Message, PublishSettings);

	UE_LOG(LogTemp, Log, TEXT("PublishWithTTLSample Finished successfully"));
}

// snippet.simple_signal
void ASample_PubSub::SimpleSignalSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Send signal message to provided channel
	FString Channel = TEXT("global_chat");
	FString SimpleMessage = TEXT("Ready to start the mission!");
	PubnubSubsystem->Signal(Channel, SimpleMessage);

	UE_LOG(LogTemp, Log, TEXT("SimpleSignalSample Finished successfully"));
}

// snippet.signal_with_settings
void ASample_PubSub::SignalWithSettingsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Set Channel and Message to Signal
	FString Channel = TEXT("guild_chat");
	FString Message = TEXT("PlayerTyping");

	//Create additional SignalSettings
	FPubnubSignalSettings SignalSettings;
	SignalSettings.CustomMessageType = "typing-indicator";

	//Send signal with settings
	PubnubSubsystem->Signal(Channel, Message, SignalSettings);

	UE_LOG(LogTemp, Log, TEXT("SignalWithSettingsSample Finished successfully"));
}

// snippet.subscribe
void ASample_PubSub::SubscribeSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Add Listener/Delegate that will broadcast whenever message is received on any subscribed channel or group
	PubnubSubsystem->OnMessageReceived.AddDynamic(this, &ASample_PubSub::OnMessageReceived_SubscribeSample);

	//Subscribe to the Channel
	FString Channel = TEXT("guild_chat");
	PubnubSubsystem->SubscribeToChannel(Channel);
}

void ASample_PubSub::OnMessageReceived_SubscribeSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("Message reveived on Channel: %s, Message Content: %s"), *Message.Channel, *Message.Message);
}

// snippet.subscribe_to_group
void ASample_PubSub::SubscribeToGroupSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);
	
	//Add Listener/Delegate that will broadcast whenever message is received on any subscribed channel or group
	PubnubSubsystem->OnMessageReceived.AddDynamic(this, &ASample_PubSub::OnMessageReceived_SubscribeToGroupSample);

	//Subscribe to the Channel
	FString Group = TEXT("my_group_chats");
	PubnubSubsystem->SubscribeToGroup(Group);
}

void ASample_PubSub::OnMessageReceived_SubscribeToGroupSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("Message reveived on Channel: %s, Message Content: %s"), *Message.Channel, *Message.Message);
}

// snippet.subscribe_with_presence
void ASample_PubSub::SubscribeWitPresenceSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Add Listener/Delegate that will broadcast whenever message is received on any subscribed channel or group
	PubnubSubsystem->OnMessageReceived.AddDynamic(this, &ASample_PubSub::OnMessageReceived_SubscribeWitPresenceSample);

	//Create additional subscribe settings
	FPubnubSubscribeSettings SubscribeSettings;
	SubscribeSettings.ReceivePresenceEvents = true;
	
	//Subscribe to the Channel
	FString Channel = TEXT("guild_chat");
	PubnubSubsystem->SubscribeToChannel(Channel, SubscribeSettings);
}

void ASample_PubSub::OnMessageReceived_SubscribeWitPresenceSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("Message reveived on Channel: %s, Message Content: %s"), *Message.Channel, *Message.Message);
}

// snippet.end
