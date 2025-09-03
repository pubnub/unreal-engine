// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Samples/Sample_Entities.h"
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


//Internal function, don't copy it with the samples
void ASample_Entities::RunSamples()
{
	Super::RunSamples();
	
	SubscribeWithChannelEntitySample();
	SubscribeWithChannelGroupEntitySample();
	SubscribeWithChannelMetadataEntitySample();
	SubscribeWithUserMetadataEntitySample();
	ChannelEntityAllListenersSample();
	ChannelEntityPublishMessageSample();
	ChannelGroupEntityAddChannelSample();
	ChannelMetadataEntitySetMetadataSample();
	UserMetadataEntitySetMetadataSample();
	CreateSubscriptionSetFromNamesSample();
	CreateSubscriptionSetFromEntitiesSample();
	SubscriptionSetAddRemoveSubscriptionsSample();
	SubscriptionSetMergeOperationsSample();
}

//Internal function, don't copy it with the samples
ASample_Entities::ASample_Entities()
{
	SamplesName = "Entities";
}


/* ENTITY SAMPLE FUNCTIONS */

// snippet.subscribe_with_channel_entity
// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::SubscribeWithChannelEntitySample()
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
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	ChannelSubscription->OnPubnubMessage.AddDynamic(this, &ASample_Entities::OnMessageReceived_ChannelEntitySample);

	// Subscribe to start receiving messages
	ChannelSubscription->Subscribe();
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnMessageReceived_ChannelEntitySample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("Channel Entity - Message received: %s on channel: %s"), *Message.Message, *Message.Channel);
}

// snippet.subscribe_with_channel_group_entity
// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::SubscribeWithChannelGroupEntitySample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel group entity for the group you want to work with
	FString ChannelGroupName = TEXT("game_rooms");
	UPubnubChannelGroupEntity* ChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(ChannelGroupName);

	// Create a subscription from the channel group entity
	UPubnubSubscription* GroupSubscription = ChannelGroupEntity->CreateSubscription();

	// Add message listener to the subscription
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	GroupSubscription->OnPubnubMessage.AddDynamic(this, &ASample_Entities::OnMessageReceived_ChannelGroupEntitySample);

	// Subscribe to start receiving messages from all channels in the group
	GroupSubscription->Subscribe();
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnMessageReceived_ChannelGroupEntitySample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("Channel Group Entity - Message received: %s on channel: %s"), *Message.Message, *Message.Channel);
}

// snippet.subscribe_with_channel_metadata_entity
// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::SubscribeWithChannelMetadataEntitySample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel metadata entity for the channel you want to monitor metadata changes
	FString ChannelName = TEXT("lobby_settings");
	UPubnubChannelMetadataEntity* ChannelMetadataEntity = PubnubSubsystem->CreateChannelMetadataEntity(ChannelName);

	// Create a subscription from the channel metadata entity
	UPubnubSubscription* MetadataSubscription = ChannelMetadataEntity->CreateSubscription();

	// Add object event listener to receive App Context metadata change notifications
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	MetadataSubscription->OnPubnubObjectEvent.AddDynamic(this, &ASample_Entities::OnObjectEvent_ChannelMetadataEntitySample);

	// Subscribe to start receiving metadata change events
	MetadataSubscription->Subscribe();
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnObjectEvent_ChannelMetadataEntitySample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("Channel Metadata Entity - Object event received: %s"), *Message.Message);
}

// snippet.subscribe_with_user_metadata_entity
// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::SubscribeWithUserMetadataEntitySample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a user metadata entity for the user you want to monitor metadata changes
	FString UserToMonitor = TEXT("Player_002");
	UPubnubUserMetadataEntity* UserMetadataEntity = PubnubSubsystem->CreateUserMetadataEntity(UserToMonitor);

	// Create a subscription from the user metadata entity
	UPubnubSubscription* UserMetadataSubscription = UserMetadataEntity->CreateSubscription();

	// Add object event listener to receive App Context user metadata change notifications
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	UserMetadataSubscription->OnPubnubObjectEvent.AddDynamic(this, &ASample_Entities::OnObjectEvent_UserMetadataEntitySample);

	// Subscribe to start receiving user metadata change events
	UserMetadataSubscription->Subscribe();
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnObjectEvent_UserMetadataEntitySample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("User Metadata Entity - Object event received: %s"), *Message.Message);
}

// snippet.channel_entity_all_listeners
// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::ChannelEntityAllListenersSample()
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
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	Subscription->OnPubnubMessage.AddDynamic(this, &ASample_Entities::OnMessage_AllListenersSample);

	// 2. Signal Listener - Fires when signals are sent to the channel
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	Subscription->OnPubnubSignal.AddDynamic(this, &ASample_Entities::OnSignal_AllListenersSample);

	// 3. Presence Event Listener - Fires when users join/leave/timeout on the channel
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	Subscription->OnPubnubPresenceEvent.AddDynamic(this, &ASample_Entities::OnPresenceEvent_AllListenersSample);

	// 4. Object Event Listener - Fires when App Context metadata changes occur
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	Subscription->OnPubnubObjectEvent.AddDynamic(this, &ASample_Entities::OnObjectEvent_AllListenersSample);

	// 5. Message Action Listener - Fires when message actions/reactions are added or removed
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	Subscription->OnPubnubMessageAction.AddDynamic(this, &ASample_Entities::OnMessageAction_AllListenersSample);

	// 6. Universal Listener - Fires for ANY type of PubNub event (catch-all)
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	Subscription->FOnPubnubAnyMessageType.AddDynamic(this, &ASample_Entities::OnAnyEvent_AllListenersSample);

	// Subscribe to start receiving all event types
	Subscription->Subscribe();
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnMessage_AllListenersSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("MESSAGE LISTENER - Content: %s, Channel: %s, User: %s"), 
		*Message.Message, *Message.Channel, *Message.UserID);
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnSignal_AllListenersSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("SIGNAL LISTENER - Content: %s, Channel: %s, User: %s"), 
		*Message.Message, *Message.Channel, *Message.UserID);
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnPresenceEvent_AllListenersSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("PRESENCE LISTENER - Event: %s, Channel: %s"), 
		*Message.Message, *Message.Channel);
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnObjectEvent_AllListenersSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("OBJECT EVENT LISTENER - Event: %s, Channel: %s"), 
		*Message.Message, *Message.Channel);
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnMessageAction_AllListenersSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("MESSAGE ACTION LISTENER - Action: %s, Channel: %s"), 
		*Message.Message, *Message.Channel);
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnAnyEvent_AllListenersSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("UNIVERSAL LISTENER - Type: %d, Content: %s, Channel: %s"), 
		(int32)Message.MessageType, *Message.Message, *Message.Channel);
}

// snippet.channel_entity_publish_message
// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::ChannelEntityPublishMessageSample()
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
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	FOnPublishMessageResponse OnPublishResult;
	OnPublishResult.BindDynamic(this, &ASample_Entities::OnPublishResult_ChannelEntitySample);

	// Publish a tactical message to the team using the channel entity
	FString TacticalMessage = TEXT("Enemy spotted at coordinates B-7, requesting backup!");
	ChatChannelEntity->PublishMessage(TacticalMessage, OnPublishResult);
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnPublishResult_ChannelEntitySample(FPubnubOperationResult Result, FPubnubMessageData Message)
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

// snippet.channel_group_entity_add_channel
// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::ChannelGroupEntityAddChannelSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Server_Admin");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel group entity for managing multiplayer game rooms
	FString GameRoomsGroup = TEXT("multiplayer_game_rooms");
	UPubnubChannelGroupEntity* GameRoomsEntity = PubnubSubsystem->CreateChannelGroupEntity(GameRoomsGroup);

	// Set up callback to handle the add channel operation result
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	FOnAddChannelToGroupResponse OnAddChannelResult;
	OnAddChannelResult.BindDynamic(this, &ASample_Entities::OnAddChannelResult_GroupEntitySample);

	// Add a new battle arena channel to the multiplayer rooms group
	FString NewBattleArena = TEXT("battle_arena_storm");
	GameRoomsEntity->AddChannelToGroup(NewBattleArena, OnAddChannelResult);
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnAddChannelResult_GroupEntitySample(FPubnubOperationResult Result)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to add battle arena to game rooms. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Battle arena successfully added to multiplayer game rooms group!"));
	}
}

// snippet.channel_metadata_entity_set_metadata
// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::ChannelMetadataEntitySetMetadataSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Game_Master");
	PubnubSubsystem->SetUserID(UserID);

	// Create a channel metadata entity for a tournament lobby
	FString TournamentLobby = TEXT("tournament_lobby_finals");
	UPubnubChannelMetadataEntity* LobbyMetadataEntity = PubnubSubsystem->CreateChannelMetadataEntity(TournamentLobby);

	// Set up callback to handle metadata set result
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	FOnSetChannelMetadataResponse OnSetMetadataResult;
	OnSetMetadataResult.BindDynamic(this, &ASample_Entities::OnSetChannelMetadataResult_Sample);

	// Set tournament lobby information and rules
	FPubnubChannelData TournamentLobbyInfo;
	TournamentLobbyInfo.ChannelID = TournamentLobby;
	TournamentLobbyInfo.ChannelName = "Championship Finals Lobby";
	TournamentLobbyInfo.Description = "Final tournament matches - best of 5 rounds";
	TournamentLobbyInfo.Custom = "{\"max_players\":10,\"tournament_tier\":\"championship\",\"prize_pool\":\"50000\"}";

	LobbyMetadataEntity->SetChannelMetadata(TournamentLobbyInfo, OnSetMetadataResult);
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnSetChannelMetadataResult_Sample(FPubnubOperationResult Result, FPubnubChannelData ChannelData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set tournament lobby info. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Tournament lobby info set successfully: %s - %s"), *ChannelData.ChannelName, *ChannelData.Description);
	}
}

// snippet.user_metadata_entity_set_metadata
// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::UserMetadataEntitySetMetadataSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Create a user metadata entity for updating player profile
	FString PlayerToUpdate = TEXT("Champion_Alex");
	UPubnubUserMetadataEntity* PlayerMetadataEntity = PubnubSubsystem->CreateUserMetadataEntity(PlayerToUpdate);

	// Set up callback to handle metadata set result
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	FOnSetUserMetadataResponse OnSetPlayerInfoResult;
	OnSetPlayerInfoResult.BindDynamic(this, &ASample_Entities::OnSetUserMetadataResult_Sample);

	// Set champion player profile and statistics
	FPubnubUserData ChampionPlayerProfile;
	ChampionPlayerProfile.UserID = PlayerToUpdate;
	ChampionPlayerProfile.UserName = "Alex 'Lightning' Rodriguez";
	ChampionPlayerProfile.Email = "alex.lightning@esports.com";
	ChampionPlayerProfile.Custom = "{\"rank\":\"Champion\",\"wins\":127,\"losses\":23,\"favorite_weapon\":\"Plasma Rifle\",\"achievements\":[\"First Blood\",\"Triple Kill Master\"]}";

	PlayerMetadataEntity->SetUserMetadata(ChampionPlayerProfile, OnSetPlayerInfoResult);
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnSetUserMetadataResult_Sample(FPubnubOperationResult Result, FPubnubUserData UserData)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to update champion player profile. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Champion player profile updated successfully: %s (%s)"), *UserData.UserName, *UserData.UserID);
	}
}

// snippet.create_subscription_set_from_names
// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::CreateSubscriptionSetFromNamesSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Squad_Leader");
	PubnubSubsystem->SetUserID(UserID);

	// Define multiple channels and channel groups to monitor for team coordination
	TArray<FString> TeamChannels = {
		TEXT("squad_alpha_chat"),        // Alpha team communication
		TEXT("squad_bravo_chat"),        // Bravo team communication
		TEXT("tactical_announcements")   // Mission updates and announcements
	};

	TArray<FString> OperationChannelGroups = {
		TEXT("mission_channels"),        // All mission-related channels
		TEXT("support_channels")         // Support and logistics channels
	};

	// Create subscription set from channel and group names - monitors all team communications
	UPubnubSubscriptionSet* TeamCommSubscriptionSet = PubnubSubsystem->CreateSubscriptionSet(TeamChannels, OperationChannelGroups);

	// Add message listener to monitor all team communications
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	TeamCommSubscriptionSet->OnPubnubMessage.AddDynamic(this, &ASample_Entities::OnMessage_SubscriptionSetFromNamesSample);

	// Subscribe to start monitoring all team channels and groups simultaneously
	TeamCommSubscriptionSet->Subscribe();
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnMessage_SubscriptionSetFromNamesSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("TEAM COMMS - Message on %s: %s (from %s)"), 
		*Message.Channel, *Message.Message, *Message.UserID);
}

// snippet.create_subscription_set_from_entities
// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::CreateSubscriptionSetFromEntitiesSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Command_Center");
	PubnubSubsystem->SetUserID(UserID);

	// Create individual entities for different aspects of game monitoring
	UPubnubChannelEntity* PlayerStatsChannel = PubnubSubsystem->CreateChannelEntity(TEXT("player_statistics"));
	UPubnubChannelEntity* GameEventsChannel = PubnubSubsystem->CreateChannelEntity(TEXT("game_events_feed"));
	UPubnubChannelGroupEntity* ServerStatusGroup = PubnubSubsystem->CreateChannelGroupEntity(TEXT("server_monitoring"));
	UPubnubChannelMetadataEntity* MatchConfigEntity = PubnubSubsystem->CreateChannelMetadataEntity(TEXT("match_configuration"));

	// Combine different entity types into a comprehensive monitoring subscription set
	TArray<UPubnubBaseEntity*> MonitoringEntities = {
		PlayerStatsChannel,      // Monitor player performance data
		GameEventsChannel,       // Monitor in-game events (kills, objectives, etc.)
		ServerStatusGroup,       // Monitor all server status channels
		MatchConfigEntity        // Monitor match configuration changes
	};

	// Create subscription set from existing entities - provides unified monitoring dashboard
	UPubnubSubscriptionSet* GameMonitoringSet = PubnubSubsystem->CreateSubscriptionSetFromEntities(MonitoringEntities);

	// Add message listener to capture all monitoring data
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	GameMonitoringSet->OnPubnubMessage.AddDynamic(this, &ASample_Entities::OnMessage_SubscriptionSetFromEntitiesSample);

	// Add object event listener to monitor configuration changes
	// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
	GameMonitoringSet->OnPubnubObjectEvent.AddDynamic(this, &ASample_Entities::OnMessage_SubscriptionSetFromEntitiesSample);

	// Subscribe to start comprehensive game monitoring
	GameMonitoringSet->Subscribe();
}

// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::OnMessage_SubscriptionSetFromEntitiesSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("GAME MONITORING - Message on %s: %s (from %s)"), 
		*Message.Channel, *Message.Message, *Message.UserID);
}

// snippet.subscription_set_add_remove_subscriptions
// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::SubscriptionSetAddRemoveSubscriptionsSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Match_Coordinator");
	PubnubSubsystem->SetUserID(UserID);

	// Create a subscription set for tournament management
	TArray<FString> TournamentChannels = {TEXT("tournament_lobby"), TEXT("match_results")};
	UPubnubSubscriptionSet* TournamentSet = PubnubSubsystem->CreateSubscriptionSet(TournamentChannels, TArray<FString>());

	// Create individual subscriptions for different game areas
	UPubnubChannelEntity* PlayerFeedbackChannel = PubnubSubsystem->CreateChannelEntity(TEXT("player_feedback"));
	UPubnubSubscription* FeedbackSubscription = PlayerFeedbackChannel->CreateSubscription();

	UPubnubChannelEntity* AdminNoticesChannel = PubnubSubsystem->CreateChannelEntity(TEXT("admin_notices"));
	UPubnubSubscription* AdminSubscription = AdminNoticesChannel->CreateSubscription();

	// Add individual subscriptions to the tournament set
	TournamentSet->AddSubscription(FeedbackSubscription);
	TournamentSet->AddSubscription(AdminSubscription);

	// Check current subscriptions in the set
	TArray<UPubnubSubscription*> CurrentSubscriptions = TournamentSet->GetSubscriptions();
	UE_LOG(LogTemp, Log, TEXT("Tournament set now contains %d subscriptions"), CurrentSubscriptions.Num());

	// Remove a subscription when no longer needed
	TournamentSet->RemoveSubscription(FeedbackSubscription);

	// Check subscriptions after removal
	TArray<UPubnubSubscription*> SubscriptionsAfterRemoval = TournamentSet->GetSubscriptions();
	UE_LOG(LogTemp, Log, TEXT("Tournament set now contains %d subscriptions after removal"), SubscriptionsAfterRemoval.Num());
}

// snippet.subscription_set_merge_operations
// ACTION REQUIRED: Replace ASample_Entities with name of your Actor class
void ASample_Entities::SubscriptionSetMergeOperationsSample()
{
	// Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Set UserID
	FString UserID = TEXT("Event_Manager");
	PubnubSubsystem->SetUserID(UserID);

	// Create main subscription set for core game channels
	TArray<FString> CoreChannels = {TEXT("game_lobby"), TEXT("general_chat")};
	UPubnubSubscriptionSet* CoreGameSet = PubnubSubsystem->CreateSubscriptionSet(CoreChannels, TArray<FString>());

	// Create additional subscription set for special events
	TArray<FString> EventChannels = {TEXT("special_events"), TEXT("tournament_updates")};
	UPubnubSubscriptionSet* SpecialEventsSet = PubnubSubsystem->CreateSubscriptionSet(EventChannels, TArray<FString>());

	// Create VIP subscription set for premium features
	TArray<FString> VipChannels = {TEXT("vip_lounge"), TEXT("premium_support")};
	UPubnubSubscriptionSet* VipSet = PubnubSubsystem->CreateSubscriptionSet(VipChannels, TArray<FString>());

	UE_LOG(LogTemp, Log, TEXT("Created core game set, special events set, and VIP set"));

	// Merge special events into core game monitoring
	CoreGameSet->AddSubscriptionSet(SpecialEventsSet);
	UE_LOG(LogTemp, Log, TEXT("Merged special events into core game monitoring"));

	// Merge VIP channels for comprehensive monitoring
	CoreGameSet->AddSubscriptionSet(VipSet);
	UE_LOG(LogTemp, Log, TEXT("Merged VIP channels for comprehensive event management"));

	// Later, remove special events if no longer needed
	CoreGameSet->RemoveSubscriptionSet(SpecialEventsSet);
	UE_LOG(LogTemp, Log, TEXT("Removed special events from monitoring (event ended)"));

	// Subscribe to the final combined set
	CoreGameSet->Subscribe();
	UE_LOG(LogTemp, Log, TEXT("Now monitoring core game channels + VIP channels"));
}

// snippet.end
