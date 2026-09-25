// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/App.h"
#include "Misc/AutomationTest.h"
#include "Templates/Function.h"
#include "PubnubSubsystem.h"
#include "PubnubClient.h"
#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.h"


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

	/**
	 * Subscribe returns when the subscription is registered locally. The handshake
	 * finishes later and is reported on OnSubscriptionStatusChangedNative.
	 * Tests that publish must wait for PSS_Connected or PSS_SubscriptionChanged
	 * that lists the target channel or group.
	 */
	struct FPubnubHandshakeTracker
	{
		TSharedPtr<int32> Epoch = MakeShared<int32>(0);
		TSharedPtr<TArray<FString>> Channels = MakeShared<TArray<FString>>();
		TSharedPtr<TArray<FString>> Groups = MakeShared<TArray<FString>>();
		TSharedPtr<bool> bFailed = MakeShared<bool>(false);
		TSharedPtr<FString> FailureReason = MakeShared<FString>();
	};

	bool HandshakeListContainsAll(const TArray<FString>& Have, const TArray<FString>& Need);
	bool IsHandshakeReady(const FPubnubHandshakeTracker& Tracker, const TSharedPtr<int32>& Baseline, const TArray<FString>& ExpectedChannels, const TArray<FString>& ExpectedGroups);
	void TrackHandshake(UPubnubClient* Client, const FPubnubHandshakeTracker& Tracker);
	TSharedPtr<int32> SnapshotHandshakeEpoch(const FPubnubHandshakeTracker& Tracker);
}

class FWaitForPubnubHandshakeCommand : public IAutomationLatentCommand
{
public:
	FWaitForPubnubHandshakeCommand(PubnubTests::FPubnubHandshakeTracker InTracker, TSharedPtr<int32> InBaseline, TArray<FString> InChannels, TArray<FString> InGroups, float InTimeoutSeconds)
		: Tracker(MoveTemp(InTracker))
		, Baseline(MoveTemp(InBaseline))
		, ExpectedChannels(MoveTemp(InChannels))
		, ExpectedGroups(MoveTemp(InGroups))
		, TimeoutSeconds(InTimeoutSeconds)
		, ElapsedTime(0.0f)
	{}

	virtual bool Update() override
	{
		ElapsedTime += FApp::GetDeltaTime();
		if (*Tracker.bFailed || PubnubTests::IsHandshakeReady(Tracker, Baseline, ExpectedChannels, ExpectedGroups))
		{
			return true;
		}
		return ElapsedTime >= TimeoutSeconds;
	}

private:
	PubnubTests::FPubnubHandshakeTracker Tracker;
	TSharedPtr<int32> Baseline;
	TArray<FString> ExpectedChannels;
	TArray<FString> ExpectedGroups;
	float TimeoutSeconds;
	float ElapsedTime;
};


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
	/** Destroys the current client and creates another. Crypto is fixed at creation. */
	bool RecreateClient(UPubnubCryptoModule* CryptoModule, bool bUsePamKeys = false, bool bIncludeSecretKey = true);
	//Cleans up test systems. Call this at the end of every test
	void CleanUp();

	UPubnubSubsystem* PubnubSubsystem = nullptr;
	UGameInstance* GameInstance = nullptr;
	UPubnubClient* PubnubClient = nullptr;
};


#endif // WITH_DEV_AUTOMATION_TESTS