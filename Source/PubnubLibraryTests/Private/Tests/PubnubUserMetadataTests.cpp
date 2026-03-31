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

namespace PubnubUserMetadataTestsPrivate
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

	const FPubnubUserData* FindUserInList(const TArray<FPubnubUserData>& Users, const FString& UserId)
	{
		for (const FPubnubUserData& U : Users)
		{
			if (U.UserID == UserId)
			{
				return &U;
			}
		}
		return nullptr;
	}
}

using namespace PubnubUserMetadataTestsPrivate;

// ---------------------------------------------------------------------------
// UPubnubClient::GetAllUserMetadata
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetAllUserMetadata_HappyPath_DefaultParams, FPubnubAutomationTestBase,
	"Pubnub.Integration.UserMetadata.GetAllUserMetadata.2HappyPath.DefaultParams",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetAllUserMetadata_AllOptionalParameters, FPubnubAutomationTestBase,
	"Pubnub.Integration.UserMetadata.GetAllUserMetadata.3FullParameters.IncludeSortFilterLimitTotalCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetAllUserMetadata_FilterFindsCreatedUser, FPubnubAutomationTestBase,
	"Pubnub.Integration.UserMetadata.GetAllUserMetadata.4Advanced.FilterFindsCreatedUser",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::SetUserMetadata
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetUserMetadata_EmptyUser_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.UserMetadata.SetUserMetadata.1Validation.EmptyUser",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetUserMetadata_HappyPath_RequiredFieldsOnly, FPubnubAutomationTestBase,
	"Pubnub.Integration.UserMetadata.SetUserMetadata.2HappyPath.RequiredFieldsOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetUserMetadata_AllInputFieldsAndInclude, FPubnubAutomationTestBase,
	"Pubnub.Integration.UserMetadata.SetUserMetadata.3FullParameters.AllFieldsAndInclude",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetUserMetadata_UpdateThenGetReflectsNewValues, FPubnubAutomationTestBase,
	"Pubnub.Integration.UserMetadata.SetUserMetadata.4Advanced.UpdateOverwritesPreviousName",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::GetUserMetadata
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetUserMetadata_EmptyUser_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.UserMetadata.GetUserMetadata.1Validation.EmptyUser",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetUserMetadata_HappyPath_AfterSet, FPubnubAutomationTestBase,
	"Pubnub.Integration.UserMetadata.GetUserMetadata.2HappyPath.AfterSet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetUserMetadata_AllIncludes_ReturnsCustomStatusType, FPubnubAutomationTestBase,
	"Pubnub.Integration.UserMetadata.GetUserMetadata.3FullParameters.AllIncludes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetUserMetadata_UnknownUser_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.UserMetadata.GetUserMetadata.4Advanced.UnknownUser",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::RemoveUserMetadata
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveUserMetadata_EmptyUser_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.UserMetadata.RemoveUserMetadata.1Validation.EmptyUser",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveUserMetadata_HappyPath_ThenGetFails, FPubnubAutomationTestBase,
	"Pubnub.Integration.UserMetadata.RemoveUserMetadata.2HappyPath.ThenGetFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveUserMetadata_SetAfterRemove_RecreatesMetadata, FPubnubAutomationTestBase,
	"Pubnub.Integration.UserMetadata.RemoveUserMetadata.4Advanced.SetAfterRemoveRecreatesMetadata",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// GetAllUserMetadata
// ---------------------------------------------------------------------------

bool FPubnubGetAllUserMetadata_HappyPath_DefaultParams::RunTest(const FString& Parameters)
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
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("umeta_getall_default_caller"));

	const FPubnubGetAllUserMetadataResult R = PubnubClient->GetAllUserMetadata();

	TestFalse(TEXT("GetAllUserMetadata should succeed"), R.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), R.Result.Status, 200);

	CleanUp();
	return true;
}

bool FPubnubGetAllUserMetadata_AllOptionalParameters::RunTest(const FString& Parameters)
{
	const FString MetaUser = SDK_PREFIX + TEXT("umeta_getall_full_target");
	const FString Caller = SDK_PREFIX + TEXT("umeta_getall_full_caller");
	const FString StatusMarker = TEXT("umetaStatFullParams");

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

	FPubnubUserInputData SetInput;
	SetInput.UserName = TEXT("FullParamsListUser");
	SetInput.Status = StatusMarker;
	const FPubnubUserMetadataResult SetR = PubnubClient->SetUserMetadata(MetaUser, SetInput);
	TestFalse(TEXT("Setup SetUserMetadata should succeed"), SetR.Result.Error);

	FPubnubGetAllInclude Include = FPubnubGetAllInclude::FromValue(true);
	FPubnubGetAllSort Sort;
	Sort.GetAllSort.Add(FPubnubGetAllSingleSort{EPubnubGetAllSortType::PGAST_Name, true});
	const FString Filter = FString::Printf(TEXT("status=='%s'"), *StatusMarker);

	const FPubnubGetAllUserMetadataResult R = PubnubClient->GetAllUserMetadata(Include, 50, Filter, Sort, FPubnubPage());

	TestFalse(TEXT("GetAllUserMetadata should succeed"), R.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), R.Result.Status, 200);
	TestTrue(TEXT("Filtered list should contain created user"), FindUserInList(R.UsersData, MetaUser) != nullptr);
	TestTrue(TEXT("totalCount should reflect at least one matching user when IncludeTotalCount is set"), R.TotalCount >= 1);

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveUserMetadata(MetaUser);
	TestFalse(TEXT("RemoveUserMetadata cleanup should succeed"), RemoveR.Error);

	CleanUp();
	return true;
}

// After SetUserMetadata, GetAll with a status filter should return that user in data[].
bool FPubnubGetAllUserMetadata_FilterFindsCreatedUser::RunTest(const FString& Parameters)
{
	const FString MetaUser = SDK_PREFIX + TEXT("umeta_getall_filter_user");
	const FString Caller = SDK_PREFIX + TEXT("umeta_getall_filter_caller");
	const FString StatusValue = TEXT("umetaFilterMarker");

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

	FPubnubUserInputData In;
	In.UserName = TEXT("FilterTestDisplayName");
	In.Status = StatusValue;
	const FPubnubUserMetadataResult SetR = PubnubClient->SetUserMetadata(MetaUser, In);
	TestFalse(TEXT("SetUserMetadata should succeed"), SetR.Result.Error);

	const FString Filter = FString::Printf(TEXT("status=='%s'"), *StatusValue);
	FPubnubGetAllInclude GetAllInclude;
	GetAllInclude.IncludeStatus = true;
	const FPubnubGetAllUserMetadataResult AllR = PubnubClient->GetAllUserMetadata(GetAllInclude, 100, Filter);

	TestFalse(TEXT("GetAllUserMetadata should succeed"), AllR.Result.Error);
	const FPubnubUserData* Found = FindUserInList(AllR.UsersData, MetaUser);
	TestNotNull(TEXT("User should appear in filtered GetAll result"), Found);
	if (Found)
	{
		TestEqual(TEXT("Returned UserName should match"), Found->UserName, In.UserName);
		TestEqual(TEXT("Returned Status should match"), Found->Status, StatusValue);
	}

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveUserMetadata(MetaUser);
	TestFalse(TEXT("RemoveUserMetadata cleanup should succeed"), RemoveR.Error);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// SetUserMetadata
// ---------------------------------------------------------------------------

bool FPubnubSetUserMetadata_EmptyUser_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("umeta_set_val_caller"));

	FPubnubUserInputData In;
	In.UserName = TEXT("IgnoredBecauseUserEmpty");
	const FPubnubUserMetadataResult R = PubnubClient->SetUserMetadata(FString(), In);

	TestTrue(TEXT("Result should indicate error"), R.Result.Error);
	TestTrue(TEXT("ErrorMessage should mention User"), R.Result.ErrorMessage.Contains(TEXT("User")));
	TestTrue(TEXT("ErrorMessage should indicate field is empty"), R.Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubSetUserMetadata_HappyPath_RequiredFieldsOnly::RunTest(const FString& Parameters)
{
	const FString MetaUser = SDK_PREFIX + TEXT("umeta_set_happy_user");
	const FString Caller = SDK_PREFIX + TEXT("umeta_set_happy_caller");

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

	FPubnubUserInputData In;
	In.UserName = TEXT("HappyPathDisplayName");
	const FPubnubUserMetadataResult R = PubnubClient->SetUserMetadata(MetaUser, In);

	TestFalse(TEXT("SetUserMetadata should succeed"), R.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), R.Result.Status, 200);
	TestEqual(TEXT("Response UserID should match"), R.UserData.UserID, MetaUser);
	TestEqual(TEXT("Response UserName should match input"), R.UserData.UserName, In.UserName);

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveUserMetadata(MetaUser);
	TestFalse(TEXT("RemoveUserMetadata cleanup should succeed"), RemoveR.Error);

	CleanUp();
	return true;
}

bool FPubnubSetUserMetadata_AllInputFieldsAndInclude::RunTest(const FString& Parameters)
{
	const FString MetaUser = SDK_PREFIX + TEXT("umeta_set_full_user");
	const FString Caller = SDK_PREFIX + TEXT("umeta_set_full_caller");

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

	FPubnubUserInputData In;
	In.UserName = TEXT("FullName");
	In.ExternalID = TEXT("extid001");
	In.ProfileUrl = TEXT("https://example.com/profile");
	In.Email = TEXT("fulltest@example.com");
	In.Custom = TEXT("{\"tier\":\"gold\",\"region\":\"eu\"}");
	In.Status = TEXT("umetaStatusFull");
	In.Type = TEXT("umetaTypeFull");
	In.ForceSetUserName = true;
	In.ForceSetExternalID = true;
	In.ForceSetProfileUrl = true;
	In.ForceSetEmail = true;
	In.ForceSetCustom = true;
	In.ForceSetStatus = true;
	In.ForceSetType = true;

	const FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	const FPubnubUserMetadataResult R = PubnubClient->SetUserMetadata(MetaUser, In, Include);

	TestFalse(TEXT("SetUserMetadata should succeed"), R.Result.Error);
	TestEqual(TEXT("Response UserID should match"), R.UserData.UserID, MetaUser);
	TestEqual(TEXT("Response UserName should match"), R.UserData.UserName, In.UserName);
	TestEqual(TEXT("Response ExternalID should match"), R.UserData.ExternalID, In.ExternalID);
	TestEqual(TEXT("Response ProfileUrl should match"), R.UserData.ProfileUrl, In.ProfileUrl);
	TestEqual(TEXT("Response Email should match"), R.UserData.Email, In.Email);
	TestEqual(TEXT("Response Status should match"), R.UserData.Status, In.Status);
	TestEqual(TEXT("Response Type should match"), R.UserData.Type, In.Type);
	TestTrue(TEXT("Custom should contain tier=gold"), CustomJsonHasStringField(R.UserData.Custom, TEXT("tier"), TEXT("gold")));
	TestTrue(TEXT("Custom should contain region=eu"), CustomJsonHasStringField(R.UserData.Custom, TEXT("region"), TEXT("eu")));

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveUserMetadata(MetaUser);
	TestFalse(TEXT("RemoveUserMetadata cleanup should succeed"), RemoveR.Error);

	CleanUp();
	return true;
}

// Two sequential sets change the display name; GetUserMetadata should return the latest value.
bool FPubnubSetUserMetadata_UpdateThenGetReflectsNewValues::RunTest(const FString& Parameters)
{
	const FString MetaUser = SDK_PREFIX + TEXT("umeta_set_update_user");
	const FString Caller = SDK_PREFIX + TEXT("umeta_set_update_caller");

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

	FPubnubUserInputData First;
	First.UserName = TEXT("NameVersionA");
	TestFalse(TEXT("First SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(MetaUser, First).Result.Error);

	FPubnubUserInputData Second;
	Second.UserName = TEXT("NameVersionB");
	const FPubnubUserMetadataResult Set2 = PubnubClient->SetUserMetadata(MetaUser, Second);
	TestFalse(TEXT("Second SetUserMetadata should succeed"), Set2.Result.Error);
	TestEqual(TEXT("Set response should expose new name"), Set2.UserData.UserName, Second.UserName);

	const FPubnubUserMetadataResult GetR = PubnubClient->GetUserMetadata(MetaUser);
	TestFalse(TEXT("GetUserMetadata should succeed"), GetR.Result.Error);
	TestEqual(TEXT("Get should return updated UserName"), GetR.UserData.UserName, Second.UserName);

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveUserMetadata(MetaUser);
	TestFalse(TEXT("RemoveUserMetadata cleanup should succeed"), RemoveR.Error);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// GetUserMetadata
// ---------------------------------------------------------------------------

bool FPubnubGetUserMetadata_EmptyUser_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("umeta_get_val_caller"));

	const FPubnubUserMetadataResult R = PubnubClient->GetUserMetadata(FString());

	TestTrue(TEXT("Result should indicate error"), R.Result.Error);
	TestTrue(TEXT("ErrorMessage should mention User"), R.Result.ErrorMessage.Contains(TEXT("User")));
	TestTrue(TEXT("ErrorMessage should indicate field is empty"), R.Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubGetUserMetadata_HappyPath_AfterSet::RunTest(const FString& Parameters)
{
	const FString MetaUser = SDK_PREFIX + TEXT("umeta_get_happy_user");
	const FString Caller = SDK_PREFIX + TEXT("umeta_get_happy_caller");

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

	FPubnubUserInputData In;
	In.UserName = TEXT("GetHappyName");
	In.Status = TEXT("umetaGetHappyStat");
	const FPubnubUserMetadataResult SetR = PubnubClient->SetUserMetadata(MetaUser, In);
	TestFalse(TEXT("SetUserMetadata setup should succeed"), SetR.Result.Error);

	FPubnubGetMetadataInclude GetInclude;
	GetInclude.IncludeStatus = true;
	const FPubnubUserMetadataResult GetR = PubnubClient->GetUserMetadata(MetaUser, GetInclude);

	TestFalse(TEXT("GetUserMetadata should succeed"), GetR.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), GetR.Result.Status, 200);
	TestEqual(TEXT("UserID should match"), GetR.UserData.UserID, MetaUser);
	TestEqual(TEXT("UserName should match"), GetR.UserData.UserName, In.UserName);
	TestEqual(TEXT("Status should match"), GetR.UserData.Status, In.Status);

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveUserMetadata(MetaUser);
	TestFalse(TEXT("RemoveUserMetadata cleanup should succeed"), RemoveR.Error);

	CleanUp();
	return true;
}

bool FPubnubGetUserMetadata_AllIncludes_ReturnsCustomStatusType::RunTest(const FString& Parameters)
{
	const FString MetaUser = SDK_PREFIX + TEXT("umeta_get_inc_user");
	const FString Caller = SDK_PREFIX + TEXT("umeta_get_inc_caller");

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

	FPubnubUserInputData In;
	In.UserName = TEXT("IncludeTestName");
	In.Custom = TEXT("{\"badge\":\"vip\"}");
	In.Status = TEXT("umetaIncStat");
	In.Type = TEXT("umetaIncType");
	const FPubnubUserMetadataResult SetR = PubnubClient->SetUserMetadata(MetaUser, In);
	TestFalse(TEXT("SetUserMetadata setup should succeed"), SetR.Result.Error);

	const FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude::FromValue(true);
	const FPubnubUserMetadataResult GetR = PubnubClient->GetUserMetadata(MetaUser, Include);

	TestFalse(TEXT("GetUserMetadata should succeed"), GetR.Result.Error);
	TestEqual(TEXT("UserName should match"), GetR.UserData.UserName, In.UserName);
	TestEqual(TEXT("Status should match"), GetR.UserData.Status, In.Status);
	TestEqual(TEXT("Type should match"), GetR.UserData.Type, In.Type);
	TestTrue(TEXT("Custom should include badge"), CustomJsonHasStringField(GetR.UserData.Custom, TEXT("badge"), TEXT("vip")));

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveUserMetadata(MetaUser);
	TestFalse(TEXT("RemoveUserMetadata cleanup should succeed"), RemoveR.Error);

	CleanUp();
	return true;
}

bool FPubnubGetUserMetadata_UnknownUser_ReturnsError::RunTest(const FString& Parameters)
{
	const FString UnknownUser = SDK_PREFIX + TEXT("umeta_get_unknown_nonexistent");

	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("umeta_get_unknown_caller"));

	const FPubnubUserMetadataResult R = PubnubClient->GetUserMetadata(UnknownUser);

	TestTrue(TEXT("GetUserMetadata for unknown uuid should report error"), R.Result.Error);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// RemoveUserMetadata
// ---------------------------------------------------------------------------

bool FPubnubRemoveUserMetadata_EmptyUser_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError(TEXT("InitTest failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("umeta_rm_val_caller"));

	const FPubnubOperationResult R = PubnubClient->RemoveUserMetadata(FString());

	TestTrue(TEXT("Result should indicate error"), R.Error);
	TestTrue(TEXT("ErrorMessage should mention User"), R.ErrorMessage.Contains(TEXT("User")));
	TestTrue(TEXT("ErrorMessage should indicate field is empty"), R.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubRemoveUserMetadata_HappyPath_ThenGetFails::RunTest(const FString& Parameters)
{
	const FString MetaUser = SDK_PREFIX + TEXT("umeta_rm_happy_user");
	const FString Caller = SDK_PREFIX + TEXT("umeta_rm_happy_caller");

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

	FPubnubUserInputData In;
	In.UserName = TEXT("ToBeRemoved");
	TestFalse(TEXT("SetUserMetadata setup should succeed"), PubnubClient->SetUserMetadata(MetaUser, In).Result.Error);

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveUserMetadata(MetaUser);
	TestFalse(TEXT("RemoveUserMetadata should succeed"), RemoveR.Error);
	TestEqual(TEXT("HTTP status should be 200"), RemoveR.Status, 200);

	const FPubnubUserMetadataResult GetR = PubnubClient->GetUserMetadata(MetaUser);
	TestTrue(TEXT("GetUserMetadata after remove should fail"), GetR.Result.Error);

	CleanUp();
	return true;
}

// After RemoveUserMetadata, SetUserMetadata can create the App Context user again; GetUserMetadata returns the new payload.
bool FPubnubRemoveUserMetadata_SetAfterRemove_RecreatesMetadata::RunTest(const FString& Parameters)
{
	const FString MetaUser = SDK_PREFIX + TEXT("umeta_rm_recreate_user");
	const FString Caller = SDK_PREFIX + TEXT("umeta_rm_recreate_caller");

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

	FPubnubUserInputData First;
	First.UserName = TEXT("BeforeRemove");
	TestFalse(TEXT("Initial SetUserMetadata should succeed"), PubnubClient->SetUserMetadata(MetaUser, First).Result.Error);

	const FPubnubOperationResult RemoveR = PubnubClient->RemoveUserMetadata(MetaUser);
	TestFalse(TEXT("RemoveUserMetadata should succeed"), RemoveR.Error);

	FPubnubUserInputData Second;
	Second.UserName = TEXT("AfterRemove");
	const FPubnubUserMetadataResult SetAgain = PubnubClient->SetUserMetadata(MetaUser, Second);
	TestFalse(TEXT("SetUserMetadata after remove should succeed"), SetAgain.Result.Error);
	TestEqual(TEXT("Set response should carry new name"), SetAgain.UserData.UserName, Second.UserName);

	const FPubnubUserMetadataResult GetR = PubnubClient->GetUserMetadata(MetaUser);
	TestFalse(TEXT("GetUserMetadata should succeed after recreate"), GetR.Result.Error);
	TestEqual(TEXT("Get should return recreated UserName"), GetR.UserData.UserName, Second.UserName);

	const FPubnubOperationResult FinalRemove = PubnubClient->RemoveUserMetadata(MetaUser);
	TestFalse(TEXT("Final RemoveUserMetadata cleanup should succeed"), FinalRemove.Error);

	CleanUp();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
