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

namespace PubnubChannelMetadataTestsPrivate
{
	bool TryGetStringFieldFromCustomJson(const FString& CustomJson, const FString& Key, FString& OutValue)
	{
		if (CustomJson.IsEmpty())
		{
			return false;
		}
		TSharedPtr<FJsonObject> Obj;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(CustomJson);
		if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid())
		{
			return false;
		}
		return Obj->TryGetStringField(Key, OutValue);
	}

	bool CustomJsonHasStringField(const FString& CustomJson, const FString& Key, const FString& ExpectedValue)
	{
		FString V;
		return TryGetStringFieldFromCustomJson(CustomJson, Key, V) && V == ExpectedValue;
	}

	const FPubnubChannelData* FindChannelInList(const TArray<FPubnubChannelData>& Channels, const FString& ChannelId)
	{
		for (const FPubnubChannelData& C : Channels)
		{
			if (C.ChannelID == ChannelId)
			{
				return &C;
			}
		}
		return nullptr;
	}
}

using namespace PubnubChannelMetadataTestsPrivate;

// ---------------------------------------------------------------------------
// UPubnubClient::GetAllChannelMetadata
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetAllChannelMetadata_HappyPath_DefaultParams, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMetadata.GetAllChannelMetadata.2HappyPath.DefaultParams",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetAllChannelMetadata_AllOptionalParameters, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMetadata.GetAllChannelMetadata.3FullParams.IncludeSortFilterLimitTotalCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetAllChannelMetadata_FilterFindsCreatedChannel, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMetadata.GetAllChannelMetadata.4Advanced.FilterFindsCreatedChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::SetChannelMetadata
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetChannelMetadata_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMetadata.SetChannelMetadata.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetChannelMetadata_HappyPath_RequiredFieldsOnly, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMetadata.SetChannelMetadata.2HappyPath.RequiredFieldsOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetChannelMetadata_AllInputFieldsAndInclude, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMetadata.SetChannelMetadata.3FullParams.AllFieldsAndInclude",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetChannelMetadata_UpdateThenGetReflectsNewValues, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMetadata.SetChannelMetadata.4Advanced.UpdateOverwritesPreviousName",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::GetChannelMetadata
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetChannelMetadata_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMetadata.GetChannelMetadata.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetChannelMetadata_HappyPath_AfterSet, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMetadata.GetChannelMetadata.2HappyPath.AfterSet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetChannelMetadata_AllIncludes_ReturnsCustomStatusType, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMetadata.GetChannelMetadata.3FullParams.AllIncludes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetChannelMetadata_UnknownChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMetadata.GetChannelMetadata.4Advanced.UnknownChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::RemoveChannelMetadata
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveChannelMetadata_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMetadata.RemoveChannelMetadata.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveChannelMetadata_HappyPath_ThenGetFails, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMetadata.RemoveChannelMetadata.2HappyPath.ThenGetFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveChannelMetadata_SetAfterRemove_RecreatesMetadata, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelMetadata.RemoveChannelMetadata.4Advanced.SetAfterRemoveRecreatesMetadata",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// GetAllChannelMetadata
// ---------------------------------------------------------------------------

bool FPubnubGetAllChannelMetadata_HappyPath_DefaultParams::RunTest(const FString& Parameters)
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
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("cmeta_getall_default_caller"));

	const FPubnubGetAllChannelMetadataResult R = PubnubClient->GetAllChannelMetadata();

	TestFalse(TEXT("GetAllChannelMetadata should succeed"), R.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), R.Result.Status, 200);

	CleanUp();
	return true;
}

bool FPubnubGetAllChannelMetadata_AllOptionalParameters::RunTest(const FString& Parameters)
{
	const FString ChannelId = SDK_PREFIX + TEXT("cmeta_getall_full_ch");
	const FString Caller = SDK_PREFIX + TEXT("cmeta_getall_full_caller");
	const FString StatusMarker = TEXT("chmetaStatFullParams");

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

	FPubnubChannelInputData SetInput;
	SetInput.ChannelName = TEXT("FullParamsChannelName");
	SetInput.Status = StatusMarker;
	const FPubnubChannelMetadataResult SetR = PubnubClient->SetChannelMetadata(ChannelId, SetInput);
	TestFalse(TEXT("Setup SetChannelMetadata should succeed"), SetR.Result.Error);

	FPubnubGetAllInclude Include = FPubnubGetAllInclude::FromValue(true);
	FPubnubGetAllSort Sort;
	Sort.GetAllSort.Add(FPubnubGetAllSingleSort{EPubnubGetAllSortType::PGAST_Name, true});
	const FString Filter = FString::Printf(TEXT("status=='%s'"), *StatusMarker);

	const FPubnubGetAllChannelMetadataResult R = PubnubClient->GetAllChannelMetadata(Include, 50, Filter, Sort, FPubnubPage());

	TestFalse(TEXT("GetAllChannelMetadata should succeed"), R.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), R.Result.Status, 200);
	TestTrue(TEXT("Filtered list should contain created channel"), FindChannelInList(R.ChannelsData, ChannelId) != nullptr);
	TestTrue(TEXT("totalCount should reflect at least one matching channel when IncludeTotalCount is set"), R.TotalCount >= 1);

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveChannelMetadata(ChannelId);
	TestFalse(TEXT("RemoveChannelMetadata cleanup should succeed"), RemoveR.Error);

	CleanUp();
	return true;
}

// After SetChannelMetadata, GetAll with a status filter should return that channel in data[].
bool FPubnubGetAllChannelMetadata_FilterFindsCreatedChannel::RunTest(const FString& Parameters)
{
	const FString ChannelId = SDK_PREFIX + TEXT("cmeta_getall_filter_ch");
	const FString Caller = SDK_PREFIX + TEXT("cmeta_getall_filter_caller");
	const FString StatusValue = TEXT("chmetaFilterMarker");

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

	FPubnubChannelInputData In;
	In.ChannelName = TEXT("FilterTestChannelName");
	In.Status = StatusValue;
	const FPubnubChannelMetadataResult SetR = PubnubClient->SetChannelMetadata(ChannelId, In);
	TestFalse(TEXT("SetChannelMetadata should succeed"), SetR.Result.Error);

	const FString Filter = FString::Printf(TEXT("status=='%s'"), *StatusValue);
	FPubnubGetAllInclude GetAllInclude;
	GetAllInclude.IncludeStatus = true;
	const FPubnubGetAllChannelMetadataResult AllR = PubnubClient->GetAllChannelMetadata(GetAllInclude, 100, Filter);

	TestFalse(TEXT("GetAllChannelMetadata should succeed"), AllR.Result.Error);
	const FPubnubChannelData* Found = FindChannelInList(AllR.ChannelsData, ChannelId);
	TestNotNull(TEXT("Channel should appear in filtered GetAll result"), Found);
	if (Found)
	{
		TestEqual(TEXT("Returned ChannelName should match"), Found->ChannelName, In.ChannelName);
		TestEqual(TEXT("Returned Status should match"), Found->Status, StatusValue);
	}

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveChannelMetadata(ChannelId);
	TestFalse(TEXT("RemoveChannelMetadata cleanup should succeed"), RemoveR.Error);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// SetChannelMetadata
// ---------------------------------------------------------------------------

bool FPubnubSetChannelMetadata_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("cmeta_set_val_caller"));

	FPubnubChannelInputData In;
	In.ChannelName = TEXT("IgnoredBecauseChannelEmpty");
	const FPubnubChannelMetadataResult R = PubnubClient->SetChannelMetadata(FString(), In);

	TestTrue(TEXT("Result should indicate error"), R.Result.Error);
	TestTrue(TEXT("ErrorMessage should mention Channel"), R.Result.ErrorMessage.Contains(TEXT("Channel")));
	TestTrue(TEXT("ErrorMessage should indicate field is empty"), R.Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubSetChannelMetadata_HappyPath_RequiredFieldsOnly::RunTest(const FString& Parameters)
{
	const FString ChannelId = SDK_PREFIX + TEXT("cmeta_set_happy_ch");
	const FString Caller = SDK_PREFIX + TEXT("cmeta_set_happy_caller");

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

	FPubnubChannelInputData In;
	In.ChannelName = TEXT("HappyPathChannelName");
	const FPubnubChannelMetadataResult R = PubnubClient->SetChannelMetadata(ChannelId, In);

	TestFalse(TEXT("SetChannelMetadata should succeed"), R.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), R.Result.Status, 200);
	TestEqual(TEXT("Response ChannelID should match"), R.ChannelData.ChannelID, ChannelId);
	TestEqual(TEXT("Response ChannelName should match input"), R.ChannelData.ChannelName, In.ChannelName);

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveChannelMetadata(ChannelId);
	TestFalse(TEXT("RemoveChannelMetadata cleanup should succeed"), RemoveR.Error);

	CleanUp();
	return true;
}

bool FPubnubSetChannelMetadata_AllInputFieldsAndInclude::RunTest(const FString& Parameters)
{
	const FString ChannelId = SDK_PREFIX + TEXT("cmeta_set_full_ch");
	const FString Caller = SDK_PREFIX + TEXT("cmeta_set_full_caller");

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

	FPubnubChannelInputData In;
	In.ChannelName = TEXT("FullChannelName");
	In.Description = TEXT("Full description text for channel metadata test");
	In.Custom = TEXT("{\"tier\":\"gold\",\"region\":\"eu\"}");
	In.Status = TEXT("chmetaStatusFull");
	In.Type = TEXT("chmetaTypeFull");
	In.ForceSetChannelName = true;
	In.ForceSetDescription = true;
	In.ForceSetCustom = true;
	In.ForceSetStatus = true;
	In.ForceSetType = true;

	const FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	const FPubnubChannelMetadataResult R = PubnubClient->SetChannelMetadata(ChannelId, In, Include);

	TestFalse(TEXT("SetChannelMetadata should succeed"), R.Result.Error);
	TestEqual(TEXT("Response ChannelID should match"), R.ChannelData.ChannelID, ChannelId);
	TestEqual(TEXT("Response ChannelName should match"), R.ChannelData.ChannelName, In.ChannelName);
	TestEqual(TEXT("Response Description should match"), R.ChannelData.Description, In.Description);
	TestEqual(TEXT("Response Status should match"), R.ChannelData.Status, In.Status);
	TestEqual(TEXT("Response Type should match"), R.ChannelData.Type, In.Type);
	TestTrue(TEXT("Custom should contain tier=gold"), CustomJsonHasStringField(R.ChannelData.Custom, TEXT("tier"), TEXT("gold")));
	TestTrue(TEXT("Custom should contain region=eu"), CustomJsonHasStringField(R.ChannelData.Custom, TEXT("region"), TEXT("eu")));

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveChannelMetadata(ChannelId);
	TestFalse(TEXT("RemoveChannelMetadata cleanup should succeed"), RemoveR.Error);

	CleanUp();
	return true;
}

// Two sequential sets change the channel display name; GetChannelMetadata should return the latest value.
bool FPubnubSetChannelMetadata_UpdateThenGetReflectsNewValues::RunTest(const FString& Parameters)
{
	const FString ChannelId = SDK_PREFIX + TEXT("cmeta_set_update_ch");
	const FString Caller = SDK_PREFIX + TEXT("cmeta_set_update_caller");

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

	FPubnubChannelInputData First;
	First.ChannelName = TEXT("ChannelNameVersionA");
	TestFalse(TEXT("First SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, First).Result.Error);

	FPubnubChannelInputData Second;
	Second.ChannelName = TEXT("ChannelNameVersionB");
	const FPubnubChannelMetadataResult Set2 = PubnubClient->SetChannelMetadata(ChannelId, Second);
	TestFalse(TEXT("Second SetChannelMetadata should succeed"), Set2.Result.Error);
	TestEqual(TEXT("Set response should expose new name"), Set2.ChannelData.ChannelName, Second.ChannelName);

	const FPubnubChannelMetadataResult GetR = PubnubClient->GetChannelMetadata(ChannelId);
	TestFalse(TEXT("GetChannelMetadata should succeed"), GetR.Result.Error);
	TestEqual(TEXT("Get should return updated ChannelName"), GetR.ChannelData.ChannelName, Second.ChannelName);

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveChannelMetadata(ChannelId);
	TestFalse(TEXT("RemoveChannelMetadata cleanup should succeed"), RemoveR.Error);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// GetChannelMetadata
// ---------------------------------------------------------------------------

bool FPubnubGetChannelMetadata_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("cmeta_get_val_caller"));

	const FPubnubChannelMetadataResult R = PubnubClient->GetChannelMetadata(FString());

	TestTrue(TEXT("Result should indicate error"), R.Result.Error);
	TestTrue(TEXT("ErrorMessage should mention Channel"), R.Result.ErrorMessage.Contains(TEXT("Channel")));
	TestTrue(TEXT("ErrorMessage should indicate field is empty"), R.Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubGetChannelMetadata_HappyPath_AfterSet::RunTest(const FString& Parameters)
{
	const FString ChannelId = SDK_PREFIX + TEXT("cmeta_get_happy_ch");
	const FString Caller = SDK_PREFIX + TEXT("cmeta_get_happy_caller");

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

	FPubnubChannelInputData In;
	In.ChannelName = TEXT("GetHappyChannelName");
	In.Status = TEXT("chmetaGetHappyStat");
	const FPubnubChannelMetadataResult SetR = PubnubClient->SetChannelMetadata(ChannelId, In);
	TestFalse(TEXT("SetChannelMetadata setup should succeed"), SetR.Result.Error);

	FPubnubGetMetadataInclude GetInclude;
	GetInclude.IncludeStatus = true;
	const FPubnubChannelMetadataResult GetR = PubnubClient->GetChannelMetadata(ChannelId, GetInclude);

	TestFalse(TEXT("GetChannelMetadata should succeed"), GetR.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), GetR.Result.Status, 200);
	TestEqual(TEXT("ChannelID should match"), GetR.ChannelData.ChannelID, ChannelId);
	TestEqual(TEXT("ChannelName should match"), GetR.ChannelData.ChannelName, In.ChannelName);
	TestEqual(TEXT("Status should match"), GetR.ChannelData.Status, In.Status);

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveChannelMetadata(ChannelId);
	TestFalse(TEXT("RemoveChannelMetadata cleanup should succeed"), RemoveR.Error);

	CleanUp();
	return true;
}

bool FPubnubGetChannelMetadata_AllIncludes_ReturnsCustomStatusType::RunTest(const FString& Parameters)
{
	const FString ChannelId = SDK_PREFIX + TEXT("cmeta_get_inc_ch");
	const FString Caller = SDK_PREFIX + TEXT("cmeta_get_inc_caller");

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

	FPubnubChannelInputData In;
	In.ChannelName = TEXT("IncludeTestChannelName");
	In.Custom = TEXT("{\"badge\":\"vip\"}");
	In.Status = TEXT("chmetaIncStat");
	In.Type = TEXT("chmetaIncType");
	const FPubnubChannelMetadataResult SetR = PubnubClient->SetChannelMetadata(ChannelId, In);
	TestFalse(TEXT("SetChannelMetadata setup should succeed"), SetR.Result.Error);

	const FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	const FPubnubChannelMetadataResult GetR = PubnubClient->GetChannelMetadata(ChannelId, Include);

	TestFalse(TEXT("GetChannelMetadata should succeed"), GetR.Result.Error);
	TestEqual(TEXT("ChannelName should match"), GetR.ChannelData.ChannelName, In.ChannelName);
	TestEqual(TEXT("Status should match"), GetR.ChannelData.Status, In.Status);
	TestEqual(TEXT("Type should match"), GetR.ChannelData.Type, In.Type);
	TestTrue(TEXT("Custom should include badge"), CustomJsonHasStringField(GetR.ChannelData.Custom, TEXT("badge"), TEXT("vip")));

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveChannelMetadata(ChannelId);
	TestFalse(TEXT("RemoveChannelMetadata cleanup should succeed"), RemoveR.Error);

	CleanUp();
	return true;
}

bool FPubnubGetChannelMetadata_UnknownChannel_ReturnsError::RunTest(const FString& Parameters)
{
	const FString UnknownChannel = SDK_PREFIX + TEXT("cmeta_get_unknown_nonexistent");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("cmeta_get_unknown_caller"));

	const FPubnubChannelMetadataResult R = PubnubClient->GetChannelMetadata(UnknownChannel);

	TestTrue(TEXT("GetChannelMetadata for unknown channel should report error"), R.Result.Error);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// RemoveChannelMetadata
// ---------------------------------------------------------------------------

bool FPubnubRemoveChannelMetadata_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("cmeta_rm_val_caller"));

	const FPubnubOperationResult R = PubnubClient->RemoveChannelMetadata(FString());

	TestTrue(TEXT("Result should indicate error"), R.Error);
	TestTrue(TEXT("ErrorMessage should mention Channel"), R.ErrorMessage.Contains(TEXT("Channel")));
	TestTrue(TEXT("ErrorMessage should indicate field is empty"), R.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubRemoveChannelMetadata_HappyPath_ThenGetFails::RunTest(const FString& Parameters)
{
	const FString ChannelId = SDK_PREFIX + TEXT("cmeta_rm_happy_ch");
	const FString Caller = SDK_PREFIX + TEXT("cmeta_rm_happy_caller");

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

	FPubnubChannelInputData In;
	In.ChannelName = TEXT("ToBeRemovedChannel");
	TestFalse(TEXT("SetChannelMetadata setup should succeed"), PubnubClient->SetChannelMetadata(ChannelId, In).Result.Error);

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveChannelMetadata(ChannelId);
	TestFalse(TEXT("RemoveChannelMetadata should succeed"), RemoveR.Error);
	TestEqual(TEXT("HTTP status should be 200"), RemoveR.Status, 200);

	const FPubnubChannelMetadataResult GetR = PubnubClient->GetChannelMetadata(ChannelId);
	TestTrue(TEXT("GetChannelMetadata after remove should fail"), GetR.Result.Error);

	CleanUp();
	return true;
}

// After RemoveChannelMetadata, SetChannelMetadata can create the App Context channel again; GetChannelMetadata returns the new payload.
bool FPubnubRemoveChannelMetadata_SetAfterRemove_RecreatesMetadata::RunTest(const FString& Parameters)
{
	const FString ChannelId = SDK_PREFIX + TEXT("cmeta_rm_recreate_ch");
	const FString Caller = SDK_PREFIX + TEXT("cmeta_rm_recreate_caller");

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

	FPubnubChannelInputData First;
	First.ChannelName = TEXT("BeforeRemoveChannel");
	TestFalse(TEXT("Initial SetChannelMetadata should succeed"), PubnubClient->SetChannelMetadata(ChannelId, First).Result.Error);

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveChannelMetadata(ChannelId);
	TestFalse(TEXT("RemoveChannelMetadata should succeed"), RemoveR.Error);

	FPubnubChannelInputData Second;
	Second.ChannelName = TEXT("AfterRemoveChannel");
	const FPubnubChannelMetadataResult SetAgain = PubnubClient->SetChannelMetadata(ChannelId, Second);
	TestFalse(TEXT("SetChannelMetadata after remove should succeed"), SetAgain.Result.Error);
	TestEqual(TEXT("Set response should carry new name"), SetAgain.ChannelData.ChannelName, Second.ChannelName);

	const FPubnubChannelMetadataResult GetR = PubnubClient->GetChannelMetadata(ChannelId);
	TestFalse(TEXT("GetChannelMetadata should succeed after recreate"), GetR.Result.Error);
	TestEqual(TEXT("Get should return recreated ChannelName"), GetR.ChannelData.ChannelName, Second.ChannelName);

	const FPubnubOperationResult FinalRemove = PubnubClient->RemoveChannelMetadata(ChannelId);
	TestFalse(TEXT("Final RemoveChannelMetadata cleanup should succeed"), FinalRemove.Error);

	CleanUp();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
