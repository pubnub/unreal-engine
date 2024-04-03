// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubNub.h"
#include "PubnubSubsystem.generated.h"


class UPubnubSettings;
class FPubnubFunctionThread;
class FPubnubLoopingThread;

UENUM(BlueprintType)
enum class EPubnubPublishMethod : uint8
{
	pubnubSendViaGET,
	pubnubSendViaPOST,
	pubnubUsePATCH,
	pubnubSendViaPOSTwithGZIP,
	pubnubUsePATCHwithGZIP,
	pubnubUseDELETE
};


USTRUCT(BlueprintType)
struct FPubnubPublishSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool StoreInHistory = true;
	//TODO: CipherKey is deprecated, decide what to do (hide variable or mark is as deprecated)
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString CipherKey = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool Replicate = true;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString MetaData = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) EPubnubPublishMethod PublishMethod = EPubnubPublishMethod::pubnubSendViaGET;
};


UCLASS()
class PUBNUBLIBRARY_API UPubnubSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

#pragma region BLUEPRINT EXPOSED
	
	/* BLUEPRINT EXPOSED FUNCTIONS */

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Init")
	void InitPubnub();
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Init")
	void DeinitPubnub();

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Users")
	void SetUserID(FString UserID);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Init")
	void SetSecretKey();

	UFUNCTION(BlueprintCallable, Category = "CCore|Publish")
	void PublishMessage(FString ChannelName, FString Message, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());


#pragma endregion
	
private:

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

	/* INITIALIZATION CHECKS */
	
	bool IsInitialized = false;
	bool IsUserIDSet = false;
	bool CheckIsPubnubInitialized();
	bool CheckIsUserIDSet();


#pragma region PRIVATE FUNCTIONS

	/* PRIVATE FUNCTIONS */
	//These functions shouldn't be used directly by the SDK users. They are called from "BLUEPRINT EXPOSED" functions on PubNub threads.
	
	void InitPubnub_priv();
	void DeinitPubnub_priv();
	void SetUserID_priv(FString UserID);
	void SetSecretKey_priv();
	void PublishMessage_priv(FString ChannelName, FString Message, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

#pragma endregion 
};

