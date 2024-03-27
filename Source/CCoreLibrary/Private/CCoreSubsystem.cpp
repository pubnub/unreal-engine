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

void UCCoreSubsystem::InitCCore_priv()
{
	if(IsInitialized)
	{return;}
	
	ctx_pub = pubnub_alloc();
	ctx_sub = pubnub_alloc();

	pubnub_init(ctx_pub, PublishKey, SubscribeKey);
	pubnub_init(ctx_sub, PublishKey, SubscribeKey);

	IsInitialized = true;
	UE_LOG(LogTemp, Warning, TEXT("Context initialized"));
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
	UE_LOG(LogTemp, Warning, TEXT("Context Deinitialized"));
}
