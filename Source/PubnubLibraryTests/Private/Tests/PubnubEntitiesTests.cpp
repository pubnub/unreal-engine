// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubClient.h"
#include "PubnubStructLibrary.h"
#include "PubnubEnumLibrary.h"
#include "Entities/PubnubChannelEntity.h"
#include "Entities/PubnubChannelGroupEntity.h"
#include "Entities/PubnubChannelMetadataEntity.h"
#include "Entities/PubnubUserMetadataEntity.h"
#include "Entities/PubnubSubscription.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

namespace PubnubEntitiesTestsPrivate
{
	void CleanupChannelGroupScenario(UPubnubClient* Client, const FString& GroupId, const FString& MemberChannelId)
	{
		if (!Client || GroupId.IsEmpty())
		{
			return;
		}
		if (!MemberChannelId.IsEmpty())
		{
			(void)Client->RemoveChannelFromGroup(MemberChannelId, GroupId);
		}
		(void)Client->RemoveChannelGroup(GroupId);
	}

	void CleanupMetadataScenario(UPubnubClient* Client, const FString& ChannelId, const FString& UserId)
	{
		if (!Client)
		{
			return;
		}
		if (!ChannelId.IsEmpty())
		{
			(void)Client->RemoveChannelMetadata(ChannelId);
		}
		if (!UserId.IsEmpty())
		{
			(void)Client->RemoveUserMetadata(UserId);
		}
	}
}

// ---------------------------------------------------------------------------
// UPubnubClient::CreateChannelEntity
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_ChannelEntity_EmptyChannel_ReturnsNull, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateChannelEntity.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_ChannelEntity_HappyPath_EntityIdAndType, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateChannelEntity.2HappyPath.EntityIdAndType",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_ChannelEntity_SubscribeDefaults_ThenUnsubscribe, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateChannelEntity.3Subscription.DefaultSettingsSubscribeUnsubscribe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_ChannelEntity_Subscribe_WithPresenceSettings, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateChannelEntity.3FullParams.SubscriptionReceivePresence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// Second Subscribe on the same UPubnubSubscription without Unsubscribe should be rejected.
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_ChannelEntity_DoubleSubscribe_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateChannelEntity.4Advanced.DoubleSubscribeRejected",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::CreateChannelGroupEntity
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_GroupEntity_EmptyGroup_ReturnsNull, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateChannelGroupEntity.1Validation.EmptyGroup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_GroupEntity_AddListSubscribe_ThenCleanup, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateChannelGroupEntity.2HappyPath.AddListSubscribeCleanup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_GroupEntity_Subscribe_WithPresenceSettings, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateChannelGroupEntity.3FullParams.SubscriptionReceivePresence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::CreateChannelMetadataEntity
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_ChannelMetadataEntity_EmptyId_ReturnsNull, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateChannelMetadataEntity.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_ChannelMetadataEntity_SetGetSubscribeRemove, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateChannelMetadataEntity.2HappyPath.SetGetSubscribeRemove",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_ChannelMetadataEntity_GetWithAllIncludes, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateChannelMetadataEntity.3FullParams.GetWithIncludeCustomStatusType",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::CreateUserMetadataEntity
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_UserMetadataEntity_EmptyId_ReturnsNull, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateUserMetadataEntity.1Validation.EmptyUser",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_UserMetadataEntity_SetGetSubscribeRemove, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateUserMetadataEntity.2HappyPath.SetGetSubscribeRemove",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_UserMetadataEntity_SetWithIncludeAndFields, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateUserMetadataEntity.3FullParams.SetWithIncludeAndOptionalFields",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::CreateSubscriptionSet / CreateSubscriptionSetFromEntities
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_SubscriptionSet_EmptyInputs_SubscribeFails, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateSubscriptionSet.1Validation.EmptyChannelsAndGroupsSubscribeFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_SubscriptionSet_SingleChannel_SubscribeUnsubscribe, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateSubscriptionSet.2HappyPath.SingleChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_SubscriptionSet_MultiChannelGroupPresence, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateSubscriptionSet.3FullParams.MultiChannelGroupPresence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_SubscriptionSetFromEntities_Empty_SubscribeFails, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateSubscriptionSetFromEntities.1Validation.EmptyEntities",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_SubscriptionSetFromEntities_ChannelAndGroup, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.CreateSubscriptionSetFromEntities.2HappyPath.ChannelAndGroupEntities",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// Add a second subscription to an existing set, then subscribe once for the combined set.
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_SubscriptionSet_AddSubscription_ThenSubscribe, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.SubscriptionSet.4Advanced.AddSubscriptionBeforeSubscribe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// Merge two subscription sets via AddSubscriptionSet before subscribing.
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_SubscriptionSet_AddSubscriptionSet_MergeBeforeSubscribe, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.SubscriptionSet.4Advanced.AddSubscriptionSetMerge",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::GetActiveSubscriptions / GetActiveSubscriptionSets
// ---------------------------------------------------------------------------

// C-Core exposes subscription *sets* only once a set is subscribed; creating a UObject set is not enough.
// Flat GetActiveSubscriptions() reflects subscribed UPubnubSubscription instances (entity subscriptions), not
// merely being part of a subscribed set—so this test subscribes a SubscriptionSet and a standalone subscription.
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubEntities_GetActiveLists_AfterSubscribe, FPubnubAutomationTestBase,
	"Pubnub.Integration.Entities.Subscriptions.4Advanced.GetActiveSubscriptionsAndSets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// RunTest implementations
// ---------------------------------------------------------------------------

bool FPubnubEntities_ChannelEntity_EmptyChannel_ReturnsNull::RunTest(const FString& Parameters)
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
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("ent_ch_empty_caller"));

	UPubnubChannelEntity* const Ent = PubnubClient->CreateChannelEntity(TEXT(""));
	TestNull(TEXT("CreateChannelEntity with empty string should return nullptr"), Ent);

	CleanUp();
	return true;
}

bool FPubnubEntities_ChannelEntity_HappyPath_EntityIdAndType::RunTest(const FString& Parameters)
{
	const FString Ch = SDK_PREFIX + TEXT("ent_ch_happy");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("ent_ch_happy_caller"));

	UPubnubChannelEntity* const Ent = PubnubClient->CreateChannelEntity(Ch);
	TestNotNull(TEXT("Channel entity should be created"), Ent);
	if (Ent)
	{
		TestEqual(TEXT("EntityID should match channel name"), Ent->EntityID, Ch);
		TestEqual(TEXT("EntityType should be Channel"), Ent->EntityType, EPubnubEntityType::PEnT_Channel);
	}

	CleanUp();
	return true;
}

bool FPubnubEntities_ChannelEntity_SubscribeDefaults_ThenUnsubscribe::RunTest(const FString& Parameters)
{
	const FString Ch = SDK_PREFIX + TEXT("ent_ch_sub_def");
	const FString Caller = SDK_PREFIX + TEXT("ent_ch_sub_def_caller");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	UPubnubChannelEntity* const Ent = PubnubClient->CreateChannelEntity(Ch);
	TestNotNull(TEXT("Channel entity should exist"), Ent);
	if (!Ent)
	{
		CleanUp();
		return false;
	}

	UPubnubSubscription* const Sub = Ent->CreateSubscription(FPubnubSubscribeSettings());
	TestNotNull(TEXT("CreateSubscription should return subscription"), Sub);

	const FPubnubOperationResult SubRes = Sub->Subscribe();
	TestFalse(TEXT("Subscribe should succeed"), SubRes.Error);
	TestEqual(TEXT("Subscribe HTTP status"), SubRes.Status, 200);

	const FPubnubOperationResult UnsubRes = Sub->Unsubscribe();
	TestFalse(TEXT("Unsubscribe should succeed"), UnsubRes.Error);

	CleanUp();
	return true;
}

bool FPubnubEntities_ChannelEntity_Subscribe_WithPresenceSettings::RunTest(const FString& Parameters)
{
	const FString Ch = SDK_PREFIX + TEXT("ent_ch_sub_pres");
	const FString Caller = SDK_PREFIX + TEXT("ent_ch_sub_pres_caller");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	UPubnubChannelEntity* const Ent = PubnubClient->CreateChannelEntity(Ch);
	TestNotNull(TEXT("Channel entity should exist"), Ent);
	if (!Ent)
	{
		CleanUp();
		return false;
	}

	FPubnubSubscribeSettings SubOpts;
	SubOpts.ReceivePresenceEvents = true;
	UPubnubSubscription* const Sub = Ent->CreateSubscription(SubOpts);
	TestNotNull(TEXT("CreateSubscription with presence option should return subscription"), Sub);
	if (!Sub)
	{
		CleanUp();
		return false;
	}

	const FPubnubOperationResult SubRes = Sub->Subscribe();
	TestFalse(TEXT("Subscribe with presence-enabled subscription should succeed"), SubRes.Error);

	(void)Sub->Unsubscribe();

	CleanUp();
	return true;
}

bool FPubnubEntities_ChannelEntity_DoubleSubscribe_ReturnsError::RunTest(const FString& Parameters)
{
	const FString Ch = SDK_PREFIX + TEXT("ent_ch_dblsub");
	const FString Caller = SDK_PREFIX + TEXT("ent_ch_dblsub_caller");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	UPubnubChannelEntity* const Ent = PubnubClient->CreateChannelEntity(Ch);
	UPubnubSubscription* const Sub = Ent ? Ent->CreateSubscription(FPubnubSubscribeSettings()) : nullptr;
	TestNotNull(TEXT("Subscription should exist"), Sub);
	if (!Sub)
	{
		CleanUp();
		return false;
	}

	const FPubnubOperationResult First = Sub->Subscribe();
	TestFalse(TEXT("First Subscribe should succeed"), First.Error);

	const FPubnubOperationResult Second = Sub->Subscribe();
	TestTrue(TEXT("Second Subscribe should fail"), Second.Error);
	TestTrue(TEXT("Error should mention already subscribed"), Second.ErrorMessage.Contains(TEXT("already subscribed")));

	(void)Sub->Unsubscribe();

	CleanUp();
	return true;
}

bool FPubnubEntities_GroupEntity_EmptyGroup_ReturnsNull::RunTest(const FString& Parameters)
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
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("ent_grp_empty_caller"));

	UPubnubChannelGroupEntity* const Ent = PubnubClient->CreateChannelGroupEntity(TEXT(""));
	TestNull(TEXT("CreateChannelGroupEntity with empty string should return nullptr"), Ent);

	CleanUp();
	return true;
}

bool FPubnubEntities_GroupEntity_AddListSubscribe_ThenCleanup::RunTest(const FString& Parameters)
{
	const FString GroupId = SDK_PREFIX + TEXT("ent_grp_add_grp");
	const FString MemberCh = SDK_PREFIX + TEXT("ent_grp_add_ch");
	const FString Caller = SDK_PREFIX + TEXT("ent_grp_add_caller");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	UPubnubChannelGroupEntity* const Grp = PubnubClient->CreateChannelGroupEntity(GroupId);
	TestNotNull(TEXT("Group entity should exist"), Grp);
	if (!Grp)
	{
		CleanUp();
		return false;
	}

	TestEqual(TEXT("Group entity type"), Grp->EntityType, EPubnubEntityType::PEnT_ChannelGroup);
	TestEqual(TEXT("Group EntityID"), Grp->EntityID, GroupId);

	const FPubnubOperationResult AddRes = Grp->AddChannelToGroup(MemberCh);
	TestFalse(TEXT("AddChannelToGroup should succeed"), AddRes.Error);

	const FPubnubListChannelsFromGroupResult ListRes = Grp->ListChannelsFromGroup();
	TestFalse(TEXT("ListChannelsFromGroup should succeed"), ListRes.Result.Error);
	TestTrue(TEXT("Listed channels should contain member"), ListRes.Channels.Contains(MemberCh));

	UPubnubSubscription* const Sub = Grp->CreateSubscription(FPubnubSubscribeSettings());
	TestNotNull(TEXT("Group subscription should be created"), Sub);
	if (Sub)
	{
		const FPubnubOperationResult Sr = Sub->Subscribe();
		TestFalse(TEXT("Subscribe to channel group should succeed"), Sr.Error);
		(void)Sub->Unsubscribe();
	}

	PubnubEntitiesTestsPrivate::CleanupChannelGroupScenario(PubnubClient, GroupId, MemberCh);

	CleanUp();
	return true;
}

bool FPubnubEntities_GroupEntity_Subscribe_WithPresenceSettings::RunTest(const FString& Parameters)
{
	const FString GroupId = SDK_PREFIX + TEXT("ent_grp_pres_grp");
	const FString MemberCh = SDK_PREFIX + TEXT("ent_grp_pres_ch");
	const FString Caller = SDK_PREFIX + TEXT("ent_grp_pres_caller");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	UPubnubChannelGroupEntity* const Grp = PubnubClient->CreateChannelGroupEntity(GroupId);
	TestNotNull(TEXT("Group entity should exist"), Grp);
	if (!Grp)
	{
		CleanUp();
		return false;
	}

	TestFalse(TEXT("AddChannelToGroup setup should succeed"), Grp->AddChannelToGroup(MemberCh).Error);

	FPubnubSubscribeSettings Opts;
	Opts.ReceivePresenceEvents = true;
	UPubnubSubscription* const Sub = Grp->CreateSubscription(Opts);
	TestNotNull(TEXT("Subscription should exist"), Sub);
	if (Sub)
	{
		TestFalse(TEXT("Subscribe should succeed"), Sub->Subscribe().Error);
		(void)Sub->Unsubscribe();
	}

	PubnubEntitiesTestsPrivate::CleanupChannelGroupScenario(PubnubClient, GroupId, MemberCh);

	CleanUp();
	return true;
}

bool FPubnubEntities_ChannelMetadataEntity_EmptyId_ReturnsNull::RunTest(const FString& Parameters)
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
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("ent_chmeta_empty_caller"));

	UPubnubChannelMetadataEntity* const Ent = PubnubClient->CreateChannelMetadataEntity(TEXT(""));
	TestNull(TEXT("CreateChannelMetadataEntity with empty id should return nullptr"), Ent);

	CleanUp();
	return true;
}

bool FPubnubEntities_ChannelMetadataEntity_SetGetSubscribeRemove::RunTest(const FString& Parameters)
{
	const FString Ch = SDK_PREFIX + TEXT("ent_chmeta_sg");
	const FString Caller = SDK_PREFIX + TEXT("ent_chmeta_sg_caller");
	const FString ChName = TEXT("EntChMetaSetGet");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	UPubnubChannelMetadataEntity* const Ent = PubnubClient->CreateChannelMetadataEntity(Ch);
	TestNotNull(TEXT("Channel metadata entity should exist"), Ent);
	if (!Ent)
	{
		CleanUp();
		return false;
	}

	TestEqual(TEXT("Channel metadata entity type"), Ent->EntityType, EPubnubEntityType::PEnT_ChannelMetadata);

	FPubnubChannelInputData In;
	In.ChannelName = ChName;
	const FPubnubChannelMetadataResult SetR = Ent->SetChannelMetadata(In);
	TestFalse(TEXT("SetChannelMetadata should succeed"), SetR.Result.Error);

	const FPubnubChannelMetadataResult GetR = Ent->GetChannelMetadata();
	TestFalse(TEXT("GetChannelMetadata should succeed"), GetR.Result.Error);
	TestEqual(TEXT("Round-trip channel name"), GetR.ChannelData.ChannelName, ChName);

	UPubnubSubscription* const Sub = Ent->CreateSubscription(FPubnubSubscribeSettings());
	TestNotNull(TEXT("Objects subscription should be created"), Sub);
	if (Sub)
	{
		TestFalse(TEXT("Subscribe on channel metadata entity should succeed"), Sub->Subscribe().Error);
		(void)Sub->Unsubscribe();
	}

	const FPubnubOperationResult Rem = Ent->RemoveChannelMetadata();
	TestFalse(TEXT("RemoveChannelMetadata should succeed"), Rem.Error);

	CleanUp();
	return true;
}

bool FPubnubEntities_ChannelMetadataEntity_GetWithAllIncludes::RunTest(const FString& Parameters)
{
	const FString Ch = SDK_PREFIX + TEXT("ent_chmeta_inc");
	const FString Caller = SDK_PREFIX + TEXT("ent_chmeta_inc_caller");
	const FString StatusVal = TEXT("chmetaStatInc");
	const FString TypeVal = TEXT("chmetaTypeInc");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	UPubnubChannelMetadataEntity* const Ent = PubnubClient->CreateChannelMetadataEntity(Ch);
	TestNotNull(TEXT("Entity should exist"), Ent);
	if (!Ent)
	{
		CleanUp();
		return false;
	}

	FPubnubChannelInputData In;
	In.ChannelName = TEXT("IncChName");
	In.Status = StatusVal;
	In.Type = TypeVal;
	In.Custom = TEXT("{\"tier\":\"gold\"}");
	FPubnubGetMetadataInclude SetInc = FPubnubGetMetadataInclude::FromValue(true);
	const FPubnubChannelMetadataResult SetR = Ent->SetChannelMetadata(In, SetInc);
	TestFalse(TEXT("Set with include should succeed"), SetR.Result.Error);

	FPubnubGetMetadataInclude GetInc = FPubnubGetMetadataInclude::FromValue(true);
	const FPubnubChannelMetadataResult GetR = Ent->GetChannelMetadata(GetInc);
	TestFalse(TEXT("Get with includes should succeed"), GetR.Result.Error);
	TestEqual(TEXT("Status round-trip"), GetR.ChannelData.Status, StatusVal);
	TestEqual(TEXT("Type round-trip"), GetR.ChannelData.Type, TypeVal);
	TestTrue(TEXT("Custom should contain tier"), GetR.ChannelData.Custom.Contains(TEXT("gold")));

	(void)Ent->RemoveChannelMetadata();

	CleanUp();
	return true;
}

bool FPubnubEntities_UserMetadataEntity_EmptyId_ReturnsNull::RunTest(const FString& Parameters)
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
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("ent_umeta_empty_caller"));

	UPubnubUserMetadataEntity* const Ent = PubnubClient->CreateUserMetadataEntity(TEXT(""));
	TestNull(TEXT("CreateUserMetadataEntity with empty id should return nullptr"), Ent);

	CleanUp();
	return true;
}

bool FPubnubEntities_UserMetadataEntity_SetGetSubscribeRemove::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("ent_umeta_sg");
	const FString Caller = SDK_PREFIX + TEXT("ent_umeta_sg_caller");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	UPubnubUserMetadataEntity* const Ent = PubnubClient->CreateUserMetadataEntity(UserId);
	TestNotNull(TEXT("User metadata entity should exist"), Ent);
	if (!Ent)
	{
		CleanUp();
		return false;
	}

	TestEqual(TEXT("User metadata entity type"), Ent->EntityType, EPubnubEntityType::PEnT_UserMetadata);

	FPubnubUserInputData In;
	In.UserName = TEXT("EntUserMeta");
	const FPubnubUserMetadataResult SetR = Ent->SetUserMetadata(In);
	TestFalse(TEXT("SetUserMetadata should succeed"), SetR.Result.Error);

	const FPubnubUserMetadataResult GetR = Ent->GetUserMetadata();
	TestFalse(TEXT("GetUserMetadata should succeed"), GetR.Result.Error);
	TestEqual(TEXT("UserName round-trip"), GetR.UserData.UserName, In.UserName);

	UPubnubSubscription* const Sub = Ent->CreateSubscription(FPubnubSubscribeSettings());
	TestNotNull(TEXT("User metadata subscription should be created"), Sub);
	if (Sub)
	{
		TestFalse(TEXT("Subscribe should succeed"), Sub->Subscribe().Error);
		(void)Sub->Unsubscribe();
	}

	const FPubnubOperationResult Rem = Ent->RemoveUserMetadata();
	TestFalse(TEXT("RemoveUserMetadata should succeed"), Rem.Error);

	CleanUp();
	return true;
}

bool FPubnubEntities_UserMetadataEntity_SetWithIncludeAndFields::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("ent_umeta_full");
	const FString Caller = SDK_PREFIX + TEXT("ent_umeta_full_caller");
	const FString St = TEXT("umetaStatFull");
	const FString Ty = TEXT("umetaTypeFull");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	UPubnubUserMetadataEntity* const Ent = PubnubClient->CreateUserMetadataEntity(UserId);
	TestNotNull(TEXT("Entity should exist"), Ent);
	if (!Ent)
	{
		CleanUp();
		return false;
	}

	FPubnubUserInputData In;
	In.UserName = TEXT("FullUser");
	In.Status = St;
	In.Type = Ty;
	In.Custom = TEXT("{\"role\":\"tester\"}");
	FPubnubGetMetadataInclude Inc = FPubnubGetMetadataInclude::FromValue(true);
	const FPubnubUserMetadataResult SetR = Ent->SetUserMetadata(In, Inc);
	TestFalse(TEXT("SetUserMetadata with include should succeed"), SetR.Result.Error);

	const FPubnubUserMetadataResult GetR = Ent->GetUserMetadata(Inc);
	TestFalse(TEXT("GetUserMetadata with include should succeed"), GetR.Result.Error);
	TestEqual(TEXT("Status round-trip"), GetR.UserData.Status, St);
	TestEqual(TEXT("Type round-trip"), GetR.UserData.Type, Ty);
	TestTrue(TEXT("Custom should contain role"), GetR.UserData.Custom.Contains(TEXT("tester")));

	(void)Ent->RemoveUserMetadata();

	CleanUp();
	return true;
}

bool FPubnubEntities_SubscriptionSet_EmptyInputs_SubscribeFails::RunTest(const FString& Parameters)
{
	const FString Caller = SDK_PREFIX + TEXT("ent_sset_empty_caller");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	UPubnubSubscriptionSet* const Set = PubnubClient->CreateSubscriptionSet({}, {}, FPubnubSubscribeSettings());
	TestNotNull(TEXT("CreateSubscriptionSet still returns UObject (caller must avoid subscribing if misconfigured)"), Set);
	if (Set)
	{
		const FPubnubOperationResult Sr = Set->Subscribe();
		TestTrue(TEXT("Subscribe on empty set should fail"), Sr.Error);
	}

	CleanUp();
	return true;
}

bool FPubnubEntities_SubscriptionSet_SingleChannel_SubscribeUnsubscribe::RunTest(const FString& Parameters)
{
	const FString Ch = SDK_PREFIX + TEXT("ent_sset_one");
	const FString Caller = SDK_PREFIX + TEXT("ent_sset_one_caller");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	TArray<FString> Chans;
	Chans.Add(Ch);
	UPubnubSubscriptionSet* const Set = PubnubClient->CreateSubscriptionSet(Chans, {}, FPubnubSubscribeSettings());
	TestNotNull(TEXT("Subscription set should exist"), Set);
	if (!Set)
	{
		CleanUp();
		return false;
	}

	const FPubnubOperationResult Sr = Set->Subscribe();
	TestFalse(TEXT("Subscribe should succeed"), Sr.Error);
	const FPubnubOperationResult Ur = Set->Unsubscribe();
	TestFalse(TEXT("Unsubscribe should succeed"), Ur.Error);

	CleanUp();
	return true;
}

bool FPubnubEntities_SubscriptionSet_MultiChannelGroupPresence::RunTest(const FString& Parameters)
{
	const FString ChA = SDK_PREFIX + TEXT("ent_sset_m_cha");
	const FString ChB = SDK_PREFIX + TEXT("ent_sset_m_chb");
	const FString Grp = SDK_PREFIX + TEXT("ent_sset_m_grp");
	const FString Member = SDK_PREFIX + TEXT("ent_sset_m_member");
	const FString Caller = SDK_PREFIX + TEXT("ent_sset_m_caller");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	TestFalse(TEXT("AddChannelToGroup setup"), PubnubClient->AddChannelToGroup(Member, Grp).Error);

	TArray<FString> Chans;
	Chans.Add(ChA);
	Chans.Add(ChB);
	TArray<FString> Grps;
	Grps.Add(Grp);

	FPubnubSubscribeSettings Opts;
	Opts.ReceivePresenceEvents = true;
	UPubnubSubscriptionSet* const Set = PubnubClient->CreateSubscriptionSet(Chans, Grps, Opts);
	TestNotNull(TEXT("Subscription set should exist"), Set);
	if (Set)
	{
		TestFalse(TEXT("Subscribe multi channel+group should succeed"), Set->Subscribe().Error);
		(void)Set->Unsubscribe();
	}

	PubnubEntitiesTestsPrivate::CleanupChannelGroupScenario(PubnubClient, Grp, Member);

	CleanUp();
	return true;
}

bool FPubnubEntities_SubscriptionSetFromEntities_Empty_SubscribeFails::RunTest(const FString& Parameters)
{
	const FString Caller = SDK_PREFIX + TEXT("ent_ssfe_empty_caller");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	UPubnubSubscriptionSet* const Set = PubnubClient->CreateSubscriptionSetFromEntities({}, FPubnubSubscribeSettings());
	TestNotNull(TEXT("Factory still returns UObject"), Set);
	if (Set)
	{
		TestTrue(TEXT("Subscribe with no entities backing should fail"), Set->Subscribe().Error);
	}

	CleanUp();
	return true;
}

bool FPubnubEntities_SubscriptionSetFromEntities_ChannelAndGroup::RunTest(const FString& Parameters)
{
	const FString Ch = SDK_PREFIX + TEXT("ent_ssfe_ch");
	const FString Grp = SDK_PREFIX + TEXT("ent_ssfe_grp");
	const FString Member = SDK_PREFIX + TEXT("ent_ssfe_member");
	const FString Caller = SDK_PREFIX + TEXT("ent_ssfe_caller");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	TestFalse(TEXT("AddChannelToGroup setup"), PubnubClient->AddChannelToGroup(Member, Grp).Error);

	UPubnubChannelEntity* const ChEnt = PubnubClient->CreateChannelEntity(Ch);
	UPubnubChannelGroupEntity* const GrpEnt = PubnubClient->CreateChannelGroupEntity(Grp);
	TestNotNull(TEXT("Channel entity"), ChEnt);
	TestNotNull(TEXT("Group entity"), GrpEnt);
	if (!ChEnt || !GrpEnt)
	{
		PubnubEntitiesTestsPrivate::CleanupChannelGroupScenario(PubnubClient, Grp, Member);
		CleanUp();
		return false;
	}

	TArray<UPubnubBaseEntity*> Ents;
	Ents.Add(ChEnt);
	Ents.Add(GrpEnt);

	UPubnubSubscriptionSet* const Set = PubnubClient->CreateSubscriptionSetFromEntities(Ents, FPubnubSubscribeSettings());
	TestNotNull(TEXT("Subscription set from entities should exist"), Set);
	if (Set)
	{
		TestFalse(TEXT("Subscribe should succeed"), Set->Subscribe().Error);
		(void)Set->Unsubscribe();
	}

	PubnubEntitiesTestsPrivate::CleanupChannelGroupScenario(PubnubClient, Grp, Member);

	CleanUp();
	return true;
}

bool FPubnubEntities_SubscriptionSet_AddSubscription_ThenSubscribe::RunTest(const FString& Parameters)
{
	const FString ChA = SDK_PREFIX + TEXT("ent_ss_add_a");
	const FString ChB = SDK_PREFIX + TEXT("ent_ss_add_b");
	const FString Caller = SDK_PREFIX + TEXT("ent_ss_add_caller");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	TArray<FString> One;
	One.Add(ChA);
	UPubnubSubscriptionSet* const Set = PubnubClient->CreateSubscriptionSet(One, {}, FPubnubSubscribeSettings());
	UPubnubChannelEntity* const EntB = PubnubClient->CreateChannelEntity(ChB);
	TestNotNull(TEXT("Set should exist"), Set);
	TestNotNull(TEXT("Second channel entity"), EntB);
	if (!Set || !EntB)
	{
		CleanUp();
		return false;
	}

	UPubnubSubscription* const SubB = EntB->CreateSubscription(FPubnubSubscribeSettings());
	TestNotNull(TEXT("Second subscription"), SubB);
	if (!SubB)
	{
		CleanUp();
		return false;
	}

	Set->AddSubscription(SubB);
	TestEqual(TEXT("Tracked subscriptions after AddSubscription"), Set->GetSubscriptions().Num(), 1);

	TestFalse(TEXT("Subscribe combined set should succeed"), Set->Subscribe().Error);
	(void)Set->Unsubscribe();

	CleanUp();
	return true;
}

bool FPubnubEntities_SubscriptionSet_AddSubscriptionSet_MergeBeforeSubscribe::RunTest(const FString& Parameters)
{
	const FString ChA = SDK_PREFIX + TEXT("ent_ss_union_a");
	const FString ChB = SDK_PREFIX + TEXT("ent_ss_union_b");
	const FString Caller = SDK_PREFIX + TEXT("ent_ss_union_caller");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	TArray<FString> A;
	A.Add(ChA);
	TArray<FString> B;
	B.Add(ChB);
	UPubnubSubscriptionSet* const SetA = PubnubClient->CreateSubscriptionSet(A, {}, FPubnubSubscribeSettings());
	UPubnubSubscriptionSet* const SetB = PubnubClient->CreateSubscriptionSet(B, {}, FPubnubSubscribeSettings());
	TestNotNull(TEXT("Set A"), SetA);
	TestNotNull(TEXT("Set B"), SetB);
	if (!SetA || !SetB)
	{
		CleanUp();
		return false;
	}

	SetA->AddSubscriptionSet(SetB);
	TestFalse(TEXT("Subscribe merged sets should succeed"), SetA->Subscribe().Error);
	(void)SetA->Unsubscribe();

	CleanUp();
	return true;
}

bool FPubnubEntities_GetActiveLists_AfterSubscribe::RunTest(const FString& Parameters)
{
	const FString ChInSet = SDK_PREFIX + TEXT("ent_act_list");
	const FString ChStandalone = SDK_PREFIX + TEXT("ent_act_sub");
	const FString Grp = SDK_PREFIX + TEXT("ent_act_grp");
	const FString Member = SDK_PREFIX + TEXT("ent_act_member");
	const FString Caller = SDK_PREFIX + TEXT("ent_act_caller");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(Caller);

	TestFalse(TEXT("AddChannelToGroup for active-list test"), PubnubClient->AddChannelToGroup(Member, Grp).Error);

	TestEqual(TEXT("Baseline: no active subscription sets before any Subscribe"), PubnubClient->GetActiveSubscriptionSets().Num(), 0);
	TestEqual(TEXT("Baseline: no active subscriptions before any Subscribe"), PubnubClient->GetActiveSubscriptions().Num(), 0);

	TArray<FString> Chans;
	Chans.Add(ChInSet);
	TArray<FString> Grps;
	Grps.Add(Grp);
	UPubnubSubscriptionSet* const Set = PubnubClient->CreateSubscriptionSet(Chans, Grps, FPubnubSubscribeSettings());
	TestNotNull(TEXT("Set should exist"), Set);
	if (!Set)
	{
		PubnubEntitiesTestsPrivate::CleanupChannelGroupScenario(PubnubClient, Grp, Member);
		CleanUp();
		return false;
	}

	TestEqual(TEXT("Creating a SubscriptionSet UObject does not activate C-Core set until Subscribe"), PubnubClient->GetActiveSubscriptionSets().Num(), 0);

	const FPubnubOperationResult SetSubRes = Set->Subscribe();
	TestFalse(TEXT("SubscriptionSet Subscribe should succeed"), SetSubRes.Error);

	const TArray<UPubnubSubscriptionSet*> ActiveSetsAfterSet = PubnubClient->GetActiveSubscriptionSets();
	TestTrue(TEXT("After Set->Subscribe, GetActiveSubscriptionSets should list at least one set"), ActiveSetsAfterSet.Num() >= 1);

	UPubnubChannelEntity* const StandaloneEnt = PubnubClient->CreateChannelEntity(ChStandalone);
	TestNotNull(TEXT("Standalone channel entity for flat subscription list"), StandaloneEnt);
	if (!StandaloneEnt)
	{
		(void)Set->Unsubscribe();
		PubnubEntitiesTestsPrivate::CleanupChannelGroupScenario(PubnubClient, Grp, Member);
		CleanUp();
		return false;
	}

	UPubnubSubscription* const StandaloneSub = StandaloneEnt->CreateSubscription(FPubnubSubscribeSettings());
	TestNotNull(TEXT("Standalone UPubnubSubscription should be created"), StandaloneSub);
	if (!StandaloneSub)
	{
		(void)Set->Unsubscribe();
		PubnubEntitiesTestsPrivate::CleanupChannelGroupScenario(PubnubClient, Grp, Member);
		CleanUp();
		return false;
	}

	const FPubnubOperationResult StandaloneSubRes = StandaloneSub->Subscribe();
	TestFalse(TEXT("Standalone subscription Subscribe should succeed"), StandaloneSubRes.Error);

	const TArray<UPubnubSubscription*> ActiveSubs = PubnubClient->GetActiveSubscriptions();
	TestTrue(TEXT("After entity subscription Subscribe, GetActiveSubscriptions should list at least one subscription"), ActiveSubs.Num() >= 1);

	(void)StandaloneSub->Unsubscribe();
	(void)Set->Unsubscribe();

	PubnubEntitiesTestsPrivate::CleanupChannelGroupScenario(PubnubClient, Grp, Member);

	CleanUp();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
