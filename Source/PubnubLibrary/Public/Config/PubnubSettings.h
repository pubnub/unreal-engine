// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "PubnubSettings.generated.h"

UCLASS(Config=PubnubSDK, meta=(DisplayName="C-Core SDK"))
class PUBNUBLIBRARY_API UPubnubSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPubnubSettings();

	UPROPERTY(Config, EditAnywhere, Category = "Keys")
	FString PublishKey = "default";
	UPROPERTY(Config, EditAnywhere, Category = "Keys")
	FString SubscribeKey = "default";
	UPROPERTY(Config, EditAnywhere, Category = "Keys")
	FString SecretKey = "";

	//Should Pubnub initialize automatically. If set to false, InitPubnub() has to be called manually before using other Pubnub functionalities.
	UPROPERTY(Config, EditAnywhere)
	bool InitializeAutomatically = true;
};