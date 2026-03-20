// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubClient.h"
#include "PubnubStructLibrary.h"
#include "PubnubDefaultLogger.h"
#include "Interfaces/PubnubLoggerInterface.h"
#include "Entities/PubnubChannelEntity.h"
#include "Entities/PubnubSubscription.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

// ---------------------------------------------------------------------------
// SetUserID / GetUserID
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGeneral_SetUserID_GetUserID_HappyPath, FPubnubAutomationTestBase,
	"Pubnub.Integration.General.UserID.2HappyPath.SetThenGetMatches",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGeneral_SetUserID_EmptyString_DoesNotChangeUserId, FPubnubAutomationTestBase,
	"Pubnub.Integration.General.UserID.1Validation.EmptyStringIgnored",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// DisconnectSubscriptions / ReconnectSubscriptions
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGeneral_DisconnectSubscriptions_HappyPath, FPubnubAutomationTestBase,
	"Pubnub.Integration.General.Subscriptions.Disconnect.2HappyPath.NoError",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGeneral_ReconnectSubscriptions_AfterDisconnect_EmptyTimetoken, FPubnubAutomationTestBase,
	"Pubnub.Integration.General.Subscriptions.Reconnect.2HappyPath.AfterDisconnectDefaultCursor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// Publish to capture a timetoken, then reconnect with that cursor after disconnect.
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGeneral_ReconnectSubscriptions_WithTimetokenParameter, FPubnubAutomationTestBase,
	"Pubnub.Integration.General.Subscriptions.Reconnect.3FullParams.WithTimetokenAfterDisconnect",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// Logger: AddLogger / RemoveLogger / ClearLoggers / GetLoggers
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGeneral_Logger_AddRemove_GetLoggersCount, FPubnubAutomationTestBase,
	"Pubnub.Integration.General.Logger.2HappyPath.AddRemoveRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGeneral_Logger_DuplicateAdd_IsIdempotent, FPubnubAutomationTestBase,
	"Pubnub.Integration.General.Logger.4Advanced.DuplicateAddSameInstance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGeneral_Logger_ClearLoggers_EmptiesList, FPubnubAutomationTestBase,
	"Pubnub.Integration.General.Logger.3FullParams.ClearLoggers",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// RunTest implementations
// ---------------------------------------------------------------------------

bool FPubnubGeneral_SetUserID_GetUserID_HappyPath::RunTest(const FString& Parameters)
{
	const FString ExpectedFromConfig = TEXT("UE_SDK_Test_User");
	const FString NewUserId = SDK_PREFIX + TEXT("gen_uid_happy");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});

	TestEqual(TEXT("Initial GetUserID should match FPubnubConfig.UserID from InitTest"), PubnubClient->GetUserID(), ExpectedFromConfig);

	PubnubClient->SetUserID(NewUserId);
	TestEqual(TEXT("GetUserID after SetUserID should return new id"), PubnubClient->GetUserID(), NewUserId);

	CleanUp();
	return true;
}

bool FPubnubGeneral_SetUserID_EmptyString_DoesNotChangeUserId::RunTest(const FString& Parameters)
{
	const FString ExpectedFromConfig = TEXT("UE_SDK_Test_User");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});

	TestEqual(TEXT("Baseline user id from config"), PubnubClient->GetUserID(), ExpectedFromConfig);

	PubnubClient->SetUserID(TEXT(""));
	TestEqual(TEXT("Empty SetUserID should not clear user id (SetUserID_priv aborts on empty)"), PubnubClient->GetUserID(), ExpectedFromConfig);

	CleanUp();
	return true;
}

bool FPubnubGeneral_DisconnectSubscriptions_HappyPath::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("gen_disc_user"));

	const FPubnubOperationResult Disc = PubnubClient->DisconnectSubscriptions();
	TestFalse(TEXT("DisconnectSubscriptions should succeed"), Disc.Error);
	TestEqual(TEXT("DisconnectSubscriptions status"), Disc.Status, 200);

	CleanUp();
	return true;
}

bool FPubnubGeneral_ReconnectSubscriptions_AfterDisconnect_EmptyTimetoken::RunTest(const FString& Parameters)
{
	const FString Ch = SDK_PREFIX + TEXT("gen_reconn_ch");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("gen_reconn_user"));

	UPubnubChannelEntity* const Ent = PubnubClient->CreateChannelEntity(Ch);
	TestNotNull(TEXT("Channel entity"), Ent);
	if (!Ent)
	{
		CleanUp();
		return false;
	}

	UPubnubSubscription* const Sub = Ent->CreateSubscription(FPubnubSubscribeSettings());
	TestNotNull(TEXT("Subscription"), Sub);
	if (!Sub)
	{
		CleanUp();
		return false;
	}

	TestFalse(TEXT("Subscribe should succeed"), Sub->Subscribe().Error);

	const FPubnubOperationResult Disc = PubnubClient->DisconnectSubscriptions();
	TestFalse(TEXT("Disconnect after active subscribe should succeed"), Disc.Error);

	const FPubnubOperationResult Recon = PubnubClient->ReconnectSubscriptions(TEXT(""));
	TestFalse(TEXT("Reconnect with default cursor should succeed"), Recon.Error);
	TestEqual(TEXT("Reconnect status"), Recon.Status, 200);

	(void)Sub->Unsubscribe();

	CleanUp();
	return true;
}

bool FPubnubGeneral_ReconnectSubscriptions_WithTimetokenParameter::RunTest(const FString& Parameters)
{
	const FString Ch = SDK_PREFIX + TEXT("gen_reconn_tt_ch");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("gen_reconn_tt_user"));

	const FPubnubPublishMessageResult Pub = PubnubClient->PublishMessage(Ch, TEXT("{\"m\":\"reconnect_cursor\"}"));
	TestFalse(TEXT("Publish for timetoken should succeed"), Pub.Result.Error);
	const FString Tt = Pub.PublishedMessage.Timetoken;
	TestFalse(TEXT("Published message should include timetoken"), Tt.IsEmpty());

	UPubnubChannelEntity* const Ent = PubnubClient->CreateChannelEntity(Ch);
	UPubnubSubscription* const Sub = Ent ? Ent->CreateSubscription(FPubnubSubscribeSettings()) : nullptr;
	TestNotNull(TEXT("Subscription"), Sub);
	if (!Sub)
	{
		CleanUp();
		return false;
	}

	TestFalse(TEXT("Subscribe should succeed"), Sub->Subscribe().Error);

	TestFalse(TEXT("Disconnect should succeed"), PubnubClient->DisconnectSubscriptions().Error);

	const FPubnubOperationResult Recon = PubnubClient->ReconnectSubscriptions(Tt);
	TestFalse(TEXT("Reconnect with publish timetoken should succeed"), Recon.Error);

	(void)Sub->Unsubscribe();

	CleanUp();
	return true;
}

bool FPubnubGeneral_Logger_AddRemove_GetLoggersCount::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("gen_log_user"));

	const int32 Baseline = PubnubClient->GetLoggers().Num();

	UPubnubDefaultLogger* const Extra = NewObject<UPubnubDefaultLogger>(PubnubClient);
	TestNotNull(TEXT("Extra logger object"), Extra);
	if (!Extra)
	{
		CleanUp();
		return false;
	}

	TScriptInterface<IPubnubLoggerInterface> Iface;
	Iface.SetObject(Extra);
	Iface.SetInterface(Cast<IPubnubLoggerInterface>(Extra));

	PubnubClient->AddLogger(Iface);
	const int32 AfterAdd = PubnubClient->GetLoggers().Num();
	TestEqual(TEXT("GetLoggers count should increase by one after AddLogger"), AfterAdd, Baseline + 1);

	PubnubClient->RemoveLogger(Iface);
	const int32 AfterRemove = PubnubClient->GetLoggers().Num();
	TestEqual(TEXT("GetLoggers count should return to baseline after RemoveLogger"), AfterRemove, Baseline);

	CleanUp();
	return true;
}

bool FPubnubGeneral_Logger_DuplicateAdd_IsIdempotent::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("gen_log_dup_user"));

	const int32 Baseline = PubnubClient->GetLoggers().Num();

	UPubnubDefaultLogger* const Extra = NewObject<UPubnubDefaultLogger>(PubnubClient);
	TScriptInterface<IPubnubLoggerInterface> Iface;
	Iface.SetObject(Extra);
	Iface.SetInterface(Cast<IPubnubLoggerInterface>(Extra));

	PubnubClient->AddLogger(Iface);
	const int32 AfterFirst = PubnubClient->GetLoggers().Num();
	TestEqual(TEXT("After first AddLogger"), AfterFirst, Baseline + 1);

	PubnubClient->AddLogger(Iface);
	const int32 AfterDuplicate = PubnubClient->GetLoggers().Num();
	TestEqual(TEXT("Duplicate AddLogger same UObject should not register twice"), AfterDuplicate, AfterFirst);

	PubnubClient->RemoveLogger(Iface);

	CleanUp();
	return true;
}

bool FPubnubGeneral_Logger_ClearLoggers_EmptiesList::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("gen_log_clr_user"));

	PubnubClient->ClearLoggers();
	TestEqual(TEXT("ClearLoggers should leave GetLoggers empty"), PubnubClient->GetLoggers().Num(), 0);

	UPubnubDefaultLogger* const Replacement = NewObject<UPubnubDefaultLogger>(PubnubClient);
	TScriptInterface<IPubnubLoggerInterface> Iface;
	Iface.SetObject(Replacement);
	Iface.SetInterface(Cast<IPubnubLoggerInterface>(Replacement));
	PubnubClient->AddLogger(Iface);
	TestEqual(TEXT("Re-adding one logger after clear"), PubnubClient->GetLoggers().Num(), 1);

	CleanUp();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
