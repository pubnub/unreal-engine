// Copyright Epic Games, Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "Config/PubnubSettings.h"
#include "Threads/PubnubFunctionThread.h"
#include "Threads/PubnubLoopingThread.h"


void UPubnubSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//Create new threads - separate for subscribe and all other operations
	PublishThread = new FPubnubFunctionThread;
	SubscribeThread = new FPubnubLoopingThread;

	//Load all settings from plugin config
	LoadPluginSettings();
	if(PubnubSettings->InitializeAutomatically)
	{
		InitPubnub();
	}
}

void UPubnubSubsystem::Deinitialize()
{
	DeinitPubnub_priv();
	Super::Deinitialize();
}

void UPubnubSubsystem::InitPubnub()
{
	if(!PublishThread)
	{return;}

	PublishThread->AddFunctionToQueue( [this]
	{
		InitPubnub_priv();
	});
}

void UPubnubSubsystem::DeinitPubnub()
{
	if(!PublishThread)
	{return;}
	
	PublishThread->AddFunctionToQueue( [this]
	{
		DeinitPubnub_priv();
	});
}

void UPubnubSubsystem::SetUserID(FString UserID)
{
	if(!PublishThread)
	{return;}
	
	PublishThread->AddFunctionToQueue( [this, UserID]
	{
		SetUserID_priv(UserID);
	});
}

void UPubnubSubsystem::LoadPluginSettings()
{
	//Save all settings
	PubnubSettings = GetMutableDefault<UPubnubSettings>();
	
	//Copy memory for chars containing keys
	memcpy_s(PublishKey, 42, TCHAR_TO_ANSI(*PubnubSettings->PublishKey), 42);
	memcpy_s(SubscribeKey,42,  TCHAR_TO_ANSI(*PubnubSettings->SubscribeKey), 42);
	memcpy_s(SecretKey,54,  TCHAR_TO_ANSI(*PubnubSettings->SecretKey), 54);
	PublishKey[42] = '\0';
	SubscribeKey[42] = '\0';
	SecretKey[54] = '\0';
}



/* PRIV FUNCTIONS */

void UPubnubSubsystem::InitPubnub_priv()
{
	if(IsInitialized)
	{return;}
	
	ctx_pub = pubnub_alloc();
	ctx_sub = pubnub_alloc();

	pubnub_init(ctx_pub, PublishKey, SubscribeKey);
	pubnub_init(ctx_sub, PublishKey, SubscribeKey);

	IsInitialized = true;
}

void UPubnubSubsystem::DeinitPubnub_priv()
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

void UPubnubSubsystem::SetUserID_priv(FString UserID)
{
	if(!IsInitialized)
	{return;}

	pubnub_set_user_id(ctx_pub, TCHAR_TO_ANSI(*UserID));
	pubnub_set_user_id(ctx_sub, TCHAR_TO_ANSI(*UserID));
}
