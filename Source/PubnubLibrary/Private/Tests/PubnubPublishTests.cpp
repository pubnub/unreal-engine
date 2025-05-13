#include "Misc/AutomationTest.h"
#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/PubnubTestsUtils.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessageTest, FPubnubAutomationTestBase, "Pubnub.E2E.PubSub.PublishMessage", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessageWithSettingsTest, FPubnubAutomationTestBase, "Pubnub.E2E.PubSub.PublishMessageWithSettingsTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSignalTest, FPubnubAutomationTestBase, "Pubnub.E2E.PubSub.SendSignal", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSignalWithSettingsTest, FPubnubAutomationTestBase, "Pubnub.E2E.PubSub.SendSignalWithSettings", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);


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
		AddError(FString::Printf(TEXT("Pubnub Error in FPubnubSignalTest: %s"), *ErrorMessage));
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
		AddError(FString::Printf(TEXT("Pubnub Error in FPubnubSignalWithSettingsTest: %s"), *ErrorMessage));
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
