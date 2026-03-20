// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/App.h"
#include "Misc/AutomationTest.h"
#include "Templates/Function.h"
#include "PubnubSubsystem.h"


class UPubnubSubsystem;
class UGameInstance;


namespace PubnubTests
{
	constexpr float MAX_WAIT_TIME = 10.0f;
	const FString SDK_PREFIX = "UE_SDK_";
	/**
	 * Gets the Publish Key from environment variable PN_PUB_KEY
	 * Falls back to "demo" if not set
	 */
	FString GetTestPublishKey();
	
	/**
	 * Gets the Subscribe Key from environment variable PN_SUB_KEY
	 * Falls back to "demo" if not set
	 */
	FString GetTestSubscribeKey();
	/**
	 * Gets the Secret Key from environment variable PN_SEC_KEY
	 * Falls back to "demo" if not set
	 */
	FString GetTestSecretKey();

	/**
	 * Gets the Publish Key for PAM-enabled tests from environment variable PN_PUB_KEY_PAM
	 * Falls back to "demo" if not set
	 */
	FString GetTestPublishKeyWithPAM();

	/**
	 * Gets the Subscribe Key for PAM-enabled tests from environment variable PN_SUB_KEY_PAM
	 * Falls back to "demo" if not set
	 */
	FString GetTestSubscribeKeyWithPAM();

	/**
	 * Gets the Secret Key for PAM-enabled tests from environment variable PN_SEC_KEY_PAM
	 * Falls back to "demo" if not set
	 */
	FString GetTestSecretKeyWithPAM();
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
	
	//Initializes systems required by the test using PAM keysets. This (or InitTest) has to be called at the beginning of every test.
	bool InitTestWithPAM();
	//Cleans up test systems. Call this at the end of every test
	void CleanUp();

	UPubnubSubsystem* PubnubSubsystem = nullptr;
	UGameInstance* GameInstance = nullptr;
	UPubnubClient* PubnubClient = nullptr;
};


#endif // WITH_DEV_AUTOMATION_TESTS