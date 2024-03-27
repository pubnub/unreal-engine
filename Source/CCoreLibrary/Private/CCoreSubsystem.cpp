// Copyright Epic Games, Inc. All Rights Reserved.

#include "CCoreSubsystem.h"
#include "Config/CCoreSettings.h"
#include "Threads/CCoreFunctionThread.h"
#include "Threads/CCoreLoopingThread.h"


void UCCoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//Create new threads - separate for subscribe and all other operations
	PublishThread = new FCCoreFunctionThread;
	SubscribeThread = new FCCoreLoopingThread;

	//Load all settings from plugin config
	LoadPluginSettings();
	if(CCoreSettings->InitializeAutomatically)
	{
		InitCCore();
	}
}

void UCCoreSubsystem::Deinitialize()
{
	DeinitCCore_priv();
	Super::Deinitialize();
}

void UCCoreSubsystem::InitCCore()
{
	if(!PublishThread)
	{return;}

	PublishThread->AddFunctionToQueue( [this]
	{
		InitCCore_priv();
	});
}

void UCCoreSubsystem::DeinitCCore()
{
	if(!PublishThread)
	{return;}
	
	PublishThread->AddFunctionToQueue( [this]
	{
		DeinitCCore_priv();
	});
}

void UCCoreSubsystem::SetUserID(FString UserID)
{
	if(!PublishThread)
	{return;}
	
	PublishThread->AddFunctionToQueue( [this, UserID]
	{
		SetUserID_priv(UserID);
	});
}

void UCCoreSubsystem::LoadPluginSettings()
{
	//Save all settings
	CCoreSettings = GetMutableDefault<UCCoreSettings>();
	
	//Copy memory for chars containing keys
	memcpy_s(PublishKey, 42, TCHAR_TO_ANSI(*CCoreSettings->PublishKey), 42);
	memcpy_s(SubscribeKey,42,  TCHAR_TO_ANSI(*CCoreSettings->SubscribeKey), 42);
	memcpy_s(SecretKey,54,  TCHAR_TO_ANSI(*CCoreSettings->SecretKey), 54);
	PublishKey[42] = '\0';
	SubscribeKey[42] = '\0';
	SecretKey[54] = '\0';
}



/* PRIV FUNCTIONS */

void UCCoreSubsystem::InitCCore_priv()
{
	if(IsInitialized)
	{return;}
	
	ctx_pub = pubnub_alloc();
	ctx_sub = pubnub_alloc();

	pubnub_init(ctx_pub, PublishKey, SubscribeKey);
	pubnub_init(ctx_sub, PublishKey, SubscribeKey);

	IsInitialized = true;
}

void UCCoreSubsystem::DeinitCCore_priv()
{
	if(!IsInitialized)
	{return;}

	//Add Unsubscribe from all channels here
	
	if(ctx_pub)
	{
		pubnub_free(ctx_pub);
		ctx_pub = nullptr;
	}
	if(ctx_sub)
	{
		pubnub_free(ctx_sub);
		ctx_sub = nullptr;
	}
	IsInitialized = false;
}

void UCCoreSubsystem::SetUserID_priv(FString UserID)
{
	if(!IsInitialized)
	{return;}

	pubnub_set_user_id(ctx_pub, TCHAR_TO_ANSI(*UserID));
	pubnub_set_user_id(ctx_sub, TCHAR_TO_ANSI(*UserID));
}
