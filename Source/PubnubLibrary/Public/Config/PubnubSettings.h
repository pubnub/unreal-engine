// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "PubnubSettings.generated.h"

UCLASS(Config = Engine, defaultconfig, meta=(DisplayName="Pubnub SDK"))
class PUBNUBLIBRARY_API UPubnubSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPubnubSettings();

	UPROPERTY(Config, EditAnywhere, Category = "Keys")
	FString PublishKey = "demo";
	UPROPERTY(Config, EditAnywhere, Category = "Keys")
	FString SubscribeKey = "demo";
	UPROPERTY(Config, EditAnywhere, Category = "Keys")
	FString SecretKey = "";

	//Should Pubnub initialize automatically. If set to false, InitPubnub() has to be called manually before using other Pubnub functionalities.
	UPROPERTY(Config, EditAnywhere, Category = "Init")
	bool InitializeAutomatically = true;

	//Should Secret Key be used automatically (secret key gives host permissions to the user). If set to false, SetSecretKey needs to be called manually to give host permissions.
	UPROPERTY(Config, EditAnywhere, Category = "Init")
	bool SetSecretKeyAutomatically = false;
};