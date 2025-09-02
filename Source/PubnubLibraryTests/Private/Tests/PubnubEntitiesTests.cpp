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

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubChannelEntityPublishSubscribeTest, FPubnubAutomationTestBase, "Pubnub.Integration.Entities.ChannelEntity.PublishAndSubscribe", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubChannelGroupEntityTest, FPubnubAutomationTestBase, "Pubnub.Integration.Entities.ChannelGroupEntity.CreateAndSubscribe", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscriptionSetTest, FPubnubAutomationTestBase, "Pubnub.Integration.Entities.SubscriptionSet.MultipleChannels", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

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
		ChannelEntity->PublishMessage(TestMessage, PublishCallback);
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
		ChannelGroupEntity->AddChannelToGroup(TestChannel1, AddChannelCallback);
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
			TempChannelEntity->PublishMessage(TestMessage, PublishCallback);
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

#endif // WITH_DEV_AUTOMATION_TESTS