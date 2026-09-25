// Copyright 2026 PubNub Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"

#include "PubnubClient.h"
#include "Crypto/PubnubCryptoModule.h"
#include "Tests/AutomationCommon.h"
#include "PubnubSubsystem.h"
#include "Engine/GameInstance.h"
#include "UnrealEngine.h"


FString PubnubTests::GetTestPublishKey()
{
	FString PublishKey = FPlatformMisc::GetEnvironmentVariable(TEXT("PN_PUB_KEY"));
	if(!PublishKey.IsEmpty())
	{
		return PublishKey;
	}
	// Fallback to demo key if environment variable is not set
	return TEXT("demo");
}

FString PubnubTests::GetTestSubscribeKey()
{
	FString SubscribeKey = FPlatformMisc::GetEnvironmentVariable(TEXT("PN_SUB_KEY"));
	if(!SubscribeKey.IsEmpty())
	{
		return SubscribeKey;
	}
	// Fallback to demo key if environment variable is not set
	return TEXT("demo");
}

FString PubnubTests::GetTestSecretKey()
{
	FString SecretKey = FPlatformMisc::GetEnvironmentVariable(TEXT("PN_SEC_KEY"));
	if (!SecretKey.IsEmpty())
	{
		return SecretKey;
	}
	// Fallback to demo key if environment variable is not set
	return TEXT("demo");
}

FString PubnubTests::GetTestPublishKeyWithPAM()
{
	FString PublishKey = FPlatformMisc::GetEnvironmentVariable(TEXT("PN_PUB_KEY_PAM"));
	if (!PublishKey.IsEmpty())
	{
		return PublishKey;
	}
	return TEXT("demo");
}

FString PubnubTests::GetTestSubscribeKeyWithPAM()
{
	FString SubscribeKey = FPlatformMisc::GetEnvironmentVariable(TEXT("PN_SUB_KEY_PAM"));
	if (!SubscribeKey.IsEmpty())
	{
		return SubscribeKey;
	}
	return TEXT("demo");
}

bool PubnubTests::HandshakeListContainsAll(const TArray<FString>& Have, const TArray<FString>& Need)
{
	for (const FString& Name : Need)
	{
		if (!Have.Contains(Name))
		{
			return false;
		}
	}
	return true;
}

bool PubnubTests::IsHandshakeReady(const FPubnubHandshakeTracker& Tracker, const TSharedPtr<int32>& Baseline, const TArray<FString>& ExpectedChannels, const TArray<FString>& ExpectedGroups)
{
	return *Tracker.Epoch > *Baseline
		&& HandshakeListContainsAll(*Tracker.Channels, ExpectedChannels)
		&& HandshakeListContainsAll(*Tracker.Groups, ExpectedGroups);
}

void PubnubTests::TrackHandshake(UPubnubClient* Client, const FPubnubHandshakeTracker& Tracker)
{
	Client->OnSubscriptionStatusChangedNative.AddLambda([Tracker](EPubnubSubscriptionStatus Status, const FPubnubSubscriptionStatusData& Data)
	{
		if (Status == EPubnubSubscriptionStatus::PSS_ConnectionError || Status == EPubnubSubscriptionStatus::PSS_DisconnectedUnexpectedly)
		{
			*Tracker.bFailed = true;
			*Tracker.FailureReason = Data.Reason;
			++(*Tracker.Epoch);
			return;
		}

		if (Status == EPubnubSubscriptionStatus::PSS_Connected || Status == EPubnubSubscriptionStatus::PSS_SubscriptionChanged)
		{
			*Tracker.Channels = Data.Channels;
			*Tracker.Groups = Data.ChannelGroups;
			*Tracker.bFailed = false;
			++(*Tracker.Epoch);
			return;
		}

		if (Status == EPubnubSubscriptionStatus::PSS_Disconnected)
		{
			Tracker.Channels->Reset();
			Tracker.Groups->Reset();
			++(*Tracker.Epoch);
		}
	});
}

TSharedPtr<int32> PubnubTests::SnapshotHandshakeEpoch(const FPubnubHandshakeTracker& Tracker)
{
	return MakeShared<int32>(*Tracker.Epoch);
}

FString PubnubTests::GetTestSecretKeyWithPAM()
{
	FString SecretKey = FPlatformMisc::GetEnvironmentVariable(TEXT("PN_SEC_KEY_PAM"));
	if (!SecretKey.IsEmpty())
	{
		return SecretKey;
	}
	return TEXT("demo");
}

bool FPubnubAutomationTestBase::InitTest()
{
	//Initialize GameInstance and PubnubSubsystem
	GameInstance = NewObject<UGameInstance>(GEngine);
	GameInstance->InitializeStandalone();
	
	if (!TestNotNull("GameInstance exists", GameInstance))
	{return false;}

	PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	if (!TestNotNull(" Pubnub Subsystem exists", PubnubSubsystem))
	{return false;}
	
	FPubnubConfig Config;
	Config.LoggerConfig.DefaultLoggerMinLevel = EPubnubLogLevel::PLL_Debug;
	Config.UserID = "UE_SDK_Test_User";
	Config.PublishKey = PubnubTests::GetTestPublishKey();
	Config.SubscribeKey = PubnubTests::GetTestSubscribeKey();
	Config.SecretKey = PubnubTests::GetTestSecretKey();
	
	PubnubClient = PubnubSubsystem->CreatePubnubClient(Config);
	
	// We need to disable logs, because they would make tests fail during intentional errors
	bSuppressLogErrors = true;
	bSuppressLogWarnings = true;

	return true;
}

bool FPubnubAutomationTestBase::InitTestWithPAM()
{
	//Initialize GameInstance and PubnubSubsystem
	GameInstance = NewObject<UGameInstance>(GEngine);
	GameInstance->InitializeStandalone();
	
	if (!TestNotNull("GameInstance exists", GameInstance))
	{return false;}

	PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	if (!TestNotNull(" Pubnub Subsystem exists", PubnubSubsystem))
	{return false;}
	
	FPubnubConfig Config;
	Config.LoggerConfig.DefaultLoggerMinLevel = EPubnubLogLevel::PLL_Debug;
	Config.UserID = "UE_SDK_Test_User";
	Config.PublishKey = PubnubTests::GetTestPublishKeyWithPAM();
	Config.SubscribeKey = PubnubTests::GetTestSubscribeKeyWithPAM();
	Config.SecretKey = PubnubTests::GetTestSecretKeyWithPAM();
	
	PubnubClient = PubnubSubsystem->CreatePubnubClient(Config);
	
	// We need to disable logs, because they would make tests fail during intentional errors
	bSuppressLogErrors = true;
	bSuppressLogWarnings = true;

	return true;
}

bool FPubnubAutomationTestBase::RecreateClient(UPubnubCryptoModule* CryptoModule, bool bUsePamKeys, bool bIncludeSecretKey)
{
	if (!PubnubSubsystem)
	{
		return false;
	}

	if (PubnubClient)
	{
		PubnubClient->DestroyClient();
		PubnubClient = nullptr;
	}

	FPubnubConfig Config;
	Config.LoggerConfig.DefaultLoggerMinLevel = EPubnubLogLevel::PLL_Debug;
	Config.UserID = "UE_SDK_Test_User";
	Config.PublishKey = bUsePamKeys ? PubnubTests::GetTestPublishKeyWithPAM() : PubnubTests::GetTestPublishKey();
	Config.SubscribeKey = bUsePamKeys ? PubnubTests::GetTestSubscribeKeyWithPAM() : PubnubTests::GetTestSubscribeKey();
	if (bIncludeSecretKey)
	{
		Config.SecretKey = bUsePamKeys ? PubnubTests::GetTestSecretKeyWithPAM() : PubnubTests::GetTestSecretKey();
	}
	Config.CryptoModule = CryptoModule;

	PubnubClient = PubnubSubsystem->CreatePubnubClient(Config);
	return PubnubClient != nullptr;
}

void FPubnubAutomationTestBase::CleanUp()
{
	//Final clean up
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this]()
	{
		if (PubnubClient)
		{
			PubnubClient->DestroyClient();
		}
	}, 0.1f));
	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this]()
	{
		GameInstance->Shutdown();
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(0.2f));
}


#endif // WITH_DEV_AUTOMATION_TESTS