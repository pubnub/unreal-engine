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


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMessageReceived, FString, MessageJson, FString, Channel);



UCLASS()
class PUBNUBLIBRARY_API UPubnubSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

#pragma region DELEGATE VARIABLES
	
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Subscribe")
	FOnMessageReceived OnMessageReceived;

#pragma endregion


#pragma region BLUEPRINT EXPOSED
	
	/* BLUEPRINT EXPOSED FUNCTIONS */
	//These functions don't have actual logic, they just call corresponding private functions on Pubnub threads
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Init")
	void InitPubnub();
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Init")
	void DeinitPubnub();

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Users")
	void SetUserID(FString UserID);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Init")
	void SetSecretKey();

	/**
	 * Publish message in Json format to a specified channel
	 * @param ChannelName Channel to publish message to. Can't be empty.
	 * @param Message Message to publish. Has to be in Json format.
	 * @param PublishSettings Optional advanced publish settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Publish")
	void PublishMessage(FString ChannelName, FString Message, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void SubscribeToChannel(FString ChannelName);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void SubscribeToGroup(FString GroupName);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void UnsubscribeFromChannel(FString ChannelName);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void UnsubscribeFromGroup(FString GroupName);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void UnsubscribeFromAll();

#pragma endregion
	
private:

	//New threads to call all C-Core functions asynchronously
	TObjectPtr<FPubnubFunctionThread> PublishThread = nullptr;
	TObjectPtr<FPubnubLoopingThread> SubscribeThread = nullptr;

	//Pubnub contexts for publishing data and subscribing to channels
	pubnub_t *ctx_pub = nullptr;
	pubnub_t *ctx_sub = nullptr;

	TArray<FString> SubscribedChannels;
	TArray<FString> SubscribedGroups;
	
	//Publish to the first subscribed channel to unlock subscribe context
	void SystemPublish();

	//Register to PubnubLoopingThread function to check in loop for messages from subscribed channels and groups
	void StartPubnubSubscribeLoop();

	//Useful for subscribing into multiple channels/groups. Returns Strings in format String1,String2,...
	FString StringArrayToCommaSeparated(TArray<FString> StringArray);


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
	bool CheckPublishThreadValidity();


#pragma region PRIVATE FUNCTIONS

	/* PRIVATE FUNCTIONS */
	//These functions are called from "BLUEPRINT EXPOSED" functions on PubNub threads. They shouldn't be called directly on Game Thread.
	
	void InitPubnub_priv();
	void DeinitPubnub_priv();
	void SetUserID_priv(FString UserID);
	void SetSecretKey_priv();
	void PublishMessage_priv(FString ChannelName, FString Message, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());
	void SubscribeToChannel_priv(FString ChannelName);
	void SubscribeToGroup_priv(FString GroupName);
	void UnsubscribeFromChannel_priv(FString ChannelName);
	void UnsubscribeFromGroup_priv(FString GroupName);
	void UnsubscribeFromAll_priv();

#pragma endregion
	
};

