// Copyright 2024 PubNub Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"


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

	return true;
}

void FPubnubAutomationTestBase::CleanUp()
{
	//Final clean up
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this]()
	{
		PubnubSubsystem->DeinitPubnub();
	}, 0.2f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this]()
	{
		GameInstance->Shutdown();
	}, 0.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(0.2f));
}


#endif // WITH_DEV_AUTOMATION_TESTS