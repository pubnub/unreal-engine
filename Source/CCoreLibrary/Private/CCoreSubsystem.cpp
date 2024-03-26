// Copyright Epic Games, Inc. All Rights Reserved.

#include "CCoreSubsystem.h"
#include "Config/CCoreSettings.h"


void UCCoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadPluginSettings();
	if(CCoreSettings->InitializeAutomatically)
	{
		//Init CCore
	}

}

void UCCoreSubsystem::Deinitialize()
{
	Super::Deinitialize();
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
