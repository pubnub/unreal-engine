// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubClient.h"
#include "PubnubEnumLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "Entities/PubnubChannelEntity.h"
#include "Entities/PubnubSubscription.h"
#include "PubnubStructLibrary.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessageTest, FPubnubAutomationTestBase, "Pubnub.Integration.PubSub.PublishMessage", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessageWithSettingsTest, FPubnubAutomationTestBase, "Pubnub.Integration.PubSub.PublishMessageWithSettingsTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSignalTest, FPubnubAutomationTestBase, "Pubnub.Integration.PubSub.SendSignal", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSignalWithSettingsTest, FPubnubAutomationTestBase, "Pubnub.Integration.PubSub.SendSignalWithSettings", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUnsubscribeTest, FPubnubAutomationTestBase, "Pubnub.Integration.PubSub.UnsubscribeFromChannel", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishVariousMessageTypesTest, FPubnubAutomationTestBase, "Pubnub.Integration.PubSub.PublishVariousMessageTypes", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// Subscription status delegate matching edge cases (use GetPubnubClient(0) + PubnubClient only)
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscriptionStatusDelegateMatch_SameChannelClientAndEntity, FPubnubAutomationTestBase, "Pubnub.Integration.PubSub.SubscriptionStatusDelegateMatch.SameChannelClientAndEntity", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscriptionStatusDelegateMatch_EntitySubscribeUnsubscribeBlocking, FPubnubAutomationTestBase, "Pubnub.Integration.PubSub.SubscriptionStatusDelegateMatch.EntitySubscribeUnsubscribeBlocking", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscriptionStatusDelegateMatch_TwoDifferentChannels, FPubnubAutomationTestBase, "Pubnub.Integration.PubSub.SubscriptionStatusDelegateMatch.TwoDifferentChannels", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FPubnubPublishMessageTest::RunTest(const FString& Parameters)
{
	//Initial variables
	const FString TestMessage = "\"Message from test\"";
	const FString TestUser = SDK_PREFIX + "test_user";
	const FString TestChannel = SDK_PREFIX + "test_channel";
	TSharedPtr<bool> TestMessageReceived = MakeShared<bool>(false);
	TSharedPtr<bool> TestPublishResultReceived = MakeShared<bool>(false);
	TSharedPtr<bool> TestSubscribeResultReceived = MakeShared<bool>(false);
	TSharedPtr<FPubnubOperationResult> PublishResult = MakeShared<FPubnubOperationResult>();
	TSharedPtr<FPubnubMessageData> PublishedMessageData = MakeShared<FPubnubMessageData>();
	
	if(!InitTest())
	{
		AddError("TestInitialization failed");
		return false;
	}

	//Check for any errors on the way
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	
	//Set User ID - it's necessary for any operation
	PubnubSubsystem->SetUserID(TestUser);

	//Add listener for received messages
	PubnubSubsystem->OnMessageReceivedNative.AddLambda([this, TestMessage, TestChannel, TestUser, TestMessageReceived](FPubnubMessageData ReceivedMessage)
	{
		*TestMessageReceived = true;
		TestEqual("Published message content", TestMessage, ReceivedMessage.Message);
		TestEqual("Published message channel", TestChannel, ReceivedMessage.Channel);
		TestEqual("Published message UserID", TestUser, ReceivedMessage.UserID);
		TestEqual("Published message MessageType", EPubnubMessageType::PMT_Published, ReceivedMessage.MessageType);
	});

	//Create subscribe result callback
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

	//Create publish result callback
	FOnPublishMessageResponseNative PublishCallback;
	PublishCallback.BindLambda([this, TestMessage, TestChannel, TestUser, TestPublishResultReceived, PublishResult, PublishedMessageData]
		(const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
	{
		*TestPublishResultReceived = true;
		*PublishResult = Result;
		*PublishedMessageData = PublishedMessage;
		
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

	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SubscribeCallback]()
	{
		PubnubSubsystem->SubscribeToChannel(TestChannel, SubscribeCallback);
	}, 0.1f));

	//Wait until subscribe result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestSubscribeResultReceived]() -> bool {
		return *TestSubscribeResultReceived;
	}, MAX_WAIT_TIME));

	//Check whether subscribe result was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestSubscribeResultReceived]()
	{
		if(!*TestSubscribeResultReceived)
		{
			AddError("Subscribe result callback was not received");
		}
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage, PublishCallback]()
	{
		PubnubSubsystem->PublishMessage(TestChannel, TestMessage, PublishCallback);
	}, 0.1f));

	//Wait until publish result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestPublishResultReceived]() -> bool {
		return *TestPublishResultReceived;
	}, MAX_WAIT_TIME));

	//Check whether publish result was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestPublishResultReceived]()
	{
		if(!*TestPublishResultReceived)
		{
			AddError("Publish result callback was not received");
		}
	}, 0.1f));

	//This will wait until message is received or timeout is reached
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestMessageReceived]() -> bool {
		return *TestMessageReceived;
	}, MAX_WAIT_TIME));

	//Check whether message was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestMessageReceived]()
	{
		if(!*TestMessageReceived)
		{
			AddError("Message was not received");
		}
	}, 0.1f));

	CleanUp();
	return true;
}

bool FPubnubPublishMessageWithSettingsTest::RunTest(const FString& Parameters)
{
	//Initial variables
	const FString TestMessage = "\"Message from test\"";
	const FString TestUser = SDK_PREFIX + "test_user";
	const FString TestChannel = SDK_PREFIX + "test_channel";
	const FString TestMetaData = "{\"metadata\":\"from test\"}";
	const FString TestCustomMessageType = "custom_type";
	TSharedPtr<bool> TestMessageReceived = MakeShared<bool>(false);
	TSharedPtr<bool> TestSubscribeResultReceived = MakeShared<bool>(false);
	
	if(!InitTest())
	{
		AddError("TestInitialization failed");
		return false;
	}

	//Check for any errors on the way
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	
	//Set User ID - it's necessary for any operation
	PubnubSubsystem->SetUserID(TestUser);

	//Add listener for received messages
	PubnubSubsystem->OnMessageReceivedNative.AddLambda([this, TestMessage, TestChannel, TestUser, TestMessageReceived, TestMetaData, TestCustomMessageType](FPubnubMessageData ReceivedMessage)
	{
		*TestMessageReceived = true;
		TestEqual("Published message content", TestMessage, ReceivedMessage.Message);
		TestEqual("Published message channel", TestChannel, ReceivedMessage.Channel);
		TestEqual("Published message UserID", TestUser, ReceivedMessage.UserID);
		TestEqual("Published message MessageType", EPubnubMessageType::PMT_Published, ReceivedMessage.MessageType);
		TestEqual("Published message Metadata", TestMetaData, ReceivedMessage.Metadata);
		TestEqual("Published message CustomMessageType", TestCustomMessageType, ReceivedMessage.CustomMessageType);
	});

	//Create subscribe result callback
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

	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SubscribeCallback]()
	{
		PubnubSubsystem->SubscribeToChannel(TestChannel, SubscribeCallback);
	}, 0.1f));

	//Wait until subscribe result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestSubscribeResultReceived]() -> bool {
		return *TestSubscribeResultReceived;
	}, MAX_WAIT_TIME));

	//Check whether subscribe result was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestSubscribeResultReceived]()
	{
		if(!*TestSubscribeResultReceived)
		{
			AddError("Subscribe result callback was not received");
		}
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage, TestMetaData, TestCustomMessageType]()
	{
		FPubnubPublishSettings PublishSettings;
		PublishSettings.MetaData = TestMetaData;
		PublishSettings.CustomMessageType = TestCustomMessageType;
		PublishSettings.Ttl = 5;
		PubnubSubsystem->PublishMessage(TestChannel, TestMessage, PublishSettings);
	}, 0.1f));

	//This will wait until message is received or timeout is reached
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestMessageReceived]() -> bool {
		return *TestMessageReceived;
	}, MAX_WAIT_TIME));

	//Check whether message was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestMessageReceived]()
	{
		if(!*TestMessageReceived)
		{
			AddError("Message was not received");
		}
	}, 0.1f));

	CleanUp();
	return true;
}

bool FPubnubSignalTest::RunTest(const FString& Parameters)
{
	//Initial variables
	const FString TestSignalMessage = "\"Signal from E2E test\"";
	const FString TestUser = SDK_PREFIX + "test_user";
	const FString TestChannel = SDK_PREFIX + "test_channel";
	TSharedPtr<bool> TestSignalReceived = MakeShared<bool>(false);
	TSharedPtr<bool> TestSignalResultReceived = MakeShared<bool>(false);
	TSharedPtr<bool> TestSubscribeResultReceived = MakeShared<bool>(false);
	TSharedPtr<FPubnubOperationResult> SignalResult = MakeShared<FPubnubOperationResult>();
	TSharedPtr<FPubnubMessageData> SignalMessageData = MakeShared<FPubnubMessageData>();
	
	if(!InitTest())
	{
		AddError("TestInitialization failed for FPubnubSignalTest");
		return false;
	}

	//Check for any errors on the way
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	
	//Set User ID - it's necessary for any operation
	PubnubSubsystem->SetUserID(TestUser);

	//Add listener for received messages (hoping signals come this way)
	PubnubSubsystem->OnMessageReceivedNative.AddLambda([this, TestSignalMessage, TestChannel, TestSignalReceived](FPubnubMessageData ReceivedMessage)
	{
		*TestSignalReceived = true;
		TestEqual("Received signal content matches", TestSignalMessage, ReceivedMessage.Message);
		TestEqual("Received signal channel matches", TestChannel, ReceivedMessage.Channel);
		TestEqual("Received signal MessageType matches", EPubnubMessageType::PMT_Signal, ReceivedMessage.MessageType);
	});

	//Create subscribe result callback
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

	//Create signal result callback
	FOnSignalResponseNative SignalCallback;
	SignalCallback.BindLambda([this, TestSignalMessage, TestChannel, TestUser, TestSignalResultReceived, SignalResult, SignalMessageData]
		(const FPubnubOperationResult& Result, const FPubnubMessageData& SignalMessage)
	{
		*TestSignalResultReceived = true;
		*SignalResult = Result;
		*SignalMessageData = SignalMessage;
		
		// Verify signal result
		TestFalse("Signal operation should not have failed", Result.Error);
		TestEqual("Signal HTTP status should be 200", Result.Status, 200);
		
		if (!Result.Error)
		{
			// Verify signal message data matches what we sent
			TestEqual("Signal message data - content", TestSignalMessage, SignalMessage.Message);
			TestEqual("Signal message data - channel", TestChannel, SignalMessage.Channel);
			TestEqual("Signal message data - user ID", TestUser, SignalMessage.UserID);
			TestEqual("Signal message data - message type", EPubnubMessageType::PMT_Signal, SignalMessage.MessageType);
			TestFalse("Signal message timetoken should not be empty", SignalMessage.Timetoken.IsEmpty());
		}
		else
		{
			AddError(FString::Printf(TEXT("Signal failed with error: %s"), *Result.ErrorMessage));
		}
	});
	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SubscribeCallback]()
	{
		PubnubSubsystem->SubscribeToChannel(TestChannel, SubscribeCallback);
	}, 0.1f));

	//Wait until subscribe result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestSubscribeResultReceived]() -> bool {
		return *TestSubscribeResultReceived;
	}, MAX_WAIT_TIME));

	//Check whether subscribe result was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestSubscribeResultReceived]()
	{
		if(!*TestSubscribeResultReceived)
		{
			AddError("Subscribe result callback was not received");
		}
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestSignalMessage, SignalCallback]()
	{
		PubnubSubsystem->Signal(TestChannel, TestSignalMessage, SignalCallback);
	}, 0.1f));

	//Wait until signal result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestSignalResultReceived]() -> bool {
		return *TestSignalResultReceived;
	}, MAX_WAIT_TIME));

	//Check whether signal result was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestSignalResultReceived]()
	{
		if(!*TestSignalResultReceived)
		{
			AddError("Signal result callback was not received");
		}
	}, 0.1f));

	//This will wait until signal is received or timeout is reached
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestSignalReceived]() -> bool {
		return *TestSignalReceived;
	}, MAX_WAIT_TIME));

	//Check whether signal was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestSignalReceived]()
	{
		if(!*TestSignalReceived)
		{
			AddError("Signal was not received on the channel in FPubnubSignalTest.");
		}
	}, 0.1f));

	CleanUp();
	return true;
}

bool FPubnubSignalWithSettingsTest::RunTest(const FString& Parameters)
{
	//Initial variables
	const FString TestSignalMessage = "\"Signal from E2E test with settings\"";
	const FString TestUser = SDK_PREFIX + "test_user_signal_settings";
	const FString TestChannel = SDK_PREFIX + "test_channel_signal_settings";
	const FString TestCustomSignalType = "custom_signal_type_e2e";
	TSharedPtr<bool> TestSignalReceived = MakeShared<bool>(false);
	TSharedPtr<bool> TestSubscribeResultReceived = MakeShared<bool>(false);
	
	if(!InitTest())
	{
		AddError("TestInitialization failed for FPubnubSignalWithSettingsTest");
		return false;
	}

	//Check for any errors on the way
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	
	//Set User ID - it's necessary for any operation
	PubnubSubsystem->SetUserID(TestUser);

	//Add listener for received messages
	PubnubSubsystem->OnMessageReceivedNative.AddLambda([this, TestSignalMessage, TestChannel, TestCustomSignalType, TestSignalReceived](FPubnubMessageData ReceivedMessage)
	{
		if (ReceivedMessage.Channel == TestChannel && ReceivedMessage.Message == TestSignalMessage)
		{
			*TestSignalReceived = true;
			TestEqual("Received signal content (with settings) matches", TestSignalMessage, ReceivedMessage.Message);
			TestEqual("Received signal channel (with settings) matches", TestChannel, ReceivedMessage.Channel);
			TestEqual("Received signal MessageType (with settings) matches", EPubnubMessageType::PMT_Signal, ReceivedMessage.MessageType);
			TestEqual("Received signal CustomMessageType (with settings) matches", TestCustomSignalType, ReceivedMessage.CustomMessageType);
		}
	});

	//Create subscribe result callback
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
	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SubscribeCallback]()
	{
		PubnubSubsystem->SubscribeToChannel(TestChannel, SubscribeCallback);
	}, 0.1f));

	//Wait until subscribe result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestSubscribeResultReceived]() -> bool {
		return *TestSubscribeResultReceived;
	}, MAX_WAIT_TIME));

	//Check whether subscribe result was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestSubscribeResultReceived]()
	{
		if(!*TestSubscribeResultReceived)
		{
			AddError("Subscribe result callback was not received");
		}
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestSignalMessage, TestCustomSignalType]()
	{
		FPubnubSignalSettings SignalSettings;
		SignalSettings.CustomMessageType = TestCustomSignalType;
		PubnubSubsystem->Signal(TestChannel, TestSignalMessage, SignalSettings);
	}, 0.1f));

	//This will wait until signal is received or timeout is reached
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestSignalReceived]() -> bool {
		return *TestSignalReceived;
	}, MAX_WAIT_TIME));

	//Check whether signal was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestSignalReceived]()
	{
		if(!*TestSignalReceived)
		{
			AddError("Signal with settings was not received on the channel in FPubnubSignalWithSettingsTest.");
		}
	}, 0.1f));

	CleanUp();
	return true;
}

bool FPubnubUnsubscribeTest::RunTest(const FString& Parameters)
{
	//Initial variables
	const FString TestMessage = "\"Message for unsubscribe test\"";
	const FString TestUser = SDK_PREFIX + "test_user_unsubscribe";
	const FString TestChannel = SDK_PREFIX + "test_channel_unsubscribe";
	TSharedPtr<bool> TestMessageReceivedAfterUnsubscribe = MakeShared<bool>(false);
	TSharedPtr<bool> TestSubscribeResultReceived = MakeShared<bool>(false);
	TSharedPtr<bool> TestUnsubscribeResultReceived = MakeShared<bool>(false);
	
	if(!InitTest())
	{
		AddError("TestInitialization failed for FPubnubUnsubscribeTest");
		return false;
	}

	//Check for any unexpected errors on the way
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	
	//Set User ID
	PubnubSubsystem->SetUserID(TestUser);

	//Add listener for received messages.
	//If this fires for our target channel and message, it's an error because we should be unsubscribed.
	PubnubSubsystem->OnMessageReceivedNative.AddLambda([this, TestMessage, TestChannel, TestMessageReceivedAfterUnsubscribe](FPubnubMessageData ReceivedMessage)
	{
		if (ReceivedMessage.Channel == TestChannel && ReceivedMessage.Message == TestMessage)
		{
			*TestMessageReceivedAfterUnsubscribe = true;
			AddError(FString::Printf(TEXT("Message '%s' received on channel '%s' after unsubscribing."), *TestMessage, *TestChannel));
		}
	});

	//Create subscribe result callback
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

	//Create unsubscribe result callback
	FOnSubscribeOperationResponseNative UnsubscribeCallback;
	UnsubscribeCallback.BindLambda([this, TestUnsubscribeResultReceived](const FPubnubOperationResult& Result)
	{
		*TestUnsubscribeResultReceived = true;
		TestFalse("Unsubscribe operation should not have failed", Result.Error);
		TestEqual("Unsubscribe HTTP status should be 200", Result.Status, 200);
		
		if (Result.Error)
		{
			AddError(FString::Printf(TEXT("Unsubscribe failed with error: %s"), *Result.ErrorMessage));
		}
	});

	// Subscribe to the channel
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SubscribeCallback]()
	{
		PubnubSubsystem->SubscribeToChannel(TestChannel, SubscribeCallback);
	}, 0.1f));

	//Wait until subscribe result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestSubscribeResultReceived]() -> bool {
		return *TestSubscribeResultReceived;
	}, MAX_WAIT_TIME));

	//Check whether subscribe result was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestSubscribeResultReceived]()
	{
		if(!*TestSubscribeResultReceived)
		{
			AddError("Subscribe result callback was not received");
		}
	}, 0.1f));
	
	// Unsubscribe from the channel
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, UnsubscribeCallback]()
	{
		PubnubSubsystem->UnsubscribeFromChannel(TestChannel, UnsubscribeCallback);
	}, 0.1f));

	//Wait until unsubscribe result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestUnsubscribeResultReceived]() -> bool {
		return *TestUnsubscribeResultReceived;
	}, MAX_WAIT_TIME));

	//Check whether unsubscribe result was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUnsubscribeResultReceived]()
	{
		if(!*TestUnsubscribeResultReceived)
		{
			AddError("Unsubscribe result callback was not received");
		}
	}, 0.1f));
	
	// Publish a message to the channel we just unsubscribed from
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		PubnubSubsystem->PublishMessage(TestChannel, TestMessage);
	}, 0.1f));

	// Wait for a period to see if the message is incorrectly received
	// MAX_WAIT_TIME / 2.0f should be sufficient for a message to arrive if unsubscribe failed.
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(MAX_WAIT_TIME / 2.0f));

	// Check whether the message was received after unsubscribing
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestMessageReceivedAfterUnsubscribe]()
	{
		TestFalse("Message should NOT have been received on channel after unsubscribing.", *TestMessageReceivedAfterUnsubscribe);
	}, 0.1f));

	CleanUp();
	return true;
}

// Helper struct for Publish test cases
struct FMessageTestCase
{
	FString Description;
	FString MessageToSend; // This is the JSON string payload
	FString ExpectedReceivedMessage; // Usually the same as MessageToSend

	FMessageTestCase(const FString& InDesc, const FString& InMsgSend, const FString& InMsgExpect)
		: Description(InDesc), MessageToSend(InMsgSend), ExpectedReceivedMessage(InMsgExpect) {}
	FMessageTestCase(const FString& InDesc, const FString& InMsg)
		: Description(InDesc), MessageToSend(InMsg), ExpectedReceivedMessage(InMsg) {}
};

bool FPubnubPublishVariousMessageTypesTest::RunTest(const FString& Parameters)
{
	const FString TestUser = SDK_PREFIX + "test_user_various_msg";
	const FString TestChannel = SDK_PREFIX + "test_channel_various_msg";

	TArray<FMessageTestCase> TestCases;
	TestCases.Add(FMessageTestCase("Simple String", "hello world"));
	TestCases.Add(FMessageTestCase("Simple String as JSON String", "\"hello world\""));
	TestCases.Add(FMessageTestCase("JSON Object", "{\"message\":\"this is an object\",\"count\":1}"));
	TestCases.Add(FMessageTestCase("Integer Number as JSON Number", "789"));
	TestCases.Add(FMessageTestCase("Floating Point Number as JSON Number", "123.456"));
	TestCases.Add(FMessageTestCase("Boolean true as JSON Boolean", "true"));
	TestCases.Add(FMessageTestCase("Boolean false as JSON Boolean", "false"));
	TestCases.Add(FMessageTestCase("JSON Array", "[\"element1\",2,{\"nested_key\":\"nested_val\"},false]"));
	TestCases.Add(FMessageTestCase("Empty String as JSON string", "\"\""));
	TestCases.Add(FMessageTestCase("String with escapes as JSON string", "\"Text with \\\"quotes\\\", \\\\backslashes\\\\, a /slash, and a newline\\ncharacter.\""));
	TestCases.Add(FMessageTestCase("JSON null", "null"));


	if (!InitTest())
	{
		AddError("TestInitialization failed for FPubnubPublishVariousMessageTypesTest");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(FString::Printf(TEXT("Pubnub Error in FPubnubPublishVariousMessageTypesTest: %s, Type: %d"), *ErrorMessage, ErrorType));
	});

	PubnubSubsystem->SetUserID(TestUser);

	TSharedPtr<int32> CurrentTestCaseIndex = MakeShared<int32>(-1); // Start at -1, will be incremented before use
	TSharedPtr<bool> bExpectedMessageReceived = MakeShared<bool>(false);
	TSharedPtr<FPubnubMessageData> LastReceivedMessageData = MakeShared<FPubnubMessageData>();
	TSharedPtr<bool> TestSubscribeResultReceived = MakeShared<bool>(false);

	PubnubSubsystem->OnMessageReceivedNative.AddLambda(
		[this, TestCases, CurrentTestCaseIndex, bExpectedMessageReceived, LastReceivedMessageData, TestChannel](FPubnubMessageData ReceivedMessage)
		{
			if (*CurrentTestCaseIndex >= 0 && *CurrentTestCaseIndex < TestCases.Num() && ReceivedMessage.Channel == TestChannel)
			{
				// Only consider the message if we are actively waiting for one for the current test case
				if (!(*bExpectedMessageReceived)) //Process only the first message received for the current test case
				{
					*LastReceivedMessageData = ReceivedMessage;
					*bExpectedMessageReceived = true;
				}
			}
		});

	//Create subscribe result callback
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
	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SubscribeCallback]()
	{
		PubnubSubsystem->SubscribeToChannel(TestChannel, SubscribeCallback);
	}, 0.1f));

	//Wait until subscribe result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestSubscribeResultReceived]() -> bool {
		return *TestSubscribeResultReceived;
	}, MAX_WAIT_TIME));

	//Check whether subscribe result was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestSubscribeResultReceived]()
	{
		if(!*TestSubscribeResultReceived)
		{
			AddError("Subscribe result callback was not received");
		}
	}, 0.1f));


	for (int32 i = 0; i < TestCases.Num(); ++i)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, CurrentTestCaseIndex, bExpectedMessageReceived, i]()
		{
			*CurrentTestCaseIndex = i;
			*bExpectedMessageReceived = false;
		}, 0.2f));
		
		ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestCases, i]()
		{
			PubnubSubsystem->PublishMessage(TestChannel, TestCases[i].MessageToSend);
		}, 0.2f));

		ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bExpectedMessageReceived]() {
			return *bExpectedMessageReceived;
		}, MAX_WAIT_TIME));

		ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
			[this, TestCases, i, bExpectedMessageReceived, LastReceivedMessageData, TestUser, TestChannel]()
			{
				if (!*bExpectedMessageReceived)
				{
					AddError(FString::Printf(TEXT("Test case '%s' (%d): Message was not received."), *TestCases[i].Description, i));
					return;
				}

				const FMessageTestCase& CurrentCase = TestCases[i];
				TestEqual(CurrentCase.Description + " - Channel", TestChannel, LastReceivedMessageData->Channel);
				TestEqual(CurrentCase.Description + " - UserID", TestUser, LastReceivedMessageData->UserID);
				TestEqual(CurrentCase.Description + " - MessageType", EPubnubMessageType::PMT_Published, LastReceivedMessageData->MessageType);

				bool bIsJsonLike = CurrentCase.ExpectedReceivedMessage.StartsWith(TEXT("{")) || CurrentCase.ExpectedReceivedMessage.StartsWith(TEXT("["));
				
				if (bIsJsonLike)
				{
					bool bJsonStringsEqual = UPubnubJsonUtilities::AreJsonObjectStringsEqual(CurrentCase.ExpectedReceivedMessage, LastReceivedMessageData->Message);
					if (!bJsonStringsEqual)
					{
						// Log more details if JSON comparison fails
						AddError(FString::Printf(TEXT("Test case '%s' (%d): JSON content mismatch. Expected: %s, Actual: %s"), *CurrentCase.Description, i, *CurrentCase.ExpectedReceivedMessage, *LastReceivedMessageData->Message));
					}
					else
					{
						TestTrue(CurrentCase.Description + " - Content (JSON)", bJsonStringsEqual);
					}
				}
				else
				{
					TestEqual(CurrentCase.Description + " - Content", CurrentCase.ExpectedReceivedMessage, LastReceivedMessageData->Message);
				}
			}, 0.1f));
	}

	CleanUp();
	return true;
}

// --- Subscription status delegate matching edge-case tests (use GetPubnubClient(0) + PubnubClient only) ---

bool FPubnubSubscriptionStatusDelegateMatch_SameChannelClientAndEntity::RunTest(const FString& Parameters)
{
	const FString TestUser = SDK_PREFIX + "status_match_user";
	const FString TestChannel = SDK_PREFIX + "status_match_same_ch";

	if (!InitTest())
	{
		AddError("TestInitialization failed for FPubnubSubscriptionStatusDelegateMatch_SameChannelClientAndEntity");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubSubsystem->SetUserID(TestUser);

	UPubnubClient* Client = PubnubSubsystem->GetPubnubClient(0);
	if (!Client)
	{
		AddError("GetPubnubClient(0) returned null");
		CleanUp();
		return false;
	}

	TSharedPtr<bool> SubscribeClientDone = MakeShared<bool>(false);
	TSharedPtr<bool> SubscribeEntityDone = MakeShared<bool>(false);
	TSharedPtr<bool> UnsubscribeClientDone = MakeShared<bool>(false);
	TSharedPtr<bool> UnsubscribeEntityDone = MakeShared<bool>(false);

	FOnPubnubSubscribeOperationResponseNative SubscribeClientCb = FOnPubnubSubscribeOperationResponseNative::CreateLambda(
		[this, SubscribeClientDone](const FPubnubOperationResult& Result)
		{
			*SubscribeClientDone = true;
			TestFalse("Subscribe (client) should not have failed", Result.Error);
			TestEqual("Subscribe (client) status", Result.Status, 200);
		});
	FOnPubnubSubscribeOperationResponseNative SubscribeEntityCb = FOnPubnubSubscribeOperationResponseNative::CreateLambda(
		[this, SubscribeEntityDone](const FPubnubOperationResult& Result)
		{
			*SubscribeEntityDone = true;
			TestFalse("Subscribe (entity) should not have failed", Result.Error);
			TestEqual("Subscribe (entity) status", Result.Status, 200);
		});
	FOnPubnubSubscribeOperationResponseNative UnsubscribeClientCb = FOnPubnubSubscribeOperationResponseNative::CreateLambda(
		[UnsubscribeClientDone](const FPubnubOperationResult& Result) { *UnsubscribeClientDone = true; });
	FOnPubnubSubscribeOperationResponseNative UnsubscribeEntityCb = FOnPubnubSubscribeOperationResponseNative::CreateLambda(
		[UnsubscribeEntityDone](const FPubnubOperationResult& Result) { *UnsubscribeEntityDone = true; });

	struct FSubscriptionHolder { UPubnubSubscription* Sub = nullptr; };
	TSharedPtr<FSubscriptionHolder> SubHolder = MakeShared<FSubscriptionHolder>();

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Client, TestChannel, SubscribeClientCb, SubscribeEntityCb, SubHolder]()
	{
		UPubnubChannelEntity* ChannelEntity = Client->CreateChannelEntity(TestChannel);
		if (!ChannelEntity) { AddError("CreateChannelEntity failed"); return; }
		UPubnubSubscription* Subscription = ChannelEntity->CreateSubscription();
		if (!Subscription) { AddError("CreateSubscription failed"); return; }
		SubHolder->Sub = Subscription;
		Client->SubscribeToChannelAsync(TestChannel, SubscribeClientCb);
		Subscription->SubscribeAsync(SubscribeEntityCb, FPubnubSubscriptionCursor());
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([SubscribeClientDone, SubscribeEntityDone]() -> bool {
		return *SubscribeClientDone && *SubscribeEntityDone;
	}, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SubscribeClientDone, SubscribeEntityDone]()
	{
		if (!*SubscribeClientDone) AddError("Subscribe (client) callback was not received");
		if (!*SubscribeEntityDone) AddError("Subscribe (entity) callback was not received");
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([Client, TestChannel, SubHolder, UnsubscribeClientCb, UnsubscribeEntityCb]()
	{
		Client->UnsubscribeFromChannelAsync(TestChannel, UnsubscribeClientCb);
		if (SubHolder->Sub)
		{
			SubHolder->Sub->UnsubscribeAsync(UnsubscribeEntityCb);
		}
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([UnsubscribeClientDone, UnsubscribeEntityDone]() -> bool {
		return *UnsubscribeClientDone && *UnsubscribeEntityDone;
	}, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, UnsubscribeClientDone, UnsubscribeEntityDone]()
	{
		if (!*UnsubscribeClientDone) AddError("Unsubscribe (client) callback was not received");
		if (!*UnsubscribeEntityDone) AddError("Unsubscribe (entity) callback was not received");
	}, 0.1f));

	CleanUp();
	return true;
}

bool FPubnubSubscriptionStatusDelegateMatch_EntitySubscribeUnsubscribeBlocking::RunTest(const FString& Parameters)
{
	const FString TestUser = SDK_PREFIX + "status_match_entity_block_user";
	const FString TestChannel = SDK_PREFIX + "status_match_entity_block_ch";

	if (!InitTest())
	{
		AddError("TestInitialization failed for FPubnubSubscriptionStatusDelegateMatch_EntitySubscribeUnsubscribeBlocking");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubSubsystem->SetUserID(TestUser);

	UPubnubClient* Client = PubnubSubsystem->GetPubnubClient(0);
	if (!Client)
	{
		AddError("GetPubnubClient(0) returned null");
		CleanUp();
		return false;
	}

	TSharedPtr<FPubnubOperationResult> SubscribeResult = MakeShared<FPubnubOperationResult>();
	TSharedPtr<FPubnubOperationResult> UnsubscribeResult = MakeShared<FPubnubOperationResult>();

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Client, TestChannel, SubscribeResult, UnsubscribeResult]()
	{
		UPubnubChannelEntity* ChannelEntity = Client->CreateChannelEntity(TestChannel);
		if (!ChannelEntity) { AddError("CreateChannelEntity failed"); return; }
		UPubnubSubscription* Subscription = ChannelEntity->CreateSubscription();
		if (!Subscription) { AddError("CreateSubscription failed"); return; }

		*SubscribeResult = Subscription->Subscribe(FPubnubSubscriptionCursor());
		TestFalse("Subscribe should not have failed", SubscribeResult->Error);
		TestEqual("Subscribe status", SubscribeResult->Status, 200);

		*UnsubscribeResult = Subscription->Unsubscribe();
		TestFalse("Unsubscribe should not have failed", UnsubscribeResult->Error);
		TestEqual("Unsubscribe status", UnsubscribeResult->Status, 200);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SubscribeResult, UnsubscribeResult]()
	{
		if (SubscribeResult->Error) AddError(FString::Printf(TEXT("Subscribe failed: %s"), *SubscribeResult->ErrorMessage));
		if (UnsubscribeResult->Error) AddError(FString::Printf(TEXT("Unsubscribe failed: %s"), *UnsubscribeResult->ErrorMessage));
	}, 0.1f));

	CleanUp();
	return true;
}

bool FPubnubSubscriptionStatusDelegateMatch_TwoDifferentChannels::RunTest(const FString& Parameters)
{
	const FString TestUser = SDK_PREFIX + "status_match_two_ch_user";
	const FString TestChannelA = SDK_PREFIX + "status_match_ch_a";
	const FString TestChannelB = SDK_PREFIX + "status_match_ch_b";

	if (!InitTest())
	{
		AddError("TestInitialization failed for FPubnubSubscriptionStatusDelegateMatch_TwoDifferentChannels");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubSubsystem->SetUserID(TestUser);

	UPubnubClient* Client = PubnubSubsystem->GetPubnubClient(0);
	if (!Client)
	{
		AddError("GetPubnubClient(0) returned null");
		CleanUp();
		return false;
	}

	TSharedPtr<FPubnubOperationResult> SubAResult = MakeShared<FPubnubOperationResult>();
	TSharedPtr<FPubnubOperationResult> SubBResult = MakeShared<FPubnubOperationResult>();
	TSharedPtr<FPubnubOperationResult> UnsubAResult = MakeShared<FPubnubOperationResult>();
	TSharedPtr<FPubnubOperationResult> UnsubBResult = MakeShared<FPubnubOperationResult>();

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Client, TestChannelA, TestChannelB, SubAResult, SubBResult, UnsubAResult, UnsubBResult]()
	{
		// Subscribe to channel A via client (blocking)
		*SubAResult = Client->SubscribeToChannel(TestChannelA);
		TestFalse("SubscribeToChannel(A) should not have failed", SubAResult->Error);
		TestEqual("SubscribeToChannel(A) status", SubAResult->Status, 200);

		// Subscribe to channel B via real subscription (blocking)
		UPubnubChannelEntity* EntityB = Client->CreateChannelEntity(TestChannelB);
		if (!EntityB) { AddError("CreateChannelEntity(B) failed"); return; }
		UPubnubSubscription* SubB = EntityB->CreateSubscription();
		if (!SubB) { AddError("CreateSubscription(B) failed"); return; }
		*SubBResult = SubB->Subscribe(FPubnubSubscriptionCursor());
		TestFalse("Subscription->Subscribe(B) should not have failed", SubBResult->Error);
		TestEqual("Subscription->Subscribe(B) status", SubBResult->Status, 200);

		// Unsubscribe both
		*UnsubAResult = Client->UnsubscribeFromChannel(TestChannelA);
		TestFalse("UnsubscribeFromChannel(A) should not have failed", UnsubAResult->Error);
		*UnsubBResult = SubB->Unsubscribe();
		TestFalse("Subscription->Unsubscribe(B) should not have failed", UnsubBResult->Error);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SubAResult, SubBResult, UnsubAResult, UnsubBResult]()
	{
		if (SubAResult->Error) AddError(FString::Printf(TEXT("Subscribe A failed: %s"), *SubAResult->ErrorMessage));
		if (SubBResult->Error) AddError(FString::Printf(TEXT("Subscribe B failed: %s"), *SubBResult->ErrorMessage));
		if (UnsubAResult->Error) AddError(FString::Printf(TEXT("Unsubscribe A failed: %s"), *UnsubAResult->ErrorMessage));
		if (UnsubBResult->Error) AddError(FString::Printf(TEXT("Unsubscribe B failed: %s"), *UnsubBResult->ErrorMessage));
	}, 0.1f));

	CleanUp();
	return true;
}


#endif // WITH_DEV_AUTOMATION_TESTS