// Copyright 2026 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_PubSub.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	//Publish simple text message to provided channel
	FString Channel = TEXT("global_chat");
	FString SimpleMessage = TEXT("Ready to start the mission!");
	PubnubClient->PublishMessageAsync(Channel, SimpleMessage);
}

// snippet.publish_with_settings
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::PublishWithSettingsSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Set Channel and Message to Publish
	FString Channel = TEXT("global_chat");
	FString JsonMessage = R"({"event": "PowerUpUsed", "powerup": "Invisibility Cloak", "duration": 10})";

	//Create additional PublishSettings
	FPubnubPublishSettings PublishSettings;
	PublishSettings.MetaData = R"({"map": "DesertStrike", "match_id": "MATCH-42"})";
	PublishSettings.CustomMessageType = "game-event";

	//Publish message with settings
	PubnubClient->PublishMessageAsync(Channel, JsonMessage, PublishSettings);
}

// snippet.publish_with_result
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::PublishWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Bind PublishedMessageResponse to be fired with PublishMessage result
	// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
	FOnPubnubPublishMessageResponse OnPublishMessageResponse;
	OnPublishMessageResponse.BindDynamic(this, &ASample_PubSub::PublishedMessageResponse);
	
	//Publish simple text message to provided channel
	FString Channel = TEXT("global_chat");
	FString SimpleMessage = TEXT("Ready to start the mission!");
	PubnubClient->PublishMessageAsync(Channel, SimpleMessage, OnPublishMessageResponse);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Bind lambda function to PublishMessageResponse delegate
	FOnPubnubPublishMessageResponseNative OnPublishMessageResponse;
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
	PubnubClient->PublishMessageAsync(Channel, SimpleMessage, OnPublishMessageResponse);
}

// snippet.simple_signal
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::SimpleSignalSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Send signal message to provided channel
	FString Channel = TEXT("global_chat");
	FString SimpleMessage = TEXT("Ready to start the mission!");
	PubnubClient->SignalAsync(Channel, SimpleMessage);
}

// snippet.signal_with_settings
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::SignalWithSettingsSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Set Channel and Message to Signal
	FString Channel = TEXT("guild_chat");
	FString Message = TEXT("PlayerTyping");

	//Create additional SignalSettings
	FPubnubSignalSettings SignalSettings;
	SignalSettings.CustomMessageType = "typing-indicator";

	//Send signal with settings
	PubnubClient->SignalAsync(Channel, Message, SignalSettings);
}

// snippet.signal_with_result
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::SignalWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Bind SignalMessageResponse to be fired with Signal result
	// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
	FOnPubnubSignalResponse OnSignalResponse;
	OnSignalResponse.BindDynamic(this, &ASample_PubSub::SignalMessageResponse);
	
	//Send signal message to provided channel
	FString Channel = TEXT("global_chat");
	FString SimpleMessage = TEXT("Player is aiming");
	PubnubClient->SignalAsync(Channel, SimpleMessage, OnSignalResponse);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Bind lambda function to SignalResponse delegate
	FOnPubnubSignalResponseNative OnSignalResponse;
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
	PubnubClient->SignalAsync(Channel, SimpleMessage, OnSignalResponse);
}

// snippet.subscribe
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::SubscribeSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Add Listener/Delegate that will broadcast whenever message is received on any subscribed channel or group
	// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
	PubnubClient->OnMessageReceived.AddDynamic(this, &ASample_PubSub::OnMessageReceived_SubscribeSample);

	//Subscribe to the Channel
	FString Channel = TEXT("guild_chat");
	PubnubClient->SubscribeToChannelAsync(Channel);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	//Add Lambda Listener/Delegate that will broadcast whenever message is received on any subscribed channel or group
	PubnubClient->OnMessageReceivedNative.AddLambda([](const FPubnubMessageData& Message)
	{
		UE_LOG(LogTemp, Log, TEXT("Message received on Channel: %s, Message Content: %s"), *Message.Channel, *Message.Message);
	});

	//Subscribe to the Channel
	FString Channel = TEXT("guild_chat");
	PubnubClient->SubscribeToChannelAsync(Channel);
}

// snippet.subscribe_to_group
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::SubscribeToGroupSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set
	
	//Add Listener/Delegate that will broadcast whenever message is received on any subscribed channel or group
	// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
	PubnubClient->OnMessageReceived.AddDynamic(this, &ASample_PubSub::OnMessageReceived_SubscribeToGroupSample);

	//Subscribe to the Channel
	FString Group = TEXT("my_group_chats");
	PubnubClient->SubscribeToGroupAsync(Group);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Add Listener/Delegate that will broadcast whenever message is received on any subscribed channel or group
	// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
	PubnubClient->OnMessageReceived.AddDynamic(this, &ASample_PubSub::OnMessageReceived_SubscribeWithPresenceSample);

	//Create additional subscribe settings
	FPubnubSubscribeSettings SubscribeSettings;
	SubscribeSettings.ReceivePresenceEvents = true;
	
	//Subscribe to the Channel
	FString Channel = TEXT("guild_chat");
	PubnubClient->SubscribeToChannelAsync(Channel, SubscribeSettings);
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

// snippet.subscribe_with_result
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::SubscribeWithResultSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Add Listener/Delegate that will broadcast whenever message is received on any subscribed channel or group
	// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
	PubnubClient->OnMessageReceived.AddDynamic(this, &ASample_PubSub::OnMessageReceived_SubscribeWithResultSample);

	//Add Listener/Delegate that will be called when subscribe operation is established with it's result
	//Note:: this is not listener for messages incoming to the subscribed channel
	// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
	FOnPubnubSubscribeOperationResponse OnSubscribeOperationResponse;
	OnSubscribeOperationResponse.BindDynamic(this, &ASample_PubSub::OnSubscribeResultReceived);
	
	//Subscribe to the Channel
	FString Channel = TEXT("guild_chat");
	PubnubClient->SubscribeToChannelAsync(Channel, OnSubscribeOperationResponse);
}

// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::OnMessageReceived_SubscribeWithResultSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("Message received on Channel: %s, Message Content: %s"), *Message.Channel, *Message.Message);
}

// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::OnSubscribeResultReceived(FPubnubOperationResult Result)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to subscribe to channel. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Subscribed to channel successfully."));
	}
}

// snippet.unsubscribe_from_channel
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::UnsubscribeFromChannelSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Unsubscribe from the Channel - stop listening for messages on that channel
	FString Channel = TEXT("guild_chat");
	PubnubClient->UnsubscribeFromChannelAsync(Channel);
}

// snippet.unsubscribe_from_group
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::UnsubscribeFromGroupSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Unsubscribe from the Group - stop listening for messages on all channels that belong to that group
	FString Group = TEXT("my_group_chats");
	PubnubClient->UnsubscribeFromGroupAsync(Group);
}

// snippet.unsubscribe_from_all
// ACTION REQUIRED: Replace ASample_PubSub with name of your Actor class
void ASample_PubSub::UnsubscribeFromAllSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Unsubscribe from all - stop receiving any messages from all channels and groups
	PubnubClient->UnsubscribeFromAllAsync();
}

// snippet.end

UPubnubClient* ASample_PubSub::GetPubnubClient()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	
	//Get default PubnubClient - created automatically if PluginSettings are set to do so
	UPubnubClient* PubnubClient = PubnubSubsystem->GetPubnubClient(0);
	
	PubnubClient->SetUserID(TEXT("player_001"));
	return PubnubClient;
}
