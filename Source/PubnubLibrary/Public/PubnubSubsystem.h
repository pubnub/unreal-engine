// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubNub.h"
#include "PubnubSubsystem.generated.h"


class UPubnubSettings;
class FPubnubFunctionThread;
class FPubnubLoopingThread;

UCLASS()
class PUBNUBLIBRARY_API UPubnubSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

#pragma region BLUEPRINT EXPOSED
	
	/* BLUEPRINT EXPOSED FUNCTIONS */

	UFUNCTION(BlueprintCallable, Category = "Pubnub")
	void InitPubnub();
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub")
	void DeinitPubnub();

	UFUNCTION(BlueprintCallable, Category = "Pubnub")
	void SetUserID(FString UserID);

#pragma endregion
	
private:

	bool IsInitialized = false;

	//New threads to call all C-Core functions asynchronously
	TObjectPtr<FPubnubFunctionThread> PublishThread = nullptr;
	TObjectPtr<FPubnubLoopingThread> SubscribeThread = nullptr;

	//Pubnub contexts for publishing data and subscribing to channels
	pubnub_t *ctx_pub = nullptr;
	pubnub_t *ctx_sub = nullptr;

#pragma region PLUGIN SETTINGS
	
	/* PLUGIN SETTINGS */
	
	TObjectPtr<UPubnubSettings> PubnubSettings = nullptr;
	
	//Containers for keys stored from settings
	char PublishKey[43];
	char SubscribeKey[43];
	char SecretKey[55];
	
	void LoadPluginSettings();

#pragma endregion 


	void InitPubnub_priv();
	void DeinitPubnub_priv();
	void SetUserID_priv(FString UserID);
};