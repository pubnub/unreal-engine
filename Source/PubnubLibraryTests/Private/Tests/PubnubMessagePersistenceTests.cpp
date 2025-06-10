// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "FunctionLibraries/PubnubTimetokenUtilities.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubMessageCountsTest, FPubnubAutomationTestBase, "Pubnub.Integration.MessagePersistence.MessageCounts", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubFetchHistoryTest, FPubnubAutomationTestBase, "Pubnub.Integration.MessagePersistence.FetchHistory", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubDeleteMessagesTest, FPubnubAutomationTestBase, "Pubnub.Integration.MessagePersistence.DeleteMessages", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

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
	}, 1.2f));

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

bool FPubnubDeleteMessagesTest::RunTest(const FString& Parameters)
{
	const FString TestUser = SDK_PREFIX + TEXT("user_del_results_v3");
	const FString TestChannel = SDK_PREFIX + TEXT("chan_del_results_v3");

	// Shared state for callbacks
	TSharedPtr<FPubnubMessageData> LastReceivedMessageDataViaSub = MakeShared<FPubnubMessageData>();
	TSharedPtr<bool> bExpectedMessageReceivedViaSub = MakeShared<bool>(false);
	TSharedPtr<FString> ExpectedMessageContentForTTCapture = MakeShared<FString>(); // Used to identify which message TT to capture

	TSharedPtr<bool> bFetchHistoryDone = MakeShared<bool>(false);
	TSharedPtr<bool> bFetchHistorySuccess = MakeShared<bool>(false);
	TSharedPtr<TArray<FPubnubHistoryMessageData>> ReceivedHistoryMessages = MakeShared<TArray<FPubnubHistoryMessageData>>();

	TSharedPtr<bool> bDeleteMessagesDone = MakeShared<bool>(false);
	TSharedPtr<FPubnubOperationResult> DeleteMessagesResult = MakeShared<FPubnubOperationResult>();

	FString TestStartTimetoken = UPubnubTimetokenUtilities::GetCurrentUnixTimetoken();

	if (!InitTest() || !PubnubSubsystem)
	{
		AddError(TEXT("TestInitialization failed for FPubnubDeleteMessagesResultTest or PubnubSubsystem is null."));
		return false;
	}

	PubnubSubsystem->SetUserID(TestUser);

	// Setup OnMessageReceivedNative to capture timetokens
	PubnubSubsystem->OnMessageReceivedNative.AddLambda(
		[this, TestChannel, LastReceivedMessageDataViaSub, bExpectedMessageReceivedViaSub, ExpectedMessageContentForTTCapture]
		(FPubnubMessageData ReceivedMessage) {
			if (ReceivedMessage.Channel == TestChannel) {
				*LastReceivedMessageDataViaSub = ReceivedMessage;
				if (!ExpectedMessageContentForTTCapture->IsEmpty() && ReceivedMessage.Message == *ExpectedMessageContentForTTCapture) {
					*bExpectedMessageReceivedViaSub = true;
				}
			}
		});

	// Subscribe to the test channel
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel]() {
		PubnubSubsystem->SubscribeToChannel(TestChannel);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(0.5f)); 

	// Define other Callbacks
	FOnFetchHistoryResponseNative FetchHistoryCallback;
	FetchHistoryCallback.BindLambda([this, bFetchHistoryDone, bFetchHistorySuccess, ReceivedHistoryMessages](bool Error, int Status, FString ErrorMessage, const TArray<FPubnubHistoryMessageData>& Messages) {
		*bFetchHistoryDone = true;
		*bFetchHistorySuccess = !Error && (Status == 200);
		*ReceivedHistoryMessages = Messages;
		if (Error || Status != 200) AddError(FString::Printf(TEXT("FetchHistory Error: %s, Status: %d, Msg: %s"), Error ? TEXT("true") : TEXT("false"), Status, *ErrorMessage));
	});

	FOnDeleteMessagesResponseNative DeleteMessagesCallback;
	DeleteMessagesCallback.BindLambda([this, bDeleteMessagesDone, DeleteMessagesResult](const FPubnubOperationResult& Result) {
		*bDeleteMessagesDone = true;
		*DeleteMessagesResult = Result;
	});

	// Helper lambda to publish a message and capture its timetoken via subscription
	auto PublishAndCaptureTT = 
		[this, TestChannel, bExpectedMessageReceivedViaSub, ExpectedMessageContentForTTCapture, LastReceivedMessageDataViaSub]
		(const FString& MsgContent, TSharedPtr<FString> TargetTimetokenPtr, const FString& LogMessageID) 
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, MsgContent, bExpectedMessageReceivedViaSub, ExpectedMessageContentForTTCapture, LogMessageID]() { 
			*bExpectedMessageReceivedViaSub = false; 
			*ExpectedMessageContentForTTCapture = MsgContent; 
			PubnubSubsystem->PublishMessage(TestChannel, MsgContent); 
		}, 0.2f));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bExpectedMessageReceivedViaSub]() { return *bExpectedMessageReceivedViaSub; }, MAX_WAIT_TIME));
		ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TargetTimetokenPtr, LastReceivedMessageDataViaSub, LogMessageID, MsgContent, bExpectedMessageReceivedViaSub, ExpectedMessageContentForTTCapture]() { 
			if(*bExpectedMessageReceivedViaSub && LastReceivedMessageDataViaSub->Message == MsgContent) {
				*TargetTimetokenPtr = LastReceivedMessageDataViaSub->Timetoken;
				 TestFalse(LogMessageID + TEXT(" timetoken should not be empty."), TargetTimetokenPtr->IsEmpty());
			} else {
				AddError(LogMessageID + TEXT(" FAILED to capture timetoken. Expected msg not received or data mismatch."));
				if(TargetTimetokenPtr.IsValid()) *TargetTimetokenPtr = TEXT("");
			}
			*ExpectedMessageContentForTTCapture = TEXT("");
		}, 0.1f));
	};
	
	// Helper lambda to fetch history and check for specific messages
	auto FetchAndVerifyMessages = 
		[this, TestChannel, FetchHistoryCallback, bFetchHistoryDone, bFetchHistorySuccess, ReceivedHistoryMessages, TestStartTimetoken]
		(const FString& LogContext, const TArray<TPair<FString, bool>>& ExpectedMessagesWithPresence)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, FetchHistoryCallback, bFetchHistoryDone, TestStartTimetoken]() {
			*bFetchHistoryDone = false;
			FPubnubFetchHistorySettings Settings;
			Settings.MaxPerChannel = 30; 
			Settings.End = TestStartTimetoken;
			PubnubSubsystem->FetchHistory(TestChannel, FetchHistoryCallback, Settings);
		}, 0.5f));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bFetchHistoryDone]() { return *bFetchHistoryDone; }, MAX_WAIT_TIME));
		ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, LogContext, bFetchHistorySuccess, ReceivedHistoryMessages, ExpectedMessagesWithPresence]() {
			TestTrue(LogContext + TEXT(": FetchHistory API call success"), *bFetchHistorySuccess);
			if (*bFetchHistorySuccess) {
				for (const auto& ExpectedPair : ExpectedMessagesWithPresence) {
					bool bFound = false;
					for (const auto& ReceivedMsg : *ReceivedHistoryMessages) {
						if (ReceivedMsg.Message == ExpectedPair.Key) {
							bFound = true;
							break;
						}
					}
					if (ExpectedPair.Value) { 
						TestTrue(LogContext + TEXT(": Message '") + ExpectedPair.Key + TEXT("' should be present."), bFound);
					} else { 
						TestFalse(LogContext + TEXT(": Message '") + ExpectedPair.Key + TEXT("' should be absent."), bFound);
					}
				}
			}
		}, 0.1f));
	};

	// --- Phase 1: Test Deletion with Empty Settings (should delete all) ---
	const FString MsgAContent_P1 = TEXT("\"MsgA_P1\"");
	TSharedPtr<FString> TT_MsgA_P1 = MakeShared<FString>();

	PublishAndCaptureTT(MsgAContent_P1, TT_MsgA_P1, TEXT("MsgA_P1"));
	FetchAndVerifyMessages(TEXT("P1 After MsgA Publish (before delete all)"), {{MsgAContent_P1, true}});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, DeleteMessagesCallback, bDeleteMessagesDone]() {
		*bDeleteMessagesDone = false;
		PubnubSubsystem->DeleteMessages(TestChannel, DeleteMessagesCallback, FPubnubDeleteMessagesSettings()); 
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bDeleteMessagesDone]() { return *bDeleteMessagesDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bDeleteMessagesDone, DeleteMessagesResult]() {
		TestTrue(TEXT("P1: Delete (empty settings) callback invoked"), *bDeleteMessagesDone);
		if (*bDeleteMessagesDone) {
			TestFalse(TEXT("P1: Delete (empty settings/delete all) Result.Error should be false"), DeleteMessagesResult->Error);
			TestEqual(TEXT("P1: Delete (empty settings/delete all) Result.Status (expect 200)"), DeleteMessagesResult->Status, 200); 
		}
	}, 0.1f));
	FetchAndVerifyMessages(TEXT("P1 After Delete All"), {{MsgAContent_P1, false}}); 

	// --- Phase 2: Attempt to delete MsgA_P1 again using its old timetoken (already deleted in P1) ---
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, DeleteMessagesCallback, bDeleteMessagesDone, TT_MsgA_P1]() {
		*bDeleteMessagesDone = false; FPubnubDeleteMessagesSettings DeleteSettings;
		if (TT_MsgA_P1->IsEmpty()) { AddError(TEXT("P2: MsgA_P1 TT (old) is empty! This is unexpected even if message is gone.")); *bDeleteMessagesDone=true; return; }
		DeleteSettings.Start = TEXT("0"); 
		DeleteSettings.End = *TT_MsgA_P1; 
		PubnubSubsystem->DeleteMessages(TestChannel, DeleteMessagesCallback, DeleteSettings);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bDeleteMessagesDone]() { return *bDeleteMessagesDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, DeleteMessagesResult]() {
		TestFalse(TEXT("P2: Delete already-deleted MsgA_P1 Result.Error should be false"), DeleteMessagesResult->Error);
		TestEqual(TEXT("P2: Delete already-deleted MsgA_P1 Result.Status should be 200"), DeleteMessagesResult->Status, 200);
	},0.1f));
	FetchAndVerifyMessages(TEXT("P2 After attempting to re-delete MsgA_P1"), {{MsgAContent_P1, false}}); 
	
	// --- Phase 3: Test Start and End Timetokens for Range Deletion ---
	const FString MsgB_P3 = TEXT("\"MsgB_P3\""); const FString MsgC_P3 = TEXT("\"MsgC_P3\""); const FString MsgD_P3 = TEXT("\"MsgD_P3\"");
	TSharedPtr<FString> TT_B_P3 = MakeShared<FString>(); TSharedPtr<FString> TT_C_P3 = MakeShared<FString>(); TSharedPtr<FString> TT_D_P3 = MakeShared<FString>();

	PublishAndCaptureTT(MsgB_P3, TT_B_P3, TEXT("MsgB_P3"));
	PublishAndCaptureTT(MsgC_P3, TT_C_P3, TEXT("MsgC_P3"));
	PublishAndCaptureTT(MsgD_P3, TT_D_P3, TEXT("MsgD_P3"));
	FetchAndVerifyMessages(TEXT("P3 Baseline B,C,D"), {{MsgB_P3, true}, {MsgC_P3, true}, {MsgD_P3, true}});

	// Test 3.1: Delete Middle Message (MsgC_P3) -> Start=TT_B_P3 (exclusive), End=TT_C_P3 (inclusive)
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, DeleteMessagesCallback, bDeleteMessagesDone, TT_B_P3, TT_C_P3]() {
		*bDeleteMessagesDone = false; FPubnubDeleteMessagesSettings Settings;
		if (TT_B_P3->IsEmpty() || TT_C_P3->IsEmpty()) { AddError(TEXT("P3.1 TT empty")); *bDeleteMessagesDone=true; return; }
		Settings.Start = *TT_B_P3; Settings.End = *TT_C_P3;
		PubnubSubsystem->DeleteMessages(TestChannel, DeleteMessagesCallback, Settings);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bDeleteMessagesDone]() { return *bDeleteMessagesDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, DeleteMessagesResult](){ TestFalse(TEXT("P3.1 DelC Err"),DeleteMessagesResult->Error); TestEqual(TEXT("P3.1 DelC Status"),DeleteMessagesResult->Status,200);},0.1f));
	FetchAndVerifyMessages(TEXT("P3.1 After MsgC_P3 Delete"), {{MsgB_P3, true}, {MsgC_P3, false}, {MsgD_P3, true}});

	// P3.1.1: Re-Delete MsgC_P3 (should be no-op, 200 OK as it's already deleted)
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, DeleteMessagesCallback, bDeleteMessagesDone, TT_B_P3, TT_C_P3]() {
		*bDeleteMessagesDone = false; FPubnubDeleteMessagesSettings Settings;
		if (TT_B_P3->IsEmpty() || TT_C_P3->IsEmpty()) { AddError(TEXT("P3.1.1 TT empty for re-delete")); *bDeleteMessagesDone=true; return; }
		Settings.Start = *TT_B_P3; Settings.End = *TT_C_P3; 
		PubnubSubsystem->DeleteMessages(TestChannel, DeleteMessagesCallback, Settings);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bDeleteMessagesDone]() { return *bDeleteMessagesDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, DeleteMessagesResult](){
		TestFalse(TEXT("P3.1.1 Re-DelC Err should be false"),DeleteMessagesResult->Error);
		TestEqual(TEXT("P3.1.1 Re-DelC Status should be 200"),DeleteMessagesResult->Status,200);
	},0.1f));
	FetchAndVerifyMessages(TEXT("P3.1.1 After Re-Delete MsgC_P3"), {{MsgB_P3, true}, {MsgC_P3, false}, {MsgD_P3, true}}); 

	// Test 3.2: Delete with Start only (deletes MsgD_P3). Start=TT_C_P3 (exclusive, C is deleted) End=""
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, DeleteMessagesCallback, bDeleteMessagesDone, TT_C_P3]() {
		*bDeleteMessagesDone = false; FPubnubDeleteMessagesSettings Settings;
		if (TT_C_P3->IsEmpty()) { AddError(TEXT("P3.2 TT_C_P3 empty")); *bDeleteMessagesDone=true; return; }
		Settings.Start = *TT_C_P3; Settings.End = TEXT("");
		PubnubSubsystem->DeleteMessages(TestChannel, DeleteMessagesCallback, Settings);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bDeleteMessagesDone]() { return *bDeleteMessagesDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, DeleteMessagesResult](){ TestFalse(TEXT("P3.2 DelD Err"),DeleteMessagesResult->Error); TestEqual(TEXT("P3.2 DelD Status"),DeleteMessagesResult->Status,200);},0.1f));
	FetchAndVerifyMessages(TEXT("P3.2 After MsgD_P3 Delete"), {{MsgB_P3, false}, {MsgC_P3, false}, {MsgD_P3, true}});

	// Test 3.3: Delete with End only
	const FString MsgE_P3 = TEXT("\"MsgE_P3\""); const FString MsgF_P3 = TEXT("\"MsgF_P3\""); const FString MsgG_P3 = TEXT("\"MsgG_P3\"");
	TSharedPtr<FString> TT_E_P3 = MakeShared<FString>(); TSharedPtr<FString> TT_F_P3 = MakeShared<FString>(); TSharedPtr<FString> TT_G_P3 = MakeShared<FString>();
	PublishAndCaptureTT(MsgE_P3, TT_E_P3, TEXT("MsgE_P3"));
	PublishAndCaptureTT(MsgF_P3, TT_F_P3, TEXT("MsgF_P3"));
	PublishAndCaptureTT(MsgG_P3, TT_G_P3, TEXT("MsgG_P3"));
	FetchAndVerifyMessages(TEXT("P3.3 Baseline E,F,G"), {{MsgE_P3, true}, {MsgF_P3, true}, {MsgG_P3, true}});
	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, DeleteMessagesCallback, bDeleteMessagesDone, TT_F_P3]() {
		*bDeleteMessagesDone = false; FPubnubDeleteMessagesSettings Settings;
		if (TT_F_P3->IsEmpty()) { AddError(TEXT("P3.3 TT_F_P3 empty")); *bDeleteMessagesDone=true; return; }
		Settings.Start = TEXT(""); Settings.End = *TT_F_P3; 
		PubnubSubsystem->DeleteMessages(TestChannel, DeleteMessagesCallback, Settings);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bDeleteMessagesDone]() { return *bDeleteMessagesDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, DeleteMessagesResult](){ TestFalse(TEXT("P3.3 DelEF Err"),DeleteMessagesResult->Error); TestEqual(TEXT("P3.3 DelEF Status"),DeleteMessagesResult->Status,200);},0.1f));
	FetchAndVerifyMessages(TEXT("P3.3 After EF Delete"), {{MsgE_P3, true}, {MsgF_P3, false}, {MsgG_P3, false}});

	// --- Phase 4: Test messages outside delete range are not deleted ---
	const FString MsgX_P4 = TEXT("\"MsgX_P4_NoDel\""); const FString MsgY_P4 = TEXT("\"MsgY_P4_Del\""); const FString MsgZ_P4 = TEXT("\"MsgZ_P4_NoDel\"");
	TSharedPtr<FString> TT_X_P4 = MakeShared<FString>(); TSharedPtr<FString> TT_Y_P4 = MakeShared<FString>(); TSharedPtr<FString> TT_Z_P4 = MakeShared<FString>();
	PublishAndCaptureTT(MsgX_P4, TT_X_P4, TEXT("MsgX_P4"));
	PublishAndCaptureTT(MsgY_P4, TT_Y_P4, TEXT("MsgY_P4"));
	PublishAndCaptureTT(MsgZ_P4, TT_Z_P4, TEXT("MsgZ_P4"));
	FetchAndVerifyMessages(TEXT("P4 Baseline X,Y,Z"), {{MsgX_P4, true}, {MsgY_P4, true}, {MsgZ_P4, true}});

	// Delete only Message Y: Start = TT_X_P4 (exclusive), End = TT_Y_P4 (inclusive)
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, DeleteMessagesCallback, bDeleteMessagesDone, TT_X_P4, TT_Y_P4]() {
		*bDeleteMessagesDone = false; FPubnubDeleteMessagesSettings Settings;
		if (TT_X_P4->IsEmpty() || TT_Y_P4->IsEmpty()) { AddError(TEXT("P4 TT_X/Y empty")); *bDeleteMessagesDone=true; return; }
		Settings.Start = *TT_X_P4; Settings.End = *TT_Y_P4;
		PubnubSubsystem->DeleteMessages(TestChannel, DeleteMessagesCallback, Settings);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bDeleteMessagesDone]() { return *bDeleteMessagesDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, DeleteMessagesResult](){ TestFalse(TEXT("P4 DelY Err"),DeleteMessagesResult->Error); TestEqual(TEXT("P4 DelY Status"),DeleteMessagesResult->Status,200);},0.1f));
	FetchAndVerifyMessages(TEXT("P4 After Y Delete"), {{MsgX_P4, true}, {MsgY_P4, false}, {MsgZ_P4, true}});

	CleanUp(); 
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS