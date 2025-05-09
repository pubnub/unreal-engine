#include "Misc/AutomationTest.h"
#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/PubnubTestsUtils.h"


IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessageTest, FPubnubAutomationTestBase, "PubnubE2E.PubSub.PublishMessage", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessageWithSettingsTest, FPubnubAutomationTestBase, "PubnubE2E.PubSub.PublishMessageWithSettingsTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);


bool FPubnubPublishMessageTest::RunTest(const FString& Parameters)
{
	//Initial variables
	FString TestMessage = "\"Message from test\"";
	FString TestUser = "test_user";
	FString TestChannel = "test_channel";
	TSharedPtr<bool> TestMessageReceived = MakeShared<bool>(false);
	
	if(!InitTest())
	{
		AddError("TestInitialization failed");
		return false;
	}

	//Set User ID - it's necessary for any operation
	PubnubSubsystem->SetUserID(TestUser);

	//Check for any errors on the way
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});

	//Add listener for received messages
	PubnubSubsystem->OnMessageReceivedNative.AddLambda([this, TestMessage, TestChannel, TestUser, TestMessageReceived](FPubnubMessageData ReceivedMessage)
	{
		*TestMessageReceived = true;
		TestEqual("Published message content", TestMessage, ReceivedMessage.Message);
		TestEqual("Published message channel", TestChannel, ReceivedMessage.Channel);
		TestEqual("Published message UserID", TestUser, ReceivedMessage.UserID);
	});

	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel]()
	{
		PubnubSubsystem->SubscribeToChannel(TestChannel);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		PubnubSubsystem->PublishMessage(TestChannel, TestMessage);
	}, 0.3f));

	//This will wait until message is received or timeout is reached
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestMessageReceived]() -> bool {
		return *TestMessageReceived;
	}, 5.0f));

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
	FString TestMessage = "\"Message from test\"";
	FString TestUser = "test_user";
	FString TestChannel = "test_channel";
	FString TestMetaData = "{\"metadata\": \"from test\"}";
	FString TestCustomMessageType = "custom_type";
	TSharedPtr<bool> TestMessageReceived = MakeShared<bool>(false);
	
	if(!InitTest())
	{
		AddError("TestInitialization failed");
		return false;
	}

	//Set User ID - it's necessary for any operation
	PubnubSubsystem->SetUserID(TestUser);

	//Check for any errors on the way
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});

	//Add listener for received messages
	PubnubSubsystem->OnMessageReceivedNative.AddLambda([this, TestMessage, TestChannel, TestUser, TestMessageReceived, TestMetaData, TestCustomMessageType](FPubnubMessageData ReceivedMessage)
	{
		*TestMessageReceived = true;
		TestEqual("Published message content", TestMessage, ReceivedMessage.Message);
		TestEqual("Published message channel", TestChannel, ReceivedMessage.Channel);
		TestEqual("Published message UserID", TestUser, ReceivedMessage.UserID);
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
	}, 0.3f));

	//This will wait until message is received or timeout is reached
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestMessageReceived]() -> bool {
		return *TestMessageReceived;
	}, 5.0f));

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
