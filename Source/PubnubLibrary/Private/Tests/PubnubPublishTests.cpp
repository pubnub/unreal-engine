#include "Misc/AutomationTest.h"
#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessageTest, FPubnubAutomationTestBase, "Pubnub.Integration.PubSub.PublishMessage", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessageWithSettingsTest, FPubnubAutomationTestBase, "Pubnub.Integration.PubSub.PublishMessageWithSettingsTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSignalTest, FPubnubAutomationTestBase, "Pubnub.Integration.PubSub.SendSignal", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSignalWithSettingsTest, FPubnubAutomationTestBase, "Pubnub.Integration.PubSub.SendSignalWithSettings", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUnsubscribeTest, FPubnubAutomationTestBase, "Pubnub.Integration.PubSub.UnsubscribeFromChannel", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishVariousMessageTypesTest, FPubnubAutomationTestBase, "Pubnub.Integration.PubSub.PublishVariousMessageTypes", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FPubnubPublishMessageTest::RunTest(const FString& Parameters)
{
	//Initial variables
	const FString TestMessage = "\"Message from test\"";
	const FString TestUser = SDK_PREFIX + "test_user";
	const FString TestChannel = SDK_PREFIX + "test_channel";
	TSharedPtr<bool> TestMessageReceived = MakeShared<bool>(false);
	
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

	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel]()
	{
		PubnubSubsystem->SubscribeToChannel(TestChannel);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		PubnubSubsystem->PublishMessage(TestChannel, TestMessage);
	}, 0.5f));

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
	const FString TestMetaData = "{\"metadata\": \"from test\"}";
	const FString TestCustomMessageType = "custom_type";
	TSharedPtr<bool> TestMessageReceived = MakeShared<bool>(false);
	
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

	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel]()
	{
		PubnubSubsystem->SubscribeToChannel(TestChannel);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage, TestMetaData, TestCustomMessageType]()
	{
		FPubnubPublishSettings PublishSettings;
		PublishSettings.MetaData = TestMetaData;
		PublishSettings.CustomMessageType = TestCustomMessageType;
		PublishSettings.Ttl = 5;
		PubnubSubsystem->PublishMessage(TestChannel, TestMessage, PublishSettings);
	}, 0.5f));

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
	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel]()
	{
		PubnubSubsystem->SubscribeToChannel(TestChannel);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestSignalMessage]()
	{
		PubnubSubsystem->Signal(TestChannel, TestSignalMessage);
	}, 0.5f));

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
	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel]()
	{
		PubnubSubsystem->SubscribeToChannel(TestChannel);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestSignalMessage, TestCustomSignalType]()
	{
		FPubnubSignalSettings SignalSettings;
		SignalSettings.CustomMessageType = TestCustomSignalType;
		PubnubSubsystem->Signal(TestChannel, TestSignalMessage, SignalSettings);
	}, 0.5f));

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

	// Subscribe to the channel
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel]()
	{
		PubnubSubsystem->SubscribeToChannel(TestChannel);
	}, 0.5f));
	
	// Unsubscribe from the channel
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel]()
	{
		PubnubSubsystem->UnsubscribeFromChannel(TestChannel);
	}, 1.0f));
	
	// Publish a message to the channel we just unsubscribed from
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		PubnubSubsystem->PublishMessage(TestChannel, TestMessage);
	}, 1.0f));

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
	TestCases.Add(FMessageTestCase("JSON Array", "[\"element1\", 2, {\"nested_key\":\"nested_val\"}, false]"));
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
	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel]()
	{
		PubnubSubsystem->SubscribeToChannel(TestChannel);
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
