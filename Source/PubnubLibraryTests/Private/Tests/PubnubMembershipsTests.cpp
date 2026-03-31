// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubClient.h"
#include "PubnubStructLibrary.h"
#include "PubnubEnumLibrary.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

using namespace PubnubTests;

namespace PubnubMembershipsTestsPrivate
{
	bool TryGetStringFieldFromCustomJson(const FString& CustomJson, const FString& Key, FString& OutValue)
	{
		if (CustomJson.IsEmpty())
		{
			return false;
		}
		TSharedPtr<FJsonObject> Obj;
		const TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(CustomJson);
		if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid())
		{
			return false;
		}
		return Obj->TryGetStringField(Key, OutValue);
	}

	const FPubnubMembershipData* FindMembershipByChannelId(const TArray<FPubnubMembershipData>& Items, const FString& ChannelId)
	{
		for (const FPubnubMembershipData& M : Items)
		{
			if (M.Channel.ChannelID == ChannelId)
			{
				return &M;
			}
		}
		return nullptr;
	}

	const FPubnubChannelMemberData* FindChannelMemberByUserId(const TArray<FPubnubChannelMemberData>& Items, const FString& UserId)
	{
		for (const FPubnubChannelMemberData& M : Items)
		{
			if (M.User.UserID == UserId)
			{
				return &M;
			}
		}
		return nullptr;
	}

	void CleanupMembershipScenario(UPubnubClient* Client, const FString& UserId, const FString& ChannelId)
	{
		if (!Client)
		{
			return;
		}
		if (!UserId.IsEmpty() && !ChannelId.IsEmpty())
		{
			TArray<FString> ToRemove;
			ToRemove.Add(ChannelId);
			const FPubnubMembershipsResult RemoveMem = Client->RemoveMemberships(UserId, ToRemove);
			(void)RemoveMem;
		}
		if (!ChannelId.IsEmpty())
		{
			const FPubnubOperationResult RemoveCh = Client->RemoveChannelMetadata(ChannelId);
			(void)RemoveCh;
		}
		if (!UserId.IsEmpty())
		{
			const FPubnubOperationResult RemoveU = Client->RemoveUserMetadata(UserId);
			(void)RemoveU;
		}
	}
}

// Do not `using namespace` here: unity builds merge test .cpp files, and other modules
// define the same helper name (TryGetStringFieldFromCustomJson) in their own namespaces.

// ---------------------------------------------------------------------------
// UPubnubClient::GetMemberships
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetMemberships_EmptyUser_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Memberships.GetMemberships.1Validation.EmptyUser",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetMemberships_HappyPath_AfterSetMembership, FPubnubAutomationTestBase,
	"Pubnub.Integration.Memberships.GetMemberships.2HappyPath.AfterSetMembership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetMemberships_AllOptionalParameters, FPubnubAutomationTestBase,
	"Pubnub.Integration.Memberships.GetMemberships.3FullParameters.IncludeFilterSortLimitTotalCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetMemberships_UserWithNoMemberships_ReturnsEmptyData, FPubnubAutomationTestBase,
	"Pubnub.Integration.Memberships.GetMemberships.4Advanced.UserMetadataOnlyEmptyMemberships",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetMemberships_AfterRemove_ChannelNotListed, FPubnubAutomationTestBase,
	"Pubnub.Integration.Memberships.GetMemberships.4Advanced.AfterRemoveChannelNotInList",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::SetMemberships
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetMemberships_EmptyUser_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Memberships.SetMemberships.1Validation.EmptyUser",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetMemberships_HappyPath_ChannelIdOnly, FPubnubAutomationTestBase,
	"Pubnub.Integration.Memberships.SetMemberships.2HappyPath.ChannelIdOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetMemberships_AllInputFieldsAndListParams, FPubnubAutomationTestBase,
	"Pubnub.Integration.Memberships.SetMemberships.3FullParameters.FieldsIncludeFilterSortLimitPage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetMemberships_UpdateStatus_ThenGetReflects, FPubnubAutomationTestBase,
	"Pubnub.Integration.Memberships.SetMemberships.4Advanced.SecondSetUpdatesStatus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::RemoveMemberships
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveMemberships_EmptyUser_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Memberships.RemoveMemberships.1Validation.EmptyUser",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveMemberships_HappyPath_ThenGetMissing, FPubnubAutomationTestBase,
	"Pubnub.Integration.Memberships.RemoveMemberships.2HappyPath.ThenGetMissing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveMemberships_AllOptionalParameters, FPubnubAutomationTestBase,
	"Pubnub.Integration.Memberships.RemoveMemberships.3FullParameters.IncludeFilterSortLimitTotalCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveMemberships_SecondRemove_StillSucceeds, FPubnubAutomationTestBase,
	"Pubnub.Integration.Memberships.RemoveMemberships.4Advanced.SecondRemoveIdempotent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::GetChannelMembers
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetChannelMembers_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMembers.GetChannelMembers.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetChannelMembers_HappyPath_AfterSetMember, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMembers.GetChannelMembers.2HappyPath.AfterSetChannelMember",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetChannelMembers_AllOptionalParameters, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMembers.GetChannelMembers.3FullParameters.IncludeFilterSortLimitTotalCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetChannelMembers_ChannelWithNoMembers_UserNotListed, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMembers.GetChannelMembers.4Advanced.MetadataOnlyNoMembers",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetChannelMembers_AfterRemove_UserNotListed, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMembers.GetChannelMembers.4Advanced.AfterRemoveUserNotInList",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::SetChannelMembers
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetChannelMembers_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMembers.SetChannelMembers.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetChannelMembers_HappyPath_UserIdOnly, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMembers.SetChannelMembers.2HappyPath.UserIdOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetChannelMembers_AllInputFieldsAndListParams, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMembers.SetChannelMembers.3FullParameters.FieldsIncludeFilterSortLimitPage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetChannelMembers_UpdateStatus_ThenGetReflects, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMembers.SetChannelMembers.4Advanced.SecondSetUpdatesStatus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::RemoveChannelMembers
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveChannelMembers_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMembers.RemoveChannelMembers.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveChannelMembers_HappyPath_ThenGetMissing, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMembers.RemoveChannelMembers.2HappyPath.ThenGetMissing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveChannelMembers_AllOptionalParameters, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMembers.RemoveChannelMembers.3FullParameters.IncludeFilterSortLimitTotalCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveChannelMembers_SecondRemove_StillSucceeds, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMembers.RemoveChannelMembers.4Advanced.SecondRemoveIdempotent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// GetMemberships
// ---------------------------------------------------------------------------

bool FPubnubGetMemberships_EmptyUser_ReturnsError::RunTest(const FString& Parameters)
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
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("memb_get_emptyuser_caller"));

	const FPubnubMembershipsResult R = PubnubClient->GetMemberships(TEXT(""));

	TestTrue(TEXT("GetMemberships with empty User should report error"), R.Result.Error);

	CleanUp();
	return true;
}

bool FPubnubGetMemberships_HappyPath_AfterSetMembership::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("memb_get_happy_u");
	const FString ChannelId = SDK_PREFIX + TEXT("memb_get_happy_ch");
	const FString Caller = SDK_PREFIX + TEXT("memb_get_happy_caller");

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

	FPubnubUserInputData UserIn;
	UserIn.UserName = TEXT("MembGetHappyUser");
	const FPubnubUserMetadataResult SetUserR = PubnubClient->SetUserMetadata(UserId, UserIn);
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), SetUserR.Result.Error);

	FPubnubChannelInputData ChIn;
	ChIn.ChannelName = TEXT("MembGetHappyChannel");
	const FPubnubChannelMetadataResult SetChR = PubnubClient->SetChannelMetadata(ChannelId, ChIn);
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), SetChR.Result.Error);

	TArray<FPubnubMembershipInputData> ToSet;
	FPubnubMembershipInputData M;
	M.Channel = ChannelId;
	ToSet.Add(M);
	const FPubnubMembershipsResult SetMemR = PubnubClient->SetMemberships(UserId, ToSet);
	TestFalse(TEXT("SetMemberships should succeed"), SetMemR.Result.Error);

	const FPubnubMembershipsResult GetR = PubnubClient->GetMemberships(UserId);
	TestFalse(TEXT("GetMemberships should succeed"), GetR.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), GetR.Result.Status, 200);
	const FPubnubMembershipData* Found = PubnubMembershipsTestsPrivate::FindMembershipByChannelId(GetR.MembershipsData, ChannelId);
	TestNotNull(TEXT("Membership list should contain the channel"), Found);

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

bool FPubnubGetMemberships_AllOptionalParameters::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("memb_get_full_u");
	const FString ChannelId = SDK_PREFIX + TEXT("memb_get_full_ch");
	const FString Caller = SDK_PREFIX + TEXT("memb_get_full_caller");
	const FString MemStatus = TEXT("membStatFullParams");

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

	FPubnubUserInputData UserIn;
	UserIn.UserName = TEXT("MembGetFullUser");
	const FPubnubUserMetadataResult SetUserR = PubnubClient->SetUserMetadata(UserId, UserIn);
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), SetUserR.Result.Error);

	FPubnubChannelInputData ChIn;
	ChIn.ChannelName = TEXT("MembGetFullChannel");
	const FPubnubChannelMetadataResult SetChR = PubnubClient->SetChannelMetadata(ChannelId, ChIn);
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), SetChR.Result.Error);

	FPubnubMembershipInputData M;
	M.Channel = ChannelId;
	M.Status = MemStatus;
	M.Type = TEXT("fullType");
	M.Custom = TEXT("{\"slot\":\"alpha\"}");
	TArray<FPubnubMembershipInputData> SetupMem;
	SetupMem.Add(M);
	const FPubnubMembershipsResult SetMemR = PubnubClient->SetMemberships(UserId, SetupMem);
	TestFalse(TEXT("Setup SetMemberships should succeed"), SetMemR.Result.Error);

	FPubnubMembershipInclude Include = FPubnubMembershipInclude::FromValue(true);
	FPubnubMembershipSort Sort;
	FPubnubMembershipSingleSort GetSort;
	GetSort.SortType = EPubnubMembershipSortType::PMST_Status;
	GetSort.SortOrder = false;
	Sort.MembershipSort.Add(GetSort);
	const FString Filter = FString::Printf(TEXT("status=='%s'"), *MemStatus);

	const FPubnubMembershipsResult GetR = PubnubClient->GetMemberships(UserId, Include, 50, Filter, Sort, FPubnubPage());

	TestFalse(TEXT("GetMemberships should succeed"), GetR.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), GetR.Result.Status, 200);
	TestTrue(TEXT("totalCount should be present when IncludeTotalCount is true"), GetR.TotalCount >= 1);

	const FPubnubMembershipData* Found = PubnubMembershipsTestsPrivate::FindMembershipByChannelId(GetR.MembershipsData, ChannelId);
	TestNotNull(TEXT("Filtered result should include the membership"), Found);
	if (Found)
	{
		TestEqual(TEXT("Membership status should match filter/source"), Found->Status, MemStatus);
		TestEqual(TEXT("Membership type should be returned"), Found->Type, TEXT("fullType"));
		FString CustomVal;
		const bool bHasSlot = PubnubMembershipsTestsPrivate::TryGetStringFieldFromCustomJson(Found->Custom, TEXT("slot"), CustomVal);
		TestTrue(TEXT("Custom should deserialize"), bHasSlot);
		TestEqual(TEXT("Custom.slot should match"), CustomVal, TEXT("alpha"));
		TestTrue(TEXT("Include channel should populate Channel.ChannelID"), !Found->Channel.ChannelID.IsEmpty());
	}

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

// User exists in Objects but has no channel memberships; list should be empty (or not contain our channel).
bool FPubnubGetMemberships_UserWithNoMemberships_ReturnsEmptyData::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("memb_get_nomemb_u");
	const FString ChannelId = SDK_PREFIX + TEXT("memb_get_nomemb_ch");
	const FString Caller = SDK_PREFIX + TEXT("memb_get_nomemb_caller");

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

	FPubnubUserInputData UserIn;
	UserIn.UserName = TEXT("NoMembUser");
	const FPubnubUserMetadataResult SetUserR = PubnubClient->SetUserMetadata(UserId, UserIn);
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), SetUserR.Result.Error);

	FPubnubChannelInputData ChIn;
	ChIn.ChannelName = TEXT("NoMembChannel");
	const FPubnubChannelMetadataResult SetChR = PubnubClient->SetChannelMetadata(ChannelId, ChIn);
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), SetChR.Result.Error);

	const FPubnubMembershipsResult GetR = PubnubClient->GetMemberships(UserId);
	TestFalse(TEXT("GetMemberships should succeed for user with no memberships"), GetR.Result.Error);
	TestTrue(TEXT("Membership list should not reference the orphan channel"), PubnubMembershipsTestsPrivate::FindMembershipByChannelId(GetR.MembershipsData, ChannelId) == nullptr);

	PubnubClient->RemoveChannelMetadata(ChannelId);
	PubnubClient->RemoveUserMetadata(UserId);

	CleanUp();
	return true;
}

// After RemoveMemberships, GetMemberships should no longer return that channel.
bool FPubnubGetMemberships_AfterRemove_ChannelNotListed::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("memb_get_afterrm_u");
	const FString ChannelId = SDK_PREFIX + TEXT("memb_get_afterrm_ch");
	const FString Caller = SDK_PREFIX + TEXT("memb_get_afterrm_caller");

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

	FPubnubUserInputData UserIn;
	UserIn.UserName = TEXT("AfterRmUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, UserIn).Result.Error);

	FPubnubChannelInputData ChIn;
	ChIn.ChannelName = TEXT("AfterRmChannel");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, ChIn).Result.Error);

	FPubnubMembershipInputData M;
	M.Channel = ChannelId;
	TArray<FPubnubMembershipInputData> AfterRmMem;
	AfterRmMem.Add(M);
	TestFalse(TEXT("Setup SetMemberships should succeed"), PubnubClient->SetMemberships(UserId, AfterRmMem).Result.Error);

	TArray<FString> AfterRmCh;
	AfterRmCh.Add(ChannelId);
	TestFalse(TEXT("RemoveMemberships should succeed"), PubnubClient->RemoveMemberships(UserId, AfterRmCh).Result.Error);

	const FPubnubMembershipsResult GetR = PubnubClient->GetMemberships(UserId);
	TestFalse(TEXT("GetMemberships should succeed after remove"), GetR.Result.Error);
	TestTrue(TEXT("Removed channel should not appear in memberships"), PubnubMembershipsTestsPrivate::FindMembershipByChannelId(GetR.MembershipsData, ChannelId) == nullptr);

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// SetMemberships
// ---------------------------------------------------------------------------

bool FPubnubSetMemberships_EmptyUser_ReturnsError::RunTest(const FString& Parameters)
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
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("memb_set_emptyuser_caller"));

	FPubnubMembershipInputData M;
	M.Channel = SDK_PREFIX + TEXT("memb_set_dummy_ch");
	TArray<FPubnubMembershipInputData> EmptyUserMem;
	EmptyUserMem.Add(M);
	const FPubnubMembershipsResult R = PubnubClient->SetMemberships(TEXT(""), EmptyUserMem);

	TestTrue(TEXT("SetMemberships with empty User should report error"), R.Result.Error);

	CleanUp();
	return true;
}

bool FPubnubSetMemberships_HappyPath_ChannelIdOnly::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("memb_set_happy_u");
	const FString ChannelId = SDK_PREFIX + TEXT("memb_set_happy_ch");
	const FString Caller = SDK_PREFIX + TEXT("memb_set_happy_caller");

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

	FPubnubUserInputData SuIn;
	SuIn.UserName = TEXT("SetHappyUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, SuIn).Result.Error);
	FPubnubChannelInputData ScIn;
	ScIn.ChannelName = TEXT("SetHappyCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, ScIn).Result.Error);

	FPubnubMembershipInputData M;
	M.Channel = ChannelId;
	TArray<FPubnubMembershipInputData> HappyMem;
	HappyMem.Add(M);
	const FPubnubMembershipsResult SetR = PubnubClient->SetMemberships(UserId, HappyMem);

	TestFalse(TEXT("SetMemberships should succeed"), SetR.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), SetR.Result.Status, 200);
	const FPubnubMembershipData* InResponse = PubnubMembershipsTestsPrivate::FindMembershipByChannelId(SetR.MembershipsData, ChannelId);
	TestNotNull(TEXT("Response data should include the new membership"), InResponse);

	const FPubnubMembershipsResult GetR = PubnubClient->GetMemberships(UserId);
	TestFalse(TEXT("GetMemberships should find membership"), GetR.Result.Error);
	TestNotNull(TEXT("Get should list the channel"), PubnubMembershipsTestsPrivate::FindMembershipByChannelId(GetR.MembershipsData, ChannelId));

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

bool FPubnubSetMemberships_AllInputFieldsAndListParams::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("memb_set_full_u");
	const FString ChannelId = SDK_PREFIX + TEXT("memb_set_full_ch");
	const FString Caller = SDK_PREFIX + TEXT("memb_set_full_caller");
	const FString MemStatus = TEXT("membSetFullStat");

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

	FPubnubUserInputData SfuIn;
	SfuIn.UserName = TEXT("SetFullUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, SfuIn).Result.Error);
	FPubnubChannelInputData SfcIn;
	SfcIn.ChannelName = TEXT("SetFullCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, SfcIn).Result.Error);

	FPubnubMembershipInputData M;
	M.Channel = ChannelId;
	M.Status = MemStatus;
	M.Type = TEXT("setFullType");
	M.Custom = TEXT("{\"tier\":\"gold\"}");

	FPubnubMembershipInclude Include = FPubnubMembershipInclude::FromValue(true);
	FPubnubMembershipSort Sort;
	FPubnubMembershipSingleSort Ss;
	Ss.SortType = EPubnubMembershipSortType::PMST_ChannelID;
	Ss.SortOrder = true;
	Sort.MembershipSort.Add(Ss);
	const FString Filter = FString::Printf(TEXT("status=='%s'"), *MemStatus);
	FPubnubPage Page;

	TArray<FPubnubMembershipInputData> FullMem;
	FullMem.Add(M);
	const FPubnubMembershipsResult SetR = PubnubClient->SetMemberships(UserId, FullMem, Include, 40, Filter, Sort, Page);

	TestFalse(TEXT("SetMemberships with full parameters should succeed"), SetR.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), SetR.Result.Status, 200);
	TestTrue(TEXT("totalCount should be set when IncludeTotalCount is true"), SetR.TotalCount >= 1);

	const FPubnubMembershipData* Found = PubnubMembershipsTestsPrivate::FindMembershipByChannelId(SetR.MembershipsData, ChannelId);
	TestNotNull(TEXT("Response should list the membership under filter"), Found);
	if (Found)
	{
		TestEqual(TEXT("Status should round-trip in response"), Found->Status, MemStatus);
		TestEqual(TEXT("Type should round-trip in response"), Found->Type, TEXT("setFullType"));
		FString Tier;
		const bool bHasTier = PubnubMembershipsTestsPrivate::TryGetStringFieldFromCustomJson(Found->Custom, TEXT("tier"), Tier);
		TestTrue(TEXT("Custom should parse"), bHasTier);
		TestEqual(TEXT("Custom.tier"), Tier, TEXT("gold"));
	}

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

// Second SetMemberships for the same channel updates membership fields visible on Get.
bool FPubnubSetMemberships_UpdateStatus_ThenGetReflects::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("memb_set_upd_u");
	const FString ChannelId = SDK_PREFIX + TEXT("memb_set_upd_ch");
	const FString Caller = SDK_PREFIX + TEXT("memb_set_upd_caller");

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

	FPubnubUserInputData UuIn;
	UuIn.UserName = TEXT("UpdUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, UuIn).Result.Error);
	FPubnubChannelInputData UcIn;
	UcIn.ChannelName = TEXT("UpdCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, UcIn).Result.Error);

	FPubnubMembershipInputData First;
	First.Channel = ChannelId;
	First.Status = TEXT("firstStatus");
	TArray<FPubnubMembershipInputData> FirstMem;
	FirstMem.Add(First);
	TestFalse(TEXT("First SetMemberships should succeed"), PubnubClient->SetMemberships(UserId, FirstMem).Result.Error);

	FPubnubMembershipInputData Second;
	Second.Channel = ChannelId;
	Second.Status = TEXT("secondStatus");
	TArray<FPubnubMembershipInputData> SecondMem;
	SecondMem.Add(Second);
	TestFalse(TEXT("Second SetMemberships should succeed"), PubnubClient->SetMemberships(UserId, SecondMem).Result.Error);

	FPubnubMembershipInclude Inc;
	Inc.IncludeStatus = true;
	const FPubnubMembershipsResult GetR = PubnubClient->GetMemberships(UserId, Inc);
	TestFalse(TEXT("GetMemberships should succeed"), GetR.Result.Error);
	const FPubnubMembershipData* Found = PubnubMembershipsTestsPrivate::FindMembershipByChannelId(GetR.MembershipsData, ChannelId);
	TestNotNull(TEXT("Membership should still exist"), Found);
	if (Found)
	{
		TestEqual(TEXT("Status should reflect last Set"), Found->Status, TEXT("secondStatus"));
	}

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// RemoveMemberships
// ---------------------------------------------------------------------------

bool FPubnubRemoveMemberships_EmptyUser_ReturnsError::RunTest(const FString& Parameters)
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
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("memb_rm_emptyuser_caller"));

	TArray<FString> RmDummy;
	RmDummy.Add(SDK_PREFIX + TEXT("memb_rm_dummy_ch"));
	const FPubnubMembershipsResult R = PubnubClient->RemoveMemberships(TEXT(""), RmDummy);

	TestTrue(TEXT("RemoveMemberships with empty User should report error"), R.Result.Error);

	CleanUp();
	return true;
}

bool FPubnubRemoveMemberships_HappyPath_ThenGetMissing::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("memb_rm_happy_u");
	const FString ChannelId = SDK_PREFIX + TEXT("memb_rm_happy_ch");
	const FString Caller = SDK_PREFIX + TEXT("memb_rm_happy_caller");

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

	FPubnubUserInputData RhuIn;
	RhuIn.UserName = TEXT("RmHappyUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, RhuIn).Result.Error);
	FPubnubChannelInputData RhcIn;
	RhcIn.ChannelName = TEXT("RmHappyCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, RhcIn).Result.Error);
	FPubnubMembershipInputData Rhm;
	Rhm.Channel = ChannelId;
	TArray<FPubnubMembershipInputData> RhmArr;
	RhmArr.Add(Rhm);
	TestFalse(TEXT("Setup SetMemberships should succeed"), PubnubClient->SetMemberships(UserId, RhmArr).Result.Error);

	TArray<FString> RmHappyCh;
	RmHappyCh.Add(ChannelId);
	const FPubnubMembershipsResult RemoveR = PubnubClient->RemoveMemberships(UserId, RmHappyCh);
	TestFalse(TEXT("RemoveMemberships should succeed"), RemoveR.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), RemoveR.Result.Status, 200);

	const FPubnubMembershipsResult GetR = PubnubClient->GetMemberships(UserId);
	TestFalse(TEXT("GetMemberships should succeed"), GetR.Result.Error);
	TestTrue(TEXT("Channel should be gone from memberships"), PubnubMembershipsTestsPrivate::FindMembershipByChannelId(GetR.MembershipsData, ChannelId) == nullptr);

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

bool FPubnubRemoveMemberships_AllOptionalParameters::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("memb_rm_full_u");
	const FString ChannelId = SDK_PREFIX + TEXT("memb_rm_full_ch");
	const FString Caller = SDK_PREFIX + TEXT("memb_rm_full_caller");
	const FString MemStatus = TEXT("membRmFullStat");

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

	FPubnubUserInputData RfuIn;
	RfuIn.UserName = TEXT("RmFullUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, RfuIn).Result.Error);
	FPubnubChannelInputData RfcIn;
	RfcIn.ChannelName = TEXT("RmFullCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, RfcIn).Result.Error);

	FPubnubMembershipInputData M;
	M.Channel = ChannelId;
	M.Status = MemStatus;
	TArray<FPubnubMembershipInputData> RfMem;
	RfMem.Add(M);
	TestFalse(TEXT("Setup SetMemberships should succeed"), PubnubClient->SetMemberships(UserId, RfMem).Result.Error);

	FPubnubMembershipInclude Include = FPubnubMembershipInclude::FromValue(true);
	FPubnubMembershipSort Sort;
	FPubnubMembershipSingleSort RfSort;
	RfSort.SortType = EPubnubMembershipSortType::PMST_Status;
	RfSort.SortOrder = false;
	Sort.MembershipSort.Add(RfSort);
	const FString Filter = FString::Printf(TEXT("status=='%s'"), *MemStatus);

	TArray<FString> RmFullCh;
	RmFullCh.Add(ChannelId);
	const FPubnubMembershipsResult RemoveR = PubnubClient->RemoveMemberships(UserId, RmFullCh, Include, 35, Filter, Sort, FPubnubPage());

	TestFalse(TEXT("RemoveMemberships with full parameters should succeed"), RemoveR.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), RemoveR.Result.Status, 200);
	TestTrue(TEXT("totalCount should be set when requested"), RemoveR.TotalCount >= 0);

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

// Calling RemoveMemberships again after the link was already removed should not fail the client flow.
bool FPubnubRemoveMemberships_SecondRemove_StillSucceeds::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("memb_rm_idem_u");
	const FString ChannelId = SDK_PREFIX + TEXT("memb_rm_idem_ch");
	const FString Caller = SDK_PREFIX + TEXT("memb_rm_idem_caller");

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

	FPubnubUserInputData IdU;
	IdU.UserName = TEXT("IdemUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, IdU).Result.Error);
	FPubnubChannelInputData IdC;
	IdC.ChannelName = TEXT("IdemCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, IdC).Result.Error);
	FPubnubMembershipInputData IdM;
	IdM.Channel = ChannelId;
	TArray<FPubnubMembershipInputData> IdMArr;
	IdMArr.Add(IdM);
	TestFalse(TEXT("Setup SetMemberships should succeed"), PubnubClient->SetMemberships(UserId, IdMArr).Result.Error);

	TArray<FString> IdemCh;
	IdemCh.Add(ChannelId);
	TestFalse(TEXT("First RemoveMemberships should succeed"), PubnubClient->RemoveMemberships(UserId, IdemCh).Result.Error);
	const FPubnubMembershipsResult Second = PubnubClient->RemoveMemberships(UserId, IdemCh);
	TestFalse(TEXT("Second RemoveMemberships should not report error"), Second.Result.Error);

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// GetChannelMembers
// ---------------------------------------------------------------------------

bool FPubnubGetChannelMembers_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
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
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("chmem_get_emptych_caller"));

	const FPubnubChannelMembersResult R = PubnubClient->GetChannelMembers(TEXT(""));

	TestTrue(TEXT("GetChannelMembers with empty Channel should report error"), R.Result.Error);

	CleanUp();
	return true;
}

bool FPubnubGetChannelMembers_HappyPath_AfterSetMember::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("chmem_get_happy_u");
	const FString ChannelId = SDK_PREFIX + TEXT("chmem_get_happy_ch");
	const FString Caller = SDK_PREFIX + TEXT("chmem_get_happy_caller");

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

	FPubnubUserInputData UIn;
	UIn.UserName = TEXT("ChMemGetHappyUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, UIn).Result.Error);
	FPubnubChannelInputData CIn;
	CIn.ChannelName = TEXT("ChMemGetHappyCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, CIn).Result.Error);

	FPubnubChannelMemberInputData MemIn;
	MemIn.User = UserId;
	TArray<FPubnubChannelMemberInputData> MemArr;
	MemArr.Add(MemIn);
	const FPubnubChannelMembersResult SetR = PubnubClient->SetChannelMembers(ChannelId, MemArr);
	TestFalse(TEXT("Setup SetChannelMembers should succeed"), SetR.Result.Error);

	const FPubnubChannelMembersResult GetR = PubnubClient->GetChannelMembers(ChannelId);
	TestFalse(TEXT("GetChannelMembers should succeed"), GetR.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), GetR.Result.Status, 200);
	TestNotNull(TEXT("Members list should contain the user"), PubnubMembershipsTestsPrivate::FindChannelMemberByUserId(GetR.MembersData, UserId));

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

bool FPubnubGetChannelMembers_AllOptionalParameters::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("chmem_get_full_u");
	const FString ChannelId = SDK_PREFIX + TEXT("chmem_get_full_ch");
	const FString Caller = SDK_PREFIX + TEXT("chmem_get_full_caller");
	const FString MemStatus = TEXT("chMemStatFullParams");

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

	FPubnubUserInputData FullU;
	FullU.UserName = TEXT("ChMemFullUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, FullU).Result.Error);
	FPubnubChannelInputData FullC;
	FullC.ChannelName = TEXT("ChMemFullCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, FullC).Result.Error);

	FPubnubChannelMemberInputData MemIn;
	MemIn.User = UserId;
	MemIn.Status = MemStatus;
	MemIn.Type = TEXT("chMemFullType");
	MemIn.Custom = TEXT("{\"badge\":\"mod\"}");
	TArray<FPubnubChannelMemberInputData> MemArr;
	MemArr.Add(MemIn);
	TestFalse(TEXT("Setup SetChannelMembers should succeed"), PubnubClient->SetChannelMembers(ChannelId, MemArr).Result.Error);

	FPubnubMemberInclude Include = FPubnubMemberInclude::FromValue(true);
	FPubnubMemberSort Sort;
	FPubnubMemberSingleSort Ms;
	Ms.SortType = EPubnubMemberSortType::PMeST_Status;
	Ms.SortOrder = false;
	Sort.MemberSort.Add(Ms);
	const FString Filter = FString::Printf(TEXT("status=='%s'"), *MemStatus);

	const FPubnubChannelMembersResult GetR = PubnubClient->GetChannelMembers(ChannelId, Include, 50, Filter, Sort, FPubnubPage());

	TestFalse(TEXT("GetChannelMembers should succeed"), GetR.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), GetR.Result.Status, 200);
	TestTrue(TEXT("totalCount should be set when IncludeTotalCount is true"), (GetR.TotalCount >= 1));

	const FPubnubChannelMemberData* Found = PubnubMembershipsTestsPrivate::FindChannelMemberByUserId(GetR.MembersData, UserId);
	TestNotNull(TEXT("Filtered list should include the member"), Found);
	if (Found)
	{
		TestEqual(TEXT("Member status should match"), Found->Status, MemStatus);
		TestEqual(TEXT("Member type should match"), Found->Type, TEXT("chMemFullType"));
		FString Badge;
		const bool bBadge = PubnubMembershipsTestsPrivate::TryGetStringFieldFromCustomJson(Found->Custom, TEXT("badge"), Badge);
		TestTrue(TEXT("Custom should parse"), bBadge);
		TestEqual(TEXT("Custom.badge"), Badge, TEXT("mod"));
		TestTrue(TEXT("Include uuid should populate User.UserID"), !Found->User.UserID.IsEmpty());
	}

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

// Channel and user exist in Objects but no member link; GetChannelMembers must not list that user.
bool FPubnubGetChannelMembers_ChannelWithNoMembers_UserNotListed::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("chmem_get_nomem_u");
	const FString ChannelId = SDK_PREFIX + TEXT("chmem_get_nomem_ch");
	const FString Caller = SDK_PREFIX + TEXT("chmem_get_nomem_caller");

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

	FPubnubUserInputData NoLinkU;
	NoLinkU.UserName = TEXT("ChMemNoLinkUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, NoLinkU).Result.Error);
	FPubnubChannelInputData NoLinkC;
	NoLinkC.ChannelName = TEXT("ChMemNoLinkCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, NoLinkC).Result.Error);

	const FPubnubChannelMembersResult GetR = PubnubClient->GetChannelMembers(ChannelId);
	TestFalse(TEXT("GetChannelMembers should succeed"), GetR.Result.Error);
	TestTrue(TEXT("User should not appear as member without SetChannelMembers"),
		PubnubMembershipsTestsPrivate::FindChannelMemberByUserId(GetR.MembersData, UserId) == nullptr);

	PubnubClient->RemoveChannelMetadata(ChannelId);
	PubnubClient->RemoveUserMetadata(UserId);

	CleanUp();
	return true;
}

// After RemoveChannelMembers, GetChannelMembers should not return that user.
bool FPubnubGetChannelMembers_AfterRemove_UserNotListed::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("chmem_get_aftrm_u");
	const FString ChannelId = SDK_PREFIX + TEXT("chmem_get_aftrm_ch");
	const FString Caller = SDK_PREFIX + TEXT("chmem_get_aftrm_caller");

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

	FPubnubUserInputData ArU;
	ArU.UserName = TEXT("ChMemAfterRmUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, ArU).Result.Error);
	FPubnubChannelInputData ArC;
	ArC.ChannelName = TEXT("ChMemAfterRmCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, ArC).Result.Error);

	FPubnubChannelMemberInputData MemIn;
	MemIn.User = UserId;
	TArray<FPubnubChannelMemberInputData> One;
	One.Add(MemIn);
	TestFalse(TEXT("Setup SetChannelMembers should succeed"), PubnubClient->SetChannelMembers(ChannelId, One).Result.Error);

	TArray<FString> RmUsers;
	RmUsers.Add(UserId);
	TestFalse(TEXT("RemoveChannelMembers should succeed"), PubnubClient->RemoveChannelMembers(ChannelId, RmUsers).Result.Error);

	const FPubnubChannelMembersResult GetR = PubnubClient->GetChannelMembers(ChannelId);
	TestFalse(TEXT("GetChannelMembers should succeed after remove"), GetR.Result.Error);
	TestTrue(TEXT("Removed user should not appear in members"), PubnubMembershipsTestsPrivate::FindChannelMemberByUserId(GetR.MembersData, UserId) == nullptr);

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// SetChannelMembers
// ---------------------------------------------------------------------------

bool FPubnubSetChannelMembers_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
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
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("chmem_set_emptych_caller"));

	FPubnubChannelMemberInputData M;
	M.User = SDK_PREFIX + TEXT("chmem_set_dummy_u");
	TArray<FPubnubChannelMemberInputData> Arr;
	Arr.Add(M);
	const FPubnubChannelMembersResult R = PubnubClient->SetChannelMembers(TEXT(""), Arr);

	TestTrue(TEXT("SetChannelMembers with empty Channel should report error"), R.Result.Error);

	CleanUp();
	return true;
}

bool FPubnubSetChannelMembers_HappyPath_UserIdOnly::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("chmem_set_happy_u");
	const FString ChannelId = SDK_PREFIX + TEXT("chmem_set_happy_ch");
	const FString Caller = SDK_PREFIX + TEXT("chmem_set_happy_caller");

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

	FPubnubUserInputData UIn;
	UIn.UserName = TEXT("ChMemSetHappyUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, UIn).Result.Error);
	FPubnubChannelInputData CIn;
	CIn.ChannelName = TEXT("ChMemSetHappyCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, CIn).Result.Error);

	FPubnubChannelMemberInputData MemIn;
	MemIn.User = UserId;
	TArray<FPubnubChannelMemberInputData> MemArr;
	MemArr.Add(MemIn);
	const FPubnubChannelMembersResult SetR = PubnubClient->SetChannelMembers(ChannelId, MemArr);

	TestFalse(TEXT("SetChannelMembers should succeed"), SetR.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), SetR.Result.Status, 200);
	TestNotNull(TEXT("Response should list the new member"), PubnubMembershipsTestsPrivate::FindChannelMemberByUserId(SetR.MembersData, UserId));

	const FPubnubChannelMembersResult GetR = PubnubClient->GetChannelMembers(ChannelId);
	TestFalse(TEXT("GetChannelMembers should list member"), GetR.Result.Error);
	TestNotNull(TEXT("Get should find user id"), PubnubMembershipsTestsPrivate::FindChannelMemberByUserId(GetR.MembersData, UserId));

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

bool FPubnubSetChannelMembers_AllInputFieldsAndListParams::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("chmem_set_full_u");
	const FString ChannelId = SDK_PREFIX + TEXT("chmem_set_full_ch");
	const FString Caller = SDK_PREFIX + TEXT("chmem_set_full_caller");
	const FString MemStatus = TEXT("chMemSetFullStat");

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

	FPubnubUserInputData Su;
	Su.UserName = TEXT("ChMemSetFullUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, Su).Result.Error);
	FPubnubChannelInputData Sc;
	Sc.ChannelName = TEXT("ChMemSetFullCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, Sc).Result.Error);

	FPubnubChannelMemberInputData MemIn;
	MemIn.User = UserId;
	MemIn.Status = MemStatus;
	MemIn.Type = TEXT("chSetMemType");
	MemIn.Custom = TEXT("{\"role\":\"lead\"}");
	TArray<FPubnubChannelMemberInputData> MemArr;
	MemArr.Add(MemIn);

	FPubnubMemberInclude Include = FPubnubMemberInclude::FromValue(true);
	FPubnubMemberSort Sort;
	FPubnubMemberSingleSort Ss;
	Ss.SortType = EPubnubMemberSortType::PMeST_UserID;
	Ss.SortOrder = true;
	Sort.MemberSort.Add(Ss);
	const FString Filter = FString::Printf(TEXT("status=='%s'"), *MemStatus);
	FPubnubPage Page;

	const FPubnubChannelMembersResult SetR = PubnubClient->SetChannelMembers(ChannelId, MemArr, Include, 40, Filter, Sort, Page);

	TestFalse(TEXT("SetChannelMembers with full parameters should succeed"), SetR.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), SetR.Result.Status, 200);
	TestTrue(TEXT("totalCount should be set when IncludeTotalCount is true"), (SetR.TotalCount >= 1));

	const FPubnubChannelMemberData* Found = PubnubMembershipsTestsPrivate::FindChannelMemberByUserId(SetR.MembersData, UserId);
	TestNotNull(TEXT("Response should include member under filter"), Found);
	if (Found)
	{
		TestEqual(TEXT("Status in response"), Found->Status, MemStatus);
		TestEqual(TEXT("Type in response"), Found->Type, TEXT("chSetMemType"));
		FString Role;
		const bool bRole = PubnubMembershipsTestsPrivate::TryGetStringFieldFromCustomJson(Found->Custom, TEXT("role"), Role);
		TestTrue(TEXT("Custom parses"), bRole);
		TestEqual(TEXT("Custom.role"), Role, TEXT("lead"));
	}

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

// Two SetChannelMembers calls for the same user; last status wins on GetChannelMembers.
bool FPubnubSetChannelMembers_UpdateStatus_ThenGetReflects::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("chmem_set_upd_u");
	const FString ChannelId = SDK_PREFIX + TEXT("chmem_set_upd_ch");
	const FString Caller = SDK_PREFIX + TEXT("chmem_set_upd_caller");

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

	FPubnubUserInputData UIn;
	UIn.UserName = TEXT("ChMemUpdUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, UIn).Result.Error);
	FPubnubChannelInputData CIn;
	CIn.ChannelName = TEXT("ChMemUpdCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, CIn).Result.Error);

	FPubnubChannelMemberInputData First;
	First.User = UserId;
	First.Status = TEXT("chFirstStat");
	TArray<FPubnubChannelMemberInputData> A1;
	A1.Add(First);
	TestFalse(TEXT("First SetChannelMembers should succeed"), PubnubClient->SetChannelMembers(ChannelId, A1).Result.Error);

	FPubnubChannelMemberInputData Second;
	Second.User = UserId;
	Second.Status = TEXT("chSecondStat");
	TArray<FPubnubChannelMemberInputData> A2;
	A2.Add(Second);
	TestFalse(TEXT("Second SetChannelMembers should succeed"), PubnubClient->SetChannelMembers(ChannelId, A2).Result.Error);

	FPubnubMemberInclude Inc;
	Inc.IncludeStatus = true;
	const FPubnubChannelMembersResult GetR = PubnubClient->GetChannelMembers(ChannelId, Inc);
	TestFalse(TEXT("GetChannelMembers should succeed"), GetR.Result.Error);
	const FPubnubChannelMemberData* Found = PubnubMembershipsTestsPrivate::FindChannelMemberByUserId(GetR.MembersData, UserId);
	TestNotNull(TEXT("Member should still exist"), Found);
	if (Found)
	{
		TestEqual(TEXT("Status should reflect last Set"), Found->Status, TEXT("chSecondStat"));
	}

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// RemoveChannelMembers
// ---------------------------------------------------------------------------

bool FPubnubRemoveChannelMembers_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
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
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("chmem_rm_emptych_caller"));

	TArray<FString> Users;
	Users.Add(SDK_PREFIX + TEXT("chmem_rm_dummy_u"));
	const FPubnubChannelMembersResult R = PubnubClient->RemoveChannelMembers(TEXT(""), Users);

	TestTrue(TEXT("RemoveChannelMembers with empty Channel should report error"), R.Result.Error);

	CleanUp();
	return true;
}

bool FPubnubRemoveChannelMembers_HappyPath_ThenGetMissing::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("chmem_rm_happy_u");
	const FString ChannelId = SDK_PREFIX + TEXT("chmem_rm_happy_ch");
	const FString Caller = SDK_PREFIX + TEXT("chmem_rm_happy_caller");

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

	FPubnubUserInputData UIn;
	UIn.UserName = TEXT("ChMemRmHappyUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, UIn).Result.Error);
	FPubnubChannelInputData CIn;
	CIn.ChannelName = TEXT("ChMemRmHappyCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, CIn).Result.Error);

	FPubnubChannelMemberInputData MemIn;
	MemIn.User = UserId;
	TArray<FPubnubChannelMemberInputData> MemArr;
	MemArr.Add(MemIn);
	TestFalse(TEXT("Setup SetChannelMembers should succeed"), PubnubClient->SetChannelMembers(ChannelId, MemArr).Result.Error);

	TArray<FString> RmList;
	RmList.Add(UserId);
	const FPubnubChannelMembersResult RemoveR = PubnubClient->RemoveChannelMembers(ChannelId, RmList);
	TestFalse(TEXT("RemoveChannelMembers should succeed"), RemoveR.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), RemoveR.Result.Status, 200);

	const FPubnubChannelMembersResult GetR = PubnubClient->GetChannelMembers(ChannelId);
	TestFalse(TEXT("GetChannelMembers should succeed"), GetR.Result.Error);
	TestTrue(TEXT("User should be gone from members"), PubnubMembershipsTestsPrivate::FindChannelMemberByUserId(GetR.MembersData, UserId) == nullptr);

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

bool FPubnubRemoveChannelMembers_AllOptionalParameters::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("chmem_rm_full_u");
	const FString ChannelId = SDK_PREFIX + TEXT("chmem_rm_full_ch");
	const FString Caller = SDK_PREFIX + TEXT("chmem_rm_full_caller");
	const FString MemStatus = TEXT("chMemRmFullStat");

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

	FPubnubUserInputData Ru;
	Ru.UserName = TEXT("ChMemRmFullUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, Ru).Result.Error);
	FPubnubChannelInputData Rc;
	Rc.ChannelName = TEXT("ChMemRmFullCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, Rc).Result.Error);

	FPubnubChannelMemberInputData MemIn;
	MemIn.User = UserId;
	MemIn.Status = MemStatus;
	TArray<FPubnubChannelMemberInputData> MemArr;
	MemArr.Add(MemIn);
	TestFalse(TEXT("Setup SetChannelMembers should succeed"), PubnubClient->SetChannelMembers(ChannelId, MemArr).Result.Error);

	FPubnubMemberInclude Include = FPubnubMemberInclude::FromValue(true);
	FPubnubMemberSort Sort;
	FPubnubMemberSingleSort Rs;
	Rs.SortType = EPubnubMemberSortType::PMeST_Status;
	Rs.SortOrder = false;
	Sort.MemberSort.Add(Rs);
	const FString Filter = FString::Printf(TEXT("status=='%s'"), *MemStatus);

	TArray<FString> RmList;
	RmList.Add(UserId);
	const FPubnubChannelMembersResult RemoveR = PubnubClient->RemoveChannelMembers(ChannelId, RmList, Include, 35, Filter, Sort, FPubnubPage());

	TestFalse(TEXT("RemoveChannelMembers with full parameters should succeed"), RemoveR.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), RemoveR.Result.Status, 200);
	TestTrue(TEXT("totalCount should be available when requested"), (RemoveR.TotalCount >= 0));

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

// Second RemoveChannelMembers after the user was already removed should still succeed at the client layer.
bool FPubnubRemoveChannelMembers_SecondRemove_StillSucceeds::RunTest(const FString& Parameters)
{
	const FString UserId = SDK_PREFIX + TEXT("chmem_rm_idem_u");
	const FString ChannelId = SDK_PREFIX + TEXT("chmem_rm_idem_ch");
	const FString Caller = SDK_PREFIX + TEXT("chmem_rm_idem_caller");

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

	FPubnubUserInputData UIn;
	UIn.UserName = TEXT("ChMemIdemUser");
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(UserId, UIn).Result.Error);
	FPubnubChannelInputData CIn;
	CIn.ChannelName = TEXT("ChMemIdemCh");
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, CIn).Result.Error);

	FPubnubChannelMemberInputData MemIn;
	MemIn.User = UserId;
	TArray<FPubnubChannelMemberInputData> MemArr;
	MemArr.Add(MemIn);
	TestFalse(TEXT("Setup SetChannelMembers should succeed"), PubnubClient->SetChannelMembers(ChannelId, MemArr).Result.Error);

	TArray<FString> RmList;
	RmList.Add(UserId);
	TestFalse(TEXT("First RemoveChannelMembers should succeed"), PubnubClient->RemoveChannelMembers(ChannelId, RmList).Result.Error);
	const FPubnubChannelMembersResult Second = PubnubClient->RemoveChannelMembers(ChannelId, RmList);
	TestFalse(TEXT("Second RemoveChannelMembers should not report error"), Second.Result.Error);

	PubnubMembershipsTestsPrivate::CleanupMembershipScenario(PubnubClient, UserId, ChannelId);

	CleanUp();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS