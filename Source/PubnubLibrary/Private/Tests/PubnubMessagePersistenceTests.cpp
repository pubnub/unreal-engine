#include "Misc/AutomationTest.h"
#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/PubnubTestsUtils.h"
#include "FunctionLibraries/PubnubTimetokenUtilities.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "Tests/AutomationCommon.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubMessageCountsTest, FPubnubAutomationTestBase, "Pubnub.Integration.MessagePersistence.MessageCounts", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubFetchHistoryTest, FPubnubAutomationTestBase, "Pubnub.Integration.MessagePersistence.FetchHistory", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FPubnubMessageCountsTest::RunTest(const FString& Parameters)
{
	// Initial variables
	const FString TestUser = SDK_PREFIX + "user_msg_counts";
	const FString TestChannel = SDK_PREFIX + "chan_msg_counts";
	const FString TestMessage1 = "\"Message one for counts test\"";
	const FString TestMessage2 = "\"Message two for counts test (no store)\"";

	TSharedPtr<FString> InitialTimetoken = MakeShared<FString>(UPubnubTimetokenUtilities::GetCurrentUnixTimetoken());
	TSharedPtr<bool> bMessageCountsOperationDone = MakeShared<bool>(false);
	TSharedPtr<bool> bMessageCountsOperationSuccess = MakeShared<bool>(false);
	TSharedPtr<int32> MessageCountResult = MakeShared<int32>(-1); // Initialize to an invalid count

	if (!InitTest())
	{
		AddError("TestInitialization failed for FPubnubMessageCountsTest");
		return false;
	}

	// General error handler
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(FString::Printf(TEXT("Pubnub Error in FPubnubMessageCountsTest: %s, Type: %d"), *ErrorMessage, ErrorType));
	});

	// MessageCounts callback handler
	FOnPubnubIntResponseNative MessageCountsCallback;
	MessageCountsCallback.BindLambda([this, bMessageCountsOperationDone, bMessageCountsOperationSuccess, MessageCountResult](int Count)
	{
		*bMessageCountsOperationDone = true;
		*bMessageCountsOperationSuccess = true; // Success is implied by the callback firing for MessageCounts
		*MessageCountResult = Count;
	});

	// Set UserID
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUser]()
	{
		PubnubSubsystem->SetUserID(TestUser);
	}, 0.1f));

	// Step 1: Call MessageCounts with initial timetoken, expect 0
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, InitialTimetoken, MessageCountsCallback, bMessageCountsOperationDone, bMessageCountsOperationSuccess, MessageCountResult]()
	{
		*bMessageCountsOperationDone = false;
		*bMessageCountsOperationSuccess = false;
		*MessageCountResult = -1;
		PubnubSubsystem->MessageCounts(TestChannel, *InitialTimetoken, MessageCountsCallback);
	}, 0.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageCountsOperationDone]() { return *bMessageCountsOperationDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, MessageCountResult, bMessageCountsOperationSuccess, bMessageCountsOperationDone]()
	{
		TestTrue("MessageCounts (initial) operation should have completed (callback fired)", *bMessageCountsOperationDone);
		if (*bMessageCountsOperationSuccess) // Still useful to check if error was caught by general handler
		{
			TestEqual("Initial message count should be 0", *MessageCountResult, 0);
		} else {
			AddError("MessageCounts (initial) operation was marked as failed, likely via OnPubnubErrorNative.");
		}
	}, 0.1f));

	// Step 2: Publish message 1 - (StoreInHistory = true)
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage1]()
	{
		FPubnubPublishSettings PublishSettings;
		PublishSettings.StoreInHistory = true;
		PubnubSubsystem->PublishMessage(TestChannel, TestMessage1, PublishSettings);
	}, 0.2f));

	// Step 3: Call MessageCounts again, expect 1
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, InitialTimetoken, MessageCountsCallback, bMessageCountsOperationDone, bMessageCountsOperationSuccess, MessageCountResult]()
	{
		*bMessageCountsOperationDone = false;
		*bMessageCountsOperationSuccess = false;
		*MessageCountResult = -1;
		PubnubSubsystem->MessageCounts(TestChannel, *InitialTimetoken, MessageCountsCallback);
	}, 1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageCountsOperationDone]() { return *bMessageCountsOperationDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, MessageCountResult, bMessageCountsOperationSuccess, bMessageCountsOperationDone]()
	{
		TestTrue("MessageCounts (after 1st publish) operation should have completed (callback fired)", *bMessageCountsOperationDone);
		if (*bMessageCountsOperationSuccess)
		{
			TestEqual("Message count after 1st publish should be 1", *MessageCountResult, 1);
		} else {
			AddError("MessageCounts (after 1st publish) operation was marked as failed, likely via OnPubnubErrorNative.");
		}
	}, 0.1f));

	// Step 4: Publish message 2 (StoreInHistory = false)
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage2]()
	{
		FPubnubPublishSettings PublishSettings;
		PublishSettings.StoreInHistory = false;
		PubnubSubsystem->PublishMessage(TestChannel, TestMessage2, PublishSettings);
	}, 0.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f)); // Wait for publish to be processed

	// Step 5: Call MessageCounts again, expect 1 (still)
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, InitialTimetoken, MessageCountsCallback, bMessageCountsOperationDone, bMessageCountsOperationSuccess, MessageCountResult]()
	{
		*bMessageCountsOperationDone = false;
		*bMessageCountsOperationSuccess = false;
		*MessageCountResult = -1;
		PubnubSubsystem->MessageCounts(TestChannel, *InitialTimetoken, MessageCountsCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageCountsOperationDone]() { return *bMessageCountsOperationDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, MessageCountResult, bMessageCountsOperationSuccess, bMessageCountsOperationDone]()
	{
		TestTrue("MessageCounts (after 2nd publish, no store) operation should have completed (callback fired)", *bMessageCountsOperationDone);
		if (*bMessageCountsOperationSuccess)
		{
			TestEqual("Message count after 2nd publish (no store) should still be 1", *MessageCountResult, 1);
		} else {
			AddError("MessageCounts (after 2nd publish, no store) operation was marked as failed, likely via OnPubnubErrorNative.");
		}
	}, 0.1f));
	
	CleanUp();
	return true;
}

bool FPubnubFetchHistoryTest::RunTest(const FString& Parameters)
{
	// Initial variables
	const FString TestUser = SDK_PREFIX + "user_fetch_history";
	const FString TestChannel = SDK_PREFIX + "chan_fetch_history";
	const FString TestMessage1Content = "\"History message one (oldest)\"";
	const FString TestMessage2Content = "\"History message two (middle)\"";
	const FString TestJsonMessageContent = "{\"key\":\"value\", \"number\":123, \"data\":{\"status\":\"active\"}}";

	TSharedPtr<FString> TimetokenAtTestStart = MakeShared<FString>(UPubnubTimetokenUtilities::GetCurrentUnixTimetoken());

	TSharedPtr<bool> bFetchHistoryDone = MakeShared<bool>(false);
	TSharedPtr<bool> bFetchHistorySuccess = MakeShared<bool>(false);
	TSharedPtr<TArray<FPubnubHistoryMessageData>> ReceivedHistoryMessages = MakeShared<TArray<FPubnubHistoryMessageData>>();

	// Callback Definition
	FOnFetchHistoryResponseNative FetchHistoryCallback;
	FetchHistoryCallback.BindLambda([this, bFetchHistoryDone, bFetchHistorySuccess, ReceivedHistoryMessages](bool Error, int Status, FString ErrorMessage, const TArray<FPubnubHistoryMessageData>& Messages)
	{
		*bFetchHistoryDone = true;
		if (!Error && Status == 200)
		{
			*bFetchHistorySuccess = true;
			*ReceivedHistoryMessages = Messages;
		}
		else
		{
			*bFetchHistorySuccess = false;
			AddError(FString::Printf(TEXT("FetchHistory failed. Error: %s, Status: %d, Message: %s"), Error ? TEXT("true") : TEXT("false"), Status, *ErrorMessage));
		}
	});

	if (!InitTest())
	{
		AddError("TestInitialization failed for FPubnubFetchHistoryTest");
		return false;
	}

	PubnubSubsystem->SetUserID(TestUser);
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(FString::Printf(TEXT("General Pubnub Error in FPubnubFetchHistoryTest: %s, Type: %d"), *ErrorMessage, ErrorType));
	});

	// Step 2: Publish Message 1
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage1Content]()
	{
		PubnubSubsystem->PublishMessage(TestChannel, TestMessage1Content);
	}, 0.2f));

	// Step 3: Publish Message 2
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage2Content]()
	{
		PubnubSubsystem->PublishMessage(TestChannel, TestMessage2Content);
	}, 0.2f));
	
	// Step 4: Publish JSON Message (New)
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestJsonMessageContent]()
	{
		PubnubSubsystem->PublishMessage(TestChannel, TestJsonMessageContent);
	}, 0.2f));

	// Step 6: Fetch History
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, FetchHistoryCallback, TimetokenAtTestStart]()
	{
		FPubnubFetchHistorySettings Settings;
		Settings.End = *TimetokenAtTestStart;
		Settings.Start = UPubnubTimetokenUtilities::GetCurrentUnixTimetoken();
		Settings.MaxPerChannel = 10;
		Settings.IncludeUserID = true;

		PubnubSubsystem->FetchHistory(TestChannel, FetchHistoryCallback, Settings);
	}, 1.5f));

	// Step 7: Verify History
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bFetchHistoryDone]() { return *bFetchHistoryDone; }, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUser, TestMessage1Content, TestMessage2Content, TestJsonMessageContent, ReceivedHistoryMessages, bFetchHistorySuccess]()
	{
		TestTrue("FetchHistory operation was reported as successful by callback.", *bFetchHistorySuccess);
		if (!*bFetchHistorySuccess)
		{
			AddError("Skipping history content verification due to FetchHistory operation failure.");
			return;
		}

		TestEqual("Number of history messages received within the specified timetoken range.", ReceivedHistoryMessages->Num(), 3);

		if (ReceivedHistoryMessages->Num() == 3)
		{
			TestEqual("History: Message 1 content.", (*ReceivedHistoryMessages)[0].Message, TestMessage1Content);
			TestEqual("History: Message 1 UserID.", (*ReceivedHistoryMessages)[0].UserID, TestUser);
			
			TestEqual("History: Message 2 content.", (*ReceivedHistoryMessages)[1].Message, TestMessage2Content);
			TestEqual("History: Message 2 UserID.", (*ReceivedHistoryMessages)[1].UserID, TestUser);

			TestTrue(FString::Printf(TEXT("History: Message 3 (JSON) content. Expected: %s, Actual: %s"), *TestJsonMessageContent, *(*ReceivedHistoryMessages)[2].Message),
				UPubnubJsonUtilities::AreJsonObjectStringsEqual(TestJsonMessageContent, (*ReceivedHistoryMessages)[2].Message));
			TestEqual("History: Message 3 (JSON) UserID.", (*ReceivedHistoryMessages)[2].UserID, TestUser);
		}
		else if (ReceivedHistoryMessages->Num() > 3)
		{
			AddWarning(FString::Printf(TEXT("FetchHistory returned %d messages, expected 3. This might indicate pre-existing messages. Verification will be lenient."), ReceivedHistoryMessages->Num()));
			bool bFoundMessage1 = false;
			bool bFoundMessage2 = false;
			bool bFoundJsonMessage = false;
			for(const auto& Msg : *ReceivedHistoryMessages)
			{
				if(Msg.Message == TestMessage1Content && Msg.UserID == TestUser) bFoundMessage1 = true;
				if(Msg.Message == TestMessage2Content && Msg.UserID == TestUser) bFoundMessage2 = true;
				if(UPubnubJsonUtilities::AreJsonObjectStringsEqual(Msg.Message, TestJsonMessageContent) && Msg.UserID == TestUser) bFoundJsonMessage = true;
			}
			TestTrue("History: TestMessage1Content found.", bFoundMessage1);
			TestTrue("History: TestMessage2Content found.", bFoundMessage2);
			TestTrue("History: TestJsonMessageContent found.", bFoundJsonMessage);
		}
		else
		{
			AddError(FString::Printf(TEXT("Expected 3 history messages, but got %d."), ReceivedHistoryMessages->Num()));
		}

	}, 0.1f));

	// Step 8: CleanUp
	CleanUp();
	return true;
}

