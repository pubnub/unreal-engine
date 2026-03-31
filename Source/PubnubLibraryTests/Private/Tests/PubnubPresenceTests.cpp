// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubClient.h"
#include "PubnubStructLibrary.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"
#include "Dom/JsonObject.h"

using namespace PubnubTests;

namespace PubnubPresenceTestsHelper
{
	/** Parse Presence API JSON: root object field "payload" as object (single-channel state / here-now payload). */
	static bool TryGetPayloadObject(const FString& JsonResponse, TSharedPtr<FJsonObject>& OutPayload)
	{
		TSharedPtr<FJsonObject> Root = MakeShareable(new FJsonObject);
		if (!UPubnubJsonUtilities::StringToJsonObject(JsonResponse, Root) || !Root.IsValid())
		{
			return false;
		}
		const TSharedPtr<FJsonObject>* PayloadPtr = nullptr;
		if (!Root->TryGetObjectField(TEXT("payload"), PayloadPtr) || !PayloadPtr || !PayloadPtr->IsValid())
		{
			return false;
		}
		OutPayload = *PayloadPtr;
		return true;
	}
}

// ---------------------------------------------------------------------------
// UPubnubClient::ListUsersFromChannel
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUsersFromChannel_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.ListUsersFromChannel.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUsersFromChannel_LimitNegative_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.ListUsersFromChannel.1Validation.LimitNegative",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUsersFromChannel_OffsetNegative_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.ListUsersFromChannel.1Validation.OffsetNegative",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUsersFromChannel_HappyPath_DefaultSettings_Occupancy, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.ListUsersFromChannel.2HappyPath.DefaultSettings",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUsersFromChannel_FullSettings_UuidsStateLimitOffset, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.ListUsersFromChannel.3FullParameters.UuidsStateLimitOffset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUsersFromChannel_WithChannelGroupOption_SubscribeViaGroup, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.ListUsersFromChannel.3FullParameters.ChannelGroupOption",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUsersFromChannel_ConcurrentSyncWhileAsync_ReturnsMutexError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.ListUsersFromChannel.4Advanced.ConcurrentSyncWhileAsync",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::ListUserSubscribedChannels
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUserSubscribedChannels_EmptyUserID_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.ListUserSubscribedChannels.1Validation.EmptyUserID",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUserSubscribedChannels_HappyPath_SubscribedChannelListed, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.ListUserSubscribedChannels.2HappyPath.SingleSubscription",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUserSubscribedChannels_TwoChannels_BothListed, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.ListUserSubscribedChannels.4Advanced.TwoSubscriptions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::SetState
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetState_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.SetState.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetState_EmptyStateJson_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.SetState.1Validation.EmptyStateJson",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetState_InvalidStateJson_NotJsonObject_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.SetState.1Validation.InvalidStateJson",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetState_HappyPath_RequiredParamsOnly, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.SetState.2HappyPath.RequiredParamsOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetState_FullSettings_HeartBeatAndExplicitUserID, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.SetState.3FullParameters.HeartBeatAndUserID",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetState_ConcurrentSyncWhileAsync_ReturnsMutexError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.SetState.4Advanced.ConcurrentSyncWhileAsync",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::GetState
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetState_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.GetState.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetState_HappyPath_PayloadMatchesSetState, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.GetState.2HappyPath.PayloadMatchesSetState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetState_AfterUpdatingState_PayloadReflectsNewValue, FPubnubAutomationTestBase,
	"Pubnub.Integration.Presence.GetState.4Advanced.UpdateStateThenGet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// ListUsersFromChannel — validation
// ---------------------------------------------------------------------------

bool FPubnubListUsersFromChannel_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + "presence_luc_val_user");

	FPubnubListUsersFromChannelResult Result = PubnubClient->ListUsersFromChannel(FString(), FPubnubListUsersFromChannelSettings());

	TestTrue("Result should indicate error", Result.Result.Error);
	TestTrue("ErrorMessage should mention Channel", Result.Result.ErrorMessage.Contains(TEXT("Channel")));
	TestTrue("ErrorMessage should indicate field is empty", Result.Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubListUsersFromChannel_LimitNegative_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + "presence_luc_lim_user");

	FPubnubListUsersFromChannelSettings Settings;
	Settings.Limit = -1;

	FPubnubListUsersFromChannelResult Result = PubnubClient->ListUsersFromChannel(SDK_PREFIX + "presence_luc_lim_ch", Settings);

	TestTrue("Result should indicate error", Result.Result.Error);
	TestEqual("ErrorMessage should match validation text", Result.Result.ErrorMessage, TEXT("Limit can't be below 0."));

	CleanUp();
	return true;
}

bool FPubnubListUsersFromChannel_OffsetNegative_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + "presence_luc_off_user");

	FPubnubListUsersFromChannelSettings Settings;
	Settings.Offset = -3;

	FPubnubListUsersFromChannelResult Result = PubnubClient->ListUsersFromChannel(SDK_PREFIX + "presence_luc_off_ch", Settings);

	TestTrue("Result should indicate error", Result.Result.Error);
	TestEqual("ErrorMessage should match validation text", Result.Result.ErrorMessage, TEXT("Offset can't be below 0."));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// ListUsersFromChannel — behaviour
// ---------------------------------------------------------------------------

bool FPubnubListUsersFromChannel_HappyPath_DefaultSettings_Occupancy::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "presence_luc_happy_ch";
	const FString TestUser = SDK_PREFIX + "presence_luc_happy_user";

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

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel]()
	{
		FPubnubListUsersFromChannelResult HereResult = PubnubClient->ListUsersFromChannel(TestChannel, FPubnubListUsersFromChannelSettings());
		TestFalse("ListUsersFromChannel should succeed", HereResult.Result.Error);
		TestEqual("ListUsersFromChannel status", HereResult.Result.Status, 200);
		TestTrue("Occupancy should include current subscriber", HereResult.Data.Occupancy >= 1);
	}, 0.45f));

	CleanUp();
	return true;
}

// Exercises every FPubnubListUsersFromChannelSettings field: UUIDs + per-UUID state, limit/offset, and optional channel group reference.
bool FPubnubListUsersFromChannel_FullSettings_UuidsStateLimitOffset::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "presence_luc_full_ch";
	const FString TestUser = SDK_PREFIX + "presence_luc_full_user";
	const FString StateJson = TEXT("{\"hereNowStateKey\":\"hereNowStateVal\"}");

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

	FPubnubOperationResult SetStateResult = PubnubClient->SetState(TestChannel, StateJson);
	TestFalse("SetState should succeed", SetStateResult.Error);
	TestEqual("SetState status", SetStateResult.Status, 200);

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestUser]()
	{
		FPubnubListUsersFromChannelSettings Settings;
		Settings.ChannelGroup = FString();
		Settings.DisableUserID = false;
		Settings.State = true;
		Settings.Limit = 88;
		Settings.Offset = 0;

		FPubnubListUsersFromChannelResult HereResult = PubnubClient->ListUsersFromChannel(TestChannel, Settings);
		TestFalse("ListUsersFromChannel should succeed", HereResult.Result.Error);
		TestEqual("ListUsersFromChannel status", HereResult.Result.Status, 200);
		TestTrue("Occupancy should be at least 1", HereResult.Data.Occupancy >= 1);
		TestTrue("UsersState should contain our uuid", HereResult.Data.UsersState.Contains(TestUser));
		const FString* StoredState = HereResult.Data.UsersState.Find(TestUser);
		TestNotNull("UsersState entry should exist", StoredState);
		TestTrue("Per-uuid state JSON should include key from SetState", StoredState && StoredState->Contains(TEXT("hereNowStateKey")));
		TestTrue("Per-uuid state JSON should include value from SetState", StoredState && StoredState->Contains(TEXT("hereNowStateVal")));
	}, 0.55f));

	CleanUp();
	return true;
}

// ChannelGroup in ListUsersFromChannelSettings maps to Here Now "channel group" query parameter alongside the channel name.
// PubNub treats that as a multi-target request: the HTTP response uses payload.total_occupancy / payload.channels, not root occupancy.
// Subscribing with SubscribeToGroup is valid — the client is present on the group's member channels; direct SubscribeToChannel is not required.
bool FPubnubListUsersFromChannel_WithChannelGroupOption_SubscribeViaGroup::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "presence_luc_grp_ch";
	const FString TestGroup = SDK_PREFIX + "presence_luc_grp";
	const FString TestUser = SDK_PREFIX + "presence_luc_grp_user";

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

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestGroup]()
	{
		FPubnubListUsersFromChannelSettings Settings;
		Settings.ChannelGroup = TestGroup;
		Settings.DisableUserID = true;
		Settings.State = false;
		Settings.Limit = 1000;
		Settings.Offset = 0;

		FPubnubListUsersFromChannelResult HereResult = PubnubClient->ListUsersFromChannel(TestChannel, Settings);
		TestFalse("ListUsersFromChannel with channel group option should succeed", HereResult.Result.Error);
		TestEqual("ListUsersFromChannel status", HereResult.Result.Status, 200);
		TestTrue("Occupancy should reflect subscriber via group", HereResult.Data.Occupancy >= 1);
	}, 0.55f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup]()
	{
		FPubnubOperationResult Unsub = PubnubClient->UnsubscribeFromGroup(TestGroup);
		TestFalse("UnsubscribeFromGroup cleanup should succeed", Unsub.Error);
		FPubnubOperationResult RemoveGrp = PubnubClient->RemoveChannelGroup(TestGroup);
		TestFalse("RemoveChannelGroup cleanup should succeed", RemoveGrp.Error);
	}, 0.2f));

	CleanUp();
	return true;
}

bool FPubnubListUsersFromChannel_ConcurrentSyncWhileAsync_ReturnsMutexError::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "presence_luc_mutex_ch";
	const FString TestUser = SDK_PREFIX + "presence_luc_mutex_user";

	TSharedPtr<FPubnubListUsersFromChannelResult> SyncResult = MakeShared<FPubnubListUsersFromChannelResult>();

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

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel]()
	{
		PubnubClient->ListUsersFromChannelAsync(TestChannel, FOnPubnubListUsersFromChannelResponseNative(), FPubnubListUsersFromChannelSettings());
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SyncResult]()
	{
		*SyncResult = PubnubClient->ListUsersFromChannel(TestChannel, FPubnubListUsersFromChannelSettings());
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

// ---------------------------------------------------------------------------
// ListUserSubscribedChannels
// ---------------------------------------------------------------------------

bool FPubnubListUserSubscribedChannels_EmptyUserID_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + "presence_wn_val_user");

	FPubnubListUsersSubscribedChannelsResult Result = PubnubClient->ListUserSubscribedChannels(FString());

	TestTrue("Result should indicate error", Result.Result.Error);
	TestTrue("ErrorMessage should mention UserID", Result.Result.ErrorMessage.Contains(TEXT("UserID")));
	TestTrue("ErrorMessage should indicate field is empty", Result.Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubListUserSubscribedChannels_HappyPath_SubscribedChannelListed::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "presence_wn_happy_ch";
	const FString TestUser = SDK_PREFIX + "presence_wn_happy_user";

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

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestUser]()
	{
		FPubnubListUsersSubscribedChannelsResult WnResult = PubnubClient->ListUserSubscribedChannels(TestUser);
		TestFalse("ListUserSubscribedChannels should succeed", WnResult.Result.Error);
		TestEqual("ListUserSubscribedChannels status", WnResult.Result.Status, 200);
		TestTrue("Channels should include subscribed channel", WnResult.Channels.Contains(TestChannel));
	}, 0.45f));

	CleanUp();
	return true;
}

// Two channel subscriptions; where_now should list both channel names for the uuid.
bool FPubnubListUserSubscribedChannels_TwoChannels_BothListed::RunTest(const FString& Parameters)
{
	const FString ChannelA = SDK_PREFIX + "presence_wn_two_a";
	const FString ChannelB = SDK_PREFIX + "presence_wn_two_b";
	const FString TestUser = SDK_PREFIX + "presence_wn_two_user";

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

	TestFalse("Subscribe A", PubnubClient->SubscribeToChannel(ChannelA).Error);
	TestFalse("Subscribe B", PubnubClient->SubscribeToChannel(ChannelB).Error);

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelA, ChannelB, TestUser]()
	{
		FPubnubListUsersSubscribedChannelsResult WnResult = PubnubClient->ListUserSubscribedChannels(TestUser);
		TestFalse("ListUserSubscribedChannels should succeed", WnResult.Result.Error);
		TestEqual("ListUserSubscribedChannels status", WnResult.Result.Status, 200);
		TestTrue("Should list channel A", WnResult.Channels.Contains(ChannelA));
		TestTrue("Should list channel B", WnResult.Channels.Contains(ChannelB));
	}, 0.5f));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// SetState
// ---------------------------------------------------------------------------

bool FPubnubSetState_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + "presence_ss_val_ch_user");

	FPubnubOperationResult Result = PubnubClient->SetState(FString(), TEXT("{\"a\":1}"));

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention Channel", Result.ErrorMessage.Contains(TEXT("Channel")));
	TestTrue("ErrorMessage should indicate field is empty", Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubSetState_EmptyStateJson_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + "presence_ss_val_json_user");

	FPubnubOperationResult Result = PubnubClient->SetState(SDK_PREFIX + "presence_ss_val_json_ch", FString());

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention StateJson", Result.ErrorMessage.Contains(TEXT("StateJson")));
	TestTrue("ErrorMessage should indicate field is empty", Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubSetState_InvalidStateJson_NotJsonObject_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + "presence_ss_inv_user");

	FPubnubOperationResult Result = PubnubClient->SetState(SDK_PREFIX + "presence_ss_inv_ch", TEXT("this_is_not_valid_json"));

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention Json Object", Result.ErrorMessage.Contains(TEXT("Json Object")));

	CleanUp();
	return true;
}

bool FPubnubSetState_HappyPath_RequiredParamsOnly::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "presence_ss_happy_ch";
	const FString TestUser = SDK_PREFIX + "presence_ss_happy_user";
	const FString StateJson = TEXT("{\"happyKey\":\"happyVal\"}");

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

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, StateJson]()
	{
		FPubnubOperationResult SetResult = PubnubClient->SetState(TestChannel, StateJson);
		TestFalse("SetState should succeed", SetResult.Error);
		TestEqual("SetState status", SetResult.Status, 200);
	}, 0.35f));

	CleanUp();
	return true;
}

bool FPubnubSetState_FullSettings_HeartBeatAndExplicitUserID::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "presence_ss_full_ch";
	const FString TestUser = SDK_PREFIX + "presence_ss_full_user";
	const FString StateJson = TEXT("{\"fullSetStateKey\":42}");

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

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestUser, StateJson]()
	{
		FPubnubSetStateSettings Settings;
		Settings.ChannelGroup = FString();
		Settings.UserID = TestUser;
		Settings.HeartBeat = true;

		FPubnubOperationResult SetResult = PubnubClient->SetState(TestChannel, StateJson, Settings);
		TestFalse("SetState with full settings should succeed", SetResult.Error);
		TestEqual("SetState status", SetResult.Status, 200);
	}, 0.4f));

	CleanUp();
	return true;
}

bool FPubnubSetState_ConcurrentSyncWhileAsync_ReturnsMutexError::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "presence_ss_mutex_ch";
	const FString TestUser = SDK_PREFIX + "presence_ss_mutex_user";
	const FString StateJson = TEXT("{\"mutex\":true}");

	TSharedPtr<FPubnubOperationResult> SyncResult = MakeShared<FPubnubOperationResult>();

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

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, StateJson]()
	{
		PubnubClient->SetStateAsync(TestChannel, StateJson, FOnPubnubSetStateResponseNative(), FPubnubSetStateSettings());
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SyncResult]()
	{
		*SyncResult = PubnubClient->SetState(TestChannel, TEXT("{\"sync\":1}"));
	}, 0.15f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SyncResult]()
	{
		if (SyncResult->Error)
		{
			TestTrue("When sync fails during concurrent op, ErrorMessage should mention operation in progress",
				SyncResult->ErrorMessage.Contains(TEXT("operation is in progress")) ||
				SyncResult->ErrorMessage.Contains(TEXT("concurrently")));
		}
	}, 0.2f));

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// GetState
// ---------------------------------------------------------------------------

bool FPubnubGetState_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + "presence_gs_val_user");

	FPubnubGetStateResult Result = PubnubClient->GetState(FString(), FString(), SDK_PREFIX + "presence_gs_val_target");

	TestTrue("Result should indicate error", Result.Result.Error);
	TestTrue("ErrorMessage should mention Channel", Result.Result.ErrorMessage.Contains(TEXT("Channel")));
	TestTrue("ErrorMessage should indicate field is empty", Result.Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubGetState_HappyPath_PayloadMatchesSetState::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "presence_gs_happy_ch";
	const FString TestUser = SDK_PREFIX + "presence_gs_happy_user";
	const FString StateJson = TEXT("{\"getStateKey\":\"getStateValue\"}");

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

	FPubnubOperationResult SetResult = PubnubClient->SetState(TestChannel, StateJson);
	TestFalse("SetState should succeed", SetResult.Error);

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestUser, StateJson]()
	{
		FPubnubGetStateResult GetResult = PubnubClient->GetState(TestChannel, FString(), TestUser);
		TestFalse("GetState should succeed", GetResult.Result.Error);
		TestEqual("GetState status", GetResult.Result.Status, 200);

		TSharedPtr<FJsonObject> Payload;
		TestTrue("Response should parse with payload object", PubnubPresenceTestsHelper::TryGetPayloadObject(GetResult.StateResponse, Payload));
		TestTrue("Payload should contain key from SetState", Payload.IsValid() && Payload->HasField(TEXT("getStateKey")));
		FString V;
		TestTrue("Payload should expose string value", Payload.IsValid() && Payload->TryGetStringField(TEXT("getStateKey"), V));
		TestEqual("Payload value should match SetState", V, TEXT("getStateValue"));

		TestTrue("Round-trip JSON should match SetState object", UPubnubJsonUtilities::AreJsonObjectStringsEqual(StateJson, UPubnubJsonUtilities::JsonObjectToString(Payload)));
	}, 0.5f));

	CleanUp();
	return true;
}

// Set state twice; second GetState should read the updated field value from the payload.
bool FPubnubGetState_AfterUpdatingState_PayloadReflectsNewValue::RunTest(const FString& Parameters)
{
	const FString TestChannel = SDK_PREFIX + "presence_gs_upd_ch";
	const FString TestUser = SDK_PREFIX + "presence_gs_upd_user";
	const FString State1 = TEXT("{\"version\":\"v1\"}");
	const FString State2 = TEXT("{\"version\":\"v2\"}");

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

	TestFalse("SetState v1", PubnubClient->SetState(TestChannel, State1).Error);

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestUser]()
	{
		FPubnubGetStateResult G1 = PubnubClient->GetState(TestChannel, FString(), TestUser);
		TestFalse("GetState after v1 should succeed", G1.Result.Error);
		TSharedPtr<FJsonObject> P1;
		TestTrue("Parse payload v1", PubnubPresenceTestsHelper::TryGetPayloadObject(G1.StateResponse, P1));
		FString Ver1;
		TestTrue("version field v1", P1.IsValid() && P1->TryGetStringField(TEXT("version"), Ver1));
		TestEqual("version should be v1", Ver1, TEXT("v1"));
	}, 0.45f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, State2]()
	{
		TestFalse("SetState v2", PubnubClient->SetState(TestChannel, State2).Error);
	}, 0.15f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestUser]()
	{
		FPubnubGetStateResult G2 = PubnubClient->GetState(TestChannel, FString(), TestUser);
		TestFalse("GetState after v2 should succeed", G2.Result.Error);
		TSharedPtr<FJsonObject> P2;
		TestTrue("Parse payload v2", PubnubPresenceTestsHelper::TryGetPayloadObject(G2.StateResponse, P2));
		FString Ver2;
		TestTrue("version field v2", P2.IsValid() && P2->TryGetStringField(TEXT("version"), Ver2));
		TestEqual("version should be v2", Ver2, TEXT("v2"));
	}, 0.55f));

	CleanUp();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
