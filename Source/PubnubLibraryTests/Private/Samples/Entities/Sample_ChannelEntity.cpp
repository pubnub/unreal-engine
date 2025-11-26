// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Samples/Entities/Sample_ChannelEntity.h"
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
 * 
 * ENTITY SAMPLES demonstrate the new Entity-based approach to PubNub operations, which provides
 * a more object-oriented and intuitive way to work with PubNub channels, channel groups, and metadata.
 */

// NOTE: Comments marked with `ACTION REQUIRED` indicate lines you must change.


ASample_ChannelEntity::ASample_ChannelEntity()
{
	SamplesName = "ChannelEntity";
}

void ASample_ChannelEntity::RunSamples()
{
	Super::RunSamples();
	
	CreateChannelEntitySample();
	PublishSimpleSample();
	PublishWithSettingsSample();
	PublishWithResultSample();
	PublishWithResultLambdaSample();
	SimpleSignalSample();
	SignalWithSettingsSample();
	SignalWithResultSample();
	SignalWithResultLambdaSample();
	ListUsersFromChannelSample();
	ListUsersFromChannelWithSettingsSample();
	ListUsersFromChannelWithLambdaSample();
	SubscribeWithChannelEntitySample();
	ChannelEntityAllListenersSample();
	ChannelEntityPublishMessageSample();
}


/* CHANNEL ENTITY SAMPLE FUNCTIONS */


// snippet.create_channel_entity
void ASample_ChannelEntity::CreateChannelEntitySample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel entity for the channel you want to work with
	FString ChannelName = TEXT("game_lobby");
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(ChannelName);
}

// snippet.publish_simple_entity
// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::PublishSimpleSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel entity for the channel you want to work with
	FString ChannelName = TEXT("global_chat");
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(ChannelName);

	// Publish simple text message using the channel entity
	FString SimpleMessage = TEXT("Ready to start the mission!");
	ChannelEntity->PublishMessageAsync(SimpleMessage);
}

// snippet.publish_with_settings_entity
// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::PublishWithSettingsSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel entity for the channel you want to work with
	FString ChannelName = TEXT("global_chat");
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(ChannelName);

	// Set Message to Publish
	FString JsonMessage = R"({"event": "PowerUpUsed", "powerup": "Invisibility Cloak", "duration": 10})";

	// Create additional PublishSettings
	FPubnubPublishSettings PublishSettings;
	PublishSettings.MetaData = R"({"map": "DesertStrike", "match_id": "MATCH-42"})";
	PublishSettings.CustomMessageType = "game-event";

	// Publish message with settings using the channel entity
	ChannelEntity->PublishMessageAsync(JsonMessage, PublishSettings);
}

// snippet.publish_with_result_entity
// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::PublishWithResultSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel entity for the channel you want to work with
	FString ChannelName = TEXT("global_chat");
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(ChannelName);

	// Bind PublishedMessageResponse to be fired with PublishMessage result
	// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
	FOnPubnubPublishMessageResponse OnPublishMessageResponse;
	OnPublishMessageResponse.BindDynamic(this, &ASample_ChannelEntity::PublishedMessageResponse);
	
	// Publish simple text message using the channel entity
	FString SimpleMessage = TEXT("Ready to start the mission!");
	ChannelEntity->PublishMessageAsync(SimpleMessage, OnPublishMessageResponse);
}

// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::PublishedMessageResponse(FPubnubOperationResult Result, FPubnubMessageData Message)
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

// snippet.publish_with_result_lambda_entity
// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::PublishWithResultLambdaSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel entity for the channel you want to work with
	FString ChannelName = TEXT("global_chat");
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(ChannelName);

	// Bind lambda function to PublishMessageResponse delegate
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
	
	// Publish simple text message using the channel entity
	FString SimpleMessage = TEXT("Ready to start the mission!");
	ChannelEntity->PublishMessageAsync(SimpleMessage, OnPublishMessageResponse);
}

// snippet.simple_signal_entity
// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::SimpleSignalSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel entity for the channel you want to work with
	FString ChannelName = TEXT("global_chat");
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(ChannelName);

	// Send signal message using the channel entity
	FString SimpleMessage = TEXT("Ready to start the mission!");
	ChannelEntity->SignalAsync(SimpleMessage);
}

// snippet.signal_with_settings_entity
// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::SignalWithSettingsSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel entity for the channel you want to work with
	FString ChannelName = TEXT("guild_chat");
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(ChannelName);

	// Set Message to Signal
	FString Message = TEXT("PlayerTyping");

	// Create additional SignalSettings
	FPubnubSignalSettings SignalSettings;
	SignalSettings.CustomMessageType = "typing-indicator";

	// Send signal with settings using the channel entity
	ChannelEntity->SignalAsync(Message, SignalSettings);
}

// snippet.signal_with_result_entity
// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::SignalWithResultSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel entity for the channel you want to work with
	FString ChannelName = TEXT("global_chat");
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(ChannelName);

	// Bind SignalMessageResponse to be fired with Signal result
	// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
	FOnPubnubSignalResponse OnSignalResponse;
	OnSignalResponse.BindDynamic(this, &ASample_ChannelEntity::SignalMessageResponse);
	
	// Send signal message using the channel entity
	FString SimpleMessage = TEXT("Player is aiming");
	ChannelEntity->SignalAsync(SimpleMessage, OnSignalResponse);
}

// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::SignalMessageResponse(FPubnubOperationResult Result, FPubnubMessageData Message)
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

// snippet.signal_with_result_lambda_entity
// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::SignalWithResultLambdaSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel entity for the channel you want to work with
	FString ChannelName = TEXT("global_chat");
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(ChannelName);

	// Bind lambda function to SignalResponse delegate
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
	
	// Send signal message using the channel entity
	FString SimpleMessage = TEXT("Player is aiming");
	ChannelEntity->SignalAsync(SimpleMessage, OnSignalResponse);
}

// snippet.list_users_from_channel_entity
// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::ListUsersFromChannelSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel entity for the channel you want to list users from
	FString ChannelName = TEXT("guild-channel");
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(ChannelName);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
	FOnPubnubListUsersFromChannelResponse OnListUsersFromChannelResponse;
	OnListUsersFromChannelResponse.BindDynamic(this, &ASample_ChannelEntity::OnListUsersFromChannelResponse_Simple);

	// List users from the channel using the channel entity
	ChannelEntity->ListUsersFromChannelAsync(OnListUsersFromChannelResponse);
}

// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::OnListUsersFromChannelResponse_Simple(FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper Data)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to list users from channel. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Users successfully listed from channel. Occupancy: %d"), Data.Occupancy);
	}
}

// snippet.list_users_from_channel_with_settings_entity
// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::ListUsersFromChannelWithSettingsSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel entity for the channel you want to list users from
	FString ChannelName = TEXT("guild-channel");
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(ChannelName);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
	FOnPubnubListUsersFromChannelResponse OnListUsersFromChannelResponse;
	OnListUsersFromChannelResponse.BindDynamic(this, &ASample_ChannelEntity::OnListUsersFromChannelResponse_WithSettings);

	// Create additional settings
	FPubnubListUsersFromChannelSettings Settings;
	Settings.State = true;
	Settings.DisableUserID = false;

	// List users from the channel with settings using the channel entity
	ChannelEntity->ListUsersFromChannelAsync(OnListUsersFromChannelResponse, Settings);
}

// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::OnListUsersFromChannelResponse_WithSettings(FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper Data)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to list users from channel. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Users successfully listed from channel. Total occupancy: %d"), Data.Occupancy);
		// List all users with theirs states
		for (auto const& [UserID, UserState] : Data.UsersState)
		{
			UE_LOG(LogTemp, Log, TEXT("UserID: %s, User State: %s"), *UserID, *UserState);
		}
	}
}

// snippet.list_users_from_channel_with_lambda_entity
// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::ListUsersFromChannelWithLambdaSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel entity for the channel you want to list users from
	FString ChannelName = TEXT("guild-channel");
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(ChannelName);

	// Bind lambda to response delegate
	FOnPubnubListUsersFromChannelResponseNative OnListUsersFromChannelResponse;
	OnListUsersFromChannelResponse.BindLambda([](const FPubnubOperationResult& Result, const FPubnubListUsersFromChannelWrapper& Data)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to list users from channel. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Users successfully listed from channel. Occupancy: %d"), Data.Occupancy);
		}
	});
	
	// List users from the channel using the channel entity
	ChannelEntity->ListUsersFromChannelAsync(OnListUsersFromChannelResponse);
}

// snippet.subscribe_with_channel_entity
// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::SubscribeWithChannelEntitySample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel entity for the channel you want to work with
	FString ChannelName = TEXT("game_lobby");
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(ChannelName);

	// Create a subscription from the channel entity
	UPubnubSubscription* ChannelSubscription = ChannelEntity->CreateSubscription();

	// Add message listener to the subscription
	// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
	ChannelSubscription->OnPubnubMessage.AddDynamic(this, &ASample_ChannelEntity::OnMessageReceived_ChannelEntitySample);

	// Subscribe to start receiving messages
	ChannelSubscription->SubscribeAsync();
}

// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::OnMessageReceived_ChannelEntitySample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("Channel Entity - Message received: %s on channel: %s"), *Message.Message, *Message.Channel);
}

// snippet.channel_entity_all_listeners
// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::ChannelEntityAllListenersSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel entity
	FString ChannelName = TEXT("comprehensive_channel");
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(ChannelName);

	// Create a subscription with presence events enabled to receive all event types
	FPubnubSubscribeSettings SubscriptionSettings;
	SubscriptionSettings.ReceivePresenceEvents = true;
	UPubnubSubscription* Subscription = ChannelEntity->CreateSubscription(SubscriptionSettings);

	// Add ALL listener types to handle different PubNub events

	// 1. Message Listener - Fires when regular messages are published to the channel
	// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
	Subscription->OnPubnubMessage.AddDynamic(this, &ASample_ChannelEntity::OnMessage_AllListenersSample);

	// 2. Signal Listener - Fires when signals are sent to the channel
	// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
	Subscription->OnPubnubSignal.AddDynamic(this, &ASample_ChannelEntity::OnSignal_AllListenersSample);

	// 3. Presence Event Listener - Fires when users join/leave/timeout on the channel
	// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
	Subscription->OnPubnubPresenceEvent.AddDynamic(this, &ASample_ChannelEntity::OnPresenceEvent_AllListenersSample);

	// 4. Object Event Listener - Fires when App Context metadata changes occur
	// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
	Subscription->OnPubnubObjectEvent.AddDynamic(this, &ASample_ChannelEntity::OnObjectEvent_AllListenersSample);

	// 5. Message Action Listener - Fires when message actions/reactions are added or removed
	// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
	Subscription->OnPubnubMessageAction.AddDynamic(this, &ASample_ChannelEntity::OnMessageAction_AllListenersSample);

	// 6. Universal Listener - Fires for ANY type of PubNub event (catch-all)
	// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
	Subscription->FOnPubnubAnyMessageType.AddDynamic(this, &ASample_ChannelEntity::OnAnyEvent_AllListenersSample);

	// Subscribe to start receiving all event types
	Subscription->SubscribeAsync();
}

// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::OnMessage_AllListenersSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("MESSAGE LISTENER - Content: %s, Channel: %s, User: %s"), 
		*Message.Message, *Message.Channel, *Message.UserID);
}

// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::OnSignal_AllListenersSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("SIGNAL LISTENER - Content: %s, Channel: %s, User: %s"), 
		*Message.Message, *Message.Channel, *Message.UserID);
}

// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::OnPresenceEvent_AllListenersSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("PRESENCE LISTENER - Event: %s, Channel: %s"), 
		*Message.Message, *Message.Channel);
}

// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::OnObjectEvent_AllListenersSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("OBJECT EVENT LISTENER - Event: %s, Channel: %s"), 
		*Message.Message, *Message.Channel);
}

// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::OnMessageAction_AllListenersSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("MESSAGE ACTION LISTENER - Action: %s, Channel: %s"), 
		*Message.Message, *Message.Channel);
}

// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::OnAnyEvent_AllListenersSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("UNIVERSAL LISTENER - Type: %d, Content: %s, Channel: %s"), 
		(int32)Message.MessageType, *Message.Message, *Message.Channel);
}

// snippet.channel_entity_publish_message
// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::ChannelEntityPublishMessageSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel entity for the game chat channel
	FString GameChatChannel = TEXT("team_alpha_chat");
	UPubnubChannelEntity* ChatChannelEntity = PubnubSubsystem->CreateChannelEntity(GameChatChannel);

	// Set up callback to handle publish result
	// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
	FOnPubnubPublishMessageResponse OnPublishResult;
	OnPublishResult.BindDynamic(this, &ASample_ChannelEntity::OnPublishResult_ChannelEntitySample);

	// Publish a tactical message to the team using the channel entity
	FString TacticalMessage = TEXT("Enemy spotted at coordinates B-7, requesting backup!");
	ChatChannelEntity->PublishMessageAsync(TacticalMessage, OnPublishResult);
}

// ACTION REQUIRED: Replace ASample_ChannelEntity with name of your Actor class
void ASample_ChannelEntity::OnPublishResult_ChannelEntitySample(FPubnubOperationResult Result, FPubnubMessageData Message)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to send tactical message. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Tactical message sent successfully to team. Message timetoken: %s"), *Message.Timetoken);
	}
}

// snippet.end
