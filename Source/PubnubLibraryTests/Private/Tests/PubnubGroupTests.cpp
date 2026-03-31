// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubClient.h"
#include "PubnubStructLibrary.h"
#include "PubnubEnumLibrary.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

// ---------------------------------------------------------------------------
// UPubnubClient::AddChannelToGroup - Automated tests
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddChannelToGroup_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.AddChannelToGroup.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddChannelToGroup_EmptyChannelGroup_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.AddChannelToGroup.1Validation.EmptyChannelGroup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddChannelToGroup_HappyPath_RequiredParamsOnly, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.AddChannelToGroup.2HappyPath.RequiredParamsOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddChannelToGroup_AddThenListChannels_ChannelInList, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.AddChannelToGroup.4Advanced.AddThenListChannels_ChannelInList",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddChannelToGroup_AddSameChannelTwice_StillSucceeds, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.AddChannelToGroup.4Advanced.AddSameChannelTwice_StillSucceeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::RemoveChannelFromGroup - Automated tests
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveChannelFromGroup_EmptyChannel_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.RemoveChannelFromGroup.1Validation.EmptyChannel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveChannelFromGroup_EmptyChannelGroup_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.RemoveChannelFromGroup.1Validation.EmptyChannelGroup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveChannelFromGroup_HappyPath_RequiredParamsOnly, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.RemoveChannelFromGroup.2HappyPath.RequiredParamsOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveChannelFromGroup_RemoveThenList_ChannelNotInList, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.RemoveChannelFromGroup.4Advanced.RemoveThenList_ChannelNotInList",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::ListChannelsFromGroup - Automated tests
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListChannelsFromGroup_EmptyChannelGroup_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.ListChannelsFromGroup.1Validation.EmptyChannelGroup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListChannelsFromGroup_HappyPath_RequiredParamsOnly, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.ListChannelsFromGroup.2HappyPath.RequiredParamsOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListChannelsFromGroup_EmptyGroup_ReturnsEmptyArray, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.ListChannelsFromGroup.4Advanced.EmptyGroup_ReturnsEmptyArray",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListChannelsFromGroup_MultipleChannels_ReturnsAll, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.ListChannelsFromGroup.4Advanced.MultipleChannels_ReturnsAll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::RemoveChannelGroup - Automated tests
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveChannelGroup_EmptyChannelGroup_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.RemoveChannelGroup.1Validation.EmptyChannelGroup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveChannelGroup_HappyPath_RequiredParamsOnly, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.RemoveChannelGroup.2HappyPath.RequiredParamsOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveChannelGroup_RemoveThenList_GroupGone, FPubnubAutomationTestBase,
	"Pubnub.Integration.ChannelGroup.RemoveChannelGroup.4Advanced.RemoveThenList_GroupGone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::AddChannelToGroup - Input validation and behaviour
// ---------------------------------------------------------------------------

bool FPubnubAddChannelToGroup_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + "add_grp_val_user");

	FPubnubOperationResult Result = PubnubClient->AddChannelToGroup(FString(), SDK_PREFIX + "group");

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention Channel", Result.ErrorMessage.Contains(TEXT("Channel")));
	TestTrue("ErrorMessage should indicate field is empty", Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubAddChannelToGroup_EmptyChannelGroup_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + "add_grp_val_user");

	FPubnubOperationResult Result = PubnubClient->AddChannelToGroup(SDK_PREFIX + "ch", FString());

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention ChannelGroup", Result.ErrorMessage.Contains(TEXT("ChannelGroup")));
	TestTrue("ErrorMessage should indicate field is empty", Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubAddChannelToGroup_HappyPath_RequiredParamsOnly::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "add_grp_happy_grp";
	const FString TestChannel = SDK_PREFIX + "add_grp_happy_ch";
	const FString TestUser = SDK_PREFIX + "add_grp_happy_user";

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

	FPubnubOperationResult Result = PubnubClient->AddChannelToGroup(TestChannel, TestGroup);

	TestFalse("AddChannelToGroup should succeed", Result.Error);
	TestEqual("Status should be 200", Result.Status, 200);

	// Cleanup: remove channel group (all created data must be cleared at the end)
	FPubnubOperationResult RemoveResult = PubnubClient->RemoveChannelGroup(TestGroup);
	TestFalse("RemoveChannelGroup cleanup should succeed", RemoveResult.Error);

	CleanUp();
	return true;
}

// AddChannelToGroup then ListChannelsFromGroup; verify returned Channels array contains the added channel.
bool FPubnubAddChannelToGroup_AddThenListChannels_ChannelInList::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "add_grp_list_grp";
	const FString TestChannel = SDK_PREFIX + "add_grp_list_ch";
	const FString TestUser = SDK_PREFIX + "add_grp_list_user";

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

	FPubnubListChannelsFromGroupResult ListResult = PubnubClient->ListChannelsFromGroup(TestGroup);
	TestFalse("ListChannelsFromGroup should succeed", ListResult.Result.Error);
	TestEqual("ListChannelsFromGroup status should be 200", ListResult.Result.Status, 200);
	TestEqual("Channels count should be 1", ListResult.Channels.Num(), 1);
	TestTrue("Channels should contain added channel", ListResult.Channels.Contains(TestChannel));

	FPubnubOperationResult RemoveResult = PubnubClient->RemoveChannelGroup(TestGroup);
	TestFalse("RemoveChannelGroup cleanup should succeed", RemoveResult.Error);

	CleanUp();
	return true;
}

// Adding the same channel to a group twice is idempotent; both calls succeed and List returns single entry.
bool FPubnubAddChannelToGroup_AddSameChannelTwice_StillSucceeds::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "add_grp_twice_grp";
	const FString TestChannel = SDK_PREFIX + "add_grp_twice_ch";
	const FString TestUser = SDK_PREFIX + "add_grp_twice_user";

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

	FPubnubOperationResult Add1 = PubnubClient->AddChannelToGroup(TestChannel, TestGroup);
	TestFalse("First AddChannelToGroup should succeed", Add1.Error);
	FPubnubOperationResult Add2 = PubnubClient->AddChannelToGroup(TestChannel, TestGroup);
	TestFalse("Second AddChannelToGroup (same channel) should succeed", Add2.Error);

	FPubnubListChannelsFromGroupResult ListResult = PubnubClient->ListChannelsFromGroup(TestGroup);
	TestFalse("ListChannelsFromGroup should succeed", ListResult.Result.Error);
	TestTrue("Channels should contain the channel", ListResult.Channels.Contains(TestChannel));

	FPubnubOperationResult RemoveResult = PubnubClient->RemoveChannelGroup(TestGroup);
	TestFalse("RemoveChannelGroup cleanup should succeed", RemoveResult.Error);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// UPubnubClient::RemoveChannelFromGroup - Input validation and behaviour
// ---------------------------------------------------------------------------

bool FPubnubRemoveChannelFromGroup_EmptyChannel_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + "rem_grp_val_user");

	FPubnubOperationResult Result = PubnubClient->RemoveChannelFromGroup(FString(), SDK_PREFIX + "group");

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention Channel", Result.ErrorMessage.Contains(TEXT("Channel")));
	TestTrue("ErrorMessage should indicate field is empty", Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubRemoveChannelFromGroup_EmptyChannelGroup_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + "rem_grp_val_user");

	FPubnubOperationResult Result = PubnubClient->RemoveChannelFromGroup(SDK_PREFIX + "ch", FString());

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention ChannelGroup", Result.ErrorMessage.Contains(TEXT("ChannelGroup")));
	TestTrue("ErrorMessage should indicate field is empty", Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubRemoveChannelFromGroup_HappyPath_RequiredParamsOnly::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "rem_grp_happy_grp";
	const FString TestChannel = SDK_PREFIX + "rem_grp_happy_ch";
	const FString TestUser = SDK_PREFIX + "rem_grp_happy_user";

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

	FPubnubOperationResult RemoveResult = PubnubClient->RemoveChannelFromGroup(TestChannel, TestGroup);
	TestFalse("RemoveChannelFromGroup should succeed", RemoveResult.Error);
	TestEqual("Status should be 200", RemoveResult.Status, 200);

	FPubnubListChannelsFromGroupResult ListResult = PubnubClient->ListChannelsFromGroup(TestGroup);
	TestFalse("ListChannelsFromGroup should succeed", ListResult.Result.Error);
	TestEqual("Channels should be empty after remove", ListResult.Channels.Num(), 0);

	FPubnubOperationResult RemoveGrpResult = PubnubClient->RemoveChannelGroup(TestGroup);
	TestFalse("RemoveChannelGroup cleanup should succeed", RemoveGrpResult.Error);

	CleanUp();
	return true;
}

// RemoveChannelFromGroup then ListChannelsFromGroup; verify channel is no longer in the list.
bool FPubnubRemoveChannelFromGroup_RemoveThenList_ChannelNotInList::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "rem_grp_list_grp";
	const FString TestChannel = SDK_PREFIX + "rem_grp_list_ch";
	const FString TestUser = SDK_PREFIX + "rem_grp_list_user";

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

	FPubnubOperationResult RemoveResult = PubnubClient->RemoveChannelFromGroup(TestChannel, TestGroup);
	TestFalse("RemoveChannelFromGroup should succeed", RemoveResult.Error);

	FPubnubListChannelsFromGroupResult ListResult = PubnubClient->ListChannelsFromGroup(TestGroup);
	TestFalse("ListChannelsFromGroup should succeed", ListResult.Result.Error);
	TestFalse("Channels should not contain removed channel", ListResult.Channels.Contains(TestChannel));
	TestEqual("Channels count should be 0", ListResult.Channels.Num(), 0);

	FPubnubOperationResult RemoveGrpResult = PubnubClient->RemoveChannelGroup(TestGroup);
	TestFalse("RemoveChannelGroup cleanup should succeed", RemoveGrpResult.Error);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// UPubnubClient::ListChannelsFromGroup - Input validation and behaviour
// ---------------------------------------------------------------------------

bool FPubnubListChannelsFromGroup_EmptyChannelGroup_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + "list_grp_val_user");

	FPubnubListChannelsFromGroupResult Result = PubnubClient->ListChannelsFromGroup(FString());

	TestTrue("Result should indicate error", Result.Result.Error);
	TestTrue("ErrorMessage should mention ChannelGroup", Result.Result.ErrorMessage.Contains(TEXT("ChannelGroup")));
	TestTrue("ErrorMessage should indicate field is empty", Result.Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubListChannelsFromGroup_HappyPath_RequiredParamsOnly::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "list_grp_happy_grp";
	const FString TestChannel = SDK_PREFIX + "list_grp_happy_ch";
	const FString TestUser = SDK_PREFIX + "list_grp_happy_user";

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

	FPubnubListChannelsFromGroupResult ListResult = PubnubClient->ListChannelsFromGroup(TestGroup);
	TestFalse("ListChannelsFromGroup should succeed", ListResult.Result.Error);
	TestEqual("Status should be 200", ListResult.Result.Status, 200);
	TestEqual("Channels count should be 1", ListResult.Channels.Num(), 1);
	TestEqual("Channels[0] should match added channel", ListResult.Channels[0], TestChannel);

	FPubnubOperationResult RemoveResult = PubnubClient->RemoveChannelGroup(TestGroup);
	TestFalse("RemoveChannelGroup cleanup should succeed", RemoveResult.Error);

	CleanUp();
	return true;
}

// ListChannelsFromGroup for a group with no channels (add then remove channel) returns success and empty array.
bool FPubnubListChannelsFromGroup_EmptyGroup_ReturnsEmptyArray::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "list_grp_empty_grp";
	const FString TestChannel = SDK_PREFIX + "list_grp_empty_ch";
	const FString TestUser = SDK_PREFIX + "list_grp_empty_user";

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
	FPubnubOperationResult RemoveChResult = PubnubClient->RemoveChannelFromGroup(TestChannel, TestGroup);
	TestFalse("RemoveChannelFromGroup should succeed", RemoveChResult.Error);

	FPubnubListChannelsFromGroupResult ListResult = PubnubClient->ListChannelsFromGroup(TestGroup);
	TestFalse("ListChannelsFromGroup should succeed", ListResult.Result.Error);
	TestEqual("Channels should be empty", ListResult.Channels.Num(), 0);

	FPubnubOperationResult RemoveResult = PubnubClient->RemoveChannelGroup(TestGroup);
	TestFalse("RemoveChannelGroup cleanup should succeed", RemoveResult.Error);

	CleanUp();
	return true;
}

// Add multiple channels to group; ListChannelsFromGroup returns all of them in correct form.
bool FPubnubListChannelsFromGroup_MultipleChannels_ReturnsAll::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "list_grp_multi_grp";
	const FString ChA = SDK_PREFIX + "list_grp_multi_a";
	const FString ChB = SDK_PREFIX + "list_grp_multi_b";
	const FString ChC = SDK_PREFIX + "list_grp_multi_c";
	const FString TestUser = SDK_PREFIX + "list_grp_multi_user";

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

	FPubnubOperationResult AddA = PubnubClient->AddChannelToGroup(ChA, TestGroup);
	TestFalse("AddChannelToGroup A should succeed", AddA.Error);
	FPubnubOperationResult AddB = PubnubClient->AddChannelToGroup(ChB, TestGroup);
	TestFalse("AddChannelToGroup B should succeed", AddB.Error);
	FPubnubOperationResult AddC = PubnubClient->AddChannelToGroup(ChC, TestGroup);
	TestFalse("AddChannelToGroup C should succeed", AddC.Error);

	FPubnubListChannelsFromGroupResult ListResult = PubnubClient->ListChannelsFromGroup(TestGroup);
	TestFalse("ListChannelsFromGroup should succeed", ListResult.Result.Error);
	TestEqual("Channels count should be 3", ListResult.Channels.Num(), 3);
	TestTrue("Channels should contain ChA", ListResult.Channels.Contains(ChA));
	TestTrue("Channels should contain ChB", ListResult.Channels.Contains(ChB));
	TestTrue("Channels should contain ChC", ListResult.Channels.Contains(ChC));

	FPubnubOperationResult RemoveResult = PubnubClient->RemoveChannelGroup(TestGroup);
	TestFalse("RemoveChannelGroup cleanup should succeed", RemoveResult.Error);

	CleanUp();
	return true;
}

// ---------------------------------------------------------------------------
// UPubnubClient::RemoveChannelGroup - Input validation and behaviour
// ---------------------------------------------------------------------------

bool FPubnubRemoveChannelGroup_EmptyChannelGroup_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + "rem_grp_grp_val_user");

	FPubnubOperationResult Result = PubnubClient->RemoveChannelGroup(FString());

	TestTrue("Result should indicate error", Result.Error);
	TestTrue("ErrorMessage should mention ChannelGroup", Result.ErrorMessage.Contains(TEXT("ChannelGroup")));
	TestTrue("ErrorMessage should indicate field is empty", Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubRemoveChannelGroup_HappyPath_RequiredParamsOnly::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "rem_grp_grp_happy_grp";
	const FString TestChannel = SDK_PREFIX + "rem_grp_grp_happy_ch";
	const FString TestUser = SDK_PREFIX + "rem_grp_grp_happy_user";

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

	FPubnubOperationResult RemoveResult = PubnubClient->RemoveChannelGroup(TestGroup);
	TestFalse("RemoveChannelGroup should succeed", RemoveResult.Error);
	TestEqual("Status should be 200", RemoveResult.Status, 200);

	CleanUp();
	return true;
}

// RemoveChannelGroup then ListChannelsFromGroup for same group; list should return empty or success with no channels (group removed).
bool FPubnubRemoveChannelGroup_RemoveThenList_GroupGone::RunTest(const FString& Parameters)
{
	const FString TestGroup = SDK_PREFIX + "rem_grp_grp_gone_grp";
	const FString TestChannel = SDK_PREFIX + "rem_grp_grp_gone_ch";
	const FString TestUser = SDK_PREFIX + "rem_grp_grp_gone_user";

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

	FPubnubOperationResult RemoveResult = PubnubClient->RemoveChannelGroup(TestGroup);
	TestFalse("RemoveChannelGroup should succeed", RemoveResult.Error);

	// After removing group, listing it may return success with empty channels or an error depending on server behaviour; we only require no crash and consistent result.
	FPubnubListChannelsFromGroupResult ListResult = PubnubClient->ListChannelsFromGroup(TestGroup);
	// Either success with empty list or error is acceptable; we verify we get a valid response structure.
	TestEqual("Channels should be empty after group removal", ListResult.Channels.Num(), 0);

	CleanUp();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
