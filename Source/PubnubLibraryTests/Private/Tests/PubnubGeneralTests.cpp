// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "Kismet/GameplayStatics.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetGetUserIDTest, FPubnubAutomationTestBase, "Pubnub.Integration.General.SetGetUserID", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);


bool FPubnubSetGetUserIDTest::RunTest(const FString& Parameters)
{
	//Initial variables
	const FString TestUser = SDK_PREFIX + "test_user";
	
	if(!InitTest())
	{
		AddError("TestInitialization failed");
		return false;
	}

	//Check for any errors on the way
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});

	//Set User ID
	PubnubSubsystem->SetUserID(TestUser);

	TestEqual("Get user ID", PubnubSubsystem->GetUserID(), TestUser);

	CleanUp();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS