// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Samples/Entities/Sample_SubscriptionSet.h"
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
 * 
 * SUBSCRIPTION SET SAMPLES demonstrate the powerful subscription set functionality, which allows
 * managing multiple subscriptions as a single unit for complex monitoring and communication scenarios.
 */

// NOTE: Comments marked with `ACTION REQUIRED` indicate lines you must change/adjust.


//Internal function, don't copy it with the samples
void ASample_SubscriptionSet::RunSamples()
{
	Super::RunSamples();
	
	CreateSubscriptionSetFromNamesSample();
	CreateSubscriptionSetFromEntitiesSample();
	SubscriptionSetAddRemoveSubscriptionsSample();
	SubscriptionSetMergeOperationsSample();
}

//Internal function, don't copy it with the samples
ASample_SubscriptionSet::ASample_SubscriptionSet()
{
	SamplesName = "SubscriptionSet";
}


/* SUBSCRIPTION SET SAMPLE FUNCTIONS */

// snippet.create_subscription_set_from_names
// ACTION REQUIRED: Replace ASample_SubscriptionSet with name of your Actor class
void ASample_SubscriptionSet::CreateSubscriptionSetFromNamesSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

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
	// ACTION REQUIRED: Replace ASample_SubscriptionSet with name of your Actor class
	TeamCommSubscriptionSet->OnPubnubMessage.AddDynamic(this, &ASample_SubscriptionSet::OnMessage_SubscriptionSetFromNamesSample);

	// Subscribe to start monitoring all team channels and groups simultaneously
	TeamCommSubscriptionSet->SubscribeAsync();
}

// ACTION REQUIRED: Replace ASample_SubscriptionSet with name of your Actor class
void ASample_SubscriptionSet::OnMessage_SubscriptionSetFromNamesSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("TEAM COMMS - Message on %s: %s (from %s)"), 
		*Message.Channel, *Message.Message, *Message.UserID);
}

// snippet.create_subscription_set_from_entities
// ACTION REQUIRED: Replace ASample_SubscriptionSet with name of your Actor class
void ASample_SubscriptionSet::CreateSubscriptionSetFromEntitiesSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

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
	// ACTION REQUIRED: Replace ASample_SubscriptionSet with name of your Actor class
	GameMonitoringSet->OnPubnubMessage.AddDynamic(this, &ASample_SubscriptionSet::OnMessage_SubscriptionSetFromEntitiesSample);

	// Add object event listener to monitor configuration changes
	// ACTION REQUIRED: Replace ASample_SubscriptionSet with name of your Actor class
	GameMonitoringSet->OnPubnubObjectEvent.AddDynamic(this, &ASample_SubscriptionSet::OnMessage_SubscriptionSetFromEntitiesSample);

	// Subscribe to start comprehensive game monitoring
	GameMonitoringSet->SubscribeAsync();
}

// ACTION REQUIRED: Replace ASample_SubscriptionSet with name of your Actor class
void ASample_SubscriptionSet::OnMessage_SubscriptionSetFromEntitiesSample(FPubnubMessageData Message)
{
	UE_LOG(LogTemp, Log, TEXT("GAME MONITORING - Message on %s: %s (from %s)"), 
		*Message.Channel, *Message.Message, *Message.UserID);
}

// snippet.subscription_set_add_remove_subscriptions
// ACTION REQUIRED: Replace ASample_SubscriptionSet with name of your Actor class
void ASample_SubscriptionSet::SubscriptionSetAddRemoveSubscriptionsSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

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
// ACTION REQUIRED: Replace ASample_SubscriptionSet with name of your Actor class
void ASample_SubscriptionSet::SubscriptionSetMergeOperationsSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

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
	CoreGameSet->SubscribeAsync();
	UE_LOG(LogTemp, Log, TEXT("Now monitoring core game channels + VIP channels"));
}

// snippet.end

UPubnubClient* ASample_SubscriptionSet::GetPubnubClient()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	
	//Get default PubnubClient - created automatically if PluginSettings are set to do so
	UPubnubClient* PubnubClient = PubnubSubsystem->GetPubnubClient(0);
	
	PubnubClient->SetUserID(TEXT("player_001"));
	return PubnubClient;
}
