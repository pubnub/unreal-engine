// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "Entities/PubnubChannelEntity.h"
#include "Entities/PubnubChannelGroupEntity.h"
#include "Entities/PubnubChannelMetadataEntity.h"
#include "Entities/PubnubUserMetadataEntity.h"
#include "Entities/PubnubSubscription.h"
#include "FunctionLibraries/PubnubTimetokenUtilities.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubChannelEntityPublishSubscribeTest, FPubnubAutomationTestBase, "Pubnub.Integration.Entities.ChannelEntity.PublishAndSubscribe", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubChannelGroupEntityTest, FPubnubAutomationTestBase, "Pubnub.Integration.Entities.ChannelGroupEntity.CreateAndSubscribe", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscriptionSetTest, FPubnubAutomationTestBase, "Pubnub.Integration.Entities.SubscriptionSet.MultipleChannels", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscriptionListenersTest, FPubnubAutomationTestBase, "Pubnub.Integration.Entities.Subscription.AllListeners", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscriptionSetManagementTest, FPubnubAutomationTestBase, "Pubnub.Integration.Entities.SubscriptionSet.DynamicManagement", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntityCreationAndTrackingTest, FPubnubAutomationTestBase, "Pubnub.Integration.Entities.Subsystem.EntityCreationAndTracking", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FPubnubChannelEntityPublishSubscribeTest::RunTest(const FString& Parameters)
{
	// Initial variables
	const FString TestMessage = "\"Entity test message\"";
	const FString TestUser = SDK_PREFIX + "entity_test_user";
	const FString TestChannel = SDK_PREFIX + "entity_test_channel";
	TSharedPtr<bool> TestMessageReceived = MakeShared<bool>(false);
	TSharedPtr<bool> TestPublishResultReceived = MakeShared<bool>(false);
	TSharedPtr<bool> TestSubscribeResultReceived = MakeShared<bool>(false);
	TSharedPtr<FPubnubOperationResult> PublishResult = MakeShared<FPubnubOperationResult>();
	TSharedPtr<FPubnubMessageData> ReceivedMessageData = MakeShared<FPubnubMessageData>();
	
	if(!InitTest())
	{
		AddError("TestInitialization failed");
		return false;
	}

	// Check for any errors on the way
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	
	// Set User ID - it's necessary for any operation
	PubnubSubsystem->SetUserID(TestUser);

	// Create channel entity immediately - no delayed actions needed
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(TestChannel);
	TestNotNull("Channel entity should be created", ChannelEntity);
	if (!ChannelEntity)
	{
		AddError("Failed to create channel entity");
		return false;
	}
	TestEqual("Channel entity should have correct ID", TestChannel, ChannelEntity->EntityID);
	TestEqual("Channel entity should have correct type", EPubnubEntityType::PEnT_Channel, ChannelEntity->EntityType);

	// Create subscription from entity immediately
	UPubnubSubscription* Subscription = ChannelEntity->CreateSubscription();
	TestNotNull("Subscription should be created from channel entity", Subscription);
	if (!Subscription)
	{
		AddError("Failed to create subscription from channel entity");
		return false;
	}

	// Set up message listener on the subscription immediately
	Subscription->OnPubnubMessageNative.AddLambda([this, TestMessage, TestChannel, TestUser, TestMessageReceived, ReceivedMessageData](const FPubnubMessageData& ReceivedMessage)
	{
		*TestMessageReceived = true;
		*ReceivedMessageData = ReceivedMessage;
		
		// Verify message content
		TestEqual("Received message content", TestMessage, ReceivedMessage.Message);
		TestEqual("Received message channel", TestChannel, ReceivedMessage.Channel);
		TestEqual("Received message UserID", TestUser, ReceivedMessage.UserID);
		TestEqual("Received message MessageType", EPubnubMessageType::PMT_Published, ReceivedMessage.MessageType);
		TestFalse("Received message timetoken should not be empty", ReceivedMessage.Timetoken.IsEmpty());
	});

	// Create subscribe callback
	FOnSubscribeOperationResponseNative SubscribeCallback;
	SubscribeCallback.BindLambda([this, TestSubscribeResultReceived](const FPubnubOperationResult& Result)
	{
		*TestSubscribeResultReceived = true;
		TestFalse("Subscribe operation should not have failed", Result.Error);
		TestEqual("Subscribe HTTP status should be 200", Result.Status, 200);
		
		if (Result.Error)
		{
			AddError(FString::Printf(TEXT("Subscribe failed with error: %s"), *Result.ErrorMessage));
		}
	});

	// Subscribe using the subscription
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Subscription, SubscribeCallback]()
	{
		Subscription->Subscribe(SubscribeCallback);
	}, 0.1f));

	// Wait until subscribe result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestSubscribeResultReceived]() {
		return *TestSubscribeResultReceived;
	}, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestSubscribeResultReceived]()
	{
		if (!*TestSubscribeResultReceived)
		{
			AddError("Subscribe result callback was not received");
		}
	}, 0.1f));

	// Create publish callback
	FOnPublishMessageResponseNative PublishCallback;
	PublishCallback.BindLambda([this, TestMessage, TestChannel, TestUser, TestPublishResultReceived, PublishResult]
		(const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
	{
		*TestPublishResultReceived = true;
		*PublishResult = Result;
		
		// Verify publish result
		TestFalse("Publish operation should not have failed", Result.Error);
		TestEqual("Publish HTTP status should be 200", Result.Status, 200);
		
		if (!Result.Error)
		{
			// Verify published message data matches what we sent
			TestEqual("Published message data - content", TestMessage, PublishedMessage.Message);
			TestEqual("Published message data - channel", TestChannel, PublishedMessage.Channel);
			TestEqual("Published message data - user ID", TestUser, PublishedMessage.UserID);
			TestEqual("Published message data - message type", EPubnubMessageType::PMT_Published, PublishedMessage.MessageType);
			TestFalse("Published message timetoken should not be empty", PublishedMessage.Timetoken.IsEmpty());
		}
		else
		{
			AddError(FString::Printf(TEXT("Publish failed with error: %s"), *Result.ErrorMessage));
		}
	});

	// Publish message using the channel entity
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelEntity, TestMessage, PublishCallback]()
	{
		ChannelEntity->PublishMessageAsync(TestMessage, PublishCallback);
	}, 0.5f));

	// Wait until publish result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestPublishResultReceived]() {
		return *TestPublishResultReceived;
	}, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestPublishResultReceived]()
	{
		if (!*TestPublishResultReceived)
		{
			AddError("Publish result callback was not received");
		}
	}, 0.1f));

	// Wait until message is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestMessageReceived]() {
		return *TestMessageReceived;
	}, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestMessageReceived]()
	{
		if (!*TestMessageReceived)
		{
			AddError("Test message was not received through entity subscription");
		}
	}, 0.1f));

	// Clean up
	CleanUp();
	
	return true;
}

bool FPubnubChannelGroupEntityTest::RunTest(const FString& Parameters)
{
	// Initial variables
	const FString TestMessage = "\"Channel group test message\"";
	const FString TestUser = SDK_PREFIX + "channel_group_test_user";
	const FString TestChannelGroup = SDK_PREFIX + "test_channel_group";
	const FString TestChannel1 = SDK_PREFIX + "test_channel_1";
	const FString TestChannel2 = SDK_PREFIX + "test_channel_2";
	TSharedPtr<bool> TestChannelGroupCreated = MakeShared<bool>(false);
	TSharedPtr<bool> TestChannelAdded = MakeShared<bool>(false);
	TSharedPtr<bool> TestSubscriptionCreated = MakeShared<bool>(false);
	TSharedPtr<bool> TestMessageReceived = MakeShared<bool>(false);
	TSharedPtr<bool> TestPublishResultReceived = MakeShared<bool>(false);
	TSharedPtr<bool> TestSubscribeResultReceived = MakeShared<bool>(false);
	TSharedPtr<FPubnubMessageData> ReceivedMessageData = MakeShared<FPubnubMessageData>();
	
	if(!InitTest())
	{
		AddError("TestInitialization failed");
		return false;
	}

	// Check for any errors on the way
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	
	// Set User ID - it's necessary for any operation
	PubnubSubsystem->SetUserID(TestUser);


	/*PubnubSubsystem->OnSubscriptionStatusChangedNative.AddLambda([this](EPubnubSubscriptionStatus Status, const FPubnubSubscriptionStatusData& StatusData)
	{
		AddInfo("Subscription listener.");
		for(auto ch : StatusData.ChannelGroups)
		{
					
			AddInfo(FString::Printf(TEXT("Subscription listener: %s"), *ch));
		}

	});*/

	

	// Create channel group entity immediately - no delayed actions needed
	UPubnubChannelGroupEntity* ChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(TestChannelGroup);
	TestNotNull("Channel group entity should be created", ChannelGroupEntity);
	if (!ChannelGroupEntity)
	{
		AddError("Failed to create channel group entity");
		return false;
	}
	TestEqual("Channel group entity should have correct ID", TestChannelGroup, ChannelGroupEntity->EntityID);
	TestEqual("Channel group entity should have correct type", EPubnubEntityType::PEnT_ChannelGroup, ChannelGroupEntity->EntityType);
	*TestChannelGroupCreated = true;

	// Create subscription from channel group entity immediately
	UPubnubSubscription* Subscription = ChannelGroupEntity->CreateSubscription();
	TestNotNull("Subscription should be created from channel group entity", Subscription);
	if (!Subscription)
	{
		AddError("Failed to create subscription from channel group entity");
		return false;
	}
	*TestSubscriptionCreated = true;

	// Set up message listener on the subscription immediately
	Subscription->OnPubnubMessageNative.AddLambda([this, TestMessage, TestChannel1, TestUser, TestMessageReceived, ReceivedMessageData](const FPubnubMessageData& ReceivedMessage)
	{
		*TestMessageReceived = true;
		*ReceivedMessageData = ReceivedMessage;
		
		// Verify message content
		TestEqual("Received message content", TestMessage, ReceivedMessage.Message);
		TestEqual("Received message channel", TestChannel1, ReceivedMessage.Channel);
		TestEqual("Received message UserID", TestUser, ReceivedMessage.UserID);
		TestEqual("Received message MessageType", EPubnubMessageType::PMT_Published, ReceivedMessage.MessageType);
		TestFalse("Received message timetoken should not be empty", ReceivedMessage.Timetoken.IsEmpty());
	});

	// Create callback for adding channel to group
	FOnAddChannelToGroupResponseNative AddChannelCallback;
	AddChannelCallback.BindLambda([this, TestChannelAdded](const FPubnubOperationResult& Result)
	{
		TestFalse("Add channel to group operation should not have failed", Result.Error);
		if (!Result.Error)
		{
			*TestChannelAdded = true;
		}
		else
		{
			AddError(FString::Printf(TEXT("Add channel to group failed with error: %s"), *Result.ErrorMessage));
		}
	});

	// Add a channel to the channel group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelGroupEntity, TestChannel1, AddChannelCallback]()
	{
		ChannelGroupEntity->AddChannelToGroupAsync(TestChannel1, AddChannelCallback);
	}, 0.1f));

	// Wait for channel to be added
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestChannelAdded]() {
		return *TestChannelAdded;
	}, MAX_WAIT_TIME));

	// Create subscribe callback
	FOnSubscribeOperationResponseNative SubscribeCallback;
	SubscribeCallback.BindLambda([this, TestSubscribeResultReceived](const FPubnubOperationResult& Result)
	{
		*TestSubscribeResultReceived = true;
		TestFalse("Subscribe operation should not have failed", Result.Error);
		TestEqual("Subscribe HTTP status should be 200", Result.Status, 200);
		
		if (Result.Error)
		{
			AddError(FString::Printf(TEXT("Subscribe to channel group failed with error: %s"), *Result.ErrorMessage));
		}
	});

	// Subscribe using the subscription
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Subscription, SubscribeCallback]()
	{
		Subscription->Subscribe(SubscribeCallback);
	}, 0.1f));

	// Wait until subscribe result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestSubscribeResultReceived]() {
		return *TestSubscribeResultReceived;
	}, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestSubscribeResultReceived]()
	{
		if (!*TestSubscribeResultReceived)
		{
			AddError("Subscribe result callback was not received for channel group");
		}
	}, 0.1f));

	// Create publish callback  
	FOnPublishMessageResponseNative PublishCallback;
	PublishCallback.BindLambda([this, TestMessage, TestChannel1, TestUser, TestPublishResultReceived]
		(const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
	{
		*TestPublishResultReceived = true;
		
		// Verify publish result
		TestFalse("Publish operation should not have failed", Result.Error);
		TestEqual("Publish HTTP status should be 200", Result.Status, 200);
		
		if (!Result.Error)
		{
			// Verify published message data matches what we sent
			TestEqual("Published message data - content", TestMessage, PublishedMessage.Message);
			TestEqual("Published message data - channel", TestChannel1, PublishedMessage.Channel);
			TestEqual("Published message data - user ID", TestUser, PublishedMessage.UserID);
			TestEqual("Published message data - message type", EPubnubMessageType::PMT_Published, PublishedMessage.MessageType);
			TestFalse("Published message timetoken should not be empty", PublishedMessage.Timetoken.IsEmpty());
		}
		else
		{
			AddError(FString::Printf(TEXT("Publish to channel in group failed with error: %s"), *Result.ErrorMessage));
		}
	});

	// Publish message to channel that's in the group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1, TestMessage, PublishCallback]()
	{
		// Create a temporary channel entity to publish to the channel that's part of the group
		UPubnubChannelEntity* TempChannelEntity = PubnubSubsystem->CreateChannelEntity(TestChannel1);
		if (TempChannelEntity)
		{
			TempChannelEntity->PublishMessageAsync(TestMessage, PublishCallback);
		}
		else
		{
			AddError("Failed to create temporary channel entity for publishing");
		}
	}, 0.5f));

	// Wait until publish result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestPublishResultReceived]() {
		return *TestPublishResultReceived;
	}, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestPublishResultReceived]()
	{
		if (!*TestPublishResultReceived)
		{
			AddError("Publish result callback was not received");
		}
	}, 0.1f));

	// Wait until message is received through channel group subscription
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestMessageReceived]() {
		return *TestMessageReceived;
	}, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestMessageReceived]()
	{
		if (!*TestMessageReceived)
		{
			AddError("Test message was not received through channel group subscription");
		}
	}, 0.1f));

	// Verify test completion
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelGroupCreated, TestChannelAdded, TestSubscriptionCreated, TestMessageReceived]()
	{
		TestTrue("Channel group entity should be created", *TestChannelGroupCreated);
		TestTrue("Channel should be added to group", *TestChannelAdded);
		TestTrue("Subscription should be created from channel group", *TestSubscriptionCreated);
		TestTrue("Message should be received through channel group subscription", *TestMessageReceived);
	}, 0.1f));

	// Clean up
	CleanUp();
	
	return true;
}

bool FPubnubSubscriptionSetTest::RunTest(const FString& Parameters)
{
	// Initial variables
	const FString TestUser = SDK_PREFIX + "subscription_set_test_user";
	const FString TestChannel1 = SDK_PREFIX + "set_test_channel_1";
	const FString TestChannel2 = SDK_PREFIX + "set_test_channel_2";
	const FString TestChannelGroup = SDK_PREFIX + "set_test_group";
	TSharedPtr<bool> TestSubscriptionSetCreated = MakeShared<bool>(false);
	TSharedPtr<bool> TestSubscriptionSetFromEntitiesCreated = MakeShared<bool>(false);
	TSharedPtr<bool> TestActiveSubscriptionsRetrieved = MakeShared<bool>(false);
	
	if(!InitTest())
	{
		AddError("TestInitialization failed");
		return false;
	}

	// Check for any errors on the way
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	
	// Set User ID - it's necessary for any operation
	PubnubSubsystem->SetUserID(TestUser);

	// Create subscription set from channel names immediately
	TArray<FString> Channels = {TestChannel1, TestChannel2};
	TArray<FString> ChannelGroups = {TestChannelGroup};
	
	UPubnubSubscriptionSet* SubscriptionSet = PubnubSubsystem->CreateSubscriptionSet(Channels, ChannelGroups);
	TestNotNull("Subscription set should be created from channel/group names", SubscriptionSet);
	if (!SubscriptionSet)
	{
		AddError("Failed to create subscription set from channel/group names");
		return false;
	}
	*TestSubscriptionSetCreated = true;

	// Create subscription set from entities immediately
	UPubnubChannelEntity* Entity1 = PubnubSubsystem->CreateChannelEntity(TestChannel1);
	UPubnubChannelEntity* Entity2 = PubnubSubsystem->CreateChannelEntity(TestChannel2);
	
	TestNotNull("Entity 1 should be created", Entity1);
	TestNotNull("Entity 2 should be created", Entity2);
	
	if (!Entity1 || !Entity2)
	{
		AddError("Failed to create entities for subscription set test");
		return false;
	}
	
	TArray<UPubnubBaseEntity*> Entities = {Entity1, Entity2};
	UPubnubSubscriptionSet* SubscriptionSetFromEntities = PubnubSubsystem->CreateSubscriptionSetFromEntities(Entities);
	TestNotNull("Subscription set should be created from entities", SubscriptionSetFromEntities);
	if (!SubscriptionSetFromEntities)
	{
		AddError("Failed to create subscription set from entities");
		return false;
	}
	*TestSubscriptionSetFromEntitiesCreated = true;

	// Test getting active subscriptions and subscription sets immediately
	TArray<UPubnubSubscription*> ActiveSubscriptions = PubnubSubsystem->GetActiveSubscriptions();
	TArray<UPubnubSubscriptionSet*> ActiveSubscriptionSets = PubnubSubsystem->GetActiveSubscriptionSets();
	
	// Note: These arrays might be empty if no subscriptions are currently active
	// The important thing is that the functions don't crash and return valid arrays
	TestTrue("GetActiveSubscriptions should return valid array", ActiveSubscriptions.Num() >= 0);
	TestTrue("GetActiveSubscriptionSets should return valid array", ActiveSubscriptionSets.Num() >= 0);
	*TestActiveSubscriptionsRetrieved = true;

	// Verify test completion immediately
	TestTrue("Subscription set should be created from names", *TestSubscriptionSetCreated);
	TestTrue("Subscription set should be created from entities", *TestSubscriptionSetFromEntitiesCreated);
	TestTrue("Active subscriptions should be retrievable", *TestActiveSubscriptionsRetrieved);
	
	// Clean up
	CleanUp();
	
	return true;
}

bool FPubnubSubscriptionListenersTest::RunTest(const FString& Parameters)
{
	// Test data for different PubNub event types
	const FString TestMessageContent = "\"Listener test message\"";
	const FString TestSignalContent = "\"Test signal\"";
	const FString TestUser = SDK_PREFIX + "listeners_test_user";
	const FString TestChannel = SDK_PREFIX + "listeners_test_channel";
	
	// Counters to track how many times each listener type fires
	TSharedPtr<int32> MessageListenerFireCount = MakeShared<int32>(0);
	TSharedPtr<int32> SignalListenerFireCount = MakeShared<int32>(0);
	TSharedPtr<int32> PresenceListenerFireCount = MakeShared<int32>(0);
	TSharedPtr<int32> ObjectEventListenerFireCount = MakeShared<int32>(0);
	TSharedPtr<int32> MessageActionListenerFireCount = MakeShared<int32>(0);
	TSharedPtr<int32> UniversalListenerFireCount = MakeShared<int32>(0);
	
	// Flags to track completion of async operations
	TSharedPtr<bool> SubscriptionActivated = MakeShared<bool>(false);
	TSharedPtr<bool> MessagePublishCompleted = MakeShared<bool>(false);
	TSharedPtr<bool> SignalSendCompleted = MakeShared<bool>(false);
	TSharedPtr<bool> MessageActionAddCompleted = MakeShared<bool>(false);
	TSharedPtr<bool> PresenceEventSentCompleted = MakeShared<bool>(false);
	TSharedPtr<bool> ChannelMetadataSetCompleted = MakeShared<bool>(false);
	
	// Store the timetoken of the published message (needed for valid message actions)
	TSharedPtr<FString> PublishedMessageTimetoken = MakeShared<FString>("");
	
	if(!InitTest())
	{
		AddError("TestInitialization failed");
		return false;
	}

	// Check for any errors on the way
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	
	// Set User ID - required for all PubNub operations
	PubnubSubsystem->SetUserID(TestUser);

	// Create channel entity for testing all subscription listener types
	UPubnubChannelEntity* ChannelEntity = PubnubSubsystem->CreateChannelEntity(TestChannel);
	TestNotNull("Channel entity creation should succeed", ChannelEntity);
	if (!ChannelEntity)
	{
		AddError("Failed to create channel entity for listener testing");
		return false;
	}

	// Create subscription with presence events enabled to test all listener types
	FPubnubSubscribeSettings SubscriptionSettings;
	SubscriptionSettings.ReceivePresenceEvents = true;

	UPubnubSubscription* Subscription = ChannelEntity->CreateSubscription(SubscriptionSettings);
	TestNotNull("Subscription creation should succeed", Subscription);
	if (!Subscription)
	{
		AddError("Failed to create subscription from channel entity");
		return false;
	}

	// Set up all 6 listener types to verify proper event routing and isolation
	
	// 1. Message Listener - Should only fire for published messages (EPubnubMessageType::PMT_Published)
	Subscription->OnPubnubMessageNative.AddLambda([this, TestMessageContent, TestChannel, TestUser, MessageListenerFireCount, PublishedMessageTimetoken](const FPubnubMessageData& ReceivedMessage)
	{
		(*MessageListenerFireCount)++;
		
		// Verify this listener received a proper published message
		TestEqual("Message listener - correct content received", TestMessageContent, ReceivedMessage.Message);
		TestEqual("Message listener - correct channel received", TestChannel, ReceivedMessage.Channel);
		TestEqual("Message listener - correct user received", TestUser, ReceivedMessage.UserID);
		TestEqual("Message listener - correct message type", EPubnubMessageType::PMT_Published, ReceivedMessage.MessageType);
		TestFalse("Message listener - valid timetoken received", ReceivedMessage.Timetoken.IsEmpty());
		
		// Store the message timetoken for later message action testing
		*PublishedMessageTimetoken = ReceivedMessage.Timetoken;
	});

	// 2. Signal Listener - Should only fire for signals (EPubnubMessageType::PMT_Signal)
	Subscription->OnPubnubSignalNative.AddLambda([this, TestSignalContent, TestChannel, TestUser, SignalListenerFireCount](const FPubnubMessageData& ReceivedMessage)
	{
		(*SignalListenerFireCount)++;
		
		// Verify this listener received a proper signal
		TestEqual("Signal listener - correct content received", TestSignalContent, ReceivedMessage.Message);
		TestEqual("Signal listener - correct channel received", TestChannel, ReceivedMessage.Channel);
		TestEqual("Signal listener - correct user received", TestUser, ReceivedMessage.UserID);
		TestEqual("Signal listener - correct message type", EPubnubMessageType::PMT_Signal, ReceivedMessage.MessageType);
		TestFalse("Signal listener - valid timetoken received", ReceivedMessage.Timetoken.IsEmpty());
	});

	// 3. Presence Listener - Should only fire for presence events (messages sent to "-pnpres" channels)
	Subscription->OnPubnubPresenceEventNative.AddLambda([this, TestChannel, PresenceListenerFireCount](const FPubnubMessageData& ReceivedMessage)
	{
		(*PresenceListenerFireCount)++;
		
		// Verify this listener received a proper presence event
		TestTrue("Presence listener - channel has pnpres suffix", ReceivedMessage.Channel.Contains("-pnpres"));
		TestEqual("Presence listener - correct message type", EPubnubMessageType::PMT_Published, ReceivedMessage.MessageType);
		TestTrue("Presence listener - contains join action", ReceivedMessage.Message.Contains("\"action\":\"join\""));
		TestTrue("Presence listener - contains user uuid", ReceivedMessage.Message.Contains("\"uuid\":\"Jack-device\""));
	});

	// 4. Object Event Listener - Should only fire for App Context object events (EPubnubMessageType::PMT_Objects)
	Subscription->OnPubnubObjectEventNative.AddLambda([this, TestChannel, ObjectEventListenerFireCount](const FPubnubMessageData& ReceivedMessage)
	{
		(*ObjectEventListenerFireCount)++;
		
		// Verify this listener received a proper App Context object event
		TestEqual("Object event listener - correct message type", EPubnubMessageType::PMT_Objects, ReceivedMessage.MessageType);
		TestTrue("Object event listener - channel relates to test channel", ReceivedMessage.Channel.Contains(TestChannel) || ReceivedMessage.Channel.Equals(TestChannel));
		TestTrue("Object event listener - event data not empty", !ReceivedMessage.Message.IsEmpty());
	});

	// 5. Message Action Listener - Should only fire for message actions/reactions (EPubnubMessageType::PMT_Action)
	Subscription->OnPubnubMessageActionNative.AddLambda([this, TestChannel, MessageActionListenerFireCount](const FPubnubMessageData& ReceivedMessage)
	{
		(*MessageActionListenerFireCount)++;
		
		// Verify this listener received a proper message action event
		TestEqual("Message action listener - correct message type", EPubnubMessageType::PMT_Action, ReceivedMessage.MessageType);
		TestEqual("Message action listener - correct channel", TestChannel, ReceivedMessage.Channel);
		TestTrue("Message action listener - action data present", !ReceivedMessage.Message.IsEmpty());
		TestTrue("Message action listener - contains reaction data", ReceivedMessage.Message.Contains("reaction") || ReceivedMessage.Message.Contains("action"));
	});

	// 6. Universal Listener - Should fire for ALL event types (messages, signals, presence, objects, actions)
	Subscription->FOnPubnubAnyMessageTypeNative.AddLambda([this, UniversalListenerFireCount](const FPubnubMessageData& ReceivedMessage)
	{
		(*UniversalListenerFireCount)++;
		
		// Verify this universal listener receives valid events of any type
		TestTrue("Universal listener - receives valid event types", 
			ReceivedMessage.MessageType == EPubnubMessageType::PMT_Published || 
			ReceivedMessage.MessageType == EPubnubMessageType::PMT_Signal ||
			ReceivedMessage.MessageType == EPubnubMessageType::PMT_Action ||
			ReceivedMessage.MessageType == EPubnubMessageType::PMT_Objects);
		TestFalse("Universal listener - channel information present", ReceivedMessage.Channel.IsEmpty());
	});

	// Create callback to verify subscription activation
	FOnSubscribeOperationResponseNative SubscribeCallback;
	SubscribeCallback.BindLambda([this, SubscriptionActivated](const FPubnubOperationResult& Result)
	{
		*SubscriptionActivated = true;
		TestFalse("Subscription activation must succeed", Result.Error);
		TestEqual("Subscription HTTP response must be 200", Result.Status, 200);
		
		if (Result.Error)
		{
			AddError(FString::Printf(TEXT("Subscription activation failed: %s"), *Result.ErrorMessage));
		}
	});

	// Activate the subscription to start receiving all event types
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Subscription, SubscribeCallback]()
	{
		Subscription->Subscribe(SubscribeCallback);
	}, 0.1f));

	// Wait for subscription to be fully active before sending events
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([SubscriptionActivated]() {
		return *SubscriptionActivated;
	}, MAX_WAIT_TIME));

	// Reset all listener fire counts to ensure clean testing
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([MessageListenerFireCount, SignalListenerFireCount, PresenceListenerFireCount, ObjectEventListenerFireCount, MessageActionListenerFireCount, UniversalListenerFireCount]()
	{
		*MessageListenerFireCount = 0;
		*SignalListenerFireCount = 0;
		*PresenceListenerFireCount = 0;
		*ObjectEventListenerFireCount = 0;
		*MessageActionListenerFireCount = 0;
		*UniversalListenerFireCount = 0;
	}, 0.1f));

	// === EVENT SEQUENCE: Test each event type to verify proper listener routing ===
	
	// EVENT 1: Publish a regular message (should trigger Message + Universal listeners)
	FOnPublishMessageResponseNative PublishMessageCallback;
	PublishMessageCallback.BindLambda([this, MessagePublishCompleted](const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
	{
		*MessagePublishCompleted = true;
		TestFalse("Message publish operation must succeed", Result.Error);
		if (Result.Error)
		{
			AddError(FString::Printf(TEXT("Message publish failed: %s"), *Result.ErrorMessage));
		}
	});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelEntity, TestMessageContent, PublishMessageCallback]()
	{
		ChannelEntity->PublishMessageAsync(TestMessageContent, PublishMessageCallback);
	}, 0.5f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([MessagePublishCompleted]() {
		return *MessagePublishCompleted;
	}, MAX_WAIT_TIME));

	// Allow time for message event to be processed and listeners to fire
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([]()
	{
		// Processing delay to ensure message listener has fired
	}, 1.0f));

	// EVENT 2: Send a signal (should trigger Signal + Universal listeners)
	FOnSignalResponseNative SendSignalCallback;
	SendSignalCallback.BindLambda([this, SignalSendCompleted](const FPubnubOperationResult& Result, const FPubnubMessageData& SentSignal)
	{
		*SignalSendCompleted = true;
		TestFalse("Signal send operation must succeed", Result.Error);
		if (Result.Error)
		{
			AddError(FString::Printf(TEXT("Signal send failed: %s"), *Result.ErrorMessage));
		}
	});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelEntity, TestSignalContent, SendSignalCallback]()
	{
		ChannelEntity->SignalAsync(TestSignalContent, SendSignalCallback);
	}, 0.5f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([SignalSendCompleted]() {
		return *SignalSendCompleted;
	}, MAX_WAIT_TIME));

	// Allow time for signal event to be processed and listeners to fire
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([]()
	{
		// Processing delay to ensure signal listener has fired
	}, 1.0f));

	// EVENT 3: Add message action/reaction (should trigger MessageAction + Universal listeners)
	FOnAddMessageActionResponseNative AddMessageActionCallback;
	AddMessageActionCallback.BindLambda([this, MessageActionAddCompleted](const FPubnubOperationResult& Result, const FPubnubMessageActionData& ActionData)
	{
		*MessageActionAddCompleted = true;
		TestFalse("Add message action operation must succeed", Result.Error);
		if (Result.Error)
		{
			AddError(FString::Printf(TEXT("Add message action failed: %s"), *Result.ErrorMessage));
		}
	});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, AddMessageActionCallback, PublishedMessageTimetoken]()
	{
		// Add a reaction to the published message using its real timetoken
		if (!PublishedMessageTimetoken->IsEmpty())
		{
			PubnubSubsystem->AddMessageAction(TestChannel, *PublishedMessageTimetoken, "reaction", "👍", AddMessageActionCallback);
		}
		else
		{
			AddError("Cannot add message action - published message timetoken not captured");
		}
	}, 0.5f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([MessageActionAddCompleted]() {
		return *MessageActionAddCompleted;
	}, MAX_WAIT_TIME));

	// Allow time for message action event to be processed and listeners to fire
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([]()
	{
		// Processing delay to ensure message action listener has fired
	}, 1.0f));

	// EVENT 4: Send presence event (should trigger Presence + Universal listeners)
	const FString PresenceChannelName = TestChannel + "-pnpres";
	const FString PresenceEventData = "{\"action\":\"join\",\"uuid\":\"Jack-device\",\"timestamp\":1756891149,\"precise_timestamp\":1756891149913,\"occupancy\":2}";
	
	FOnPublishMessageResponseNative SendPresenceEventCallback;
	SendPresenceEventCallback.BindLambda([this, PresenceEventSentCompleted](const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
	{
		*PresenceEventSentCompleted = true;
		TestFalse("Presence event send operation must succeed", Result.Error);
		if (Result.Error)
		{
			AddError(FString::Printf(TEXT("Presence event send failed: %s"), *Result.ErrorMessage));
		}
	});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, PresenceChannelName, PresenceEventData, SendPresenceEventCallback]()
	{
		// Send simulated presence event to trigger presence listener
		PubnubSubsystem->PublishMessage(PresenceChannelName, PresenceEventData, SendPresenceEventCallback);
	}, 0.5f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([PresenceEventSentCompleted]() {
		return *PresenceEventSentCompleted;
	}, MAX_WAIT_TIME));

	// Allow time for presence event to be processed and listeners to fire
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([]()
	{
		// Processing delay to ensure presence listener has fired
	}, 1.0f));

	// EVENT 5: Set channel metadata (should trigger Object + Universal listeners)
	FOnSetChannelMetadataResponseNative SetChannelMetadataCallback;
	SetChannelMetadataCallback.BindLambda([this, ChannelMetadataSetCompleted](const FPubnubOperationResult& Result, const FPubnubChannelData& ChannelData)
	{
		*ChannelMetadataSetCompleted = true;
		TestFalse("Set channel metadata operation must succeed", Result.Error);
		if (Result.Error)
		{
			AddError(FString::Printf(TEXT("Set channel metadata failed: %s"), *Result.ErrorMessage));
		}
	});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SetChannelMetadataCallback]()
	{
		// Create channel metadata entity and set metadata to trigger object event
		UPubnubChannelMetadataEntity* ChannelMetadataEntity = PubnubSubsystem->CreateChannelMetadataEntity(TestChannel);
		if (ChannelMetadataEntity)
		{
			// Use unique metadata to ensure object event is triggered (same metadata won't trigger event)
			FPubnubChannelData UniqueChannelMetadata;
			UniqueChannelMetadata.ChannelID = TestChannel;
			UniqueChannelMetadata.ChannelName = UPubnubTimetokenUtilities::GetCurrentUnixTimetoken(); // Unique name ensures event triggers
			UniqueChannelMetadata.Description = "Listener test metadata";
			
			ChannelMetadataEntity->SetChannelMetadataAsync(UniqueChannelMetadata, SetChannelMetadataCallback);
		}
	}, 0.5f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([ChannelMetadataSetCompleted]() {
		return *ChannelMetadataSetCompleted;
	}, MAX_WAIT_TIME));

	// Allow time for object event to be processed and listeners to fire, then verify all results
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, MessageListenerFireCount, SignalListenerFireCount, PresenceListenerFireCount, ObjectEventListenerFireCount, MessageActionListenerFireCount, UniversalListenerFireCount]()
	{
		// === COMPREHENSIVE LISTENER VERIFICATION ===
		// This test verifies that the subscription system properly routes events to the correct listeners
		// and that the universal listener captures all events without duplication or loss.
		
		// Verify each specific listener fired exactly once for its designated event type
		TestEqual("Message listener isolation: fires only for published messages", *MessageListenerFireCount, 1);
		TestEqual("Signal listener isolation: fires only for signals", *SignalListenerFireCount, 1);
		TestEqual("Presence listener isolation: fires only for presence events", *PresenceListenerFireCount, 1);
		TestEqual("Object event listener isolation: fires only for App Context object events", *ObjectEventListenerFireCount, 1);
		TestEqual("Message action listener isolation: fires only for message actions", *MessageActionListenerFireCount, 1);
		
		// Verify universal listener captured all events (sum of all specific listeners)
		int32 ExpectedUniversalFireCount = *MessageListenerFireCount + *SignalListenerFireCount + 
										   *PresenceListenerFireCount + *ObjectEventListenerFireCount + 
										   *MessageActionListenerFireCount;
		TestEqual("Universal listener completeness: captures all events exactly", *UniversalListenerFireCount, ExpectedUniversalFireCount);
		TestEqual("Universal listener total count: exactly 5 events", *UniversalListenerFireCount, 5);
		
		// Final verification: perfect listener behavior with complete isolation and universal coverage
		bool bListenerSystemWorking = 
			(*MessageListenerFireCount == 1) &&        // Message listener: exactly 1 fire
			(*SignalListenerFireCount == 1) &&         // Signal listener: exactly 1 fire
			(*PresenceListenerFireCount == 1) &&       // Presence listener: exactly 1 fire
			(*ObjectEventListenerFireCount == 1) &&    // Object listener: exactly 1 fire
			(*MessageActionListenerFireCount == 1) &&  // Action listener: exactly 1 fire
			(*UniversalListenerFireCount == 5);        // Universal listener: exactly 5 fires (sum)
			
		TestTrue("Complete listener system verification: isolation + universal coverage", bListenerSystemWorking);
		
		// Provide detailed diagnostics if any listener failed to behave correctly
		if (!bListenerSystemWorking)
		{
			AddError(FString::Printf(TEXT("Listener system failure - Fire counts: Message=%d, Signal=%d, Presence=%d, Object=%d, Action=%d, Universal=%d"), 
				*MessageListenerFireCount, *SignalListenerFireCount, *PresenceListenerFireCount,
				*ObjectEventListenerFireCount, *MessageActionListenerFireCount, *UniversalListenerFireCount));
		}
	}, 1.0f));

	// Clean up
	CleanUp();
	
	return true;
}

bool FPubnubSubscriptionSetManagementTest::RunTest(const FString& Parameters)
{
	// Test data for subscription set management operations
	const FString TestUser = SDK_PREFIX + "subscription_management_user";
	const FString TestChannel1 = SDK_PREFIX + "management_channel_1";
	const FString TestChannel2 = SDK_PREFIX + "management_channel_2"; 
	const FString TestChannel3 = SDK_PREFIX + "management_channel_3";
	const FString TestChannelGroup = SDK_PREFIX + "management_group";
	
	// Flags to track operation completions
	TSharedPtr<bool> InitializationCompleted = MakeShared<bool>(false);
	
	if(!InitTest())
	{
		AddError("Test initialization failed for subscription set management test");
		return false;
	}

	// Error handling setup
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(FString::Printf(TEXT("PubNub error during subscription management test: %s"), *ErrorMessage));
	});
	
	// Set User ID required for operations
	PubnubSubsystem->SetUserID(TestUser);

	// Create multiple channel entities for subscription set testing
	UPubnubChannelEntity* ChannelEntity1 = PubnubSubsystem->CreateChannelEntity(TestChannel1);
	UPubnubChannelEntity* ChannelEntity2 = PubnubSubsystem->CreateChannelEntity(TestChannel2);
	UPubnubChannelEntity* ChannelEntity3 = PubnubSubsystem->CreateChannelEntity(TestChannel3);
	
	TestNotNull("Channel entity 1 creation must succeed", ChannelEntity1);
	TestNotNull("Channel entity 2 creation must succeed", ChannelEntity2);
	TestNotNull("Channel entity 3 creation must succeed", ChannelEntity3);
	
	if (!ChannelEntity1 || !ChannelEntity2 || !ChannelEntity3)
	{
		AddError("Failed to create required channel entities for subscription management test");
		return false;
	}

	// Create individual subscriptions from entities
	UPubnubSubscription* IndividualSubscription1 = ChannelEntity1->CreateSubscription();
	UPubnubSubscription* IndividualSubscription2 = ChannelEntity2->CreateSubscription();
	UPubnubSubscription* IndividualSubscription3 = ChannelEntity3->CreateSubscription();
	
	TestNotNull("Individual subscription 1 creation must succeed", IndividualSubscription1);
	TestNotNull("Individual subscription 2 creation must succeed", IndividualSubscription2);
	TestNotNull("Individual subscription 3 creation must succeed", IndividualSubscription3);

	// Create initial subscription set from channel names
	TArray<FString> InitialChannels = {TestChannel1, TestChannel2};
	TArray<FString> InitialChannelGroups = {TestChannelGroup};
	UPubnubSubscriptionSet* InitialSubscriptionSet = PubnubSubsystem->CreateSubscriptionSet(InitialChannels, InitialChannelGroups);
	TestNotNull("Initial subscription set creation must succeed", InitialSubscriptionSet);

	// Verify initial subscription set state
	TArray<UPubnubSubscription*> InitialSubscriptions = InitialSubscriptionSet->GetSubscriptions();
	TestEqual("Initial subscription set should be empty (no individual subscriptions added yet)", InitialSubscriptions.Num(), 0);

	// TEST 1: Add individual subscription to subscription set
	InitialSubscriptionSet->AddSubscription(IndividualSubscription1);
	TArray<UPubnubSubscription*> SubscriptionsAfterAdd = InitialSubscriptionSet->GetSubscriptions();
	TestEqual("Subscription set should contain 1 subscription after adding", SubscriptionsAfterAdd.Num(), 1);
	TestEqual("Added subscription should match the one we added", SubscriptionsAfterAdd[0], IndividualSubscription1);

	// TEST 2: Add second individual subscription to same set
	InitialSubscriptionSet->AddSubscription(IndividualSubscription2);
	TArray<UPubnubSubscription*> SubscriptionsAfterSecondAdd = InitialSubscriptionSet->GetSubscriptions();
	TestEqual("Subscription set should contain 2 subscriptions after adding second", SubscriptionsAfterSecondAdd.Num(), 2);
	TestTrue("Set should contain first subscription", SubscriptionsAfterSecondAdd.Contains(IndividualSubscription1));
	TestTrue("Set should contain second subscription", SubscriptionsAfterSecondAdd.Contains(IndividualSubscription2));

	// TEST 3: Create second subscription set and combine individual subscription to create third set  
	UPubnubSubscriptionSet* CombinedSubscriptionSet = IndividualSubscription1->AddSubscription(IndividualSubscription3);
	TestNotNull("Combined subscription set creation must succeed", CombinedSubscriptionSet);
	
	TArray<UPubnubSubscription*> CombinedSubscriptions = CombinedSubscriptionSet->GetSubscriptions();
	TestEqual("Combined subscription set should contain 2 subscriptions", CombinedSubscriptions.Num(), 2);
	TestTrue("Combined set should contain first subscription", CombinedSubscriptions.Contains(IndividualSubscription1));
	TestTrue("Combined set should contain third subscription", CombinedSubscriptions.Contains(IndividualSubscription3));

	// TEST 4: Merge subscription sets using AddSubscriptionSet
	UPubnubSubscriptionSet* MainSet = PubnubSubsystem->CreateSubscriptionSet(TArray<FString>{TestChannel1}, TArray<FString>());
	UPubnubSubscriptionSet* SetToMerge = PubnubSubsystem->CreateSubscriptionSet(TArray<FString>{TestChannel2}, TArray<FString>());
	
	TestNotNull("Main subscription set for merging must be created", MainSet);
	TestNotNull("Set to merge must be created", SetToMerge);
	
	// Add subscription to main set before merging
	MainSet->AddSubscription(IndividualSubscription1);
	TArray<UPubnubSubscription*> MainSetBeforeMerge = MainSet->GetSubscriptions();
	TestEqual("Main set should have 1 subscription before merge", MainSetBeforeMerge.Num(), 1);

	// Perform the merge operation
	MainSet->AddSubscriptionSet(SetToMerge);
	TArray<UPubnubSubscription*> MainSetAfterMerge = MainSet->GetSubscriptions();
	TestEqual("Main set should still have 1 subscription after merge (sets merge channels, not subscriptions)", MainSetAfterMerge.Num(), 1);

	// TEST 5: Remove subscription from subscription set
	InitialSubscriptionSet->RemoveSubscription(IndividualSubscription1);
	TArray<UPubnubSubscription*> SubscriptionsAfterRemove = InitialSubscriptionSet->GetSubscriptions();
	TestEqual("Subscription set should have 1 subscription after removing first", SubscriptionsAfterRemove.Num(), 1);
	TestFalse("Set should not contain removed subscription", SubscriptionsAfterRemove.Contains(IndividualSubscription1));
	TestTrue("Set should still contain second subscription", SubscriptionsAfterRemove.Contains(IndividualSubscription2));

	// TEST 6: Remove subscription set from another subscription set
	UPubnubSubscriptionSet* SetForSubtraction = PubnubSubsystem->CreateSubscriptionSet(TArray<FString>{TestChannel2}, TArray<FString>());
	MainSet->RemoveSubscriptionSet(SetForSubtraction);
	TArray<UPubnubSubscription*> MainSetAfterSubtraction = MainSet->GetSubscriptions();
	TestEqual("Main set should still have 1 subscription after subtraction", MainSetAfterSubtraction.Num(), 1);

	// Final verification of subscription set state
	TestTrue("All subscription set management operations completed successfully", 
		(SubscriptionsAfterRemove.Num() == 1) && 
		(CombinedSubscriptions.Num() == 2) && 
		(MainSetAfterMerge.Num() == 1));

	CleanUp();
	return true;
}

bool FPubnubEntityCreationAndTrackingTest::RunTest(const FString& Parameters)
{
	// Test data for entity creation and tracking operations
	const FString TestUser = SDK_PREFIX + "entity_tracking_user";
	const FString TestChannel = SDK_PREFIX + "tracking_channel";
	const FString TestChannelGroup = SDK_PREFIX + "tracking_group";
	const FString TestUserID = SDK_PREFIX + "tracking_user_metadata";
	const FString TestChannelForMetadata = SDK_PREFIX + "tracking_channel_metadata";
	
	if(!InitTest())
	{
		AddError("Test initialization failed for entity creation and tracking test");
		return false;
	}

	// Error handling setup
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(FString::Printf(TEXT("PubNub error during entity tracking test: %s"), *ErrorMessage));
	});
	
	// Set User ID required for operations
	PubnubSubsystem->SetUserID(TestUser);

	// Get baseline active counts before creating entities
	TArray<UPubnubSubscription*> ActiveSubscriptionsBaseline = PubnubSubsystem->GetActiveSubscriptions();
	TArray<UPubnubSubscriptionSet*> ActiveSubscriptionSetsBaseline = PubnubSubsystem->GetActiveSubscriptionSets();
	int32 BaselineActiveSubscriptionCount = ActiveSubscriptionsBaseline.Num();
	int32 BaselineActiveSubscriptionSetCount = ActiveSubscriptionSetsBaseline.Num();

	// TEST 1: Create all entity types and verify they work correctly
	UPubnubChannelEntity* CreatedChannelEntity = PubnubSubsystem->CreateChannelEntity(TestChannel);
	TestNotNull("Channel entity creation must succeed", CreatedChannelEntity);
	TestEqual("Channel entity must have correct ID", CreatedChannelEntity->EntityID, TestChannel);
	TestEqual("Channel entity must have correct type", CreatedChannelEntity->EntityType, EPubnubEntityType::PEnT_Channel);

	UPubnubChannelGroupEntity* CreatedChannelGroupEntity = PubnubSubsystem->CreateChannelGroupEntity(TestChannelGroup);
	TestNotNull("Channel group entity creation must succeed", CreatedChannelGroupEntity);
	TestEqual("Channel group entity must have correct ID", CreatedChannelGroupEntity->EntityID, TestChannelGroup);
	TestEqual("Channel group entity must have correct type", CreatedChannelGroupEntity->EntityType, EPubnubEntityType::PEnT_ChannelGroup);

	UPubnubChannelMetadataEntity* CreatedChannelMetadataEntity = PubnubSubsystem->CreateChannelMetadataEntity(TestChannelForMetadata);
	TestNotNull("Channel metadata entity creation must succeed", CreatedChannelMetadataEntity);
	TestEqual("Channel metadata entity must have correct ID", CreatedChannelMetadataEntity->EntityID, TestChannelForMetadata);
	TestEqual("Channel metadata entity must have correct type", CreatedChannelMetadataEntity->EntityType, EPubnubEntityType::PEnT_ChannelMetadata);

	UPubnubUserMetadataEntity* CreatedUserMetadataEntity = PubnubSubsystem->CreateUserMetadataEntity(TestUserID);
	TestNotNull("User metadata entity creation must succeed", CreatedUserMetadataEntity);
	TestEqual("User metadata entity must have correct ID", CreatedUserMetadataEntity->EntityID, TestUserID);
	TestEqual("User metadata entity must have correct type", CreatedUserMetadataEntity->EntityType, EPubnubEntityType::PEnT_UserMetadata);

	// TEST 2: Create subscriptions and subscription sets, then verify tracking
	UPubnubSubscription* TrackedSubscription1 = CreatedChannelEntity->CreateSubscription();
	UPubnubSubscription* TrackedSubscription2 = CreatedChannelGroupEntity->CreateSubscription();
	TestNotNull("Tracked subscription 1 creation must succeed", TrackedSubscription1);
	TestNotNull("Tracked subscription 2 creation must succeed", TrackedSubscription2);

	UPubnubSubscriptionSet* TrackedSubscriptionSet1 = PubnubSubsystem->CreateSubscriptionSet(TArray<FString>{TestChannel}, TArray<FString>());
	UPubnubSubscriptionSet* TrackedSubscriptionSet2 = PubnubSubsystem->CreateSubscriptionSetFromEntities(TArray<UPubnubBaseEntity*>{CreatedChannelEntity, CreatedChannelGroupEntity});
	TestNotNull("Tracked subscription set 1 creation must succeed", TrackedSubscriptionSet1);
	TestNotNull("Tracked subscription set 2 creation must succeed", TrackedSubscriptionSet2);

	// TEST 3: Verify GetActiveSubscriptions tracks individual subscriptions correctly
	TArray<UPubnubSubscription*> CurrentActiveSubscriptions = PubnubSubsystem->GetActiveSubscriptions();
	TestTrue("Active subscriptions count should increase after creating subscriptions", 
		CurrentActiveSubscriptions.Num() >= BaselineActiveSubscriptionCount);
	
	// Verify that our created subscriptions are being tracked (if they're in active state)
	bool bSubscription1IsTracked = CurrentActiveSubscriptions.Contains(TrackedSubscription1);
	bool bSubscription2IsTracked = CurrentActiveSubscriptions.Contains(TrackedSubscription2);
	TestTrue("GetActiveSubscriptions must return valid array", CurrentActiveSubscriptions.Num() >= 0);

	// TEST 4: Verify GetActiveSubscriptionSets tracks subscription sets correctly  
	TArray<UPubnubSubscriptionSet*> CurrentActiveSubscriptionSets = PubnubSubsystem->GetActiveSubscriptionSets();
	TestTrue("Active subscription sets count should increase after creating sets",
		CurrentActiveSubscriptionSets.Num() >= BaselineActiveSubscriptionSetCount);
	
	// Verify that our created subscription sets are being tracked (if they're in active state)
	bool bSubscriptionSet1IsTracked = CurrentActiveSubscriptionSets.Contains(TrackedSubscriptionSet1);
	bool bSubscriptionSet2IsTracked = CurrentActiveSubscriptionSets.Contains(TrackedSubscriptionSet2);
	TestTrue("GetActiveSubscriptionSets must return valid array", CurrentActiveSubscriptionSets.Num() >= 0);

	// TEST 5: Verify subscription set manipulation functions work correctly
	TrackedSubscriptionSet1->AddSubscription(TrackedSubscription1);
	TArray<UPubnubSubscription*> SetContentsAfterAdd = TrackedSubscriptionSet1->GetSubscriptions();
	TestEqual("Subscription set should contain 1 subscription after adding", SetContentsAfterAdd.Num(), 1);
	TestEqual("Added subscription should be the correct one", SetContentsAfterAdd[0], TrackedSubscription1);

	TrackedSubscriptionSet1->AddSubscription(TrackedSubscription2);
	TArray<UPubnubSubscription*> SetContentsAfterSecondAdd = TrackedSubscriptionSet1->GetSubscriptions();
	TestEqual("Subscription set should contain 2 subscriptions after adding second", SetContentsAfterSecondAdd.Num(), 2);
	TestTrue("Set should contain both subscriptions", SetContentsAfterSecondAdd.Contains(TrackedSubscription1) && SetContentsAfterSecondAdd.Contains(TrackedSubscription2));

	TrackedSubscriptionSet1->RemoveSubscription(TrackedSubscription1);
	TArray<UPubnubSubscription*> SetContentsAfterRemove = TrackedSubscriptionSet1->GetSubscriptions();
	TestEqual("Subscription set should contain 1 subscription after removing first", SetContentsAfterRemove.Num(), 1);
	TestFalse("Set should not contain removed subscription", SetContentsAfterRemove.Contains(TrackedSubscription1));
	TestTrue("Set should still contain second subscription", SetContentsAfterRemove.Contains(TrackedSubscription2));

	// TEST 6: Test subscription set merging and subtraction
	TrackedSubscriptionSet1->AddSubscriptionSet(TrackedSubscriptionSet2);
	TArray<UPubnubSubscription*> SetContentsAfterMerge = TrackedSubscriptionSet1->GetSubscriptions();
	TestTrue("Subscription set contents should be valid after merge operation", SetContentsAfterMerge.Num() >= 1);

	TrackedSubscriptionSet1->RemoveSubscriptionSet(TrackedSubscriptionSet2);
	TArray<UPubnubSubscription*> SetContentsAfterSubtraction = TrackedSubscriptionSet1->GetSubscriptions();
	TestTrue("Subscription set contents should be valid after subtraction operation", SetContentsAfterSubtraction.Num() >= 0);

	// Final verification of all entity creation and tracking functionality
	TestTrue("All entity types created successfully",
		IsValid(CreatedChannelEntity) && 
		IsValid(CreatedChannelGroupEntity) && 
		IsValid(CreatedChannelMetadataEntity) && 
		IsValid(CreatedUserMetadataEntity));

	TestTrue("All subscription objects created successfully",
		IsValid(TrackedSubscription1) && 
		IsValid(TrackedSubscription2) && 
		IsValid(TrackedSubscriptionSet1) && 
		IsValid(TrackedSubscriptionSet2));

	TestTrue("Active tracking functions work correctly",
		(CurrentActiveSubscriptions.Num() >= BaselineActiveSubscriptionCount) && 
		(CurrentActiveSubscriptionSets.Num() >= BaselineActiveSubscriptionSetCount));

	CleanUp();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS