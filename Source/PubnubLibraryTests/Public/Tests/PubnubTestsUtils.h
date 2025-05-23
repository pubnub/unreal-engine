// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Templates/Function.h"
#include "PubnubSubsystem.h"

namespace PubnubTests
{
	constexpr float MAX_WAIT_TIME = 10.0f;
	const FString SDK_PREFIX = "UE_SDK_";
}



class FWaitUntilLatentCommand : public IAutomationLatentCommand
{
public:
	FWaitUntilLatentCommand(TFunction<bool()> InCondition, float InTimeoutSeconds)
		: Condition(MoveTemp(InCondition))
		, TimeoutSeconds(InTimeoutSeconds)
		, ElapsedTime(0.0f)
	{}

	virtual bool Update() override
	{
		ElapsedTime += FApp::GetDeltaTime();
		if (Condition())
		{
			return true; // Done
		}
		return ElapsedTime >= TimeoutSeconds; // Fail if timeout
	}

private:
	TFunction<bool()> Condition;
	float TimeoutSeconds;
	float ElapsedTime;
};

/**
 * This is dedicated class for all Pubnub automated tests.
 * It has helper functions to initialize and clean up all required systems.
 */

class FPubnubAutomationTestBase: public FAutomationTestBase
{
	
public:
	FPubnubAutomationTestBase(const FString& InName, const bool bInComplexTask)
		: FAutomationTestBase(InName, bInComplexTask)
	{}

	//Initializes systems required by the test. This has to be called at the beginning of every test.
	bool InitTest();
	//Cleans up test systems. Call this at the end of every test
	void CleanUp();

	UPubnubSubsystem* PubnubSubsystem = nullptr;
	UGameInstance* GameInstance = nullptr;
};


#endif // WITH_DEV_AUTOMATION_TESTS