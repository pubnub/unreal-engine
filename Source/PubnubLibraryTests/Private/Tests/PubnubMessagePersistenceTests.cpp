// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubClient.h"
#include "PubnubStructLibrary.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"
#include "HAL/PlatformProcess.h"

using namespace PubnubTests;

namespace PubnubMessagePersistenceTestsHelper
{
	/** Baseline timetoken safely before typical live traffic (17-digit PubNub format). */
	static const TCHAR* BaselineTimetoken = TEXT("10000000000000000");

	/** Append to channel names so each test run avoids stale messages from earlier runs or other tests. */
	static FString UniqueHistoryChannelSuffix()
	{
		return FString::Printf(TEXT("_%05d"), FMath::RandRange(0, 99999));
	}

	static const FPubnubHistoryMessageData* FindMessageContaining(const TArray<FPubnubHistoryMessageData>& Messages, const FString& Marker)
	{
		for (const FPubnubHistoryMessageData& M : Messages)
		{
			if (M.Message.Contains(Marker))
			{
				return &M;
			}
		}
		return nullptr;
	}

	/** Lexicographic order matches numeric order for equal-length PubNub timetoken strings. */
	static FString OlderOfTwoTimetokens(const FString& A, const FString& B) { return (A < B) ? A : B; }
	static FString NewerOfTwoTimetokens(const FString& A, const FString& B) { return (A > B) ? A : B; }
}

// ---------------------------------------------------------------------------
// UPubnubClient::FetchHistory
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubFetchHistory_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.FetchHistory.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubFetchHistory_ClientNotInitialized_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.FetchHistory.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubFetchHistory_HappyPath_DefaultSettings_FindsPublishedMessage, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.FetchHistory.2HappyPath.DefaultSettings",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubFetchHistory_FullSettings_AllIncludes_MaxPerChannel, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.FetchHistory.3FullParameters.AllOptions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubFetchHistory_MaxPerChannel_LimitsReturnedMessages, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.FetchHistory.4Advanced.MaxPerChannelLimit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubFetchHistory_TimeWindow_StartNewerExclusive_EndOlderInclusive, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.FetchHistory.4Advanced.TimeWindowExcludesNewest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubFetchHistory_ConcurrentSyncWhileAsync_ReturnsMutexError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.FetchHistory.4Advanced.ConcurrentSyncWhileAsync",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::DeleteMessages
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubDeleteMessages_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.DeleteMessages.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubDeleteMessages_ClientNotInitialized_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.DeleteMessages.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubDeleteMessages_HappyPath_DefaultSettings_ClearsHistory, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.DeleteMessages.2HappyPath.DefaultSettingsClearsChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubDeleteMessages_FullParameters_TimeSlice_KeepsNewerMessage, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.DeleteMessages.3FullParameters.TimeSlicePreservesNewer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubDeleteMessages_ConcurrentSyncWhileAsync_ReturnsMutexError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.DeleteMessages.4Advanced.ConcurrentSyncWhileAsync",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::MessageCounts
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubMessageCounts_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.MessageCounts.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubMessageCounts_ClientNotInitialized_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.MessageCounts.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubMessageCounts_HappyPath_BaselineTimetoken_IncludesPublished, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.MessageCounts.2HappyPath.BaselineTimetoken",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubMessageCounts_UsingOlderPublishedTimetoken_CountsAtLeastOneNewer, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.MessageCounts.4Advanced.CountFromFirstMessageTimetoken",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubMessageCounts_ConcurrentSyncWhileAsync_ReturnsMutexError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.MessageCounts.4Advanced.ConcurrentSyncWhileAsync",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::MessageCountsMultiple
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubMessageCountsMultiple_EmptyChannels_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.MessageCountsMultiple.1Validation.EmptyChannels",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubMessageCountsMultiple_ChannelTimetokenCountMismatch_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.MessageCountsMultiple.1Validation.ArraySizeMismatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubMessageCountsMultiple_ClientNotInitialized_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.MessageCountsMultiple.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubMessageCountsMultiple_HappyPath_TwoChannels_IndependentCounts, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.MessageCountsMultiple.2HappyPath.TwoChannels",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubMessageCountsMultiple_ConcurrentSyncWhileAsync_ReturnsMutexError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessagePersistence.MessageCountsMultiple.4Advanced.ConcurrentSyncWhileAsync",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// FetchHistory — validation
// ---------------------------------------------------------------------------

bool FPubnubFetchHistory_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("mp_fetch_val_user"));

	FPubnubFetchHistoryResult Result = PubnubClient->FetchHistory(FString(), FPubnubFetchHistorySettings());

	TestTrue(TEXT("Result should indicate error"), Result.Result.Error);
	TestTrue(TEXT("ErrorMessage should mention Channel"), Result.Result.ErrorMessage.Contains(TEXT("Channel")));
	TestTrue(TEXT("ErrorMessage should indicate field is empty"), Result.Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubFetchHistory_ClientNotInitialized_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();

	FPubnubFetchHistoryResult Result = Client->FetchHistory(SDK_PREFIX + TEXT("mp_fetch_noinit_ch"), FPubnubFetchHistorySettings());

	TestTrue(TEXT("Result should indicate error"), Result.Result.Error);
	TestTrue(TEXT("ErrorMessage should mention not initialized"),
		Result.Result.ErrorMessage.Contains(TEXT("not initialized")) || Result.Result.ErrorMessage.Contains(TEXT("invalid")));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// FetchHistory — behaviour
// ---------------------------------------------------------------------------

bool FPubnubFetchHistory_HappyPath_DefaultSettings_FindsPublishedMessage::RunTest(const FString& Parameters)
{
	const FString Channel = SDK_PREFIX + TEXT("mp_fetch_happy_ch") + PubnubMessagePersistenceTestsHelper::UniqueHistoryChannelSuffix();
	const FString User = SDK_PREFIX + TEXT("mp_fetch_happy_user");
	const FString PayloadMarker = TEXT("mp_fetch_happy_marker");
	const FString MessageJson = FString::Printf(TEXT("\"%s\""), *PayloadMarker);

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(User);

	FPubnubPublishMessageResult Pub = PubnubClient->PublishMessage(Channel, MessageJson, FPubnubPublishSettings());
	TestFalse(TEXT("Publish should succeed"), Pub.Result.Error);

	FPubnubFetchHistoryResult Hist = PubnubClient->FetchHistory(Channel, FPubnubFetchHistorySettings());
	TestFalse(TEXT("FetchHistory should succeed"), Hist.Result.Error);
	TestTrue(TEXT("History should include published payload"),
		PubnubMessagePersistenceTestsHelper::FindMessageContaining(Hist.Messages, PayloadMarker) != nullptr);

	const FPubnubHistoryMessageData* Found = PubnubMessagePersistenceTestsHelper::FindMessageContaining(Hist.Messages, PayloadMarker);
	TestEqual(TEXT("History channel should match"), Found->Channel, Channel);
	// Default fetch settings omit IncludeUserID; do not assert on UserID here (see 3FullParameters.AllOptions).
	TestFalse(TEXT("History timetoken should be set"), Found->Timetoken.IsEmpty());

	CleanUp();
	return true;
}

bool FPubnubFetchHistory_FullSettings_AllIncludes_MaxPerChannel::RunTest(const FString& Parameters)
{
	const FString Channel = SDK_PREFIX + TEXT("mp_fetch_full_ch");
	const FString User = SDK_PREFIX + TEXT("mp_fetch_full_user");
	const FString PayloadMarker = TEXT("mp_fetch_full_marker");
	const FString MessageJson = FString::Printf(TEXT("{\"t\":\"%s\"}"), *PayloadMarker);
	const FString MetaJson = TEXT("{\"src\":\"mp_test\"}");
	const FString CustomType = TEXT("mp-custom-type");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(User);

	FPubnubPublishSettings PubSettings;
	PubSettings.MetaData = MetaJson;
	PubSettings.CustomMessageType = CustomType;
	PubSettings.StoreInHistory = true;

	FPubnubPublishMessageResult Pub = PubnubClient->PublishMessage(Channel, MessageJson, PubSettings);
	TestFalse(TEXT("Publish should succeed"), Pub.Result.Error);

	FPubnubFetchHistorySettings Fs;
	Fs.MaxPerChannel = 25;
	Fs.IncludeMeta = true;
	Fs.IncludeMessageType = true;
	Fs.IncludeUserID = true;
	Fs.IncludeMessageActions = true;
	Fs.IncludeCustomMessageType = true;

	FPubnubFetchHistoryResult Hist = PubnubClient->FetchHistory(Channel, Fs);
	TestFalse(TEXT("FetchHistory with full settings should succeed"), Hist.Result.Error);

	const FPubnubHistoryMessageData* Found = PubnubMessagePersistenceTestsHelper::FindMessageContaining(Hist.Messages, PayloadMarker);
	TestNotNull(TEXT("Should find published message"), Found);
	TestEqual(TEXT("Channel should match"), Found->Channel, Channel);
	TestEqual(TEXT("UserID should match when IncludeUserID"), Found->UserID, User);
	// History often returns "message_type": null for normal publishes; IncludeMessageType still requests the field. Assert custom type instead.
	TestEqual(TEXT("CustomMessageType should match publish"), Found->CustomMessageType, CustomType);
	TestTrue(TEXT("Meta should contain publish metadata"), Found->Meta.Contains(TEXT("src")) || Found->Meta.Contains(TEXT("mp_test")));

	CleanUp();
	return true;
}

bool FPubnubFetchHistory_MaxPerChannel_LimitsReturnedMessages::RunTest(const FString& Parameters)
{
	// Idea: default history can return many messages; MaxPerChannel must cap the batch (here 3 publishes, limit 2).
	// Publish three stored messages; MaxPerChannel=2 should not return all three.
	const FString Channel = SDK_PREFIX + TEXT("mp_fetch_max_ch") + PubnubMessagePersistenceTestsHelper::UniqueHistoryChannelSuffix();
	const FString User = SDK_PREFIX + TEXT("mp_fetch_max_user");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(User);

	for (int32 i = 0; i < 3; ++i)
	{
		const FString Msg = FString::Printf(TEXT("\"mp_max_%d\""), i);
		FPubnubPublishMessageResult Pub = PubnubClient->PublishMessage(Channel, Msg, FPubnubPublishSettings());
		TestFalse(TEXT("Publish should succeed"), Pub.Result.Error);
		FPlatformProcess::Sleep(0.05f);
	}

	FPubnubFetchHistorySettings Fs;
	Fs.MaxPerChannel = 2;

	FPubnubFetchHistoryResult Hist = PubnubClient->FetchHistory(Channel, Fs);
	TestFalse(TEXT("FetchHistory should succeed"), Hist.Result.Error);
	TestEqual(TEXT("Should return at most MaxPerChannel messages"), Hist.Messages.Num(), 2);

	CleanUp();
	return true;
}

bool FPubnubFetchHistory_TimeWindow_StartNewerExclusive_EndOlderInclusive::RunTest(const FString& Parameters)
{
	// Idea: Validate Start/End semantics — Start is the newer timetoken (exclusive), End the older (inclusive); window should exclude the newest message only.
	// Two messages with distinct timetokens; Start=newer (exclusive), End=older (inclusive) should return only the older payload.
	const FString Channel = SDK_PREFIX + TEXT("mp_fetch_win_ch") + PubnubMessagePersistenceTestsHelper::UniqueHistoryChannelSuffix();
	const FString User = SDK_PREFIX + TEXT("mp_fetch_win_user");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(User);

	TestFalse(TEXT("Publish older"), PubnubClient->PublishMessage(Channel, TEXT("\"mp_win_old\""), FPubnubPublishSettings()).Result.Error);
	FPlatformProcess::Sleep(0.05f);
	TestFalse(TEXT("Publish newer"), PubnubClient->PublishMessage(Channel, TEXT("\"mp_win_new\""), FPubnubPublishSettings()).Result.Error);

	FPubnubFetchHistoryResult All = PubnubClient->FetchHistory(Channel, FPubnubFetchHistorySettings());
	TestFalse(TEXT("Full fetch should succeed"), All.Result.Error);
	const FPubnubHistoryMessageData* OldMsg = PubnubMessagePersistenceTestsHelper::FindMessageContaining(All.Messages, TEXT("mp_win_old"));
	const FPubnubHistoryMessageData* NewMsg = PubnubMessagePersistenceTestsHelper::FindMessageContaining(All.Messages, TEXT("mp_win_new"));
	TestNotNull(TEXT("Should find older message"), OldMsg);
	TestNotNull(TEXT("Should find newer message"), NewMsg);

	const FString TTOld = OldMsg->Timetoken;
	const FString TTNew = NewMsg->Timetoken;
	const FString StartNewer = PubnubMessagePersistenceTestsHelper::NewerOfTwoTimetokens(TTOld, TTNew);
	const FString EndOlder = PubnubMessagePersistenceTestsHelper::OlderOfTwoTimetokens(TTOld, TTNew);

	FPubnubFetchHistorySettings Fs;
	Fs.Start = StartNewer;
	Fs.End = EndOlder;

	FPubnubFetchHistoryResult Win = PubnubClient->FetchHistory(Channel, Fs);
	TestFalse(TEXT("Window fetch should succeed"), Win.Result.Error);
	TestEqual(TEXT("Window should return exactly the older message"), Win.Messages.Num(), 1);
	TestTrue(TEXT("Remaining message should be older payload"), Win.Messages[0].Message.Contains(TEXT("mp_win_old")));

	CleanUp();
	return true;
}

bool FPubnubFetchHistory_ConcurrentSyncWhileAsync_ReturnsMutexError::RunTest(const FString& Parameters)
{
	// Idea: Mutex fast-fail — queue async FetchHistory then sync FetchHistory; sync may error with concurrent-operation message.
	const FString Channel = SDK_PREFIX + TEXT("mp_fetch_mutex_ch") + PubnubMessagePersistenceTestsHelper::UniqueHistoryChannelSuffix();
	const FString User = SDK_PREFIX + TEXT("mp_fetch_mutex_user");
	TSharedPtr<FPubnubFetchHistoryResult> SyncResult = MakeShared<FPubnubFetchHistoryResult>();

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(User);

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Channel]()
	{
		PubnubClient->FetchHistoryAsync(Channel, FOnPubnubFetchHistoryResponseNative(), FPubnubFetchHistorySettings());
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Channel, SyncResult]()
	{
		*SyncResult = PubnubClient->FetchHistory(Channel, FPubnubFetchHistorySettings());
	}, 0.15f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SyncResult]()
	{
		if (SyncResult->Result.Error)
		{
			TestTrue(TEXT("When sync fails concurrently, message should mention in-progress operation"),
				SyncResult->Result.ErrorMessage.Contains(TEXT("operation is in progress")) ||
				SyncResult->Result.ErrorMessage.Contains(TEXT("concurrently")));
		}
	}, 0.2f));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// DeleteMessages — validation
// ---------------------------------------------------------------------------

bool FPubnubDeleteMessages_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("mp_del_val_user"));

	FPubnubOperationResult Result = PubnubClient->DeleteMessages(FString(), FPubnubDeleteMessagesSettings());

	TestTrue(TEXT("Result should indicate error"), Result.Error);
	TestTrue(TEXT("ErrorMessage should mention Channel"), Result.ErrorMessage.Contains(TEXT("Channel")));

	CleanUp();
	return true;
}

bool FPubnubDeleteMessages_ClientNotInitialized_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();

	FPubnubOperationResult Result = Client->DeleteMessages(SDK_PREFIX + TEXT("mp_del_noinit_ch"), FPubnubDeleteMessagesSettings());

	TestTrue(TEXT("Result should indicate error"), Result.Error);
	TestTrue(TEXT("ErrorMessage should mention not initialized"),
		Result.ErrorMessage.Contains(TEXT("not initialized")) || Result.ErrorMessage.Contains(TEXT("invalid")));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// DeleteMessages — behaviour (requires Delete-from-History on keyset)
// ---------------------------------------------------------------------------

bool FPubnubDeleteMessages_HappyPath_DefaultSettings_ClearsHistory::RunTest(const FString& Parameters)
{
	const FString Channel = SDK_PREFIX + TEXT("mp_del_happy_ch") + PubnubMessagePersistenceTestsHelper::UniqueHistoryChannelSuffix();
	const FString User = SDK_PREFIX + TEXT("mp_del_happy_user");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(User);

	TestFalse(TEXT("Publish"), PubnubClient->PublishMessage(Channel, TEXT("\"mp_del_happy\""), FPubnubPublishSettings()).Result.Error);

	FPubnubFetchHistoryResult Before = PubnubClient->FetchHistory(Channel, FPubnubFetchHistorySettings());
	TestFalse(TEXT("Fetch before delete should succeed"), Before.Result.Error);
	TestTrue(TEXT("Should have history before delete"), Before.Messages.Num() > 0);

	FPubnubOperationResult Del = PubnubClient->DeleteMessages(Channel, FPubnubDeleteMessagesSettings());
	TestFalse(TEXT("DeleteMessages should succeed"), Del.Error);

	FPubnubFetchHistoryResult After = PubnubClient->FetchHistory(Channel, FPubnubFetchHistorySettings());
	TestFalse(TEXT("Fetch after delete should succeed"), After.Result.Error);
	TestEqual(TEXT("History should be empty after full delete"), After.Messages.Num(), 0);

	CleanUp();
	return true;
}

bool FPubnubDeleteMessages_FullParameters_TimeSlice_KeepsNewerMessage::RunTest(const FString& Parameters)
{
	// Idea: DeleteMessages Start/End match history semantics — delete slice removes older message only, newer message remains fetchable.
	// Start = newer timetoken (exclusive boundary), End = older (inclusive): deletes the older message only; newer remains.
	const FString Channel = SDK_PREFIX + TEXT("mp_del_slice_ch") + PubnubMessagePersistenceTestsHelper::UniqueHistoryChannelSuffix();
	const FString User = SDK_PREFIX + TEXT("mp_del_slice_user");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(User);

	TestFalse(TEXT("Publish old"), PubnubClient->PublishMessage(Channel, TEXT("\"mp_slice_old\""), FPubnubPublishSettings()).Result.Error);
	FPlatformProcess::Sleep(0.05f);
	TestFalse(TEXT("Publish new"), PubnubClient->PublishMessage(Channel, TEXT("\"mp_slice_new\""), FPubnubPublishSettings()).Result.Error);

	FPubnubFetchHistoryResult All = PubnubClient->FetchHistory(Channel, FPubnubFetchHistorySettings());
	TestFalse(TEXT("Fetch should succeed"), All.Result.Error);
	const FPubnubHistoryMessageData* OldMsg = PubnubMessagePersistenceTestsHelper::FindMessageContaining(All.Messages, TEXT("mp_slice_old"));
	const FPubnubHistoryMessageData* NewMsg = PubnubMessagePersistenceTestsHelper::FindMessageContaining(All.Messages, TEXT("mp_slice_new"));
	TestNotNull(TEXT("Find old"), OldMsg);
	TestNotNull(TEXT("Find new"), NewMsg);

	FPubnubDeleteMessagesSettings Ds;
	Ds.Start = PubnubMessagePersistenceTestsHelper::NewerOfTwoTimetokens(OldMsg->Timetoken, NewMsg->Timetoken);
	Ds.End = PubnubMessagePersistenceTestsHelper::OlderOfTwoTimetokens(OldMsg->Timetoken, NewMsg->Timetoken);

	FPubnubOperationResult Del = PubnubClient->DeleteMessages(Channel, Ds);
	TestFalse(TEXT("Partial delete should succeed"), Del.Error);

	FPubnubFetchHistoryResult After = PubnubClient->FetchHistory(Channel, FPubnubFetchHistorySettings());
	TestFalse(TEXT("Fetch after partial delete should succeed"), After.Result.Error);
	TestEqual(TEXT("Only newer message should remain"), After.Messages.Num(), 1);
	TestTrue(TEXT("Remaining should be newer payload"), After.Messages[0].Message.Contains(TEXT("mp_slice_new")));

	CleanUp();
	return true;
}

bool FPubnubDeleteMessages_ConcurrentSyncWhileAsync_ReturnsMutexError::RunTest(const FString& Parameters)
{
	// Idea: Same mutex behaviour as other sync APIs when async DeleteMessages is in flight.
	const FString Channel = SDK_PREFIX + TEXT("mp_del_mutex_ch") + PubnubMessagePersistenceTestsHelper::UniqueHistoryChannelSuffix();
	const FString User = SDK_PREFIX + TEXT("mp_del_mutex_user");
	TSharedPtr<FPubnubOperationResult> SyncResult = MakeShared<FPubnubOperationResult>();

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(User);

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Channel]()
	{
		PubnubClient->DeleteMessagesAsync(Channel, FOnPubnubDeleteMessagesResponseNative(), FPubnubDeleteMessagesSettings());
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Channel, SyncResult]()
	{
		*SyncResult = PubnubClient->DeleteMessages(Channel, FPubnubDeleteMessagesSettings());
	}, 0.15f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SyncResult]()
	{
		if (SyncResult->Error)
		{
			TestTrue(TEXT("When sync fails concurrently, message should mention in-progress operation"),
				SyncResult->ErrorMessage.Contains(TEXT("operation is in progress")) ||
				SyncResult->ErrorMessage.Contains(TEXT("concurrently")));
		}
	}, 0.2f));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// MessageCounts — validation
// ---------------------------------------------------------------------------

bool FPubnubMessageCounts_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("mp_mc_val_user"));

	FPubnubMessageCountsResult Result = PubnubClient->MessageCounts(FString(), PubnubMessagePersistenceTestsHelper::BaselineTimetoken);

	TestTrue(TEXT("Result should indicate error"), Result.Result.Error);
	TestTrue(TEXT("ErrorMessage should mention Channel"), Result.Result.ErrorMessage.Contains(TEXT("Channel")));

	CleanUp();
	return true;
}

bool FPubnubMessageCounts_ClientNotInitialized_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();

	FPubnubMessageCountsResult Result = Client->MessageCounts(SDK_PREFIX + TEXT("mp_mc_noinit_ch"), PubnubMessagePersistenceTestsHelper::BaselineTimetoken);

	TestTrue(TEXT("Result should indicate error"), Result.Result.Error);
	TestTrue(TEXT("ErrorMessage should mention not initialized"),
		Result.Result.ErrorMessage.Contains(TEXT("not initialized")) || Result.Result.ErrorMessage.Contains(TEXT("invalid")));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// MessageCounts — behaviour
// ---------------------------------------------------------------------------

bool FPubnubMessageCounts_HappyPath_BaselineTimetoken_IncludesPublished::RunTest(const FString& Parameters)
{
	const FString Channel = SDK_PREFIX + TEXT("mp_mc_happy_ch") + PubnubMessagePersistenceTestsHelper::UniqueHistoryChannelSuffix();
	const FString User = SDK_PREFIX + TEXT("mp_mc_happy_user");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(User);

	TestFalse(TEXT("Publish"), PubnubClient->PublishMessage(Channel, TEXT("\"mp_mc_happy\""), FPubnubPublishSettings()).Result.Error);

	FPubnubMessageCountsResult Mc = PubnubClient->MessageCounts(Channel, PubnubMessagePersistenceTestsHelper::BaselineTimetoken);
	TestFalse(TEXT("MessageCounts should succeed"), Mc.Result.Error);
	TestTrue(TEXT("Count should include published message"), Mc.MessageCounts >= 1);

	CleanUp();
	return true;
}

bool FPubnubMessageCounts_UsingOlderPublishedTimetoken_CountsAtLeastOneNewer::RunTest(const FString& Parameters)
{
	// Idea: Timetoken argument filters the count — after two publishes, count from the first publish timetoken must include the second message.
	// Do not call FetchHistory before MessageCounts: a prior history transaction can leave the C-core in a state where
	// pubnub_get_message_counts returns -1 while HTTP status is still 200 (wrong/parsed response path).
	const FString Channel = SDK_PREFIX + TEXT("mp_mc_tt_ch") + PubnubMessagePersistenceTestsHelper::UniqueHistoryChannelSuffix();
	const FString User = SDK_PREFIX + TEXT("mp_mc_tt_user");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(User);

	FPubnubPublishMessageResult P1 = PubnubClient->PublishMessage(Channel, TEXT("\"mp_mc_1\""), FPubnubPublishSettings());
	TestFalse(TEXT("Publish first should succeed"), P1.Result.Error);
	TestFalse(TEXT("First publish should return timetoken"), P1.PublishedMessage.Timetoken.IsEmpty());

	FPlatformProcess::Sleep(0.05f);

	TestFalse(TEXT("Publish second should succeed"), PubnubClient->PublishMessage(Channel, TEXT("\"mp_mc_2\""), FPubnubPublishSettings()).Result.Error);

	FPubnubMessageCountsResult Mc = PubnubClient->MessageCounts(Channel, P1.PublishedMessage.Timetoken);
	TestFalse(TEXT("MessageCounts should succeed"), Mc.Result.Error);
	TestTrue(TEXT("Should count at least one message after first publish timetoken"), Mc.MessageCounts >= 1);

	CleanUp();
	return true;
}

bool FPubnubMessageCounts_ConcurrentSyncWhileAsync_ReturnsMutexError::RunTest(const FString& Parameters)
{
	// Idea: MessageCounts uses same single-flight mutex as history APIs.
	const FString Channel = SDK_PREFIX + TEXT("mp_mc_mutex_ch") + PubnubMessagePersistenceTestsHelper::UniqueHistoryChannelSuffix();
	const FString User = SDK_PREFIX + TEXT("mp_mc_mutex_user");
	const FString Tt = PubnubMessagePersistenceTestsHelper::BaselineTimetoken;
	TSharedPtr<FPubnubMessageCountsResult> SyncResult = MakeShared<FPubnubMessageCountsResult>();

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(User);

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Channel, Tt]()
	{
		FOnPubnubMessageCountsResponseNative CB;
		CB.BindLambda([](const FPubnubOperationResult&, int) {});
		PubnubClient->MessageCountsAsync(Channel, Tt, CB);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Channel, Tt, SyncResult]()
	{
		*SyncResult = PubnubClient->MessageCounts(Channel, Tt);
	}, 0.15f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SyncResult]()
	{
		if (SyncResult->Result.Error)
		{
			TestTrue(TEXT("When sync fails concurrently, message should mention in-progress operation"),
				SyncResult->Result.ErrorMessage.Contains(TEXT("operation is in progress")) ||
				SyncResult->Result.ErrorMessage.Contains(TEXT("concurrently")));
		}
	}, 0.2f));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// MessageCountsMultiple — validation
// ---------------------------------------------------------------------------

bool FPubnubMessageCountsMultiple_EmptyChannels_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("mp_mcm_val_user"));

	TArray<FString> EmptyChannels;
	TArray<FString> Tokens = { PubnubMessagePersistenceTestsHelper::BaselineTimetoken };

	FPubnubMessageCountsMultipleResult Result = PubnubClient->MessageCountsMultiple(EmptyChannels, Tokens);

	TestTrue(TEXT("Result should indicate error"), Result.Result.Error);
	TestEqual(TEXT("ErrorMessage should match validation"), Result.Result.ErrorMessage, TEXT("Channels array cannot be empty."));

	CleanUp();
	return true;
}

bool FPubnubMessageCountsMultiple_ChannelTimetokenCountMismatch_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("mp_mcm_sz_user"));

	TArray<FString> Channels = { SDK_PREFIX + TEXT("mp_mcm_a"), SDK_PREFIX + TEXT("mp_mcm_b") };
	TArray<FString> Tokens = { PubnubMessagePersistenceTestsHelper::BaselineTimetoken };

	FPubnubMessageCountsMultipleResult Result = PubnubClient->MessageCountsMultiple(Channels, Tokens);

	TestTrue(TEXT("Result should indicate error"), Result.Result.Error);
	TestEqual(TEXT("ErrorMessage should match validation"), Result.Result.ErrorMessage, TEXT("Number of channels must match number of timetokens."));

	CleanUp();
	return true;
}

bool FPubnubMessageCountsMultiple_ClientNotInitialized_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();

	TArray<FString> Channels = { SDK_PREFIX + TEXT("mp_mcm_noinit_a") };
	TArray<FString> Tokens = { PubnubMessagePersistenceTestsHelper::BaselineTimetoken };

	FPubnubMessageCountsMultipleResult Result = Client->MessageCountsMultiple(Channels, Tokens);

	TestTrue(TEXT("Result should indicate error"), Result.Result.Error);
	TestTrue(TEXT("ErrorMessage should mention not initialized"),
		Result.Result.ErrorMessage.Contains(TEXT("not initialized")) || Result.Result.ErrorMessage.Contains(TEXT("invalid")));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// MessageCountsMultiple — behaviour
// ---------------------------------------------------------------------------

bool FPubnubMessageCountsMultiple_HappyPath_TwoChannels_IndependentCounts::RunTest(const FString& Parameters)
{
	const FString Suffix = PubnubMessagePersistenceTestsHelper::UniqueHistoryChannelSuffix();
	const FString ChA = SDK_PREFIX + TEXT("mp_mcm_happy_a") + Suffix;
	const FString ChB = SDK_PREFIX + TEXT("mp_mcm_happy_b") + Suffix;
	const FString User = SDK_PREFIX + TEXT("mp_mcm_happy_user");
	const FString Tt = PubnubMessagePersistenceTestsHelper::BaselineTimetoken;

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(User);

	for (int32 i = 0; i < 3; ++i)
	{
		TestFalse(TEXT("Publish to A"), PubnubClient->PublishMessage(ChA, FString::Printf(TEXT("\"mcm_a_%d\""), i), FPubnubPublishSettings()).Result.Error);
	}
	TestFalse(TEXT("Publish to B"), PubnubClient->PublishMessage(ChB, TEXT("\"mcm_b_0\""), FPubnubPublishSettings()).Result.Error);

	TArray<FString> Channels = { ChA, ChB };
	TArray<FString> Tokens = { Tt, Tt };

	FPubnubMessageCountsMultipleResult Mc = PubnubClient->MessageCountsMultiple(Channels, Tokens);
	TestFalse(TEXT("MessageCountsMultiple should succeed"), Mc.Result.Error);
	TestTrue(TEXT("Map should contain channel A"), Mc.MessageCountsPerChannel.Contains(ChA));
	TestTrue(TEXT("Map should contain channel B"), Mc.MessageCountsPerChannel.Contains(ChB));
	TestTrue(TEXT("Channel A count should reflect three publishes"), Mc.MessageCountsPerChannel[ChA] >= 3);
	TestTrue(TEXT("Channel B count should reflect one publish"), Mc.MessageCountsPerChannel[ChB] >= 1);

	CleanUp();
	return true;
}

bool FPubnubMessageCountsMultiple_ConcurrentSyncWhileAsync_ReturnsMutexError::RunTest(const FString& Parameters)
{
	// Idea: MessageCountsMultiple shares the client mutex; concurrent sync/async should hit the same guard.
	const FString Ch = SDK_PREFIX + TEXT("mp_mcm_mutex_ch") + PubnubMessagePersistenceTestsHelper::UniqueHistoryChannelSuffix();
	const FString User = SDK_PREFIX + TEXT("mp_mcm_mutex_user");
	const FString Tt = PubnubMessagePersistenceTestsHelper::BaselineTimetoken;
	TSharedPtr<FPubnubMessageCountsMultipleResult> SyncResult = MakeShared<FPubnubMessageCountsMultipleResult>();

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(User);

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Ch, Tt]()
	{
		FOnPubnubMessageCountsMultipleResponseNative CB;
		CB.BindLambda([](const FPubnubMessageCountsMultipleResult&) {});
		TArray<FString> Chs = { Ch };
		TArray<FString> Ts = { Tt };
		PubnubClient->MessageCountsMultipleAsync(Chs, Ts, CB);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Ch, Tt, SyncResult]()
	{
		TArray<FString> Chs = { Ch };
		TArray<FString> Ts = { Tt };
		*SyncResult = PubnubClient->MessageCountsMultiple(Chs, Ts);
	}, 0.15f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SyncResult]()
	{
		if (SyncResult->Result.Error)
		{
			TestTrue(TEXT("When sync fails concurrently, message should mention in-progress operation"),
				SyncResult->Result.ErrorMessage.Contains(TEXT("operation is in progress")) ||
				SyncResult->Result.ErrorMessage.Contains(TEXT("concurrently")));
		}
	}, 0.2f));

	CleanUp();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
