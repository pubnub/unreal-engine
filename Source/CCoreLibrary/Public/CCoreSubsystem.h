// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubNub.h"
#include "CCoreSubsystem.generated.h"


class UCCoreSettings;
class FCCoreFunctionThread;
class FCCoreLoopingThread;

UCLASS()
class CCORELIBRARY_API UCCoreSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

#pragma region BLUEPRINT EXPOSED
	
	/* BLUEPRINT EXPOSED FUNCTIONS */

	UFUNCTION(BlueprintCallable, Category = "CCore")
	void InitCCore();
	
	UFUNCTION(BlueprintCallable, Category = "CCore")
	void DeinitCCore();

	UFUNCTION(BlueprintCallable, Category = "CCore")
	void SetUserID(FString UserID);

#pragma endregion
	
private:

	bool IsInitialized = false;

	//New threads to call all C-Core functions asynchronously
	TObjectPtr<FCCoreFunctionThread> PublishThread = nullptr;
	TObjectPtr<FCCoreLoopingThread> SubscribeThread = nullptr;

	//Pubnub contexts for publishing data and subscribing to channels
	pubnub_t *ctx_pub = nullptr;
	pubnub_t *ctx_sub = nullptr;

#pragma region PLUGIN SETTINGS
	
	/* PLUGIN SETTINGS */
	
	TObjectPtr<UCCoreSettings> CCoreSettings = nullptr;
	
	//Containers for keys stored from settings
	char PublishKey[43];
	char SubscribeKey[43];
	char SecretKey[55];
	
	void LoadPluginSettings();

#pragma endregion 


	void InitCCore_priv();
	void DeinitCCore_priv();
	void SetUserID_priv(FString UserID);
};
