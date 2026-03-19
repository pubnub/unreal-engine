// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubClient.h"
#include "PubnubStructLibrary.h"
#include "PubnubEnumLibrary.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

/** Helper for various message type test cases: description, payload to send, and expected payload when received. */
struct FPubnubPublishMessageTestCase
{
	FString Description;
	FString MessageToSend;
	FString ExpectedReceivedMessage;

	FPubnubPublishMessageTestCase(const FString& InDesc, const FString& InMsgSend, const FString& InMsgExpect)
		: Description(InDesc), MessageToSend(InMsgSend), ExpectedReceivedMessage(InMsgExpect) {}
	FPubnubPublishMessageTestCase(const FString& InDesc, const FString& InMsg)
		: Description(InDesc), MessageToSend(InMsg), ExpectedReceivedMessage(InMsg) {}
};

// ---------------------------------------------------------------------------
// UPubnubClient::PublishMessage - Automated tests
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessage_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.PublishMessage.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessage_EmptyMessage_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.PublishMessage.1Validation.EmptyMessage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessage_ClientNotInitialized_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.PublishMessage.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessage_HappyPath_RequiredParamsOnly, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.PublishMessage.2HappyPath.RequiredParamsOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessage_FullPublishSettings_ReturnsCorrectData, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.PublishMessage.3FullParameters.AllParameters",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessage_ConcurrentSyncWhileAsyncInProgress_ReturnsMutexError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.PublishMessage.4Advanced.ConcurrentSyncWhileAsyncInProgress",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessage_VariousMessageTypes, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.PublishMessage.4Advanced.VariousMessageTypes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubPublishMessage_SubscribeThenPublish_MessageReceived, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.PublishMessage.4Advanced.SubscribeThenPublish_MessageReceived",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::Signal - Automated tests
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSignal_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.Signal.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSignal_EmptyMessage_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.Signal.1Validation.EmptyMessage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSignal_ClientNotInitialized_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.Signal.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSignal_HappyPath_RequiredParamsOnly, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.Signal.2HappyPath.RequiredParamsOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSignal_FullSignalSettings_ReturnsCorrectData, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.Signal.3FullParameters.CustomMessageType",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSignal_ConcurrentSyncWhileAsyncInProgress_ReturnsMutexError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.Signal.4Advanced.ConcurrentSyncWhileAsyncInProgress",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSignal_SubscribeThenSignal_MessageReceived, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.Signal.4Advanced.SubscribeThenSignal_MessageReceived",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::SubscribeToChannel - Automated tests (sync only)
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToChannel_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToChannel.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToChannel_ClientNotInitialized_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToChannel.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToChannel_HappyPath_RequiredParamsOnly, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToChannel.2HappyPath.RequiredParamsOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToChannel_FullSubscribeSettings_ReceivePresenceEvents, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToChannel.3FullParameters.ReceivePresenceEvents",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToChannel_AlreadySubscribed_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToChannel.4Advanced.AlreadySubscribed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToChannel_MultipleChannels_AllSucceed, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToChannel.4Advanced.MultipleChannels",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToChannel_UnsubscribeThenSubscribeAgain_Succeeds, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToChannel.4Advanced.UnsubscribeThenSubscribeAgain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToChannel_MultipleChannels_UnsubscribeOne_Resubscribe_Succeeds, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToChannel.4Advanced.MultipleChannelsUnsubscribeOneResubscribe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToChannel_SubscribeThenPublish_ReceivesMessage, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToChannel.4Advanced.SubscribeThenPublish_ReceivesMessage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::UnsubscribeFromChannel - Automated tests (sync only)
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUnsubscribeFromChannel_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.UnsubscribeFromChannel.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUnsubscribeFromChannel_ClientNotInitialized_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.UnsubscribeFromChannel.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUnsubscribeFromChannel_NoSuchSubscription_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.UnsubscribeFromChannel.1Validation.NoSuchSubscription",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUnsubscribeFromChannel_HappyPath_ThenMessageNotReceived, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.UnsubscribeFromChannel.2HappyPath.SubscribeThenUnsubscribe_MessageNotReceivedAfter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUnsubscribeFromChannel_UnsubscribeOneOfTwo_OnlySubscribedChannelReceives, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.UnsubscribeFromChannel.4Advanced.UnsubscribeOneOfTwo_OnlySubscribedReceives",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUnsubscribeFromChannel_UnsubscribeTwice_SecondReturnsNoSuchSubscription, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.UnsubscribeFromChannel.4Advanced.UnsubscribeTwice_SecondFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::SubscribeToGroup - Automated tests (sync only)
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToGroup_EmptyChannelGroup_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToGroup.1Validation.EmptyChannelGroup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToGroup_ClientNotInitialized_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToGroup.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToGroup_HappyPath_RequiredParamsOnly, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToGroup.2HappyPath.RequiredParamsOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToGroup_FullSubscribeSettings_ReceivePresenceEvents, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToGroup.3FullParameters.ReceivePresenceEvents",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToGroup_AlreadySubscribed_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToGroup.4Advanced.AlreadySubscribed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToGroup_UnsubscribeThenSubscribeAgain_ReceivesMessage, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToGroup.4Advanced.UnsubscribeThenSubscribeAgain_ReceivesMessage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSubscribeToGroup_AddChannelThenPublish_ReceivesMessage, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.SubscribeToGroup.4Advanced.AddChannelThenPublish_ReceivesMessage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::UnsubscribeFromGroup - Automated tests (sync only)
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUnsubscribeFromGroup_EmptyChannelGroup_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.UnsubscribeFromGroup.1Validation.EmptyChannelGroup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUnsubscribeFromGroup_ClientNotInitialized_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.UnsubscribeFromGroup.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUnsubscribeFromGroup_NoSuchSubscription_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.UnsubscribeFromGroup.1Validation.NoSuchSubscription",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUnsubscribeFromGroup_HappyPath_ThenMessageNotReceived, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.UnsubscribeFromGroup.2HappyPath.SubscribeThenUnsubscribe_MessageNotReceivedAfter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUnsubscribeFromGroup_UnsubscribeTwice_SecondReturnsNoSuchSubscription, FPubnubAutomationTestBase,
	"Pubnub.Integration.PubSub.UnsubscribeFromGroup.4Advanced.UnsubscribeTwice_SecondFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// Input validation (fast-fail) - PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY
// UserID is set in all tests per requirement; we do not test UserID-not-set.
// ---------------------------------------------------------------------------

bool FPubnubPublishMessage_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	const FString ValidMessage = TEXT("\"test message\"");
	FPubnubPublishSettings Settings;

	FPubnubPublishMessageResult Result = PubnubClient->PublishMessage(FString(), ValidMessage, Settings);

	TestTrue("Result should indicate error", Result.Result.Error);
	TestTrue("ErrorMessage should mention Channel", Result.Result.ErrorMessage.Contains(TEXT("Channel")));
	TestTrue("ErrorMessage should indicate field is empty", Result.Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubPublishMessage_EmptyMessage_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	const FString ValidChannel = SDK_PREFIX + "publish_empty_msg_ch";
	FPubnubPublishSettings Settings;

	FPubnubPublishMessageResult Result = PubnubClient->PublishMessage(ValidChannel, FString(), Settings);

	TestTrue("Result should indicate error", Result.Result.Error);
	TestTrue("ErrorMessage should mention Message", Result.Result.ErrorMessage.Contains(TEXT("Message")));
	TestTrue("ErrorMessage should indicate field is empty", Result.Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

// PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED: call PublishMessage after client is destroyed (deinitialized).
bool FPubnubPublishMessage_ClientNotInitialized_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();

	FPubnubPublishMessageResult Result = Client->PublishMessage(
		SDK_PREFIX + "ch",
		TEXT("\"msg\""),
		FPubnubPublishSettings());

	TestTrue("Result should indicate error", Result.Result.Error);
	TestTrue("ErrorMessage should mention not initialized",
		Result.Result.ErrorMessage.Contains(TEXT("not initialized")) || Result.Result.ErrorMessage.Contains(TEXT("invalid")));

	CleanUp();
	return true;
}

// Happy path: only required params (Channel, Message), default PublishSettings; verify result and returned data.
bool FPubnubPublishMessage_HappyPath_RequiredParamsOnly::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "publish_happy_ch";
	const FString TestMessage = TEXT("\"Happy path message\"");
	const FString TestUser = SDK_PREFIX + "publish_happy_user";

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubPublishMessageResult Result = PubnubClient->PublishMessage(TestChannel, TestMessage);

	TestFalse("Publish should succeed", Result.Result.Error);
	TestEqual("HTTP status should be 200", Result.Result.Status, 200);
	TestEqual("Published message content should match input", Result.PublishedMessage.Message, TestMessage);
	TestEqual("Published channel should match input", Result.PublishedMessage.Channel, TestChannel);
	TestEqual("Published UserID should match client", Result.PublishedMessage.UserID, TestUser);
	TestEqual("MessageType should be PMT_Published", Result.PublishedMessage.MessageType, EPubnubMessageType::PMT_Published);
	TestFalse("Timetoken should be non-empty", Result.PublishedMessage.Timetoken.IsEmpty());

	CleanUp();
	return true;
}

// Full test of optional PublishSettings: MetaData, CustomMessageType, Ttl, StoreInHistory, Replicate, PublishMethod.
// Verifies returned PublishedMessage reflects MetaData and CustomMessageType; success and basic data correctness.
bool FPubnubPublishMessage_FullPublishSettings_ReturnsCorrectData::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "publish_full_settings_ch";
	const FString TestMessage = TEXT("{\"text\":\"full settings\"}");
	const FString TestUser = SDK_PREFIX + "publish_full_settings_user";
	const FString TestMetaData = TEXT("{\"source\":\"test\",\"version\":1}");
	const FString TestCustomMessageType = TEXT("test-type-1");

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubPublishSettings Settings;
	Settings.MetaData = TestMetaData;
	Settings.CustomMessageType = TestCustomMessageType;
	Settings.Ttl = 24;
	Settings.StoreInHistory = true;
	Settings.Replicate = true;
	Settings.PublishMethod = EPubnubPublishMethod::PPM_SendViaGET;

	FPubnubPublishMessageResult Result = PubnubClient->PublishMessage(TestChannel, TestMessage, Settings);

	TestFalse("Publish with full settings should succeed", Result.Result.Error);
	TestEqual("HTTP status should be 200", Result.Result.Status, 200);
	TestEqual("Published message should match input", Result.PublishedMessage.Message, TestMessage);
	TestEqual("Published channel should match", Result.PublishedMessage.Channel, TestChannel);
	TestEqual("Published UserID should match", Result.PublishedMessage.UserID, TestUser);
	TestEqual("Published Metadata should match PublishSettings.MetaData", Result.PublishedMessage.Metadata, TestMetaData);
	TestEqual("Published CustomMessageType should match PublishSettings", Result.PublishedMessage.CustomMessageType, TestCustomMessageType);
	TestEqual("MessageType should be PMT_Published", Result.PublishedMessage.MessageType, EPubnubMessageType::PMT_Published);
	TestFalse("Timetoken should be non-empty", Result.PublishedMessage.Timetoken.IsEmpty());

	CleanUp();
	return true;
}

// Concurrent call: sync PublishMessage while an async operation is in progress may fail with "Another Pubnub operation is in progress".
// Queues async publish, then after a short delay calls sync PublishMessage. If sync fails, asserts mutex error; if sync succeeds (won race), test still passes.
bool FPubnubPublishMessage_ConcurrentSyncWhileAsyncInProgress_ReturnsMutexError::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "publish_mutex_ch";
	const FString TestMessage = TEXT("\"concurrent test\"");
	const FString TestUser = SDK_PREFIX + "publish_mutex_user";

	TSharedPtr<FPubnubPublishMessageResult> SyncResult = MakeShared<FPubnubPublishMessageResult>();

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		PubnubClient->PublishMessageAsync(TestChannel, TestMessage, FOnPubnubPublishMessageResponseNative());
	}, 0.1f));

	// Give async a chance to acquire the mutex, then call sync on main thread.
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SyncResult]()
	{
		*SyncResult = PubnubClient->PublishMessage(TestChannel, TEXT("\"sync while async\""), FPubnubPublishSettings());
	}, 0.15f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SyncResult]()
	{
		if (SyncResult->Result.Error)
		{
			TestTrue("When sync fails during concurrent op, ErrorMessage should mention operation in progress",
				SyncResult->Result.ErrorMessage.Contains(TEXT("operation is in progress")) ||
				SyncResult->Result.ErrorMessage.Contains(TEXT("concurrently")));
		}
		// If sync succeeded (won the race), test passes; we only assert error message when error path is hit.
	}, 0.2f));

	CleanUp();
	return true;
}

// Publish various message types (string, JSON, number, boolean, array, null, escapes); verify PublishMessage succeeds and received content matches.
bool FPubnubPublishMessage_VariousMessageTypes::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "publish_various_msg_ch";

	TArray<FPubnubPublishMessageTestCase> TestCases;
	// Non-JSON string is serialized by SDK to JSON string, so we expect "\"hello world\"" on receive.
	TestCases.Add(FPubnubPublishMessageTestCase("Simple String", "hello world"));
	TestCases.Add(FPubnubPublishMessageTestCase("Simple String as JSON String", "\"hello world\""));
	TestCases.Add(FPubnubPublishMessageTestCase("JSON Object", "{\"message\":\"this is an object\",\"count\":1}"));
	TestCases.Add(FPubnubPublishMessageTestCase("Integer Number as JSON Number", "789"));
	TestCases.Add(FPubnubPublishMessageTestCase("Floating Point Number as JSON Number", "123.456"));
	TestCases.Add(FPubnubPublishMessageTestCase("Boolean true as JSON Boolean", "true"));
	TestCases.Add(FPubnubPublishMessageTestCase("Boolean false as JSON Boolean", "false"));
	TestCases.Add(FPubnubPublishMessageTestCase("JSON Array", "[\"element1\",2,{\"nested_key\":\"nested_val\"},false]"));
	TestCases.Add(FPubnubPublishMessageTestCase("Empty String as JSON string", "\"\""));
	TestCases.Add(FPubnubPublishMessageTestCase("String with escapes as JSON string", "\"Text with \\\"quotes\\\", \\\\backslashes\\\\, a /slash, and a newline\\ncharacter.\""));
	TestCases.Add(FPubnubPublishMessageTestCase("JSON null", "null"));

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});

	TSharedPtr<int32> CurrentTestCaseIndex = MakeShared<int32>(-1);
	TSharedPtr<bool> bMessageReceived = MakeShared<bool>(false);
	TSharedPtr<FPubnubMessageData> LastReceivedMessage = MakeShared<FPubnubMessageData>();

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[this, TestChannel, CurrentTestCaseIndex, bMessageReceived, LastReceivedMessage](const FPubnubMessageData& ReceivedMessage)
		{
			if (*CurrentTestCaseIndex >= 0 && ReceivedMessage.Channel == TestChannel && !(*bMessageReceived))
			{
				*LastReceivedMessage = ReceivedMessage;
				*bMessageReceived = true;
			}
		});

	// Subscribe (sync) then run each test case via latent commands.
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel]()
	{
		FPubnubOperationResult SubResult = PubnubClient->SubscribeToChannel(TestChannel);
		TestFalse("Subscribe should succeed", SubResult.Error);
		TestEqual("Subscribe status", SubResult.Status, 200);
	}, 0.1f));

	for (int32 i = 0; i < TestCases.Num(); ++i)
	{
		const int32 CaseIndex = i;
		const FPubnubPublishMessageTestCase& Case = TestCases[i];

		ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([CurrentTestCaseIndex, bMessageReceived, CaseIndex]()
		{
			*CurrentTestCaseIndex = CaseIndex;
			*bMessageReceived = false;
		}, 0.1f));

		ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, Case]()
		{
			FPubnubPublishMessageResult Result = PubnubClient->PublishMessage(TestChannel, Case.MessageToSend);
			TestFalse(Case.Description + " - Publish should succeed", Result.Result.Error);
			TestEqual(Case.Description + " - Publish status", Result.Result.Status, 200);
		}, 0.1f));

		ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageReceived]() { return *bMessageReceived; }, MAX_WAIT_TIME));

		ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
			[this, Case, bMessageReceived, LastReceivedMessage, TestChannel]()
			{
				if (!*bMessageReceived)
				{
					AddError(FString::Printf(TEXT("Test case '%s': Message was not received via OnMessageReceivedNative."), *Case.Description));
					return;
				}
				TestEqual(Case.Description + " - Channel", TestChannel, LastReceivedMessage->Channel);
				TestEqual(Case.Description + " - MessageType", EPubnubMessageType::PMT_Published, LastReceivedMessage->MessageType);
				TestFalse(Case.Description + " - Timetoken non-empty", LastReceivedMessage->Timetoken.IsEmpty());

				bool bIsJsonLike = Case.ExpectedReceivedMessage.StartsWith(TEXT("{")) || Case.ExpectedReceivedMessage.StartsWith(TEXT("["));
				if (bIsJsonLike)
				{
					TestTrue(Case.Description + " - Content (JSON equal)",
						UPubnubJsonUtilities::AreJsonObjectStringsEqual(Case.ExpectedReceivedMessage, LastReceivedMessage->Message));
				}
				else
				{
					TestEqual(Case.Description + " - Content", Case.ExpectedReceivedMessage, LastReceivedMessage->Message);
				}
			}, 0.1f));
	}

	CleanUp();
	return true;
}

// Subscribe first, then PublishMessage; verify OnMessageReceivedNative fires with correct Message, Channel, UserID, MessageType, Timetoken.
bool FPubnubPublishMessage_SubscribeThenPublish_MessageReceived::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "publish_sub_then_pub_ch";
	const FString TestMessage = TEXT("\"Message after subscribe\"");
	const FString TestUser = SDK_PREFIX + "publish_sub_then_pub_user";

	TSharedPtr<bool> bMessageReceived = MakeShared<bool>(false);
	TSharedPtr<FPubnubMessageData> ReceivedMessage = MakeShared<FPubnubMessageData>();
	TSharedPtr<bool> bSubscribeDone = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[this, TestChannel, TestMessage, bMessageReceived, ReceivedMessage](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == TestChannel && Msg.Message == TestMessage)
			{
				*ReceivedMessage = Msg;
				*bMessageReceived = true;
			}
		});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, bSubscribeDone]()
	{
		FPubnubOperationResult SubResult = PubnubClient->SubscribeToChannel(TestChannel);
		*bSubscribeDone = true;
		TestFalse("Subscribe should succeed", SubResult.Error);
		TestEqual("Subscribe status", SubResult.Status, 200);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeDone]() { return *bSubscribeDone; }, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		FPubnubPublishMessageResult PubResult = PubnubClient->PublishMessage(TestChannel, TestMessage);
		TestFalse("Publish should succeed", PubResult.Result.Error);
		TestEqual("Publish status", PubResult.Result.Status, 200);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageReceived]() { return *bMessageReceived; }, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bMessageReceived, ReceivedMessage, TestChannel, TestMessage, TestUser]()
	{
		if (!*bMessageReceived)
		{
			AddError("OnMessageReceivedNative was not invoked with the published message.");
			return;
		}
		TestEqual("Received Message", TestMessage, ReceivedMessage->Message);
		TestEqual("Received Channel", TestChannel, ReceivedMessage->Channel);
		TestEqual("Received UserID", TestUser, ReceivedMessage->UserID);
		TestEqual("Received MessageType", EPubnubMessageType::PMT_Published, ReceivedMessage->MessageType);
		TestFalse("Received Timetoken non-empty", ReceivedMessage->Timetoken.IsEmpty());
	}, 0.1f));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// UPubnubClient::Signal - Input validation (fast-fail)
// ---------------------------------------------------------------------------

bool FPubnubSignal_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	const FString ValidMessage = TEXT("\"test signal\"");
	FPubnubSignalSettings Settings;

	FPubnubSignalResult Result = PubnubClient->Signal(FString(), ValidMessage, Settings);

	TestTrue("Result should indicate error", Result.Result.Error);
	TestTrue("ErrorMessage should mention Channel", Result.Result.ErrorMessage.Contains(TEXT("Channel")));
	TestTrue("ErrorMessage should indicate field is empty", Result.Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubSignal_EmptyMessage_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	const FString ValidChannel = SDK_PREFIX + "signal_empty_msg_ch";
	FPubnubSignalSettings Settings;

	FPubnubSignalResult Result = PubnubClient->Signal(ValidChannel, FString(), Settings);

	TestTrue("Result should indicate error", Result.Result.Error);
	TestTrue("ErrorMessage should mention Message", Result.Result.ErrorMessage.Contains(TEXT("Message")));
	TestTrue("ErrorMessage should indicate field is empty", Result.Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

// PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED: call Signal after client is destroyed (deinitialized).
bool FPubnubSignal_ClientNotInitialized_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();

	FPubnubSignalResult Result = Client->Signal(
		SDK_PREFIX + "ch",
		TEXT("\"signal msg\""),
		FPubnubSignalSettings());

	TestTrue("Result should indicate error", Result.Result.Error);
	TestTrue("ErrorMessage should mention not initialized or invalid",
		Result.Result.ErrorMessage.Contains(TEXT("not initialized")) || Result.Result.ErrorMessage.Contains(TEXT("invalid")));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// UPubnubClient::Signal - Happy path and full parameters
// ---------------------------------------------------------------------------

bool FPubnubSignal_HappyPath_RequiredParamsOnly::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "signal_happy_ch";
	const FString TestMessage = TEXT("\"Happy path signal\"");
	const FString TestUser = SDK_PREFIX + "signal_happy_user";

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubSignalResult Result = PubnubClient->Signal(TestChannel, TestMessage);

	TestFalse("Signal should succeed", Result.Result.Error);
	TestEqual("HTTP status should be 200", Result.Result.Status, 200);
	TestEqual("Signal message content should match input", Result.SignalMessage.Message, TestMessage);
	TestEqual("Signal channel should match input", Result.SignalMessage.Channel, TestChannel);
	TestEqual("Signal UserID should match client", Result.SignalMessage.UserID, TestUser);
	TestEqual("MessageType should be PMT_Signal", Result.SignalMessage.MessageType, EPubnubMessageType::PMT_Signal);
	TestTrue("Metadata should be empty for signals", Result.SignalMessage.Metadata.IsEmpty());
	TestFalse("Timetoken should be non-empty", Result.SignalMessage.Timetoken.IsEmpty());

	CleanUp();
	return true;
}

// Full test of optional FPubnubSignalSettings: CustomMessageType. Verifies returned SignalMessage reflects it.
bool FPubnubSignal_FullSignalSettings_ReturnsCorrectData::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "signal_full_settings_ch";
	const FString TestMessage = TEXT("{\"event\":\"signal\"}");
	const FString TestUser = SDK_PREFIX + "signal_full_settings_user";
	const FString TestCustomMessageType = TEXT("custom-signal-type");

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubSignalSettings Settings;
	Settings.CustomMessageType = TestCustomMessageType;

	FPubnubSignalResult Result = PubnubClient->Signal(TestChannel, TestMessage, Settings);

	TestFalse("Signal with CustomMessageType should succeed", Result.Result.Error);
	TestEqual("HTTP status should be 200", Result.Result.Status, 200);
	TestEqual("Signal message should match input", Result.SignalMessage.Message, TestMessage);
	TestEqual("Signal channel should match", Result.SignalMessage.Channel, TestChannel);
	TestEqual("Signal UserID should match", Result.SignalMessage.UserID, TestUser);
	TestEqual("CustomMessageType should match SignalSettings", Result.SignalMessage.CustomMessageType, TestCustomMessageType);
	TestEqual("MessageType should be PMT_Signal", Result.SignalMessage.MessageType, EPubnubMessageType::PMT_Signal);
	TestTrue("Metadata should be empty for signals", Result.SignalMessage.Metadata.IsEmpty());
	TestFalse("Timetoken should be non-empty", Result.SignalMessage.Timetoken.IsEmpty());

	CleanUp();
	return true;
}

// Concurrent call: sync Signal while an async operation is in progress may fail with "Another Pubnub operation is in progress".
bool FPubnubSignal_ConcurrentSyncWhileAsyncInProgress_ReturnsMutexError::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "signal_mutex_ch";
	const FString TestMessage = TEXT("\"concurrent signal\"");
	const FString TestUser = SDK_PREFIX + "signal_mutex_user";

	TSharedPtr<FPubnubSignalResult> SyncResult = MakeShared<FPubnubSignalResult>();

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		PubnubClient->SignalAsync(TestChannel, TestMessage, FOnPubnubSignalResponseNative());
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SyncResult]()
	{
		*SyncResult = PubnubClient->Signal(TestChannel, TEXT("\"sync while async\""), FPubnubSignalSettings());
	}, 0.15f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SyncResult]()
	{
		if (SyncResult->Result.Error)
		{
			TestTrue("When sync fails during concurrent op, ErrorMessage should mention operation in progress",
				SyncResult->Result.ErrorMessage.Contains(TEXT("operation is in progress")) ||
				SyncResult->Result.ErrorMessage.Contains(TEXT("concurrently")));
		}
	}, 0.2f));

	CleanUp();
	return true;
}

// Subscribe first, then Signal; verify OnMessageReceivedNative fires with PMT_Signal and correct Message, Channel, UserID, Timetoken.
bool FPubnubSignal_SubscribeThenSignal_MessageReceived::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "signal_sub_then_signal_ch";
	const FString TestMessage = TEXT("\"Signal after subscribe\"");
	const FString TestUser = SDK_PREFIX + "signal_sub_then_signal_user";

	TSharedPtr<bool> bMessageReceived = MakeShared<bool>(false);
	TSharedPtr<FPubnubMessageData> ReceivedMessage = MakeShared<FPubnubMessageData>();
	TSharedPtr<bool> bSubscribeDone = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[this, TestChannel, TestMessage, bMessageReceived, ReceivedMessage](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == TestChannel && Msg.Message == TestMessage && Msg.MessageType == EPubnubMessageType::PMT_Signal)
			{
				*ReceivedMessage = Msg;
				*bMessageReceived = true;
			}
		});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, bSubscribeDone]()
	{
		FPubnubOperationResult SubResult = PubnubClient->SubscribeToChannel(TestChannel);
		*bSubscribeDone = true;
		TestFalse("Subscribe should succeed", SubResult.Error);
		TestEqual("Subscribe status", SubResult.Status, 200);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeDone]() { return *bSubscribeDone; }, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		FPubnubSignalResult SignalResult = PubnubClient->Signal(TestChannel, TestMessage);
		TestFalse("Signal should succeed", SignalResult.Result.Error);
		TestEqual("Signal status", SignalResult.Result.Status, 200);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageReceived]() { return *bMessageReceived; }, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bMessageReceived, ReceivedMessage, TestChannel, TestMessage, TestUser]()
	{
		if (!*bMessageReceived)
		{
			AddError("OnMessageReceivedNative was not invoked with the signal message (PMT_Signal).");
			return;
		}
		TestEqual("Received Message", TestMessage, ReceivedMessage->Message);
		TestEqual("Received Channel", TestChannel, ReceivedMessage->Channel);
		TestEqual("Received UserID", TestUser, ReceivedMessage->UserID);
		TestEqual("Received MessageType", EPubnubMessageType::PMT_Signal, ReceivedMessage->MessageType);
		TestFalse("Received Timetoken non-empty", ReceivedMessage->Timetoken.IsEmpty());
	}, 0.1f));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// UPubnubClient::SubscribeToChannel - Input validation (fast-fail)
// ---------------------------------------------------------------------------

bool FPubnubSubscribeToChannel_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	FPubnubOperationResult Result = PubnubClient->SubscribeToChannel(FString());

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention Channel", Result.ErrorMessage.Contains(TEXT("Channel")));
	TestTrue("ErrorMessage should indicate field is empty", Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

// PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED: call SubscribeToChannel after client is destroyed.
bool FPubnubSubscribeToChannel_ClientNotInitialized_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();

	FPubnubOperationResult Result = Client->SubscribeToChannel(SDK_PREFIX + "ch");

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention not initialized or invalid",
		Result.ErrorMessage.Contains(TEXT("not initialized")) || Result.ErrorMessage.Contains(TEXT("invalid")));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// UPubnubClient::SubscribeToChannel - Happy path and full parameters
// ---------------------------------------------------------------------------

bool FPubnubSubscribeToChannel_HappyPath_RequiredParamsOnly::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "sub_happy_ch";
	const FString TestUser = SDK_PREFIX + "sub_happy_user";

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubOperationResult Result = PubnubClient->SubscribeToChannel(TestChannel);

	TestFalse("Subscribe should succeed", Result.Error);
	TestEqual("HTTP status should be 200", Result.Status, 200);

	TSharedPtr<bool> bMessageReceived = MakeShared<bool>(false);
	PubnubClient->OnMessageReceivedNative.AddLambda(
		[TestChannel, bMessageReceived](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == TestChannel) { *bMessageReceived = true; }
		});

	const FString TestMessage = TEXT("\"Happy path verify\"");
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		FPubnubPublishMessageResult PubResult = PubnubClient->PublishMessage(TestChannel, TestMessage);
		TestFalse("Publish should succeed", PubResult.Result.Error);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageReceived]() { return *bMessageReceived; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bMessageReceived]()
	{
		TestTrue("Subscription success confirmed by receiving published message", *bMessageReceived);
	}, 0.1f));

	CleanUp();
	return true;
}

// Full test of optional FPubnubSubscribeSettings: ReceivePresenceEvents. Verifies subscribe succeeds and subscription works by receiving a message.
bool FPubnubSubscribeToChannel_FullSubscribeSettings_ReceivePresenceEvents::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "sub_full_settings_ch";
	const FString TestUser = SDK_PREFIX + "sub_full_settings_user";
	const FString TestMessage = TEXT("\"Full settings verify\"");

	TSharedPtr<bool> bMessageReceived = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubSubscribeSettings Settings;
	Settings.ReceivePresenceEvents = true;

	FPubnubOperationResult Result = PubnubClient->SubscribeToChannel(TestChannel, Settings);

	TestFalse("Subscribe with ReceivePresenceEvents should succeed", Result.Error);
	TestEqual("HTTP status should be 200", Result.Status, 200);

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[TestChannel, TestMessage, bMessageReceived](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == TestChannel && Msg.Message == TestMessage) { *bMessageReceived = true; }
		});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		FPubnubPublishMessageResult PubResult = PubnubClient->PublishMessage(TestChannel, TestMessage);
		TestFalse("Publish should succeed", PubResult.Result.Error);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageReceived]() { return *bMessageReceived; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bMessageReceived]()
	{
		TestTrue("Subscription success confirmed by receiving published message", *bMessageReceived);
	}, 0.1f));

	CleanUp();
	return true;
}

// Subscribing to the same channel twice returns error "Already subscribed to this channel." First subscription confirmed by receiving a message.
bool FPubnubSubscribeToChannel_AlreadySubscribed_ReturnsError::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "sub_already_ch";
	const FString TestUser = SDK_PREFIX + "sub_already_user";
	const FString TestMessage = TEXT("\"First sub verify\"");
	TSharedPtr<bool> bMessageReceived = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubOperationResult FirstResult = PubnubClient->SubscribeToChannel(TestChannel);
	TestFalse("First subscribe should succeed", FirstResult.Error);
	TestEqual("First subscribe status", FirstResult.Status, 200);

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[TestChannel, TestMessage, bMessageReceived](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == TestChannel && Msg.Message == TestMessage) { *bMessageReceived = true; }
		});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		FPubnubPublishMessageResult PubResult = PubnubClient->PublishMessage(TestChannel, TestMessage);
		TestFalse("Publish should succeed", PubResult.Result.Error);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageReceived]() { return *bMessageReceived; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bMessageReceived]()
	{
		TestTrue("First subscription success confirmed by receiving message", *bMessageReceived);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel]()
	{
		FPubnubOperationResult SecondResult = PubnubClient->SubscribeToChannel(TestChannel);
		TestTrue("Second subscribe to same channel should fail", SecondResult.Error);
		TestTrue("ErrorMessage should mention already subscribed",
			SecondResult.ErrorMessage.Contains(TEXT("Already subscribed")) || SecondResult.ErrorMessage.Contains(TEXT("already exists")));
	}, 0.1f));

	CleanUp();
	return true;
}

// Subscribe to multiple different channels in sequence; each succeeds and subscription to each is confirmed by receiving a message on that channel.
bool FPubnubSubscribeToChannel_MultipleChannels_AllSucceed::RunTest(const FString& Parameters)
{
	const FString TestUser = SDK_PREFIX + "sub_multi_user";
	TArray<FString> Channels = { SDK_PREFIX + "sub_multi_a", SDK_PREFIX + "sub_multi_b", SDK_PREFIX + "sub_multi_c" };
	TArray<FString> Messages = { TEXT("\"msg_a\""), TEXT("\"msg_b\""), TEXT("\"msg_c\"") };
	TSharedPtr<TArray<bool>> Received = MakeShared<TArray<bool>>();
	Received->SetNum(3);
	for (int32 i = 0; i < 3; ++i) { (*Received)[i] = false; }

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	for (int32 i = 0; i < Channels.Num(); ++i)
	{
		FPubnubOperationResult Result = PubnubClient->SubscribeToChannel(Channels[i]);
		TestFalse(FString::Printf(TEXT("Subscribe to channel %d should succeed"), i), Result.Error);
		TestEqual(FString::Printf(TEXT("Subscribe to channel %d status"), i), Result.Status, 200);
	}

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[Channels, Messages, Received](const FPubnubMessageData& Msg)
		{
			for (int32 i = 0; i < 3; ++i)
			{
				if (Msg.Channel == Channels[i] && Msg.Message == Messages[i]) { (*Received)[i] = true; break; }
			}
		});

	for (int32 i = 0; i < 3; ++i)
	{
		const int32 Idx = i;
		ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Channels, Messages, Idx]()
		{
			FPubnubPublishMessageResult PubResult = PubnubClient->PublishMessage(Channels[Idx], Messages[Idx]);
			TestFalse(FString::Printf(TEXT("Publish to channel %d should succeed"), Idx), PubResult.Result.Error);
		}, 0.1f));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([Received, Idx]() { return (*Received)[Idx]; }, MAX_WAIT_TIME));
	}
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Received]()
	{
		TestTrue("Channel 0 subscription confirmed by receive", (*Received)[0]);
		TestTrue("Channel 1 subscription confirmed by receive", (*Received)[1]);
		TestTrue("Channel 2 subscription confirmed by receive", (*Received)[2]);
	}, 0.1f));

	CleanUp();
	return true;
}

// Unsubscribe then Subscribe again to the same channel; both subscribe calls succeed and messages can be received after re-subscribe.
bool FPubnubSubscribeToChannel_UnsubscribeThenSubscribeAgain_Succeeds::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "sub_unsub_resub_ch";
	const FString TestUser = SDK_PREFIX + "sub_unsub_resub_user";
	const FString TestMessage = TEXT("\"After resubscribe\"");

	TSharedPtr<bool> bMessageReceived = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubOperationResult Sub1 = PubnubClient->SubscribeToChannel(TestChannel);
	TestFalse("First subscribe should succeed", Sub1.Error);
	TestEqual("First subscribe status", Sub1.Status, 200);

	FPubnubOperationResult Unsub = PubnubClient->UnsubscribeFromChannel(TestChannel);
	TestFalse("Unsubscribe should succeed", Unsub.Error);
	TestEqual("Unsubscribe status", Unsub.Status, 200);

	FPubnubOperationResult Sub2 = PubnubClient->SubscribeToChannel(TestChannel);
	TestFalse("Second subscribe (after unsubscribe) should succeed", Sub2.Error);
	TestEqual("Second subscribe status", Sub2.Status, 200);

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[TestChannel, TestMessage, bMessageReceived](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == TestChannel && Msg.Message == TestMessage)
			{
				*bMessageReceived = true;
			}
		});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		FPubnubPublishMessageResult PubResult = PubnubClient->PublishMessage(TestChannel, TestMessage);
		TestFalse("Publish after resubscribe should succeed", PubResult.Result.Error);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageReceived]() { return *bMessageReceived; }, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bMessageReceived]()
	{
		TestTrue("Message should be received after unsubscribe-then-subscribe-again", *bMessageReceived);
	}, 0.1f));

	CleanUp();
	return true;
}

// Subscribe to two channels, unsubscribe from one, then subscribe to the unsubscribed channel again; all succeed. Confirm both A and B receive after re-subscribe.
bool FPubnubSubscribeToChannel_MultipleChannels_UnsubscribeOne_Resubscribe_Succeeds::RunTest(const FString& Parameters)
{
	const FString ChannelA = SDK_PREFIX + "sub_multi_unsub_a";
	const FString ChannelB = SDK_PREFIX + "sub_multi_unsub_b";
	const FString MessageA = TEXT("\"msg_to_a\"");
	const FString MessageB = TEXT("\"msg_to_b\"");
	const FString TestUser = SDK_PREFIX + "sub_multi_unsub_user";
	TSharedPtr<bool> bReceivedA = MakeShared<bool>(false);
	TSharedPtr<bool> bReceivedB = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubOperationResult SubA1 = PubnubClient->SubscribeToChannel(ChannelA);
	TestFalse("Subscribe to A should succeed", SubA1.Error);
	TestEqual("Subscribe A status", SubA1.Status, 200);

	FPubnubOperationResult SubB = PubnubClient->SubscribeToChannel(ChannelB);
	TestFalse("Subscribe to B should succeed", SubB.Error);
	TestEqual("Subscribe B status", SubB.Status, 200);

	FPubnubOperationResult UnsubA = PubnubClient->UnsubscribeFromChannel(ChannelA);
	TestFalse("Unsubscribe from A should succeed", UnsubA.Error);
	TestEqual("Unsubscribe A status", UnsubA.Status, 200);

	FPubnubOperationResult SubA2 = PubnubClient->SubscribeToChannel(ChannelA);
	TestFalse("Re-subscribe to A should succeed", SubA2.Error);
	TestEqual("Re-subscribe A status", SubA2.Status, 200);

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[ChannelA, ChannelB, MessageA, MessageB, bReceivedA, bReceivedB](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == ChannelA && Msg.Message == MessageA) { *bReceivedA = true; }
			if (Msg.Channel == ChannelB && Msg.Message == MessageB) { *bReceivedB = true; }
		});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelA, ChannelB, MessageA, MessageB]()
	{
		FPubnubPublishMessageResult PubA = PubnubClient->PublishMessage(ChannelA, MessageA);
		FPubnubPublishMessageResult PubB = PubnubClient->PublishMessage(ChannelB, MessageB);
		TestFalse("Publish to A should succeed", PubA.Result.Error);
		TestFalse("Publish to B should succeed", PubB.Result.Error);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bReceivedA, bReceivedB]() { return *bReceivedA && *bReceivedB; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bReceivedA, bReceivedB]()
	{
		TestTrue("Subscription to A (after resubscribe) confirmed by receive", *bReceivedA);
		TestTrue("Subscription to B confirmed by receive", *bReceivedB);
	}, 0.1f));

	CleanUp();
	return true;
}

// Subscribe to channel then publish; verify OnMessageReceivedNative fires with the published message (proves subscription is active).
bool FPubnubSubscribeToChannel_SubscribeThenPublish_ReceivesMessage::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "sub_then_pub_ch";
	const FString TestMessage = TEXT("\"Subscribe then publish\"");
	const FString TestUser = SDK_PREFIX + "sub_then_pub_user";

	TSharedPtr<bool> bMessageReceived = MakeShared<bool>(false);
	TSharedPtr<FPubnubMessageData> ReceivedMessage = MakeShared<FPubnubMessageData>();
	TSharedPtr<bool> bSubscribeDone = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[TestChannel, TestMessage, bMessageReceived, ReceivedMessage](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == TestChannel && Msg.Message == TestMessage)
			{
				*ReceivedMessage = Msg;
				*bMessageReceived = true;
			}
		});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, bSubscribeDone]()
	{
		FPubnubOperationResult SubResult = PubnubClient->SubscribeToChannel(TestChannel);
		*bSubscribeDone = true;
		TestFalse("Subscribe should succeed", SubResult.Error);
		TestEqual("Subscribe status", SubResult.Status, 200);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeDone]() { return *bSubscribeDone; }, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		FPubnubPublishMessageResult PubResult = PubnubClient->PublishMessage(TestChannel, TestMessage);
		TestFalse("Publish should succeed", PubResult.Result.Error);
		TestEqual("Publish status", PubResult.Result.Status, 200);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageReceived]() { return *bMessageReceived; }, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bMessageReceived, ReceivedMessage, TestChannel, TestMessage, TestUser]()
	{
		if (!*bMessageReceived)
		{
			AddError("OnMessageReceivedNative was not invoked with the published message.");
			return;
		}
		TestEqual("Received Message", TestMessage, ReceivedMessage->Message);
		TestEqual("Received Channel", TestChannel, ReceivedMessage->Channel);
		TestEqual("Received UserID", TestUser, ReceivedMessage->UserID);
		TestEqual("Received MessageType", EPubnubMessageType::PMT_Published, ReceivedMessage->MessageType);
		TestFalse("Received Timetoken non-empty", ReceivedMessage->Timetoken.IsEmpty());
	}, 0.1f));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// UPubnubClient::UnsubscribeFromChannel - Input validation (fast-fail)
// ---------------------------------------------------------------------------

bool FPubnubUnsubscribeFromChannel_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	FPubnubOperationResult Result = PubnubClient->UnsubscribeFromChannel(FString());

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention Channel", Result.ErrorMessage.Contains(TEXT("Channel")));
	TestTrue("ErrorMessage should indicate field is empty", Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

// PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED: call UnsubscribeFromChannel after client is destroyed.
bool FPubnubUnsubscribeFromChannel_ClientNotInitialized_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();

	FPubnubOperationResult Result = Client->UnsubscribeFromChannel(SDK_PREFIX + "ch");

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention not initialized or invalid",
		Result.ErrorMessage.Contains(TEXT("not initialized")) || Result.ErrorMessage.Contains(TEXT("invalid")));

	CleanUp();
	return true;
}

// Unsubscribe without having subscribed returns "There is no such subscription. Aborting operation."
bool FPubnubUnsubscribeFromChannel_NoSuchSubscription_ReturnsError::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "unsub_no_sub_ch";

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(SDK_PREFIX + "unsub_no_sub_user");

	FPubnubOperationResult Result = PubnubClient->UnsubscribeFromChannel(TestChannel);

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention no such subscription",
		Result.ErrorMessage.Contains(TEXT("no such subscription")));

	CleanUp();
	return true;
}

// Happy path: Subscribe then Unsubscribe; Result 200 and !Error. Confirm unsubscribe by publishing and waiting MAX_WAIT_TIME/5 — message must not be received.
bool FPubnubUnsubscribeFromChannel_HappyPath_ThenMessageNotReceived::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "unsub_happy_ch";
	const FString TestUser = SDK_PREFIX + "unsub_happy_user";
	const FString TestMessage = TEXT("\"After unsubscribe\"");
	TSharedPtr<bool> bMessageReceived = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubOperationResult SubResult = PubnubClient->SubscribeToChannel(TestChannel);
	TestFalse("Subscribe should succeed", SubResult.Error);
	TestEqual("Subscribe status", SubResult.Status, 200);

	FPubnubOperationResult UnsubResult = PubnubClient->UnsubscribeFromChannel(TestChannel);
	TestFalse("Unsubscribe should succeed", UnsubResult.Error);
	TestEqual("Unsubscribe status", UnsubResult.Status, 200);

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[TestChannel, TestMessage, bMessageReceived](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == TestChannel && Msg.Message == TestMessage) { *bMessageReceived = true; }
		});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		FPubnubPublishMessageResult PubResult = PubnubClient->PublishMessage(TestChannel, TestMessage);
		TestFalse("Publish should succeed", PubResult.Result.Error);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(MAX_WAIT_TIME / 5.0f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bMessageReceived]()
	{
		TestFalse("Unsubscribe confirmed: message was not received within MAX_WAIT_TIME/5", *bMessageReceived);
	}, 0.1f));

	CleanUp();
	return true;
}

// Unsubscribe from one of two channels; publish to both. Unsubscribed channel must not receive within MAX_WAIT_TIME/5; still-subscribed channel must receive.
bool FPubnubUnsubscribeFromChannel_UnsubscribeOneOfTwo_OnlySubscribedChannelReceives::RunTest(const FString& Parameters)
{
	const FString ChannelA = SDK_PREFIX + "unsub_one_of_two_a";
	const FString ChannelB = SDK_PREFIX + "unsub_one_of_two_b";
	const FString MessageA = TEXT("\"msg_a\"");
	const FString MessageB = TEXT("\"msg_b\"");
	const FString TestUser = SDK_PREFIX + "unsub_one_of_two_user";
	TSharedPtr<bool> bReceivedA = MakeShared<bool>(false);
	TSharedPtr<bool> bReceivedB = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubOperationResult SubA = PubnubClient->SubscribeToChannel(ChannelA);
	TestFalse("Subscribe to A should succeed", SubA.Error);
	FPubnubOperationResult SubB = PubnubClient->SubscribeToChannel(ChannelB);
	TestFalse("Subscribe to B should succeed", SubB.Error);

	FPubnubOperationResult UnsubA = PubnubClient->UnsubscribeFromChannel(ChannelA);
	TestFalse("Unsubscribe from A should succeed", UnsubA.Error);
	TestEqual("Unsubscribe A status", UnsubA.Status, 200);

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[ChannelA, ChannelB, MessageA, MessageB, bReceivedA, bReceivedB](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == ChannelA && Msg.Message == MessageA) { *bReceivedA = true; }
			if (Msg.Channel == ChannelB && Msg.Message == MessageB) { *bReceivedB = true; }
		});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelA, ChannelB, MessageA, MessageB]()
	{
		FPubnubPublishMessageResult PubA = PubnubClient->PublishMessage(ChannelA, MessageA);
		FPubnubPublishMessageResult PubB = PubnubClient->PublishMessage(ChannelB, MessageB);
		TestFalse("Publish to A should succeed", PubA.Result.Error);
		TestFalse("Publish to B should succeed", PubB.Result.Error);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(MAX_WAIT_TIME / 5.0f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bReceivedA, bReceivedB]()
	{
		TestFalse("Unsubscribed channel A must not have received message", *bReceivedA);
		TestTrue("Still-subscribed channel B must have received message", *bReceivedB);
	}, 0.1f));

	CleanUp();
	return true;
}

// Unsubscribe twice from same channel; second call returns error "There is no such subscription."
bool FPubnubUnsubscribeFromChannel_UnsubscribeTwice_SecondReturnsNoSuchSubscription::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "unsub_twice_ch";
	const FString TestUser = SDK_PREFIX + "unsub_twice_user";

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubOperationResult SubResult = PubnubClient->SubscribeToChannel(TestChannel);
	TestFalse("Subscribe should succeed", SubResult.Error);

	FPubnubOperationResult Unsub1 = PubnubClient->UnsubscribeFromChannel(TestChannel);
	TestFalse("First unsubscribe should succeed", Unsub1.Error);
	TestEqual("First unsubscribe status", Unsub1.Status, 200);

	FPubnubOperationResult Unsub2 = PubnubClient->UnsubscribeFromChannel(TestChannel);
	TestTrue("Second unsubscribe should fail", Unsub2.Error);
	TestTrue("ErrorMessage should mention no such subscription",
		Unsub2.ErrorMessage.Contains(TEXT("no such subscription")));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// UPubnubClient::SubscribeToGroup - Input validation (fast-fail)
// ---------------------------------------------------------------------------

bool FPubnubSubscribeToGroup_EmptyChannelGroup_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	FPubnubOperationResult Result = PubnubClient->SubscribeToGroup(FString());

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention ChannelGroup", Result.ErrorMessage.Contains(TEXT("ChannelGroup")));
	TestTrue("ErrorMessage should indicate field is empty", Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

// PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED: call SubscribeToGroup after client is destroyed.
bool FPubnubSubscribeToGroup_ClientNotInitialized_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();

	FPubnubOperationResult Result = Client->SubscribeToGroup(SDK_PREFIX + "group");

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention not initialized or invalid",
		Result.ErrorMessage.Contains(TEXT("not initialized")) || Result.ErrorMessage.Contains(TEXT("invalid")));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// UPubnubClient::SubscribeToGroup - Happy path and full parameters
// ---------------------------------------------------------------------------

// Subscribe to group, add channel to group, publish to channel; confirm subscription by receiving message.
bool FPubnubSubscribeToGroup_HappyPath_RequiredParamsOnly::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "sub_grp_happy";
	const FString TestChannel = SDK_PREFIX + "sub_grp_happy_ch";
	const FString TestUser = SDK_PREFIX + "sub_grp_happy_user";
	const FString TestMessage = TEXT("\"Group happy path\"");
	TSharedPtr<bool> bMessageReceived = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubOperationResult AddResult = PubnubClient->AddChannelToGroup(TestChannel, TestGroup);
	TestFalse("AddChannelToGroup should succeed", AddResult.Error);

	FPubnubOperationResult SubResult = PubnubClient->SubscribeToGroup(TestGroup);
	TestFalse("SubscribeToGroup should succeed", SubResult.Error);
	TestEqual("SubscribeToGroup status", SubResult.Status, 200);

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[TestChannel, TestMessage, bMessageReceived](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == TestChannel && Msg.Message == TestMessage) { *bMessageReceived = true; }
		});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		FPubnubPublishMessageResult PubResult = PubnubClient->PublishMessage(TestChannel, TestMessage);
		TestFalse("Publish should succeed", PubResult.Result.Error);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageReceived]() { return *bMessageReceived; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bMessageReceived]()
	{
		TestTrue("Subscription to group confirmed by receiving published message", *bMessageReceived);
	}, 0.1f));

	CleanUp();
	return true;
}

// Full test of FPubnubSubscribeSettings.ReceivePresenceEvents for group; confirm by receiving message.
bool FPubnubSubscribeToGroup_FullSubscribeSettings_ReceivePresenceEvents::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "sub_grp_full";
	const FString TestChannel = SDK_PREFIX + "sub_grp_full_ch";
	const FString TestUser = SDK_PREFIX + "sub_grp_full_user";
	const FString TestMessage = TEXT("\"Group full settings\"");
	TSharedPtr<bool> bMessageReceived = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubOperationResult AddResult = PubnubClient->AddChannelToGroup(TestChannel, TestGroup);
	TestFalse("AddChannelToGroup should succeed", AddResult.Error);

	FPubnubSubscribeSettings Settings;
	Settings.ReceivePresenceEvents = true;
	FPubnubOperationResult SubResult = PubnubClient->SubscribeToGroup(TestGroup, Settings);
	TestFalse("SubscribeToGroup with ReceivePresenceEvents should succeed", SubResult.Error);
	TestEqual("SubscribeToGroup status", SubResult.Status, 200);

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[TestChannel, TestMessage, bMessageReceived](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == TestChannel && Msg.Message == TestMessage) { *bMessageReceived = true; }
		});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		FPubnubPublishMessageResult PubResult = PubnubClient->PublishMessage(TestChannel, TestMessage);
		TestFalse("Publish should succeed", PubResult.Result.Error);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageReceived]() { return *bMessageReceived; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bMessageReceived]()
	{
		TestTrue("Subscription to group confirmed by receiving message", *bMessageReceived);
	}, 0.1f));

	CleanUp();
	return true;
}

// Subscribing to the same channel group twice returns error; first subscription confirmed by receiving message.
bool FPubnubSubscribeToGroup_AlreadySubscribed_ReturnsError::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "sub_grp_already";
	const FString TestChannel = SDK_PREFIX + "sub_grp_already_ch";
	const FString TestUser = SDK_PREFIX + "sub_grp_already_user";
	const FString TestMessage = TEXT("\"First group sub\"");
	TSharedPtr<bool> bMessageReceived = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubOperationResult AddResult = PubnubClient->AddChannelToGroup(TestChannel, TestGroup);
	TestFalse("AddChannelToGroup should succeed", AddResult.Error);

	FPubnubOperationResult FirstResult = PubnubClient->SubscribeToGroup(TestGroup);
	TestFalse("First SubscribeToGroup should succeed", FirstResult.Error);
	TestEqual("First SubscribeToGroup status", FirstResult.Status, 200);

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[TestChannel, TestMessage, bMessageReceived](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == TestChannel && Msg.Message == TestMessage) { *bMessageReceived = true; }
		});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		FPubnubPublishMessageResult PubResult = PubnubClient->PublishMessage(TestChannel, TestMessage);
		TestFalse("Publish should succeed", PubResult.Result.Error);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageReceived]() { return *bMessageReceived; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bMessageReceived]()
	{
		TestTrue("First subscription to group confirmed by receive", *bMessageReceived);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup]()
	{
		FPubnubOperationResult SecondResult = PubnubClient->SubscribeToGroup(TestGroup);
		TestTrue("Second SubscribeToGroup to same group should fail", SecondResult.Error);
		TestTrue("ErrorMessage should mention already subscribed",
			SecondResult.ErrorMessage.Contains(TEXT("Already subscribed")) || SecondResult.ErrorMessage.Contains(TEXT("already exists")));
	}, 0.1f));

	CleanUp();
	return true;
}

// Unsubscribe from group then SubscribeToGroup again; add channel, publish; confirm message received after re-subscribe.
bool FPubnubSubscribeToGroup_UnsubscribeThenSubscribeAgain_ReceivesMessage::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "sub_grp_unsub_resub";
	const FString TestChannel = SDK_PREFIX + "sub_grp_unsub_resub_ch";
	const FString TestUser = SDK_PREFIX + "sub_grp_unsub_resub_user";
	const FString TestMessage = TEXT("\"After group resubscribe\"");
	TSharedPtr<bool> bMessageReceived = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubOperationResult AddResult = PubnubClient->AddChannelToGroup(TestChannel, TestGroup);
	TestFalse("AddChannelToGroup should succeed", AddResult.Error);

	FPubnubOperationResult Sub1 = PubnubClient->SubscribeToGroup(TestGroup);
	TestFalse("First SubscribeToGroup should succeed", Sub1.Error);
	TestEqual("First SubscribeToGroup status", Sub1.Status, 200);

	FPubnubOperationResult Unsub = PubnubClient->UnsubscribeFromGroup(TestGroup);
	TestFalse("UnsubscribeFromGroup should succeed", Unsub.Error);
	TestEqual("UnsubscribeFromGroup status", Unsub.Status, 200);

	FPubnubOperationResult Sub2 = PubnubClient->SubscribeToGroup(TestGroup);
	TestFalse("Second SubscribeToGroup (after unsubscribe) should succeed", Sub2.Error);
	TestEqual("Second SubscribeToGroup status", Sub2.Status, 200);

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[TestChannel, TestMessage, bMessageReceived](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == TestChannel && Msg.Message == TestMessage) { *bMessageReceived = true; }
		});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		FPubnubPublishMessageResult PubResult = PubnubClient->PublishMessage(TestChannel, TestMessage);
		TestFalse("Publish after resubscribe should succeed", PubResult.Result.Error);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageReceived]() { return *bMessageReceived; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bMessageReceived]()
	{
		TestTrue("Message should be received after unsubscribe-then-subscribe-again to group", *bMessageReceived);
	}, 0.1f));

	CleanUp();
	return true;
}

// Subscribe to group, add channel to group, publish to channel; verify received message data (channel, message, user, type, timetoken).
bool FPubnubSubscribeToGroup_AddChannelThenPublish_ReceivesMessage::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "sub_grp_add_pub";
	const FString TestChannel = SDK_PREFIX + "sub_grp_add_pub_ch";
	const FString TestUser = SDK_PREFIX + "sub_grp_add_pub_user";
	const FString TestMessage = TEXT("\"Group add then publish\"");

	TSharedPtr<bool> bMessageReceived = MakeShared<bool>(false);
	TSharedPtr<FPubnubMessageData> ReceivedMessage = MakeShared<FPubnubMessageData>();
	TSharedPtr<bool> bSubscribeDone = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubOperationResult AddResult = PubnubClient->AddChannelToGroup(TestChannel, TestGroup);
	TestFalse("AddChannelToGroup should succeed", AddResult.Error);

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, bSubscribeDone]()
	{
		FPubnubOperationResult SubResult = PubnubClient->SubscribeToGroup(TestGroup);
		*bSubscribeDone = true;
		TestFalse("SubscribeToGroup should succeed", SubResult.Error);
		TestEqual("SubscribeToGroup status", SubResult.Status, 200);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeDone]() { return *bSubscribeDone; }, MAX_WAIT_TIME));

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[TestChannel, TestMessage, bMessageReceived, ReceivedMessage](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == TestChannel && Msg.Message == TestMessage)
			{
				*ReceivedMessage = Msg;
				*bMessageReceived = true;
			}
		});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		FPubnubPublishMessageResult PubResult = PubnubClient->PublishMessage(TestChannel, TestMessage);
		TestFalse("Publish should succeed", PubResult.Result.Error);
		TestEqual("Publish status", PubResult.Result.Status, 200);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageReceived]() { return *bMessageReceived; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bMessageReceived, ReceivedMessage, TestChannel, TestMessage, TestUser]()
	{
		if (!*bMessageReceived)
		{
			AddError("OnMessageReceivedNative was not invoked with the published message (via group).");
			return;
		}
		TestEqual("Received Message", TestMessage, ReceivedMessage->Message);
		TestEqual("Received Channel", TestChannel, ReceivedMessage->Channel);
		TestEqual("Received UserID", TestUser, ReceivedMessage->UserID);
		TestEqual("Received MessageType", EPubnubMessageType::PMT_Published, ReceivedMessage->MessageType);
		TestFalse("Received Timetoken non-empty", ReceivedMessage->Timetoken.IsEmpty());
	}, 0.1f));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// UPubnubClient::UnsubscribeFromGroup - Input validation (fast-fail)
// ---------------------------------------------------------------------------

bool FPubnubUnsubscribeFromGroup_EmptyChannelGroup_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	FPubnubOperationResult Result = PubnubClient->UnsubscribeFromGroup(FString());

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention ChannelGroup", Result.ErrorMessage.Contains(TEXT("ChannelGroup")));
	TestTrue("ErrorMessage should indicate field is empty", Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

// PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED: call UnsubscribeFromGroup after client is destroyed.
bool FPubnubUnsubscribeFromGroup_ClientNotInitialized_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();

	FPubnubOperationResult Result = Client->UnsubscribeFromGroup(SDK_PREFIX + "group");

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention not initialized or invalid",
		Result.ErrorMessage.Contains(TEXT("not initialized")) || Result.ErrorMessage.Contains(TEXT("invalid")));

	CleanUp();
	return true;
}

// UnsubscribeFromGroup without having subscribed returns "There is no such subscription."
bool FPubnubUnsubscribeFromGroup_NoSuchSubscription_ReturnsError::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "unsub_grp_no_sub";

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(SDK_PREFIX + "unsub_grp_no_sub_user");

	FPubnubOperationResult Result = PubnubClient->UnsubscribeFromGroup(TestGroup);

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention no such subscription",
		Result.ErrorMessage.Contains(TEXT("no such subscription")));

	CleanUp();
	return true;
}

// Happy path: AddChannelToGroup, SubscribeToGroup, UnsubscribeFromGroup; publish to channel, wait MAX_WAIT_TIME/5 — message must not be received.
bool FPubnubUnsubscribeFromGroup_HappyPath_ThenMessageNotReceived::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "unsub_grp_happy";
	const FString TestChannel = SDK_PREFIX + "unsub_grp_happy_ch";
	const FString TestUser = SDK_PREFIX + "unsub_grp_happy_user";
	const FString TestMessage = TEXT("\"After group unsubscribe\"");
	TSharedPtr<bool> bMessageReceived = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubOperationResult AddResult = PubnubClient->AddChannelToGroup(TestChannel, TestGroup);
	TestFalse("AddChannelToGroup should succeed", AddResult.Error);

	FPubnubOperationResult SubResult = PubnubClient->SubscribeToGroup(TestGroup);
	TestFalse("SubscribeToGroup should succeed", SubResult.Error);
	TestEqual("SubscribeToGroup status", SubResult.Status, 200);

	FPubnubOperationResult UnsubResult = PubnubClient->UnsubscribeFromGroup(TestGroup);
	TestFalse("UnsubscribeFromGroup should succeed", UnsubResult.Error);
	TestEqual("UnsubscribeFromGroup status", UnsubResult.Status, 200);

	PubnubClient->OnMessageReceivedNative.AddLambda(
		[TestChannel, TestMessage, bMessageReceived](const FPubnubMessageData& Msg)
		{
			if (Msg.Channel == TestChannel && Msg.Message == TestMessage) { *bMessageReceived = true; }
		});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		FPubnubPublishMessageResult PubResult = PubnubClient->PublishMessage(TestChannel, TestMessage);
		TestFalse("Publish should succeed", PubResult.Result.Error);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(MAX_WAIT_TIME / 5.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bMessageReceived]()
	{
		TestFalse("UnsubscribeFromGroup confirmed: message was not received within MAX_WAIT_TIME/5", *bMessageReceived);
	}, 0.1f));

	CleanUp();
	return true;
}

// UnsubscribeFromGroup twice; second call returns "There is no such subscription."
bool FPubnubUnsubscribeFromGroup_UnsubscribeTwice_SecondReturnsNoSuchSubscription::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "unsub_grp_twice";
	const FString TestChannel = SDK_PREFIX + "unsub_grp_twice_ch";
	const FString TestUser = SDK_PREFIX + "unsub_grp_twice_user";

	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(TestUser);

	FPubnubOperationResult AddResult = PubnubClient->AddChannelToGroup(TestChannel, TestGroup);
	TestFalse("AddChannelToGroup should succeed", AddResult.Error);

	FPubnubOperationResult SubResult = PubnubClient->SubscribeToGroup(TestGroup);
	TestFalse("SubscribeToGroup should succeed", SubResult.Error);

	FPubnubOperationResult Unsub1 = PubnubClient->UnsubscribeFromGroup(TestGroup);
	TestFalse("First UnsubscribeFromGroup should succeed", Unsub1.Error);
	TestEqual("First UnsubscribeFromGroup status", Unsub1.Status, 200);

	FPubnubOperationResult Unsub2 = PubnubClient->UnsubscribeFromGroup(TestGroup);
	TestTrue("Second UnsubscribeFromGroup should fail", Unsub2.Error);
	TestTrue("ErrorMessage should mention no such subscription",
		Unsub2.ErrorMessage.Contains(TEXT("no such subscription")));

	CleanUp();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
