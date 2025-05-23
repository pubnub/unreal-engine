// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "FunctionLibraries/PubnubUtilities.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAddQuotesToStringUnitTest, "Pubnub.aUnit.Utilities.AddQuotesToString", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMembershipIncludeToStringUnitTest, "Pubnub.aUnit.Utilities.MembershipIncludeToString", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemberIncludeToStringUnitTest, "Pubnub.aUnit.Utilities.MemberIncludeToString", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetAllIncludeToStringUnitTest, "Pubnub.aUnit.Utilities.GetAllIncludeToString", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMembershipSortToStringUnitTest, "Pubnub.aUnit.Utilities.MembershipSortToString", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemberSortToStringUnitTest, "Pubnub.aUnit.Utilities.MemberSortToString", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetAllSortToStringUnitTest, "Pubnub.aUnit.Utilities.GetAllSortToString", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FListChannelsFromGroupJsonToDataUnitTest, "Pubnub.aUnit.JsonUtilities.ListChannelsFromGroupJsonToData", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FListUserSubscribedChannelsJsonToDataUnitTest, "Pubnub.aUnit.JsonUtilities.ListUserSubscribedChannelsJsonToData", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FListUsersFromChannelJsonToDataUnitTest, "Pubnub.aUnit.JsonUtilities.ListUsersFromChannelJsonToData", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFetchHistoryJsonToDataUnitTest, "Pubnub.aUnit.JsonUtilities.FetchHistoryJsonToData", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetAllUserMetadataJsonToDataUnitTest, "Pubnub.aUnit.JsonUtilities.GetAllUserMetadataJsonToData", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetUserMetadataJsonToDataUnitTest, "Pubnub.aUnit.JsonUtilities.GetUserMetadataJsonToData", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetAllChannelMetadataJsonToDataUnitTest, "Pubnub.aUnit.JsonUtilities.GetAllChannelMetadataJsonToData", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetChannelMetadataJsonToDataUnitTest, "Pubnub.aUnit.JsonUtilities.GetChannelMetadataJsonToData", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetMessageActionsJsonToDataUnitTest, "Pubnub.aUnit.JsonUtilities.GetMessageActionsJsonToData", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetMembershipsJsonToDataUnitTest, "Pubnub.aUnit.JsonUtilities.GetMembershipsJsonToData", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetChannelMembersJsonToDataUnitTest, "Pubnub.aUnit.JsonUtilities.GetChannelMembersJsonToData", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);



bool FAddQuotesToStringUnitTest::RunTest(const FString& Parameters)
{
	// Test adding quotes to unquoted string
	FString Test = "abc";
	FString TestWithQuotes = UPubnubUtilities::AddQuotesToString(Test);
	TestEqual("Adding quotes to unquoted string failed", TestWithQuotes, "\"abc\"");

	// Test adding quotes to already quoted string with SkipIfHasQuotes = true
	FString Test2 = "\"abc\"";
	FString Test2WithQuotes = UPubnubUtilities::AddQuotesToString(Test2, true);
	TestEqual("Adding quotes to quoted string with SkipIfHasQuotes=true failed", Test2WithQuotes, "\"abc\"");

	// Test adding quotes to already quoted string with SkipIfHasQuotes = false
	FString Test3 = "\"abc\"";
	FString Test3WithQuotes = UPubnubUtilities::AddQuotesToString(Test3, false);
	TestEqual("Adding quotes to quoted string with SkipIfHasQuotes=false failed", Test3WithQuotes, "\"\"abc\"\"");

	// Test empty string
	FString Test4 = "";
	FString Test4WithQuotes = UPubnubUtilities::AddQuotesToString(Test4);
	TestEqual("Adding quotes to empty string failed", Test4WithQuotes, "\"\"");

	return true;
}

bool FMembershipIncludeToStringUnitTest::RunTest(const FString& Parameters)
{
	// Test all includes set to true
	FPubnubMembershipInclude MembershipInclude1 = FPubnubMembershipInclude::FromValue(true);
	FString ExpectedValue1 = "custom,status,type,channel,channel.custom,channel.status,channel.type";
	FString TestValue1 = UPubnubUtilities::MembershipIncludeToString(MembershipInclude1);
	TestEqual("FMembershipIncludeToString failed for all includes true", TestValue1, ExpectedValue1);

	// Test all includes empty
	FPubnubMembershipInclude MembershipInclude2;
	FString ExpectedValue2 = "";
	FString TestValue2 = UPubnubUtilities::MembershipIncludeToString(MembershipInclude2);
	TestEqual("FMembershipIncludeToString failed for all includes empty", TestValue2, ExpectedValue2);

	// Test some values set to true
	FPubnubMembershipInclude MembershipInclude3{
		.IncludeType = true,
		.IncludeChannel = true,
		.IncludeChannelStatus = true,
		.IncludeTotalCount = true
	};
	FString ExpectedValue3 = "type,channel,channel.status";
	FString TestValue3 = UPubnubUtilities::MembershipIncludeToString(MembershipInclude3);
	TestEqual("FMembershipIncludeToString failed for some includes true", TestValue3, ExpectedValue3);

	// Test single include
	FPubnubMembershipInclude MembershipInclude4;
	MembershipInclude4.IncludeCustom = true;
	FString ExpectedValue4 = "custom";
	FString TestValue4 = UPubnubUtilities::MembershipIncludeToString(MembershipInclude4);
	TestEqual("FMembershipIncludeToString failed for single include", TestValue4, ExpectedValue4);

	return true;
}

bool FMemberIncludeToStringUnitTest::RunTest(const FString& Parameters)
{
	// Test all includes set to true
	FPubnubMemberInclude MemberInclude1 = FPubnubMemberInclude::FromValue(true);
	FString ExpectedValue1 = "custom,status,type,uuid,uuid.custom,uuid.status,uuid.type";
	FString TestValue1 = UPubnubUtilities::MemberIncludeToString(MemberInclude1);
	TestEqual("FMemberIncludeToString failed for all includes true", TestValue1, ExpectedValue1);

	// Test all includes empty
	FPubnubMemberInclude MemberInclude2;
	FString ExpectedValue2 = "";
	FString TestValue2 = UPubnubUtilities::MemberIncludeToString(MemberInclude2);
	TestEqual("FMemberIncludeToString failed for all includes empty", TestValue2, ExpectedValue2);

	// Test some values set to true
	FPubnubMemberInclude MemberInclude3{
		.IncludeStatus = true,
		.IncludeUser = true,
		.IncludeUserType = true,
		.IncludeTotalCount = false
	};
	FString ExpectedValue3 = "status,uuid,uuid.type";
	FString TestValue3 = UPubnubUtilities::MemberIncludeToString(MemberInclude3);
	TestEqual("FMemberIncludeToString failed for some includes true", TestValue3, ExpectedValue3);

	// Test single include
	FPubnubMemberInclude MemberInclude4;
	MemberInclude4.IncludeCustom = true;
	FString ExpectedValue4 = "custom";
	FString TestValue4 = UPubnubUtilities::MemberIncludeToString(MemberInclude4);
	TestEqual("FMemberIncludeToString failed for single include", TestValue4, ExpectedValue4);

	return true;
}

bool FGetAllIncludeToStringUnitTest::RunTest(const FString& Parameters)
{
	// Test all includes set to true
	FPubnubGetAllInclude GetAllInclude1 = FPubnubGetAllInclude::FromValue(true);
	FString ExpectedValue1 = "custom,status,type";
	FString TestValue1 = UPubnubUtilities::GetAllIncludeToString(GetAllInclude1);
	TestEqual("FGetAllIncludeToString failed for all includes true", TestValue1, ExpectedValue1);

	// Test all includes empty
	FPubnubGetAllInclude GetAllInclude2;
	FString ExpectedValue2 = "";
	FString TestValue2 = UPubnubUtilities::GetAllIncludeToString(GetAllInclude2);
	TestEqual("FGetAllIncludeToString failed for all includes empty", TestValue2, ExpectedValue2);

	// Test some values set to true
	FPubnubGetAllInclude GetAllInclude3{
		.IncludeCustom = true,
		.IncludeType = true
	};
	FString ExpectedValue3 = "custom,type";
	FString TestValue3 = UPubnubUtilities::GetAllIncludeToString(GetAllInclude3);
	TestEqual("FGetAllIncludeToString failed for some includes true", TestValue3, ExpectedValue3);

	// Test single include
	FPubnubGetAllInclude GetAllInclude4;
	GetAllInclude4.IncludeStatus = true;
	FString ExpectedValue4 = "status";
	FString TestValue4 = UPubnubUtilities::GetAllIncludeToString(GetAllInclude4);
	TestEqual("FGetAllIncludeToString failed for single include", TestValue4, ExpectedValue4);

	return true;
}

bool FMembershipSortToStringUnitTest::RunTest(const FString& Parameters)
{
	// Test empty sort
	FPubnubMembershipSort EmptySort;
	FString ExpectedValue1 = "";
	FString TestValue1 = UPubnubUtilities::MembershipSortToString(EmptySort);
	TestEqual("FMembershipSortToString failed for empty sort", TestValue1, ExpectedValue1);

	// Test single sort ascending
	FPubnubMembershipSort SingleSortAsc;
	FPubnubMembershipSingleSort SingleSortItem;
	SingleSortItem.SortType = EPubnubMembershipSortType::PMST_ChannelID;
	SingleSortItem.SortOrder = false;
	SingleSortAsc.MembershipSort.Add(SingleSortItem);
	
	FString ExpectedValue2 = "channel.id";
	FString TestValue2 = UPubnubUtilities::MembershipSortToString(SingleSortAsc);
	TestEqual("FMembershipSortToString failed for single ascending sort", TestValue2, ExpectedValue2);

	// Test single sort descending
	FPubnubMembershipSort SingleSortDesc;
	SingleSortItem.SortOrder = true;
	SingleSortDesc.MembershipSort.Add(SingleSortItem);
	
	FString ExpectedValue3 = "channel.id:desc";
	FString TestValue3 = UPubnubUtilities::MembershipSortToString(SingleSortDesc);
	TestEqual("FMembershipSortToString failed for single descending sort", TestValue3, ExpectedValue3);

	// Test multiple sorts with different types
	FPubnubMembershipSort MultipleSorts;
	FPubnubMembershipSingleSort SortItem1;
	SortItem1.SortType = EPubnubMembershipSortType::PMST_ChannelID;
	SortItem1.SortOrder = false;
	FPubnubMembershipSingleSort SortItem2;
	SortItem2.SortType = EPubnubMembershipSortType::PMST_ChannelName;
	SortItem2.SortOrder = true;
	FPubnubMembershipSingleSort SortItem3;
	SortItem3.SortType = EPubnubMembershipSortType::PMST_ChannelStatus;
	SortItem3.SortOrder = false;
	
	MultipleSorts.MembershipSort.Add(SortItem1);
	MultipleSorts.MembershipSort.Add(SortItem2);
	MultipleSorts.MembershipSort.Add(SortItem3);
	
	FString ExpectedValue4 = "channel.id,channel.name:desc,channel.status";
	FString TestValue4 = UPubnubUtilities::MembershipSortToString(MultipleSorts);
	TestEqual("FMembershipSortToString failed for multiple sorts", TestValue4, ExpectedValue4);

	// Test multiple sorts with same type but different orders
	FPubnubMembershipSort SameTypeSorts;
	FPubnubMembershipSingleSort SortItem4;
	SortItem4.SortType = EPubnubMembershipSortType::PMST_ChannelID;
	SortItem4.SortOrder = false;
	FPubnubMembershipSingleSort SortItem5;
	SortItem5.SortType = EPubnubMembershipSortType::PMST_ChannelID;
	SortItem5.SortOrder = true;
	
	SameTypeSorts.MembershipSort.Add(SortItem4);
	SameTypeSorts.MembershipSort.Add(SortItem5);
	
	FString ExpectedValue5 = "channel.id,channel.id:desc";
	FString TestValue5 = UPubnubUtilities::MembershipSortToString(SameTypeSorts);
	TestEqual("FMembershipSortToString failed for same type different orders", TestValue5, ExpectedValue5);

	return true;
}

bool FMemberSortToStringUnitTest::RunTest(const FString& Parameters)
{
	// Test empty sort
	FPubnubMemberSort EmptySort;
	FString ExpectedValue1 = "";
	FString TestValue1 = UPubnubUtilities::MemberSortToString(EmptySort);
	TestEqual("FMemberSortToString failed for empty sort", TestValue1, ExpectedValue1);

	// Test single sort ascending
	FPubnubMemberSort SingleSortAsc;
	FPubnubMemberSingleSort SingleSortItem;
	SingleSortItem.SortType = EPubnubMemberSortType::PMeST_UserID;
	SingleSortItem.SortOrder = false;
	SingleSortAsc.MemberSort.Add(SingleSortItem);
	
	FString ExpectedValue2 = "uuid.id";
	FString TestValue2 = UPubnubUtilities::MemberSortToString(SingleSortAsc);
	TestEqual("FMemberSortToString failed for single ascending sort", TestValue2, ExpectedValue2);

	// Test single sort descending
	FPubnubMemberSort SingleSortDesc;
	SingleSortItem.SortOrder = true;
	SingleSortDesc.MemberSort.Add(SingleSortItem);
	
	FString ExpectedValue3 = "uuid.id:desc";
	FString TestValue3 = UPubnubUtilities::MemberSortToString(SingleSortDesc);
	TestEqual("FMemberSortToString failed for single descending sort", TestValue3, ExpectedValue3);

	// Test multiple sorts with different types
	FPubnubMemberSort MultipleSorts;
	FPubnubMemberSingleSort SortItem1;
	SortItem1.SortType = EPubnubMemberSortType::PMeST_UserID;
	SortItem1.SortOrder = false;
	FPubnubMemberSingleSort SortItem2;
	SortItem2.SortType = EPubnubMemberSortType::PMeST_UserName;
	SortItem2.SortOrder = true;
	FPubnubMemberSingleSort SortItem3;
	SortItem3.SortType = EPubnubMemberSortType::PMeST_UserStatus;
	SortItem3.SortOrder = false;
	
	MultipleSorts.MemberSort.Add(SortItem1);
	MultipleSorts.MemberSort.Add(SortItem2);
	MultipleSorts.MemberSort.Add(SortItem3);
	
	FString ExpectedValue4 = "uuid.id,uuid.name:desc,uuid.status";
	FString TestValue4 = UPubnubUtilities::MemberSortToString(MultipleSorts);
	TestEqual("FMemberSortToString failed for multiple sorts", TestValue4, ExpectedValue4);

	// Test multiple sorts with same type but different orders
	FPubnubMemberSort SameTypeSorts;
	FPubnubMemberSingleSort SortItem4;
	SortItem4.SortType = EPubnubMemberSortType::PMeST_UserID;
	SortItem4.SortOrder = false;
	FPubnubMemberSingleSort SortItem5;
	SortItem5.SortType = EPubnubMemberSortType::PMeST_UserID;
	SortItem5.SortOrder = true;
	
	SameTypeSorts.MemberSort.Add(SortItem4);
	SameTypeSorts.MemberSort.Add(SortItem5);
	
	FString ExpectedValue5 = "uuid.id,uuid.id:desc";
	FString TestValue5 = UPubnubUtilities::MemberSortToString(SameTypeSorts);
	TestEqual("FMemberSortToString failed for same type different orders", TestValue5, ExpectedValue5);

	return true;
}

bool FGetAllSortToStringUnitTest::RunTest(const FString& Parameters)
{
	// Test empty sort
	FPubnubGetAllSort EmptySort;
	FString ExpectedValue1 = "";
	FString TestValue1 = UPubnubUtilities::GetAllSortToString(EmptySort);
	TestEqual("FGetAllSortToString failed for empty sort", TestValue1, ExpectedValue1);

	// Test single sort ascending
	FPubnubGetAllSort SingleSortAsc;
	FPubnubGetAllSingleSort SingleSortItem;
	SingleSortItem.SortType = EPubnubGetAllSortType::PGAST_ID;
	SingleSortItem.SortOrder = false;
	SingleSortAsc.GetAllSort.Add(SingleSortItem);
	
	FString ExpectedValue2 = "id";
	FString TestValue2 = UPubnubUtilities::GetAllSortToString(SingleSortAsc);
	TestEqual("FGetAllSortToString failed for single ascending sort", TestValue2, ExpectedValue2);

	// Test single sort descending
	FPubnubGetAllSort SingleSortDesc;
	SingleSortItem.SortOrder = true;
	SingleSortDesc.GetAllSort.Add(SingleSortItem);
	
	FString ExpectedValue3 = "id:desc";
	FString TestValue3 = UPubnubUtilities::GetAllSortToString(SingleSortDesc);
	TestEqual("FGetAllSortToString failed for single descending sort", TestValue3, ExpectedValue3);

	// Test multiple sorts with different types
	FPubnubGetAllSort MultipleSorts;
	FPubnubGetAllSingleSort SortItem1;
	SortItem1.SortType = EPubnubGetAllSortType::PGAST_ID;
	SortItem1.SortOrder = false;
	FPubnubGetAllSingleSort SortItem2;
	SortItem2.SortType = EPubnubGetAllSortType::PGAST_Name;
	SortItem2.SortOrder = true;
	FPubnubGetAllSingleSort SortItem3;
	SortItem3.SortType = EPubnubGetAllSortType::PGAST_Status;
	SortItem3.SortOrder = false;
	
	MultipleSorts.GetAllSort.Add(SortItem1);
	MultipleSorts.GetAllSort.Add(SortItem2);
	MultipleSorts.GetAllSort.Add(SortItem3);
	
	FString ExpectedValue4 = "id,name:desc,status";
	FString TestValue4 = UPubnubUtilities::GetAllSortToString(MultipleSorts);
	TestEqual("FGetAllSortToString failed for multiple sorts", TestValue4, ExpectedValue4);

	// Test multiple sorts with same type but different orders
	FPubnubGetAllSort SameTypeSorts;
	FPubnubGetAllSingleSort SortItem4;
	SortItem4.SortType = EPubnubGetAllSortType::PGAST_ID;
	SortItem4.SortOrder = false;
	FPubnubGetAllSingleSort SortItem5;
	SortItem5.SortType = EPubnubGetAllSortType::PGAST_ID;
	SortItem5.SortOrder = true;
	
	SameTypeSorts.GetAllSort.Add(SortItem4);
	SameTypeSorts.GetAllSort.Add(SortItem5);
	
	FString ExpectedValue5 = "id,id:desc";
	FString TestValue5 = UPubnubUtilities::GetAllSortToString(SameTypeSorts);
	TestEqual("FGetAllSortToString failed for same type different orders", TestValue5, ExpectedValue5);

	// Test all possible sort types
	FPubnubGetAllSort AllTypesSort;
	FPubnubGetAllSingleSort SortItem6;
	SortItem6.SortType = EPubnubGetAllSortType::PGAST_ID;
	SortItem6.SortOrder = false;
	FPubnubGetAllSingleSort SortItem7;
	SortItem7.SortType = EPubnubGetAllSortType::PGAST_Name;
	SortItem7.SortOrder = false;
	FPubnubGetAllSingleSort SortItem8;
	SortItem8.SortType = EPubnubGetAllSortType::PGAST_Updated;
	SortItem8.SortOrder = false;
	FPubnubGetAllSingleSort SortItem9;
	SortItem9.SortType = EPubnubGetAllSortType::PGAST_Status;
	SortItem9.SortOrder = false;
	FPubnubGetAllSingleSort SortItem10;
	SortItem10.SortType = EPubnubGetAllSortType::PGAST_Type;
	SortItem10.SortOrder = false;
	
	AllTypesSort.GetAllSort.Add(SortItem6);
	AllTypesSort.GetAllSort.Add(SortItem7);
	AllTypesSort.GetAllSort.Add(SortItem8);
	AllTypesSort.GetAllSort.Add(SortItem9);
	AllTypesSort.GetAllSort.Add(SortItem10);
	
	FString ExpectedValue6 = "id,name,updated,status,type";
	FString TestValue6 = UPubnubUtilities::GetAllSortToString(AllTypesSort);
	TestEqual("FGetAllSortToString failed for all sort types", TestValue6, ExpectedValue6);

	return true;
}

bool FListChannelsFromGroupJsonToDataUnitTest::RunTest(const FString& Parameters)
{
	//Test successful response
	FString TestJson = "{\"error\":false,\"payload\":{\"channels\":[\"my_channel\",\"my_channel2\"],\"group\":\"my_group\"},\"service\":\"channel-registry\",\"status\":200}";
	bool Error = true;
	int Status = 0;
	TArray<FString> Channels;
	
	UPubnubJsonUtilities::ListChannelsFromGroupJsonToData(TestJson, Error, Status, Channels);
	
	//Verify error flag
	TestTrue("Error flag should be false", !Error);
	
	//Verify status code
	TestEqual("Status code should be 200", Status, 200);
	
	//Verify channels array
	TestEqual("Channels array should have 2 elements", Channels.Num(), 2);
	TestEqual("First channel should be 'my_channel'", Channels[0], "my_channel");
	TestEqual("Second channel should be 'my_channel2'", Channels[1], "my_channel2");

	//Test error response
	FString TestErrorJson = "{\"error\":true,\"payload\":{\"channels\":[],\"group\":\"my_group\"},\"service\":\"channel-registry\",\"status\":400}";
	Error = false;
	Status = 0;
	Channels.Empty();
	
	UPubnubJsonUtilities::ListChannelsFromGroupJsonToData(TestErrorJson, Error, Status, Channels);
	
	//Verify error flag
	TestTrue("Error flag should be true", Error);
	
	//Verify status code
	TestEqual("Status code should be 400", Status, 400);
	
	//Verify channels array is empty
	TestEqual("Channels array should be empty", Channels.Num(), 0);

	//Test invalid JSON
	FString TestInvalidJson = "invalid json";
	Error = false;
	Status = 0;
	Channels.Empty();
	
	UPubnubJsonUtilities::ListChannelsFromGroupJsonToData(TestInvalidJson, Error, Status, Channels);
	
	//Verify error flag
	TestTrue("Error flag should be true for invalid JSON", Error);
	
	//Verify channels array is empty
	TestEqual("Channels array should be empty for invalid JSON", Channels.Num(), 0);
	
	return true;
}

bool FListUserSubscribedChannelsJsonToDataUnitTest::RunTest(const FString& Parameters)
{
	//Test successful response
	FString TestJson = "{\"status\": 200, \"message\": \"OK\", \"payload\": {\"channels\": [\"my_channel\"]}, \"service\": \"Presence\"}";
	int Status = 0;
	FString Message;
	TArray<FString> Channels;
	
	UPubnubJsonUtilities::ListUserSubscribedChannelsJsonToData(TestJson, Status, Message, Channels);
	
	//Verify status code
	TestEqual("Status code should be 200", Status, 200);
	
	//Verify message
	TestEqual("Message should be 'OK'", Message, "OK");
	
	//Verify channels array
	TestEqual("Channels array should have 1 element", Channels.Num(), 1);
	TestEqual("Channel should be 'my_channel'", Channels[0], "my_channel");

	//Test multiple channels response
	FString TestMultipleChannelsJson = "{\"status\": 200, \"message\": \"OK\", \"payload\": {\"channels\": [\"my_channel\", \"my_channel2\", \"my_channel3\"]}, \"service\": \"Presence\"}";
	Status = 0;
	Message.Empty();
	Channels.Empty();
	
	UPubnubJsonUtilities::ListUserSubscribedChannelsJsonToData(TestMultipleChannelsJson, Status, Message, Channels);
	
	//Verify status code
	TestEqual("Status code should be 200 for multiple channels", Status, 200);
	
	//Verify message
	TestEqual("Message should be 'OK' for multiple channels", Message, "OK");
	
	//Verify channels array
	TestEqual("Channels array should have 3 elements", Channels.Num(), 3);
	TestEqual("First channel should be 'my_channel'", Channels[0], "my_channel");
	TestEqual("Second channel should be 'my_channel2'", Channels[1], "my_channel2");
	TestEqual("Third channel should be 'my_channel3'", Channels[2], "my_channel3");

	//Test error response
	FString TestErrorJson = "{\"status\": 400, \"message\": \"Bad Request\", \"payload\": {\"channels\": []}, \"service\": \"Presence\"}";
	Status = 0;
	Message.Empty();
	Channels.Empty();
	
	UPubnubJsonUtilities::ListUserSubscribedChannelsJsonToData(TestErrorJson, Status, Message, Channels);
	
	//Verify status code
	TestEqual("Status code should be 400 for error response", Status, 400);
	
	//Verify message
	TestEqual("Message should be 'Bad Request' for error response", Message, "Bad Request");
	
	//Verify channels array is empty
	TestEqual("Channels array should be empty for error response", Channels.Num(), 0);

	//Test invalid JSON
	FString TestInvalidJson = "invalid json";
	Status = 0;
	Message.Empty();
	Channels.Empty();
	
	UPubnubJsonUtilities::ListUserSubscribedChannelsJsonToData(TestInvalidJson, Status, Message, Channels);
	
	//Verify channels array is empty
	TestEqual("Channels array should be empty for invalid JSON", Channels.Num(), 0);
	
	return true;
}

bool FListUsersFromChannelJsonToDataUnitTest::RunTest(const FString& Parameters)
{
	//Test basic response with user list
	FString TestBasicJson = "{\"status\": 200, \"message\": \"OK\", \"occupancy\": 2, \"uuids\": [\"User2\", \"User1\"], \"service\": \"Presence\"}";
	int Status = 0;
	FString Message;
	FPubnubListUsersFromChannelWrapper Data;
	
	UPubnubJsonUtilities::ListUsersFromChannelJsonToData(TestBasicJson, Status, Message, Data);
	
	//Verify status code
	TestEqual("Status code should be 200 for basic response", Status, 200);
	
	//Verify message
	TestEqual("Message should be 'OK' for basic response", Message, "OK");
	
	//Verify occupancy
	TestEqual("Occupancy should be 2 for basic response", Data.Occupancy, 2);
	
	//Verify users state map
	TestEqual("UsersState map should have 2 elements for basic response", Data.UsersState.Num(), 2);
	TestTrue("UsersState should contain User1", Data.UsersState.Contains("User1"));
	TestTrue("UsersState should contain User2", Data.UsersState.Contains("User2"));
	TestEqual("User1 should have empty state", Data.UsersState["User1"], "");
	TestEqual("User2 should have empty state", Data.UsersState["User2"], "");

	//Test response with user states
	FString TestStatesJson = "{\"status\": 200, \"message\": \"OK\", \"occupancy\": 2, \"uuids\": [{\"uuid\": \"User2\"}, {\"uuid\": \"User1\", \"state\": {\"state\": \"new state\"}}], \"service\": \"Presence\"}";
	Status = 0;
	Message.Empty();
	Data = FPubnubListUsersFromChannelWrapper();
	
	UPubnubJsonUtilities::ListUsersFromChannelJsonToData(TestStatesJson, Status, Message, Data);
	
	//Verify status code
	TestEqual("Status code should be 200 for states response", Status, 200);
	
	//Verify message
	TestEqual("Message should be 'OK' for states response", Message, "OK");
	
	//Verify occupancy
	TestEqual("Occupancy should be 2 for states response", Data.Occupancy, 2);
	
	//Verify users state map
	TestEqual("UsersState map should have 2 elements for states response", Data.UsersState.Num(), 2);
	TestTrue("UsersState should contain User1", Data.UsersState.Contains("User1"));
	TestTrue("UsersState should contain User2", Data.UsersState.Contains("User2"));
	TestEqual("User1 should have 'new state'", Data.UsersState["User1"], "{\"state\":\"new state\"}");
	TestEqual("User2 should have empty state", Data.UsersState["User2"], "");

	//Test response with disabled UUIDs
	FString TestDisabledUuidsJson = "{\"status\": 200, \"message\": \"OK\", \"occupancy\": 2, \"service\": \"Presence\"}";
	Status = 0;
	Message.Empty();
	Data = FPubnubListUsersFromChannelWrapper();
	
	UPubnubJsonUtilities::ListUsersFromChannelJsonToData(TestDisabledUuidsJson, Status, Message, Data);
	
	//Verify status code
	TestEqual("Status code should be 200 for disabled UUIDs response", Status, 200);
	
	//Verify message
	TestEqual("Message should be 'OK' for disabled UUIDs response", Message, "OK");
	
	//Verify occupancy
	TestEqual("Occupancy should be 2 for disabled UUIDs response", Data.Occupancy, 2);
	
	//Verify users state map is empty
	TestEqual("UsersState map should be empty for disabled UUIDs response", Data.UsersState.Num(), 0);

	//Test error response
	FString TestErrorJson = "{\"status\": 400, \"message\": \"Bad Request\", \"occupancy\": 0, \"service\": \"Presence\"}";
	Status = 0;
	Message.Empty();
	Data = FPubnubListUsersFromChannelWrapper();
	
	UPubnubJsonUtilities::ListUsersFromChannelJsonToData(TestErrorJson, Status, Message, Data);
	
	//Verify status code
	TestEqual("Status code should be 400 for error response", Status, 400);
	
	//Verify message
	TestEqual("Message should be 'Bad Request' for error response", Message, "Bad Request");
	
	//Verify occupancy
	TestEqual("Occupancy should be 0 for error response", Data.Occupancy, 0);
	
	//Verify users state map is empty
	TestEqual("UsersState map should be empty for error response", Data.UsersState.Num(), 0);

	//Test invalid JSON
	FString TestInvalidJson = "invalid json";
	Status = 0;
	Message.Empty();
	Data = FPubnubListUsersFromChannelWrapper();
	
	UPubnubJsonUtilities::ListUsersFromChannelJsonToData(TestInvalidJson, Status, Message, Data);
	
	//Verify users state map is empty
	TestEqual("UsersState map should be empty for invalid JSON", Data.UsersState.Num(), 0);
	
	return true;
}

bool FFetchHistoryJsonToDataUnitTest::RunTest(const FString& Parameters)
{
	// Test basic response with simple message
	FString TestBasicJson = "{\"status\": 200, \"channels\": {\"my_channel\": [{\"message\": {\"text\": \"This is my message\"}, \"timetoken\": \"17302160534651740\", \"message_type\": null, \"custom_message_type\": \"some_type\", \"meta\": \"\", \"uuid\": \"android_user\"}]}, \"error_message\": \"\", \"error\": false}";
	bool Error = true;
	int Status = 0;
	FString ErrorMessage;
	TArray<FPubnubHistoryMessageData> Messages;
	
	UPubnubJsonUtilities::FetchHistoryJsonToData(TestBasicJson, Error, Status, ErrorMessage, Messages);
	
	// Verify error flag
	TestTrue("Error flag should be false", !Error);
	
	// Verify status code
	TestEqual("Status code should be 200", Status, 200);
	
	// Verify error message
	TestEqual("Error message should be empty", ErrorMessage, "");
	
	// Verify messages array
	TestEqual("Messages array should have 1 element", Messages.Num(), 1);
	TestEqual("Message should be 'This is my message'", Messages[0].Message, "{\"text\":\"This is my message\"}");
	TestEqual("UserID should be 'android_user'", Messages[0].UserID, "android_user");
	TestEqual("Timetoken should be '17302160534651740'", Messages[0].Timetoken, "17302160534651740");
	TestEqual("Meta should be empty", Messages[0].Meta, "");
	TestEqual("MessageType should be empty", Messages[0].MessageType, "");
	TestEqual("CustomMessageType should be 'some_type'", Messages[0].CustomMessageType, "some_type");
	TestEqual("MessageActions should be empty", Messages[0].MessageActions.Num(), 0);

	// Test response with message actions
	FString TestActionsJson = "{\"status\": 200, \"channels\": {\"my_channel\": [{\"timetoken\": \"17302756128241865\", \"meta\": \"some meta\", \"uuid\": \"User1\", \"message\": \"message from unreal to history\", \"message_type\": null, \"custom_message_type\": null, \"actions\": {\"edit\": {\"editted message\": [{\"uuid\": \"User2\", \"actionTimetoken\": \"17302814493954180\"}, {\"uuid\": \"User1\", \"actionTimetoken\": \"17302756806526160\"}]}, \"another type\": {\"bla bla\": [{\"uuid\": \"User1\", \"actionTimetoken\": \"17302763502280660\"}]}}}]}, \"error_message\": \"\", \"error\": false}";
	Error = true;
	Status = 0;
	ErrorMessage = "";
	Messages.Empty();
	
	UPubnubJsonUtilities::FetchHistoryJsonToData(TestActionsJson, Error, Status, ErrorMessage, Messages);
	
	// Verify error flag
	TestTrue("Error flag should be false for actions response", !Error);
	
	// Verify status code
	TestEqual("Status code should be 200 for actions response", Status, 200);
	
	// Verify error message
	TestEqual("Error message should be empty for actions response", ErrorMessage, "");
	
	// Verify messages array
	TestEqual("Messages array should have 1 element for actions response", Messages.Num(), 1);
	TestEqual("Message should be 'message from unreal to history'", Messages[0].Message, "message from unreal to history");
	TestEqual("UserID should be 'User1'", Messages[0].UserID, "User1");
	TestEqual("Timetoken should be '17302756128241865'", Messages[0].Timetoken, "17302756128241865");
	TestEqual("Meta should be 'some meta'", Messages[0].Meta, "some meta");
	TestEqual("MessageType should be empty", Messages[0].MessageType, "");
	TestEqual("CustomMessageType should be empty", Messages[0].CustomMessageType, "");
	
	// Verify message actions
	TestEqual("MessageActions should have 3 elements", Messages[0].MessageActions.Num(), 3);
	
	// Verify first action
	TestEqual("First action type should be 'edit'", Messages[0].MessageActions[0].Type, "edit");
	TestEqual("First action value should be 'editted message'", Messages[0].MessageActions[0].Value, "editted message");
	TestEqual("First action userID should be 'User2'", Messages[0].MessageActions[0].UserID, "User2");
	TestEqual("First action timetoken should be '17302814493954180'", Messages[0].MessageActions[0].ActionTimetoken, "17302814493954180");
	
	// Verify second action
	TestEqual("Second action type should be 'edit'", Messages[0].MessageActions[1].Type, "edit");
	TestEqual("Second action value should be 'editted message'", Messages[0].MessageActions[1].Value, "editted message");
	TestEqual("Second action userID should be 'User1'", Messages[0].MessageActions[1].UserID, "User1");
	TestEqual("Second action timetoken should be '17302756806526160'", Messages[0].MessageActions[1].ActionTimetoken, "17302756806526160");
	
	// Verify third action
	TestEqual("Third action type should be 'another type'", Messages[0].MessageActions[2].Type, "another type");
	TestEqual("Third action value should be 'bla bla'", Messages[0].MessageActions[2].Value, "bla bla");
	TestEqual("Third action userID should be 'User1'", Messages[0].MessageActions[2].UserID, "User1");
	TestEqual("Third action timetoken should be '17302763502280660'", Messages[0].MessageActions[2].ActionTimetoken, "17302763502280660");

	// Test error response
	FString TestErrorJson = "{\"status\": 400, \"channels\": {}, \"error_message\": \"Invalid channel\", \"error\": true}";
	Error = false;
	Status = 0;
	ErrorMessage = "";
	Messages.Empty();
	
	UPubnubJsonUtilities::FetchHistoryJsonToData(TestErrorJson, Error, Status, ErrorMessage, Messages);
	
	// Verify error flag
	TestTrue("Error flag should be true for error response", Error);
	
	// Verify status code
	TestEqual("Status code should be 400 for error response", Status, 400);
	
	// Verify error message
	TestEqual("Error message should be 'Invalid channel'", ErrorMessage, "Invalid channel");
	
	// Verify messages array is empty
	TestEqual("Messages array should be empty for error response", Messages.Num(), 0);

	// Test invalid JSON
	FString TestInvalidJson = "invalid json";
	Error = false;
	Status = 0;
	ErrorMessage = "";
	Messages.Empty();
	
	UPubnubJsonUtilities::FetchHistoryJsonToData(TestInvalidJson, Error, Status, ErrorMessage, Messages);
	
	// Verify messages array is empty
	TestEqual("Messages array should be empty for invalid JSON", Messages.Num(), 0);
	
	return true;
}

bool FGetAllUserMetadataJsonToDataUnitTest::RunTest(const FString& Parameters)
{
	// Test successful response with user data
	FString TestJson = "{\"status\":200,\"data\":[{\"id\":\"user1\",\"name\":\"User One\",\"externalId\":\"ext123\",\"profileUrl\":\"https://example.com/profile1\",\"email\":\"user1@example.com\",\"custom\":{\"age\":30,\"location\":\"New York\"},\"status\":\"active\",\"type\":\"premium\",\"updated\":\"2024-10-28T09:03:32.977029Z\",\"eTag\":\"AdyU1Obvqe30Dg\"},{\"id\":\"user2\",\"name\":\"User Two\",\"externalId\":\"ext456\",\"profileUrl\":\"https://example.com/profile2\",\"email\":\"user2@example.com\",\"custom\":{\"age\":25,\"location\":\"London\"},\"status\":\"inactive\",\"type\":\"basic\",\"updated\":\"2024-10-29T10:15:45.123456Z\",\"eTag\":\"BdzU2Prvrf41Eh\"}],\"next\":\"MQ\",\"prev\":\"LQ\"}";
	int Status = 0;
	TArray<FPubnubUserData> UsersData;
	FString PageNext;
	FString PagePrev;
	
	UPubnubJsonUtilities::GetAllUserMetadataJsonToData(TestJson, Status, UsersData, PageNext, PagePrev);
	
	// Verify status code
	TestEqual("Status code should be 200", Status, 200);
	
	// Verify pagination tokens
	TestEqual("Next page token should be 'MQ'", PageNext, "MQ");
	TestEqual("Previous page token should be 'LQ'", PagePrev, "LQ");
	
	// Verify number of users
	TestEqual("Should have 2 users", UsersData.Num(), 2);
	
	// Verify first user data
	TestEqual("First user ID should be 'user1'", UsersData[0].UserID, "user1");
	TestEqual("First user name should be 'User One'", UsersData[0].UserName, "User One");
	TestEqual("First user external ID should be 'ext123'", UsersData[0].ExternalID, "ext123");
	TestEqual("First user profile URL should be correct", UsersData[0].ProfileUrl, "https://example.com/profile1");
	TestEqual("First user email should be correct", UsersData[0].Email, "user1@example.com");
	TestEqual("First user custom field should be correct", UsersData[0].Custom, "{\"age\":30,\"location\":\"New York\"}");
	TestEqual("First user status should be 'active'", UsersData[0].Status, "active");
	TestEqual("First user type should be 'premium'", UsersData[0].Type, "premium");
	TestEqual("First user updated timestamp should be correct", UsersData[0].Updated, "2024-10-28T09:03:32.977029Z");
	TestEqual("First user eTag should be correct", UsersData[0].ETag, "AdyU1Obvqe30Dg");
	
	// Verify second user data
	TestEqual("Second user ID should be 'user2'", UsersData[1].UserID, "user2");
	TestEqual("Second user name should be 'User Two'", UsersData[1].UserName, "User Two");
	TestEqual("Second user external ID should be 'ext456'", UsersData[1].ExternalID, "ext456");
	TestEqual("Second user profile URL should be correct", UsersData[1].ProfileUrl, "https://example.com/profile2");
	TestEqual("Second user email should be correct", UsersData[1].Email, "user2@example.com");
	TestEqual("Second user custom field should be correct", UsersData[1].Custom, "{\"age\":25,\"location\":\"London\"}");
	TestEqual("Second user status should be 'inactive'", UsersData[1].Status, "inactive");
	TestEqual("Second user type should be 'basic'", UsersData[1].Type, "basic");
	TestEqual("Second user updated timestamp should be correct", UsersData[1].Updated, "2024-10-29T10:15:45.123456Z");
	TestEqual("Second user eTag should be correct", UsersData[1].ETag, "BdzU2Prvrf41Eh");

	// Test empty data array
	FString TestEmptyJson = "{\"status\":200,\"data\":[],\"next\":\"\",\"prev\":\"\"}";
	Status = 0;
	UsersData.Empty();
	PageNext = "";
	PagePrev = "";
	
	UPubnubJsonUtilities::GetAllUserMetadataJsonToData(TestEmptyJson, Status, UsersData, PageNext, PagePrev);
	
	TestEqual("Status code should be 200 for empty data", Status, 200);
	TestEqual("Should have 0 users for empty data", UsersData.Num(), 0);
	TestEqual("Next page token should be empty", PageNext, "");
	TestEqual("Previous page token should be empty", PagePrev, "");

	// Test error response
	FString TestErrorJson = "{\"status\":400,\"data\":[],\"next\":\"\",\"prev\":\"\"}";
	Status = 0;
	UsersData.Empty();
	PageNext = "";
	PagePrev = "";
	
	UPubnubJsonUtilities::GetAllUserMetadataJsonToData(TestErrorJson, Status, UsersData, PageNext, PagePrev);
	
	TestEqual("Status code should be 400 for error", Status, 400);
	TestEqual("Should have 0 users for error", UsersData.Num(), 0);

	// Test invalid JSON
	FString TestInvalidJson = "invalid json";
	Status = 0;
	UsersData.Empty();
	PageNext = "";
	PagePrev = "";
	
	UPubnubJsonUtilities::GetAllUserMetadataJsonToData(TestInvalidJson, Status, UsersData, PageNext, PagePrev);
	
	TestEqual("Should have 0 users for invalid JSON", UsersData.Num(), 0);

	return true;
}

bool FGetUserMetadataJsonToDataUnitTest::RunTest(const FString& Parameters)
{
	// Test successful response with user data including custom field
	FString TestJson = "{\"status\":200,\"data\":{\"id\":\"User1\",\"name\":\"abcd\",\"externalId\":null,\"profileUrl\":null,\"email\":null,\"custom\":{\"mood\":\"happy\"},\"updated\":\"2024-10-25T12:41:00.380856Z\",\"eTag\":\"ccdeed63d229d17107ccc624cbfe6aab\"}}";
	int Status = 0;
	FPubnubUserData UserData;
	
	UPubnubJsonUtilities::GetUserMetadataJsonToData(TestJson, Status, UserData);
	
	// Verify status code
	TestEqual("Status code should be 200", Status, 200);
	
	// Verify user data
	TestEqual("User ID should be 'User1'", UserData.UserID, "User1");
	TestEqual("User name should be 'abcd'", UserData.UserName, "abcd");
	TestEqual("User external ID should be empty", UserData.ExternalID, "");
	TestEqual("User profile URL should be empty", UserData.ProfileUrl, "");
	TestEqual("User email should be empty", UserData.Email, "");
	TestEqual("User custom data should be correct", UserData.Custom, "{\"mood\":\"happy\"}");
	TestEqual("User updated timestamp should be correct", UserData.Updated, "2024-10-25T12:41:00.380856Z");
	TestEqual("User eTag should be correct", UserData.ETag, "ccdeed63d229d17107ccc624cbfe6aab");

	// Test response with empty custom field
	FString TestEmptyCustomJson = "{\"status\":200,\"data\":{\"id\":\"User2\",\"name\":\"efgh\",\"externalId\":null,\"profileUrl\":null,\"email\":null,\"custom\":{},\"updated\":\"2024-10-26T12:41:00.380856Z\",\"eTag\":\"dddeed63d229d17107ccc624cbfe6aab\"}}";
	Status = 0;
	UserData = FPubnubUserData();
	
	UPubnubJsonUtilities::GetUserMetadataJsonToData(TestEmptyCustomJson, Status, UserData);
	
	TestEqual("Status code should be 200 for empty custom", Status, 200);
	TestEqual("User ID should be 'User2'", UserData.UserID, "User2");
	TestEqual("User name should be 'efgh'", UserData.UserName, "efgh");
	TestEqual("User custom data should be empty", UserData.Custom, "{}");

	// Test error response
	FString TestErrorJson = "{\"status\":400,\"data\":{}}";
	Status = 0;
	UserData = FPubnubUserData();
	
	UPubnubJsonUtilities::GetUserMetadataJsonToData(TestErrorJson, Status, UserData);
	
	TestEqual("Status code should be 400 for error", Status, 400);
	TestEqual("User ID should be empty for error", UserData.UserID, "");

	// Test invalid JSON
	FString TestInvalidJson = "invalid json";
	Status = 0;
	UserData = FPubnubUserData();
	
	UPubnubJsonUtilities::GetUserMetadataJsonToData(TestInvalidJson, Status, UserData);
	
	TestEqual("User ID should be empty for invalid JSON", UserData.UserID, "");

	return true;
}

bool FGetAllChannelMetadataJsonToDataUnitTest::RunTest(const FString& Parameters)
{
	// Test successful response with multiple channels
	FString TestJson = "{\"status\":200,\"data\":[{\"id\":\"my_channel\",\"name\":\"UE_Channel\",\"description\":null,\"updated\":\"2024-10-25T13:00:19.963635Z\",\"eTag\":\"31b6f9075656544560fabdd8db0d444b\"},{\"id\":\"my_test_channel2\",\"name\":null,\"description\":null,\"updated\":\"2024-10-11T09:41:48.926019Z\",\"eTag\":\"483589bc29065816d2ff4b32b64abc6a\"},{\"id\":\"test_channel\",\"name\":null,\"description\":null,\"updated\":\"2024-09-30T07:48:48.503855Z\",\"eTag\":\"e5672a948a68853b2b3a47de043d2b56\"}],\"next\":\"Mw\"}";
	int Status = 0;
	TArray<FPubnubChannelData> ChannelsData;
	FString PageNext;
	FString PagePrev;
	
	UPubnubJsonUtilities::GetAllChannelMetadataJsonToData(TestJson, Status, ChannelsData, PageNext, PagePrev);
	
	// Verify status code
	TestEqual("Status code should be 200", Status, 200);
	
	// Verify pagination tokens
	TestEqual("Next page token should be 'Mw'", PageNext, "Mw");
	TestEqual("Previous page token should be empty", PagePrev, "");
	
	// Verify number of channels
	TestEqual("Should have 3 channels", ChannelsData.Num(), 3);
	
	// Verify first channel data
	TestEqual("First channel ID should be 'my_channel'", ChannelsData[0].ChannelID, "my_channel");
	TestEqual("First channel name should be 'UE_Channel'", ChannelsData[0].ChannelName, "UE_Channel");
	TestEqual("First channel description should be empty", ChannelsData[0].Description, "");
	TestEqual("First channel updated timestamp should be correct", ChannelsData[0].Updated, "2024-10-25T13:00:19.963635Z");
	TestEqual("First channel eTag should be correct", ChannelsData[0].ETag, "31b6f9075656544560fabdd8db0d444b");
	
	// Verify second channel data
	TestEqual("Second channel ID should be 'my_test_channel2'", ChannelsData[1].ChannelID, "my_test_channel2");
	TestEqual("Second channel name should be empty", ChannelsData[1].ChannelName, "");
	TestEqual("Second channel description should be empty", ChannelsData[1].Description, "");
	TestEqual("Second channel updated timestamp should be correct", ChannelsData[1].Updated, "2024-10-11T09:41:48.926019Z");
	TestEqual("Second channel eTag should be correct", ChannelsData[1].ETag, "483589bc29065816d2ff4b32b64abc6a");
	
	// Verify third channel data
	TestEqual("Third channel ID should be 'test_channel'", ChannelsData[2].ChannelID, "test_channel");
	TestEqual("Third channel name should be empty", ChannelsData[2].ChannelName, "");
	TestEqual("Third channel description should be empty", ChannelsData[2].Description, "");
	TestEqual("Third channel updated timestamp should be correct", ChannelsData[2].Updated, "2024-09-30T07:48:48.503855Z");
	TestEqual("Third channel eTag should be correct", ChannelsData[2].ETag, "e5672a948a68853b2b3a47de043d2b56");

	// Test empty data array
	FString TestEmptyJson = "{\"status\":200,\"data\":[],\"next\":\"\",\"prev\":\"\"}";
	Status = 0;
	ChannelsData.Empty();
	PageNext = "";
	PagePrev = "";
	
	UPubnubJsonUtilities::GetAllChannelMetadataJsonToData(TestEmptyJson, Status, ChannelsData, PageNext, PagePrev);
	
	TestEqual("Status code should be 200 for empty data", Status, 200);
	TestEqual("Should have 0 channels for empty data", ChannelsData.Num(), 0);
	TestEqual("Next page token should be empty", PageNext, "");
	TestEqual("Previous page token should be empty", PagePrev, "");

	// Test error response
	FString TestErrorJson = "{\"status\":400,\"data\":[],\"next\":\"\",\"prev\":\"\"}";
	Status = 0;
	ChannelsData.Empty();
	PageNext = "";
	PagePrev = "";
	
	UPubnubJsonUtilities::GetAllChannelMetadataJsonToData(TestErrorJson, Status, ChannelsData, PageNext, PagePrev);
	
	TestEqual("Status code should be 400 for error", Status, 400);
	TestEqual("Should have 0 channels for error", ChannelsData.Num(), 0);

	// Test invalid JSON
	FString TestInvalidJson = "invalid json";
	Status = 0;
	ChannelsData.Empty();
	PageNext = "";
	PagePrev = "";
	
	UPubnubJsonUtilities::GetAllChannelMetadataJsonToData(TestInvalidJson, Status, ChannelsData, PageNext, PagePrev);
	
	TestEqual("Should have 0 channels for invalid JSON", ChannelsData.Num(), 0);

	return true;
}

bool FGetChannelMetadataJsonToDataUnitTest::RunTest(const FString& Parameters)
{
	// Test successful response with channel data including custom field
	FString TestJson = "{\"status\":200,\"data\":{\"id\":\"my_channel\",\"name\":\"UE_Channel\",\"description\":null,\"custom\":{\"premium\":\"ForSure\"},\"updated\":\"2024-10-25T13:00:19.963635Z\",\"eTag\":\"31b6f9075656544560fabdd8db0d444b\"}}";
	int Status = 0;
	FPubnubChannelData ChannelData;
	
	UPubnubJsonUtilities::GetChannelMetadataJsonToData(TestJson, Status, ChannelData);
	
	// Verify status code
	TestEqual("Status code should be 200", Status, 200);
	
	// Verify channel data
	TestEqual("Channel ID should be 'my_channel'", ChannelData.ChannelID, "my_channel");
	TestEqual("Channel name should be 'UE_Channel'", ChannelData.ChannelName, "UE_Channel");
	TestEqual("Channel description should be empty", ChannelData.Description, "");
	TestEqual("Channel custom field should be correct", ChannelData.Custom, "{\"premium\":\"ForSure\"}");
	TestEqual("Channel updated timestamp should be correct", ChannelData.Updated, "2024-10-25T13:00:19.963635Z");
	TestEqual("Channel eTag should be correct", ChannelData.ETag, "31b6f9075656544560fabdd8db0d444b");

	// Test response with empty custom field
	FString TestEmptyCustomJson = "{\"status\":200,\"data\":{\"id\":\"my_channel\",\"name\":\"UE_Channel\",\"description\":null,\"custom\":null,\"updated\":\"2024-10-25T13:00:19.963635Z\",\"eTag\":\"31b6f9075656544560fabdd8db0d444b\"}}";
	Status = 0;
	ChannelData = FPubnubChannelData();
	
	UPubnubJsonUtilities::GetChannelMetadataJsonToData(TestEmptyCustomJson, Status, ChannelData);
	
	TestEqual("Status code should be 200 for empty custom", Status, 200);
	TestEqual("Channel custom field should be empty", ChannelData.Custom, "");

	// Test error response
	FString TestErrorJson = "{\"status\":400,\"data\":{}}";
	Status = 0;
	ChannelData = FPubnubChannelData();
	
	UPubnubJsonUtilities::GetChannelMetadataJsonToData(TestErrorJson, Status, ChannelData);
	
	TestEqual("Status code should be 400 for error", Status, 400);
	TestEqual("Channel ID should be empty for error", ChannelData.ChannelID, "");

	// Test invalid JSON
	FString TestInvalidJson = "invalid json";
	Status = 0;
	ChannelData = FPubnubChannelData();
	
	UPubnubJsonUtilities::GetChannelMetadataJsonToData(TestInvalidJson, Status, ChannelData);
	
	TestEqual("Channel ID should be empty for invalid JSON", ChannelData.ChannelID, "");

	return true;
}

bool FGetMessageActionsJsonToDataUnitTest::RunTest(const FString& Parameters)
{
	// Test successful response with multiple message actions
	FString TestJson = "{\"status\": 200, \"data\": [{\"messageTimetoken\": \"17302756128241865\", \"type\": \"edit\", \"uuid\": \"User1\", \"value\": \"editted message\", \"actionTimetoken\": \"17302756806526160\"}, {\"messageTimetoken\": \"17302756128241865\", \"type\": \"another type\", \"uuid\": \"User1\", \"value\": \"bla bla\", \"actionTimetoken\": \"17302763502280660\"}]}";
	int Status = 0;
	TArray<FPubnubMessageActionData> MessageActions;
	
	UPubnubJsonUtilities::GetMessageActionsJsonToData(TestJson, Status, MessageActions);
	
	// Verify status code
	TestEqual("Status code should be 200", Status, 200);
	
	// Verify number of message actions
	TestEqual("Should have 2 message actions", MessageActions.Num(), 2);
	
	// Verify first message action data
	TestEqual("First action message timetoken should be correct", MessageActions[0].MessageTimetoken, "17302756128241865");
	TestEqual("First action type should be 'edit'", MessageActions[0].Type, "edit");
	TestEqual("First action user ID should be 'User1'", MessageActions[0].UserID, "User1");
	TestEqual("First action value should be 'editted message'", MessageActions[0].Value, "editted message");
	TestEqual("First action timetoken should be correct", MessageActions[0].ActionTimetoken, "17302756806526160");
	
	// Verify second message action data
	TestEqual("Second action message timetoken should be correct", MessageActions[1].MessageTimetoken, "17302756128241865");
	TestEqual("Second action type should be 'another type'", MessageActions[1].Type, "another type");
	TestEqual("Second action user ID should be 'User1'", MessageActions[1].UserID, "User1");
	TestEqual("Second action value should be 'bla bla'", MessageActions[1].Value, "bla bla");
	TestEqual("Second action timetoken should be correct", MessageActions[1].ActionTimetoken, "17302763502280660");

	// Test empty data array
	FString TestEmptyJson = "{\"status\":200,\"data\":[]}";
	Status = 0;
	MessageActions.Empty();
	
	UPubnubJsonUtilities::GetMessageActionsJsonToData(TestEmptyJson, Status, MessageActions);
	
	TestEqual("Status code should be 200 for empty data", Status, 200);
	TestEqual("Should have 0 message actions for empty data", MessageActions.Num(), 0);

	// Test error response
	FString TestErrorJson = "{\"status\":400,\"data\":[]}";
	Status = 0;
	MessageActions.Empty();
	
	UPubnubJsonUtilities::GetMessageActionsJsonToData(TestErrorJson, Status, MessageActions);
	
	TestEqual("Status code should be 400 for error", Status, 400);
	TestEqual("Should have 0 message actions for error", MessageActions.Num(), 0);

	// Test invalid JSON
	FString TestInvalidJson = "invalid json";
	Status = 0;
	MessageActions.Empty();
	
	UPubnubJsonUtilities::GetMessageActionsJsonToData(TestInvalidJson, Status, MessageActions);
	
	TestEqual("Should have 0 message actions for invalid JSON", MessageActions.Num(), 0);

	return true;
}

bool FGetMembershipsJsonToDataUnitTest::RunTest(const FString& Parameters)
{
	// Test successful response with minimal channel data
	FString TestJson1 = "{\"status\":200,\"data\":[{\"channel\":{\"id\":\"my_channel\",\"custom\":{\"channel_custom\":\"value\"},\"status\":\"active\",\"type\":\"public\"},\"custom\":{\"hot\":\"warm\"},\"status\":\"active\",\"type\":\"member\",\"updated\":\"2024-10-28T09:03:32.977029Z\",\"eTag\":\"AdyU1Obvqe30Dg\"}],\"next\":\"MQ\"}";
	int Status = 0;
	TArray<FPubnubGetMembershipsWrapper> MembershipsData;
	FString PageNext;
	FString PagePrev;
	
	UPubnubJsonUtilities::GetMembershipsJsonToData(TestJson1, Status, MembershipsData, PageNext, PagePrev);
	
	// Verify status code
	TestEqual("Status code should be 200", Status, 200);
	
	// Verify pagination tokens
	TestEqual("Next page token should be 'MQ'", PageNext, "MQ");
	TestEqual("Previous page token should be empty", PagePrev, "");
	
	// Verify number of memberships
	TestEqual("Should have 1 membership", MembershipsData.Num(), 1);
	
	// Verify first membership data
	TestEqual("First membership channel ID should be 'my_channel'", MembershipsData[0].Channel.ChannelID, "my_channel");
	TestEqual("First membership channel name should be empty", MembershipsData[0].Channel.ChannelName, "");
	TestEqual("First membership channel description should be empty", MembershipsData[0].Channel.Description, "");
	TestEqual("First membership channel custom field should be correct", MembershipsData[0].Channel.Custom, "{\"channel_custom\":\"value\"}");
	TestEqual("First membership channel status should be 'active'", MembershipsData[0].Channel.Status, "active");
	TestEqual("First membership channel type should be 'public'", MembershipsData[0].Channel.Type, "public");
	TestEqual("First membership custom field should be correct", MembershipsData[0].Custom, "{\"hot\":\"warm\"}");
	TestEqual("First membership status should be 'active'", MembershipsData[0].Status, "active");
	TestEqual("First membership type should be 'member'", MembershipsData[0].Type, "member");
	TestEqual("First membership updated timestamp should be correct", MembershipsData[0].Updated, "2024-10-28T09:03:32.977029Z");
	TestEqual("First membership eTag should be correct", MembershipsData[0].ETag, "AdyU1Obvqe30Dg");

	// Test successful response with full channel data and multiple memberships
	FString TestJson2 = "{\"status\":200,\"data\":[{\"channel\":{\"id\":\"my_channel\",\"name\":\"UE_Channel\",\"description\":\"Test Channel\",\"custom\":{\"channel_custom\":\"value1\"},\"status\":\"active\",\"type\":\"public\",\"updated\":\"2024-10-28T09:34:20.604564Z\",\"eTag\":\"cf9ba9ae8401fd369718d4f178bda4b7\"},\"type\":\"member\",\"status\":\"active\",\"custom\":{\"hotMembership\":\"warm\"},\"updated\":\"2024-10-28T09:35:26.660721Z\",\"eTag\":\"AdycwpOS7bWOKg\"},{\"channel\":{\"id\":\"my_channel2\",\"custom\":{\"channel_custom\":\"value2\"},\"status\":\"inactive\",\"type\":\"private\"},\"type\":\"admin\",\"status\":\"pending\",\"custom\":{\"hot2Membership\":\"cold\"},\"updated\":\"2024-11-04T09:03:07.341822Z\",\"eTag\":\"AYfIn/PQoIPxlwE\"}],\"next\":\"Mg\"}";
	Status = 0;
	MembershipsData.Empty();
	PageNext = "";
	PagePrev = "";
	
	UPubnubJsonUtilities::GetMembershipsJsonToData(TestJson2, Status, MembershipsData, PageNext, PagePrev);
	
	// Verify status code
	TestEqual("Status code should be 200 for second test", Status, 200);
	
	// Verify pagination tokens
	TestEqual("Next page token should be 'Mg'", PageNext, "Mg");
	TestEqual("Previous page token should be empty", PagePrev, "");
	
	// Verify number of memberships
	TestEqual("Should have 2 memberships", MembershipsData.Num(), 2);
	
	// Verify first membership data
	TestEqual("First membership channel ID should be 'my_channel'", MembershipsData[0].Channel.ChannelID, "my_channel");
	TestEqual("First membership channel name should be 'UE_Channel'", MembershipsData[0].Channel.ChannelName, "UE_Channel");
	TestEqual("First membership channel description should be 'Test Channel'", MembershipsData[0].Channel.Description, "Test Channel");
	TestEqual("First membership channel custom field should be correct", MembershipsData[0].Channel.Custom, "{\"channel_custom\":\"value1\"}");
	TestEqual("First membership channel status should be 'active'", MembershipsData[0].Channel.Status, "active");
	TestEqual("First membership channel type should be 'public'", MembershipsData[0].Channel.Type, "public");
	TestEqual("First membership custom field should be correct", MembershipsData[0].Custom, "{\"hotMembership\":\"warm\"}");
	TestEqual("First membership status should be 'active'", MembershipsData[0].Status, "active");
	TestEqual("First membership type should be 'member'", MembershipsData[0].Type, "member");
	TestEqual("First membership updated timestamp should be correct", MembershipsData[0].Updated, "2024-10-28T09:35:26.660721Z");
	TestEqual("First membership eTag should be correct", MembershipsData[0].ETag, "AdycwpOS7bWOKg");
	
	// Verify second membership data
	TestEqual("Second membership channel ID should be 'my_channel2'", MembershipsData[1].Channel.ChannelID, "my_channel2");
	TestEqual("Second membership channel name should be empty", MembershipsData[1].Channel.ChannelName, "");
	TestEqual("Second membership channel description should be empty", MembershipsData[1].Channel.Description, "");
	TestEqual("Second membership channel custom field should be correct", MembershipsData[1].Channel.Custom, "{\"channel_custom\":\"value2\"}");
	TestEqual("Second membership channel status should be 'inactive'", MembershipsData[1].Channel.Status, "inactive");
	TestEqual("Second membership channel type should be 'private'", MembershipsData[1].Channel.Type, "private");
	TestEqual("Second membership custom field should be correct", MembershipsData[1].Custom, "{\"hot2Membership\":\"cold\"}");
	TestEqual("Second membership status should be 'pending'", MembershipsData[1].Status, "pending");
	TestEqual("Second membership type should be 'admin'", MembershipsData[1].Type, "admin");
	TestEqual("Second membership updated timestamp should be correct", MembershipsData[1].Updated, "2024-11-04T09:03:07.341822Z");
	TestEqual("Second membership eTag should be correct", MembershipsData[1].ETag, "AYfIn/PQoIPxlwE");

	// Test empty data array
	FString TestEmptyJson = "{\"status\":200,\"data\":[],\"next\":\"\",\"prev\":\"\"}";
	Status = 0;
	MembershipsData.Empty();
	PageNext = "";
	PagePrev = "";
	
	UPubnubJsonUtilities::GetMembershipsJsonToData(TestEmptyJson, Status, MembershipsData, PageNext, PagePrev);
	
	TestEqual("Status code should be 200 for empty data", Status, 200);
	TestEqual("Should have 0 memberships for empty data", MembershipsData.Num(), 0);
	TestEqual("Next page token should be empty", PageNext, "");
	TestEqual("Previous page token should be empty", PagePrev, "");

	// Test error response
	FString TestErrorJson = "{\"status\":400,\"data\":[],\"next\":\"\",\"prev\":\"\"}";
	Status = 0;
	MembershipsData.Empty();
	PageNext = "";
	PagePrev = "";
	
	UPubnubJsonUtilities::GetMembershipsJsonToData(TestErrorJson, Status, MembershipsData, PageNext, PagePrev);
	
	TestEqual("Status code should be 400 for error", Status, 400);
	TestEqual("Should have 0 memberships for error", MembershipsData.Num(), 0);

	// Test invalid JSON
	FString TestInvalidJson = "invalid json";
	Status = 0;
	MembershipsData.Empty();
	PageNext = "";
	PagePrev = "";
	
	UPubnubJsonUtilities::GetMembershipsJsonToData(TestInvalidJson, Status, MembershipsData, PageNext, PagePrev);
	
	TestEqual("Should have 0 memberships for invalid JSON", MembershipsData.Num(), 0);

	return true;
}

bool FGetChannelMembersJsonToDataUnitTest::RunTest(const FString& Parameters)
{
	// Test successful response with channel members
	FString TestJson = "{\"status\":200,\"data\":[{\"uuid\":{\"id\":\"cpp_chat_user\",\"name\":\"Chat User\",\"externalId\":\"ext123\",\"profileUrl\":\"https://example.com/profile1\",\"email\":\"user1@example.com\",\"type\":\"premium\",\"status\":\"active\",\"custom\":{\"age\":25,\"location\":\"New York\"},\"updated\":\"2025-03-14T13:23:50.608735Z\",\"eTag\":\"669571c9e7371edba566c9371bfd4aec\"},\"type\":\"user\",\"status\":\"active\",\"custom\":{\"lastReadMessageTimetoken\":\"17453138062518466\",\"platform\":\"desktop\"},\"updated\":\"2025-04-22T09:23:26.406843Z\",\"eTag\":\"AYWhgd7euNT0xgE\"},{\"uuid\":{\"id\":\"CppChatUser\",\"name\":\"Chat User 2\",\"externalId\":\"ext456\",\"profileUrl\":\"https://example.com/profile2\",\"email\":\"user2@example.com\",\"type\":\"standard\",\"status\":\"inactive\",\"custom\":{\"age\":30,\"location\":\"London\"},\"updated\":\"2025-03-19T09:08:14.60887Z\",\"eTag\":\"3bbbd36590c68b080b0f258843144afe\"},\"type\":\"admin\",\"status\":\"pending\",\"custom\":{\"lastReadMessageTimetoken\":\"17447186377255230\",\"platform\":\"mobile\"},\"updated\":\"2025-04-15T12:04:00.151122Z\",\"eTag\":\"AfGZmbv/i4Hi2gE\"}],\"next\":\"Mw\"}";
	int Status = 0;
	TArray<FPubnubGetChannelMembersWrapper> MembersData;
	FString PageNext;
	FString PagePrev;
	
	UPubnubJsonUtilities::GetChannelMembersJsonToData(TestJson, Status, MembersData, PageNext, PagePrev);
	
	// Verify status code
	TestEqual("Status code should be 200", Status, 200);
	
	// Verify pagination tokens
	TestEqual("Next page token should be 'Mw'", PageNext, "Mw");
	TestEqual("Previous page token should be empty", PagePrev, "");
	
	// Verify number of members
	TestEqual("Should have 2 members", MembersData.Num(), 2);
	
	// Verify first member data
	TestEqual("First member user ID should be 'cpp_chat_user'", MembersData[0].User.UserID, "cpp_chat_user");
	TestEqual("First member user name should be 'Chat User'", MembersData[0].User.UserName, "Chat User");
	TestEqual("First member user external ID should be 'ext123'", MembersData[0].User.ExternalID, "ext123");
	TestEqual("First member user profile URL should be correct", MembersData[0].User.ProfileUrl, "https://example.com/profile1");
	TestEqual("First member user email should be correct", MembersData[0].User.Email, "user1@example.com");
	TestEqual("First member user type should be 'premium'", MembersData[0].User.Type, "premium");
	TestEqual("First member user status should be 'active'", MembersData[0].User.Status, "active");
	TestEqual("First member user custom field should be correct", MembersData[0].User.Custom, "{\"age\":25,\"location\":\"New York\"}");
	TestEqual("First member user updated timestamp should be correct", MembersData[0].User.Updated, "2025-03-14T13:23:50.608735Z");
	TestEqual("First member user eTag should be correct", MembersData[0].User.ETag, "669571c9e7371edba566c9371bfd4aec");
	TestEqual("First member type should be 'user'", MembersData[0].Type, "user");
	TestEqual("First member status should be 'active'", MembersData[0].Status, "active");
	TestEqual("First member custom field should be correct", MembersData[0].Custom, "{\"lastReadMessageTimetoken\":\"17453138062518466\",\"platform\":\"desktop\"}");
	TestEqual("First member updated timestamp should be correct", MembersData[0].Updated, "2025-04-22T09:23:26.406843Z");
	TestEqual("First member eTag should be correct", MembersData[0].ETag, "AYWhgd7euNT0xgE");
	
	// Verify second member data
	TestEqual("Second member user ID should be 'CppChatUser'", MembersData[1].User.UserID, "CppChatUser");
	TestEqual("Second member user name should be 'Chat User 2'", MembersData[1].User.UserName, "Chat User 2");
	TestEqual("Second member user external ID should be 'ext456'", MembersData[1].User.ExternalID, "ext456");
	TestEqual("Second member user profile URL should be correct", MembersData[1].User.ProfileUrl, "https://example.com/profile2");
	TestEqual("Second member user email should be correct", MembersData[1].User.Email, "user2@example.com");
	TestEqual("Second member user type should be 'standard'", MembersData[1].User.Type, "standard");
	TestEqual("Second member user status should be 'inactive'", MembersData[1].User.Status, "inactive");
	TestEqual("Second member user custom field should be correct", MembersData[1].User.Custom, "{\"age\":30,\"location\":\"London\"}");
	TestEqual("Second member user updated timestamp should be correct", MembersData[1].User.Updated, "2025-03-19T09:08:14.60887Z");
	TestEqual("Second member user eTag should be correct", MembersData[1].User.ETag, "3bbbd36590c68b080b0f258843144afe");
	TestEqual("Second member type should be 'admin'", MembersData[1].Type, "admin");
	TestEqual("Second member status should be 'pending'", MembersData[1].Status, "pending");
	TestEqual("Second member custom field should be correct", MembersData[1].Custom, "{\"lastReadMessageTimetoken\":\"17447186377255230\",\"platform\":\"mobile\"}");
	TestEqual("Second member updated timestamp should be correct", MembersData[1].Updated, "2025-04-15T12:04:00.151122Z");
	TestEqual("Second member eTag should be correct", MembersData[1].ETag, "AfGZmbv/i4Hi2gE");

	// Test empty data array
	FString TestEmptyJson = "{\"status\":200,\"data\":[],\"next\":\"\",\"prev\":\"\"}";
	Status = 0;
	MembersData.Empty();
	PageNext = "";
	PagePrev = "";
	
	UPubnubJsonUtilities::GetChannelMembersJsonToData(TestEmptyJson, Status, MembersData, PageNext, PagePrev);
	
	TestEqual("Status code should be 200 for empty data", Status, 200);
	TestEqual("Should have 0 members for empty data", MembersData.Num(), 0);
	TestEqual("Next page token should be empty", PageNext, "");
	TestEqual("Previous page token should be empty", PagePrev, "");

	// Test error response
	FString TestErrorJson = "{\"status\":400,\"data\":[],\"next\":\"\",\"prev\":\"\"}";
	Status = 0;
	MembersData.Empty();
	PageNext = "";
	PagePrev = "";
	
	UPubnubJsonUtilities::GetChannelMembersJsonToData(TestErrorJson, Status, MembersData, PageNext, PagePrev);
	
	TestEqual("Status code should be 400 for error", Status, 400);
	TestEqual("Should have 0 members for error", MembersData.Num(), 0);

	// Test invalid JSON
	FString TestInvalidJson = "invalid json";
	Status = 0;
	MembersData.Empty();
	PageNext = "";
	PagePrev = "";
	
	UPubnubJsonUtilities::GetChannelMembersJsonToData(TestInvalidJson, Status, MembersData, PageNext, PagePrev);
	
	TestEqual("Should have 0 members for invalid JSON", MembersData.Num(), 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS