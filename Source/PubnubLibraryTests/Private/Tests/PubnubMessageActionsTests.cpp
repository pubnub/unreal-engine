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

namespace PubnubMessageActionsTestsPrivate
{
	static FString UniqueChannelSuffix()
	{
		return FString::Printf(TEXT("_%05d"), FMath::RandRange(0, 99999));
	}

	static const FPubnubHistoryMessageData* FindHistoryMessageContaining(const TArray<FPubnubHistoryMessageData>& Messages, const FString& Marker)
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

	static const FPubnubMessageActionData* FindActionByTypeAndValue(const TArray<FPubnubMessageActionData>& Actions, const FString& Type, const FString& Value)
	{
		for (const FPubnubMessageActionData& A : Actions)
		{
			if (A.Type == Type && A.Value == Value)
			{
				return &A;
			}
		}
		return nullptr;
	}

	static const FPubnubMessageActionData* FindActionByActionTimetoken(const TArray<FPubnubMessageActionData>& Actions, const FString& ActionTimetoken)
	{
		for (const FPubnubMessageActionData& A : Actions)
		{
			if (A.ActionTimetoken == ActionTimetoken)
			{
				return &A;
			}
		}
		return nullptr;
	}

	/** Older bound (past); use as GetMessageActions End — server walks backward in time until this. */
	static const TCHAR* BaselineActionTimetoken = TEXT("10000000000000000");
	/** Newer bound; use as GetMessageActions Start — server begins here and moves toward older actions until End. Start must be lexicographically greater than End. */
	static const TCHAR* NewerActionTimetokenBound = TEXT("99999999999999999");

	/** Default limit when fetching all actions in the standard [End, Start] window (server pages from Start toward older). */
	static constexpr int32 DefaultMessageActionsLimit = 100;

	/** Calls GetMessageActions with valid Start/End (Start newer than End); required for the server. */
	static FPubnubGetMessageActionsResult GetMessageActionsDefaultWindow(UPubnubClient* Client, const FString& Channel, int32 Limit = DefaultMessageActionsLimit)
	{
		return Client->GetMessageActions(Channel, NewerActionTimetokenBound, BaselineActionTimetoken, Limit);
	}
}

// ---------------------------------------------------------------------------
// UPubnubClient::AddMessageAction
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddMessageAction_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.AddMessageAction.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddMessageAction_EmptyMessageTimetoken_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.AddMessageAction.1Validation.EmptyMessageTimetoken",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddMessageAction_EmptyActionType_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.AddMessageAction.1Validation.EmptyActionType",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddMessageAction_EmptyValue_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.AddMessageAction.1Validation.EmptyValue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddMessageAction_ClientNotInitialized_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.AddMessageAction.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddMessageAction_HappyPath_RequiredParams_ReturnsActionData, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.AddMessageAction.2HappyPath.RequiredParams",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddMessageAction_TwoValuesOnSameMessage_BothRetrievable, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.AddMessageAction.4Advanced.TwoDistinctValuesSameMessage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddMessageAction_ConcurrentSyncWhileAsync_ReturnsMutexError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.AddMessageAction.4Advanced.ConcurrentSyncWhileAsync",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::GetMessageActions
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetMessageActions_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.GetMessageActions.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetMessageActions_ClientNotInitialized_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.GetMessageActions.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetMessageActions_HappyPath_RequiredStartEnd, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.GetMessageActions.2HappyPath.RequiredStartEnd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetMessageActions_FullParameters_StartEndLimit, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.GetMessageActions.3FullParameters.StartEndLimit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetMessageActions_LimitOne_AfterTwoAdds_ReturnsSingleAction, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.GetMessageActions.4Advanced.LimitCapsPageSize",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::RemoveMessageAction
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveMessageAction_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.RemoveMessageAction.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveMessageAction_EmptyMessageTimetoken_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.RemoveMessageAction.1Validation.EmptyMessageTimetoken",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveMessageAction_EmptyActionTimetoken_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.RemoveMessageAction.1Validation.EmptyActionTimetoken",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveMessageAction_ClientNotInitialized_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.RemoveMessageAction.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveMessageAction_HappyPath_RemovesThenMissingFromGet, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.RemoveMessageAction.2HappyPath.RemoveThenNotListed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveMessageAction_InvalidActionTimetoken_ServerError, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.RemoveMessageAction.4Advanced.InvalidActionTimetoken",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::FetchHistory + message actions
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubFetchHistory_IncludeMessageActions_EmbedsActionsOnMessage, FPubnubAutomationTestBase,
	"Pubnub.Integration.MessageActions.FetchHistory.4Advanced.IncludeMessageActionsOnHistory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// AddMessageAction — validation
// ---------------------------------------------------------------------------

bool FPubnubAddMessageAction_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("ma_add_val_ch_user"));

	FPubnubAddMessageActionResult R = PubnubClient->AddMessageAction(FString(), TEXT("123"), TEXT("reaction"), TEXT("like"));

	TestTrue(TEXT("Should error"), R.Result.Error);
	TestTrue(TEXT("Error should mention Channel"), R.Result.ErrorMessage.Contains(TEXT("Channel")));

	CleanUp();
	return true;
}

bool FPubnubAddMessageAction_EmptyMessageTimetoken_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("ma_add_val_tt_user"));

	FPubnubAddMessageActionResult R = PubnubClient->AddMessageAction(SDK_PREFIX + TEXT("ma_add_val_tt_ch"), FString(), TEXT("reaction"), TEXT("like"));

	TestTrue(TEXT("Should error"), R.Result.Error);
	TestTrue(TEXT("Error should mention MessageTimetoken"), R.Result.ErrorMessage.Contains(TEXT("MessageTimetoken")));

	CleanUp();
	return true;
}

bool FPubnubAddMessageAction_EmptyActionType_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("ma_add_val_type_user"));

	FPubnubAddMessageActionResult R = PubnubClient->AddMessageAction(SDK_PREFIX + TEXT("ma_add_val_type_ch"), TEXT("123"), FString(), TEXT("like"));

	TestTrue(TEXT("Should error"), R.Result.Error);
	TestTrue(TEXT("Error should mention ActionType"), R.Result.ErrorMessage.Contains(TEXT("ActionType")));

	CleanUp();
	return true;
}

bool FPubnubAddMessageAction_EmptyValue_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("ma_add_val_val_user"));

	FPubnubAddMessageActionResult R = PubnubClient->AddMessageAction(SDK_PREFIX + TEXT("ma_add_val_val_ch"), TEXT("123"), TEXT("reaction"), FString());

	TestTrue(TEXT("Should error"), R.Result.Error);
	TestTrue(TEXT("Error should mention Value"), R.Result.ErrorMessage.Contains(TEXT("Value")));

	CleanUp();
	return true;
}

bool FPubnubAddMessageAction_ClientNotInitialized_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();

	FPubnubAddMessageActionResult R = Client->AddMessageAction(SDK_PREFIX + TEXT("ma_add_noinit_ch"), TEXT("1"), TEXT("reaction"), TEXT("like"));

	TestTrue(TEXT("Should error"), R.Result.Error);
	TestTrue(TEXT("Should mention not initialized"),
		R.Result.ErrorMessage.Contains(TEXT("not initialized")) || R.Result.ErrorMessage.Contains(TEXT("invalid")));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// AddMessageAction — behaviour
// ---------------------------------------------------------------------------

bool FPubnubAddMessageAction_HappyPath_RequiredParams_ReturnsActionData::RunTest(const FString& Parameters)
{
	const FString ChannelSuffix = PubnubMessageActionsTestsPrivate::UniqueChannelSuffix();
	const FString Channel = SDK_PREFIX + TEXT("ma_add_happy_ch") + ChannelSuffix;
	const FString User = SDK_PREFIX + TEXT("ma_add_happy_user");
	const FString Marker = FString::Printf(TEXT("ma_add_happy_marker%s"), *ChannelSuffix);
	const FString MessageJson = FString::Printf(TEXT("{\"m\":\"%s\"}"), *Marker);

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
	TestFalse(TEXT("Publish timetoken should be set"), Pub.PublishedMessage.Timetoken.IsEmpty());

	const FString MessageTimetoken = Pub.PublishedMessage.Timetoken;

	FPubnubAddMessageActionResult Add = PubnubClient->AddMessageAction(Channel, MessageTimetoken, TEXT("reaction"), TEXT("like"));
	TestFalse(TEXT("AddMessageAction should succeed"), Add.Result.Error);
	TestEqual(TEXT("Action type"), Add.MessageActionData.Type, TEXT("reaction"));
	TestEqual(TEXT("Action value"), Add.MessageActionData.Value, TEXT("like"));
	TestEqual(TEXT("Action user"), Add.MessageActionData.UserID, User);
	TestEqual(TEXT("Action message timetoken"), Add.MessageActionData.MessageTimetoken, MessageTimetoken);
	TestFalse(TEXT("Action timetoken should be set"), Add.MessageActionData.ActionTimetoken.IsEmpty());

	CleanUp();
	return true;
}

bool FPubnubAddMessageAction_TwoValuesOnSameMessage_BothRetrievable::RunTest(const FString& Parameters)
{
	// Idea: Same parent message can carry multiple actions; GetMessageActions should return both with correct type/value/tokens.
	const FString ChannelSuffix = PubnubMessageActionsTestsPrivate::UniqueChannelSuffix();
	const FString Channel = SDK_PREFIX + TEXT("ma_add_two_ch") + ChannelSuffix;
	const FString User = SDK_PREFIX + TEXT("ma_add_two_user");
	const FString Marker = FString::Printf(TEXT("ma_add_two_marker%s"), *ChannelSuffix);
	const FString MessageJson = FString::Printf(TEXT("{\"m\":\"%s\"}"), *Marker);

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
	const FString MessageTimetoken = Pub.PublishedMessage.Timetoken;

	FPubnubAddMessageActionResult A1 = PubnubClient->AddMessageAction(Channel, MessageTimetoken, TEXT("reaction"), TEXT("like"));
	TestFalse(TEXT("First add should succeed"), A1.Result.Error);
	FPubnubAddMessageActionResult A2 = PubnubClient->AddMessageAction(Channel, MessageTimetoken, TEXT("reaction"), TEXT("smiley_face"));
	TestFalse(TEXT("Second add should succeed"), A2.Result.Error);
	TestTrue(TEXT("Distinct action timetokens"), A1.MessageActionData.ActionTimetoken != A2.MessageActionData.ActionTimetoken);

	FPubnubGetMessageActionsResult Get = PubnubMessageActionsTestsPrivate::GetMessageActionsDefaultWindow(PubnubClient, Channel);
	TestFalse(TEXT("GetMessageActions should succeed"), Get.Result.Error);
	if (Get.Result.Error)
	{
		CleanUp();
		return true;
	}

	const FPubnubMessageActionData* G1 = PubnubMessageActionsTestsPrivate::FindActionByTypeAndValue(Get.MessageActions, TEXT("reaction"), TEXT("like"));
	const FPubnubMessageActionData* G2 = PubnubMessageActionsTestsPrivate::FindActionByTypeAndValue(Get.MessageActions, TEXT("reaction"), TEXT("smiley_face"));
	TestNotNull(TEXT("Should find like action"), G1);
	TestNotNull(TEXT("Should find smiley_face action"), G2);
	if (!G1 || !G2)
	{
		CleanUp();
		return true;
	}
	TestEqual(TEXT("like action message timetoken"), G1->MessageTimetoken, MessageTimetoken);
	TestEqual(TEXT("smiley message timetoken"), G2->MessageTimetoken, MessageTimetoken);
	TestEqual(TEXT("like action user"), G1->UserID, User);

	CleanUp();
	return true;
}

bool FPubnubAddMessageAction_ConcurrentSyncWhileAsync_ReturnsMutexError::RunTest(const FString& Parameters)
{
	// Idea: Mutex fast-fail — queue async AddMessageAction then sync AddMessageAction; sync may error with concurrent-operation message.
	const FString ChannelSuffix = PubnubMessageActionsTestsPrivate::UniqueChannelSuffix();
	const FString Channel = SDK_PREFIX + TEXT("ma_add_mutex_ch") + ChannelSuffix;
	const FString User = SDK_PREFIX + TEXT("ma_add_mutex_user");
	TSharedPtr<FPubnubAddMessageActionResult> SyncResult = MakeShared<FPubnubAddMessageActionResult>();

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

	const FString MutexPayload = FString::Printf(TEXT("\"ma_mutex%s\""), *ChannelSuffix);
	FPubnubPublishMessageResult Pub = PubnubClient->PublishMessage(Channel, MutexPayload, FPubnubPublishSettings());
	TestFalse(TEXT("Publish should succeed"), Pub.Result.Error);
	const FString MessageTimetoken = Pub.PublishedMessage.Timetoken;

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Channel, MessageTimetoken]()
	{
		PubnubClient->AddMessageActionAsync(Channel, MessageTimetoken, TEXT("reaction"), TEXT("like"), FOnPubnubAddMessageActionResponseNative());
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Channel, MessageTimetoken, SyncResult]()
	{
		*SyncResult = PubnubClient->AddMessageAction(Channel, MessageTimetoken, TEXT("reaction"), TEXT("other"));
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
// GetMessageActions — validation
// ---------------------------------------------------------------------------

bool FPubnubGetMessageActions_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("ma_get_val_user"));

	FPubnubGetMessageActionsResult R = PubnubClient->GetMessageActions(
		FString(),
		PubnubMessageActionsTestsPrivate::NewerActionTimetokenBound,
		PubnubMessageActionsTestsPrivate::BaselineActionTimetoken,
		PubnubMessageActionsTestsPrivate::DefaultMessageActionsLimit);

	TestTrue(TEXT("Should error"), R.Result.Error);
	TestTrue(TEXT("Error should mention Channel"), R.Result.ErrorMessage.Contains(TEXT("Channel")));

	CleanUp();
	return true;
}

bool FPubnubGetMessageActions_ClientNotInitialized_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();

	FPubnubGetMessageActionsResult R = Client->GetMessageActions(
		SDK_PREFIX + TEXT("ma_get_noinit_ch"),
		PubnubMessageActionsTestsPrivate::NewerActionTimetokenBound,
		PubnubMessageActionsTestsPrivate::BaselineActionTimetoken,
		PubnubMessageActionsTestsPrivate::DefaultMessageActionsLimit);

	TestTrue(TEXT("Should error"), R.Result.Error);
	TestTrue(TEXT("Should mention not initialized"),
		R.Result.ErrorMessage.Contains(TEXT("not initialized")) || R.Result.ErrorMessage.Contains(TEXT("invalid")));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// GetMessageActions — behaviour
// ---------------------------------------------------------------------------

bool FPubnubGetMessageActions_HappyPath_RequiredStartEnd::RunTest(const FString& Parameters)
{
	const FString ChannelSuffix = PubnubMessageActionsTestsPrivate::UniqueChannelSuffix();
	const FString Channel = SDK_PREFIX + TEXT("ma_get_happy_ch") + ChannelSuffix;
	const FString User = SDK_PREFIX + TEXT("ma_get_happy_user");

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

	const FString PublishPayload = FString::Printf(TEXT("\"ma_get_happy%s\""), *ChannelSuffix);
	FPubnubPublishMessageResult Pub = PubnubClient->PublishMessage(Channel, PublishPayload, FPubnubPublishSettings());
	TestFalse(TEXT("Publish should succeed"), Pub.Result.Error);

	FPubnubAddMessageActionResult Add = PubnubClient->AddMessageAction(Channel, Pub.PublishedMessage.Timetoken, TEXT("reaction"), TEXT("star"));
	TestFalse(TEXT("AddMessageAction should succeed"), Add.Result.Error);

	FPubnubGetMessageActionsResult Get = PubnubMessageActionsTestsPrivate::GetMessageActionsDefaultWindow(PubnubClient, Channel);
	TestFalse(TEXT("GetMessageActions should succeed"), Get.Result.Error);
	if (Get.Result.Error)
	{
		CleanUp();
		return true;
	}

	const FPubnubMessageActionData* Found = PubnubMessageActionsTestsPrivate::FindActionByActionTimetoken(Get.MessageActions, Add.MessageActionData.ActionTimetoken);
	TestNotNull(TEXT("Listed actions should include the one we added"), Found);
	if (!Found)
	{
		CleanUp();
		return true;
	}
	TestEqual(TEXT("Type round-trip"), Found->Type, TEXT("reaction"));
	TestEqual(TEXT("Value round-trip"), Found->Value, TEXT("star"));
	TestEqual(TEXT("User round-trip"), Found->UserID, User);

	CleanUp();
	return true;
}

bool FPubnubGetMessageActions_FullParameters_StartEndLimit::RunTest(const FString& Parameters)
{
	const FString ChannelSuffix = PubnubMessageActionsTestsPrivate::UniqueChannelSuffix();
	const FString Channel = SDK_PREFIX + TEXT("ma_get_full_ch") + ChannelSuffix;
	const FString User = SDK_PREFIX + TEXT("ma_get_full_user");

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

	const FString PublishPayload = FString::Printf(TEXT("\"ma_get_full%s\""), *ChannelSuffix);
	FPubnubPublishMessageResult Pub = PubnubClient->PublishMessage(Channel, PublishPayload, FPubnubPublishSettings());
	TestFalse(TEXT("Publish should succeed"), Pub.Result.Error);

	FPubnubAddMessageActionResult Add = PubnubClient->AddMessageAction(Channel, Pub.PublishedMessage.Timetoken, TEXT("reaction"), TEXT("fullparam"));
	TestFalse(TEXT("AddMessageAction should succeed"), Add.Result.Error);

	// Server starts at Start (newer) and pages toward the past until End (older). Start > End.
	const FString Start = PubnubMessageActionsTestsPrivate::NewerActionTimetokenBound;
	const FString End = PubnubMessageActionsTestsPrivate::BaselineActionTimetoken;
	const int32 Limit = 25;
	TestTrue(TEXT("Start timetoken should be newer (greater) than End"), Start > End);

	FPubnubGetMessageActionsResult Get = PubnubClient->GetMessageActions(Channel, Start, End, Limit);
	TestFalse(TEXT("GetMessageActions with start/end/limit should succeed"), Get.Result.Error);
	if (Get.Result.Error)
	{
		CleanUp();
		return true;
	}

	const FPubnubMessageActionData* Found = PubnubMessageActionsTestsPrivate::FindActionByActionTimetoken(Get.MessageActions, Add.MessageActionData.ActionTimetoken);
	TestNotNull(TEXT("Should still find action within start/end window"), Found);
	if (!Found)
	{
		CleanUp();
		return true;
	}
	TestEqual(TEXT("Value"), Found->Value, TEXT("fullparam"));

	CleanUp();
	return true;
}

bool FPubnubGetMessageActions_LimitOne_AfterTwoAdds_ReturnsSingleAction::RunTest(const FString& Parameters)
{
	// Idea: Limit parameter must cap how many actions are returned in one call (we add two, request limit 1).
	const FString ChannelSuffix = PubnubMessageActionsTestsPrivate::UniqueChannelSuffix();
	const FString Channel = SDK_PREFIX + TEXT("ma_get_lim_ch") + ChannelSuffix;
	const FString User = SDK_PREFIX + TEXT("ma_get_lim_user");

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

	const FString PublishPayload = FString::Printf(TEXT("\"ma_get_lim%s\""), *ChannelSuffix);
	FPubnubPublishMessageResult Pub = PubnubClient->PublishMessage(Channel, PublishPayload, FPubnubPublishSettings());
	TestFalse(TEXT("Publish should succeed"), Pub.Result.Error);
	const FString MsgTt = Pub.PublishedMessage.Timetoken;

	TestFalse(TEXT("Add a"), PubnubClient->AddMessageAction(Channel, MsgTt, TEXT("reaction"), TEXT("a")).Result.Error);
	FPlatformProcess::Sleep(0.05f);
	TestFalse(TEXT("Add b"), PubnubClient->AddMessageAction(Channel, MsgTt, TEXT("reaction"), TEXT("b")).Result.Error);

	// Same Start > End convention: scan from newer bound down toward older bound, limit page size.
	FPubnubGetMessageActionsResult Limited = PubnubClient->GetMessageActions(
		Channel,
		PubnubMessageActionsTestsPrivate::NewerActionTimetokenBound,
		PubnubMessageActionsTestsPrivate::BaselineActionTimetoken,
		1);
	TestFalse(TEXT("Get with limit should succeed"), Limited.Result.Error);
	TestEqual(TEXT("Limit 1 should return at most one action"), Limited.MessageActions.Num(), 1);

	FPubnubGetMessageActionsResult All = PubnubMessageActionsTestsPrivate::GetMessageActionsDefaultWindow(PubnubClient, Channel);
	TestFalse(TEXT("Full-window get should succeed"), All.Result.Error);
	TestTrue(TEXT("Should list both actions"), All.MessageActions.Num() >= 2);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// RemoveMessageAction — validation
// ---------------------------------------------------------------------------

bool FPubnubRemoveMessageAction_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("ma_rm_val_ch_user"));

	FPubnubOperationResult R = PubnubClient->RemoveMessageAction(FString(), TEXT("1"), TEXT("2"));

	TestTrue(TEXT("Should error"), R.Error);
	TestTrue(TEXT("Error should mention Channel"), R.ErrorMessage.Contains(TEXT("Channel")));

	CleanUp();
	return true;
}

bool FPubnubRemoveMessageAction_EmptyMessageTimetoken_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("ma_rm_val_msg_user"));

	FPubnubOperationResult R = PubnubClient->RemoveMessageAction(SDK_PREFIX + TEXT("ma_rm_val_msg_ch"), FString(), TEXT("2"));

	TestTrue(TEXT("Should error"), R.Error);
	TestTrue(TEXT("Error should mention MessageTimetoken"), R.ErrorMessage.Contains(TEXT("MessageTimetoken")));

	CleanUp();
	return true;
}

bool FPubnubRemoveMessageAction_EmptyActionTimetoken_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("ma_rm_val_act_user"));

	FPubnubOperationResult R = PubnubClient->RemoveMessageAction(SDK_PREFIX + TEXT("ma_rm_val_act_ch"), TEXT("1"), FString());

	TestTrue(TEXT("Should error"), R.Error);
	TestTrue(TEXT("Error should mention ActionTimetoken"), R.ErrorMessage.Contains(TEXT("ActionTimetoken")));

	CleanUp();
	return true;
}

bool FPubnubRemoveMessageAction_ClientNotInitialized_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();

	FPubnubOperationResult R = Client->RemoveMessageAction(SDK_PREFIX + TEXT("ma_rm_noinit_ch"), TEXT("1"), TEXT("2"));

	TestTrue(TEXT("Should error"), R.Error);
	TestTrue(TEXT("Should mention not initialized"),
		R.ErrorMessage.Contains(TEXT("not initialized")) || R.ErrorMessage.Contains(TEXT("invalid")));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// RemoveMessageAction — behaviour
// ---------------------------------------------------------------------------

bool FPubnubRemoveMessageAction_HappyPath_RemovesThenMissingFromGet::RunTest(const FString& Parameters)
{
	const FString ChannelSuffix = PubnubMessageActionsTestsPrivate::UniqueChannelSuffix();
	const FString Channel = SDK_PREFIX + TEXT("ma_rm_happy_ch") + ChannelSuffix;
	const FString User = SDK_PREFIX + TEXT("ma_rm_happy_user");

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

	const FString PublishPayload = FString::Printf(TEXT("\"ma_rm_happy%s\""), *ChannelSuffix);
	FPubnubPublishMessageResult Pub = PubnubClient->PublishMessage(Channel, PublishPayload, FPubnubPublishSettings());
	TestFalse(TEXT("Publish should succeed"), Pub.Result.Error);

	FPubnubAddMessageActionResult Add = PubnubClient->AddMessageAction(Channel, Pub.PublishedMessage.Timetoken, TEXT("reaction"), TEXT("to_remove"));
	TestFalse(TEXT("Add should succeed"), Add.Result.Error);
	const FString ActionTimetoken = Add.MessageActionData.ActionTimetoken;

	FPubnubOperationResult Rem = PubnubClient->RemoveMessageAction(Channel, Pub.PublishedMessage.Timetoken, ActionTimetoken);
	TestFalse(TEXT("Remove should succeed"), Rem.Error);

	FPubnubGetMessageActionsResult Get = PubnubMessageActionsTestsPrivate::GetMessageActionsDefaultWindow(PubnubClient, Channel);
	TestFalse(TEXT("Get after remove should succeed"), Get.Result.Error);
	TestNull(TEXT("Removed action should not appear in listing"),
		PubnubMessageActionsTestsPrivate::FindActionByActionTimetoken(Get.MessageActions, ActionTimetoken));

	CleanUp();
	return true;
}

bool FPubnubRemoveMessageAction_InvalidActionTimetoken_ServerError::RunTest(const FString& Parameters)
{
	// Idea: Bogus action timetoken should yield a server/API error, not a client validation pass-through.
	const FString ChannelSuffix = PubnubMessageActionsTestsPrivate::UniqueChannelSuffix();
	const FString Channel = SDK_PREFIX + TEXT("ma_rm_bad_ch") + ChannelSuffix;
	const FString User = SDK_PREFIX + TEXT("ma_rm_bad_user");

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

	const FString PublishPayload = FString::Printf(TEXT("\"ma_rm_bad%s\""), *ChannelSuffix);
	FPubnubPublishMessageResult Pub = PubnubClient->PublishMessage(Channel, PublishPayload, FPubnubPublishSettings());
	TestFalse(TEXT("Publish should succeed"), Pub.Result.Error);

	FPubnubOperationResult Rem = PubnubClient->RemoveMessageAction(Channel, Pub.PublishedMessage.Timetoken, TEXT("99999999999999999"));
	TestTrue(TEXT("Remove with invalid action timetoken should error"), Rem.Error);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// FetchHistory — message actions
// ---------------------------------------------------------------------------

bool FPubnubFetchHistory_IncludeMessageActions_EmbedsActionsOnMessage::RunTest(const FString& Parameters)
{
	// Idea: With IncludeMessageActions, history rows should parse nested actions (type/value/uuid/actionTimetoken) for the parent message.
	const FString ChannelSuffix = PubnubMessageActionsTestsPrivate::UniqueChannelSuffix();
	const FString Channel = SDK_PREFIX + TEXT("ma_hist_ch") + ChannelSuffix;
	const FString User = SDK_PREFIX + TEXT("ma_hist_user");
	const FString Marker = FString::Printf(TEXT("ma_hist_marker%s"), *ChannelSuffix);
	const FString MessageJson = FString::Printf(TEXT("{\"x\":\"%s\"}"), *Marker);

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
	const FString MessageTimetoken = Pub.PublishedMessage.Timetoken;

	FPubnubAddMessageActionResult Add = PubnubClient->AddMessageAction(Channel, MessageTimetoken, TEXT("reaction"), TEXT("history_like"));
	TestFalse(TEXT("AddMessageAction should succeed"), Add.Result.Error);
	FPlatformProcess::Sleep(0.1f);

	FPubnubFetchHistorySettings Without;
	Without.IncludeMessageActions = false;
	FPubnubFetchHistoryResult HistOff = PubnubClient->FetchHistory(Channel, Without);
	TestFalse(TEXT("FetchHistory without actions should succeed"), HistOff.Result.Error);
	if (HistOff.Result.Error)
	{
		CleanUp();
		return true;
	}
	const FPubnubHistoryMessageData* RowOff = PubnubMessageActionsTestsPrivate::FindHistoryMessageContaining(HistOff.Messages, Marker);
	TestNotNull(TEXT("Should find message"), RowOff);
	if (!RowOff)
	{
		CleanUp();
		return true;
	}
	TestTrue(TEXT("Without flag, message actions should be empty"), RowOff->MessageActions.IsEmpty());

	FPubnubFetchHistorySettings With;
	With.IncludeMessageActions = true;
	With.MaxPerChannel = 25;
	FPubnubFetchHistoryResult HistOn = PubnubClient->FetchHistory(Channel, With);
	TestFalse(TEXT("FetchHistory with actions should succeed"), HistOn.Result.Error);
	if (HistOn.Result.Error)
	{
		CleanUp();
		return true;
	}

	const FPubnubHistoryMessageData* RowOn = PubnubMessageActionsTestsPrivate::FindHistoryMessageContaining(HistOn.Messages, Marker);
	TestNotNull(TEXT("Should find message in history with actions"), RowOn);
	if (!RowOn)
	{
		CleanUp();
		return true;
	}

	const FPubnubMessageActionData* Embedded = PubnubMessageActionsTestsPrivate::FindActionByTypeAndValue(RowOn->MessageActions, TEXT("reaction"), TEXT("history_like"));
	TestNotNull(TEXT("Embedded action should be present"), Embedded);
	if (!Embedded)
	{
		CleanUp();
		return true;
	}
	TestEqual(TEXT("Embedded user"), Embedded->UserID, User);
	TestEqual(TEXT("Embedded message timetoken"), Embedded->MessageTimetoken, MessageTimetoken);
	TestFalse(TEXT("Embedded action timetoken"), Embedded->ActionTimetoken.IsEmpty());
	TestEqual(TEXT("Embedded action timetoken matches add response"), Embedded->ActionTimetoken, Add.MessageActionData.ActionTimetoken);

	CleanUp();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
