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
 * The samples assume that in Pubnub SDK settings sections in ProjectSettings following fields are set:
 * PublishKey and SubscribeKey have correct keys, InitializeAutomatically is true.
 */

// NOTE: Comments marked with `ACTION REQUIRED` indicate lines you must change.


//Internal function, don't copy it with the samples
void ASample_PubSub::RunSamples()
{
	Super::RunSamples();
	
	PublishSimpleSample();
	PublishWithSettingsSample();
	PublishWithResultSample();
	PublishWithResultLambdaSample();
	SimpleSignalSample();
	SignalWithSettingsSample();
	SignalWithResultSample();
	SignalWithResultLambdaSample();
	SubscribeSample();
	SubscribeWithLambdaSample();
	SubscribeToGroupSample();
	SubscribeWithPresenceSample();
	UnsubscribeFromChannelSample();
	UnsubscribeFromGroupSample();
	UnsubscribeFromAllSample();
}
//Internal function, don't copy it with the samples
ASample_PubSub::ASample_PubSub()
{
	SamplesName = "PubSub";
}


/* SAMPLE FUNCTIONS */

// snippet.publish_simple
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::PublishSimpleSample()
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
}

// snippet.publish_with_settings
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::PublishWithSettingsSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Set Channel and Message to Publish
	FString Channel = TEXT("global_chat");
	FString JsonMessage = R"({"event": "PowerUpUsed", "powerup": "Invisibility Cloak", "duration": 10})";

	//Create additional PublishSettings
	FPubnubPublishSettings PublishSettings;
	PublishSettings.MetaData = R"({"map": "DesertStrike", "match_id": "MATCH-42"})";
	PublishSettings.CustomMessageType = "game-event";

	//Publish message with settings
	PubnubSubsystem->PublishMessage(Channel, JsonMessage, PublishSettings);
}

// snippet.publish_with_result
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::PublishWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Bind PublishedMessageResponse to be fired with PublishMessage result
	// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
	FOnPublishMessageResponse OnPublishMessageResponse;
	OnPublishMessageResponse.BindDynamic(this, &ASample_PubSub::PublishedMessageResponse);
	
	//Publish simple text message to provided channel
	FString Channel = TEXT("global_chat");
	FString SimpleMessage = TEXT("Ready to start the mission!");
	PubnubSubsystem->PublishMessage(Channel, SimpleMessage, OnPublishMessageResponse);
}

// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::PublishedMessageResponse(FPubnubOperationResult Result, FPubnubMessageData Message)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to publish message. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Message published successfully. Published message timetoken: %s"), *Message.Timetoken);
	}
}

// snippet.publish_with_result_lambda
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::PublishWithResultLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Bind lambda function to PublishMessageResponse delegate
	FOnPublishMessageResponseNative OnPublishMessageResponse;
	OnPublishMessageResponse.BindLambda([](const FPubnubOperationResult& Result, const FPubnubMessageData& Message)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to publish message. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Message published successfully. Published message timetoken: %s"), *Message.Timetoken);
		}
	});
	
	//Publish simple text message to provided channel
	FString Channel = TEXT("global_chat");
	FString SimpleMessage = TEXT("Ready to start the mission!");
	PubnubSubsystem->PublishMessage(Channel, SimpleMessage, OnPublishMessageResponse);
}

// snippet.simple_signal
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
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
}

// snippet.signal_with_settings
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
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
}

// snippet.signal_with_result
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::SignalWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Bind SignalMessageResponse to be fired with Signal result
	// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
	FOnSignalResponse OnSignalResponse;
	OnSignalResponse.BindDynamic(this, &ASample_PubSub::SignalMessageResponse);
	
	//Send signal message to provided channel
	FString Channel = TEXT("global_chat");
	FString SimpleMessage = TEXT("Player is aiming");
	PubnubSubsystem->Signal(Channel, SimpleMessage, OnSignalResponse);
}

// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::SignalMessageResponse(FPubnubOperationResult Result, FPubnubMessageData Message)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to send signal. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Signal sent successfully. Signal timetoken: %s"), *Message.Timetoken);
	}
}

// snippet.signal_with_result_lambda
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::SignalWithResultLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Bind lambda function to SignalResponse delegate
	FOnSignalResponseNative OnSignalResponse;
	OnSignalResponse.BindLambda([](const FPubnubOperationResult& Result, const FPubnubMessageData& Message)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to send signal. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Signal sent successfully. Signal timetoken: %s"), *Message.Timetoken);
		}
	});
	
	//Send signal message to provided channel
	FString Channel = TEXT("global_chat");
	FString SimpleMessage = TEXT("Player is aiming");
	PubnubSubsystem->Signal(Channel, SimpleMessage, OnSignalResponse);
}

// snippet.subscribe
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::SubscribeSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Add Listener/Delegate that will broadcast whenever message is received on any subscribed channel or group
	// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
	PubnubSubsystem->OnMessageReceived.AddDynamic(this, &ASample_PubSub::OnMessageReceived_SubscribeSample);

	//Subscribe to the Channel
	FString Channel = TEXT("guild_chat");
	PubnubSubsystem->SubscribeToChannel(Channel);
}

// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::OnMessageReceived_SubscribeSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("Message received on Channel: %s, Message Content: %s"), *Message.Channel, *Message.Message);
}

// snippet.subscribe_with_lambda
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::SubscribeWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);
	
	//Add Lambda Listener/Delegate that will broadcast whenever message is received on any subscribed channel or group
	PubnubSubsystem->OnMessageReceivedNative.AddLambda([](const FPubnubMessageData& Message)
	{
		UE_LOG(LogTemp, Log, TEXT("Message received on Channel: %s, Message Content: %s"), *Message.Channel, *Message.Message);
	});

	//Subscribe to the Channel
	FString Channel = TEXT("guild_chat");
	PubnubSubsystem->SubscribeToChannel(Channel);
}

// snippet.subscribe_to_group
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::SubscribeToGroupSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);
	
	//Add Listener/Delegate that will broadcast whenever message is received on any subscribed channel or group
	// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
	PubnubSubsystem->OnMessageReceived.AddDynamic(this, &ASample_PubSub::OnMessageReceived_SubscribeToGroupSample);

	//Subscribe to the Channel
	FString Group = TEXT("my_group_chats");
	PubnubSubsystem->SubscribeToGroup(Group);
}

// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::OnMessageReceived_SubscribeToGroupSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("Message received on Channel: %s, Message Content: %s"), *Message.Channel, *Message.Message);
}

// snippet.subscribe_with_presence
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::SubscribeWithPresenceSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Add Listener/Delegate that will broadcast whenever message is received on any subscribed channel or group
	// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
	PubnubSubsystem->OnMessageReceived.AddDynamic(this, &ASample_PubSub::OnMessageReceived_SubscribeWithPresenceSample);

	//Create additional subscribe settings
	FPubnubSubscribeSettings SubscribeSettings;
	SubscribeSettings.ReceivePresenceEvents = true;
	
	//Subscribe to the Channel
	FString Channel = TEXT("guild_chat");
	PubnubSubsystem->SubscribeToChannel(Channel, SubscribeSettings);
}

// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::OnMessageReceived_SubscribeWithPresenceSample(FPubnubMessageData Message)
{
	//Presence events are received on "{channel}-pnpres" channel
	if(Message.Channel.Contains("-pnpres"))
	{
		UE_LOG(LogTemp, Log, TEXT("Event received on Channel: %s, Event Content: %s"), *Message.Channel, *Message.Message);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Message received on Channel: %s, Message Content: %s"), *Message.Channel, *Message.Message);
	}
}

// snippet.unsubscribe_from_channel
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::UnsubscribeFromChannelSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Unsubscribe from the Channel - stop listening for messages on that channel
	FString Channel = TEXT("guild_chat");
	PubnubSubsystem->UnsubscribeFromChannel(Channel);
}

// snippet.unsubscribe_from_group
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::UnsubscribeFromGroupSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Unsubscribe from the Group - stop listening for messages on all channels that belong to that group
	FString Group = TEXT("my_group_chats");
	PubnubSubsystem->UnsubscribeFromGroup(Group);
}

// snippet.unsubscribe_from_all
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::UnsubscribeFromAllSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Unsubscribe from all - stop receiving any messages from all channels and groups
	PubnubSubsystem->UnsubscribeFromAll();
}

// snippet.end
