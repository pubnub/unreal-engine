// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "CCoreSettings.generated.h"

UCLASS(Config=CCoreSDK, meta=(DisplayName="C-Core SDK"))
class CCORELIBRARY_API UCCoreSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UCCoreSettings();

	UPROPERTY(Config, EditAnywhere, Category = "Keys")
	FString PublishKey = "default";
	UPROPERTY(Config, EditAnywhere, Category = "Keys")
	FString SubscribeKey = "default";
	UPROPERTY(Config, EditAnywhere, Category = "Keys")
	FString SecretKey = "";

	//Should C-Core initialize automatically. If set to false, InitCCore() has to be called manually before using other C-Core functionalities.
	UPROPERTY(Config, EditAnywhere)
	bool InitializeAutomatically = true;
};