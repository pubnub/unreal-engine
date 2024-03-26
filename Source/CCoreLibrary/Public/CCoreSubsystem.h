// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubNub.h"
#include "CCoreSubsystem.generated.h"


class UCCoreSettings;

UCLASS()
class CCORELIBRARY_API UCCoreSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;


private:

#pragma region PLUGIN SETTINGS
	/* PLUGIN SETTINGS */
	
	UCCoreSettings* CCoreSettings = nullptr;
	
	//Containers for keys stored from settings
	char PublishKey[43];
	char SubscribeKey[43];
	char SecretKey[55];
	
	void LoadPluginSettings();

#pragma endregion 
	
};
