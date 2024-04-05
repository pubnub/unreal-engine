// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubNub.h"
#include "PubnubStructLibrary.h"
#include "PubnubEnumLibrary.h"
#include "PubnubSubsystem.generated.h"


class UPubnubSettings;
class FPubnubFunctionThread;
class FPubnubLoopingThread;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMessageReceived, FString, MessageJson, FString, Channel);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnListChannelsFromGroupResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHereNowResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnWhereNowResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetStateResponse, FString, JsonResponse);


UCLASS()
class PUBNUBLIBRARY_API UPubnubSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FOnMessageReceived OnMessageReceived;

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

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channels")
	void AddChannelToGroup(FString ChannelName, FString ChannelGroup);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channels")
	void RemoveChannelFromGroup(FString ChannelName, FString ChannelGroup);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channels")
	void ListChannelsFromGroup(FString ChannelGroup, FOnListChannelsFromGroupResponse OnListChannelsResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channels")
	void RemoveChannelGroup(FString ChannelGroup);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void HereNow(FString ChannelName, FOnHereNowResponse HereNowResponse, FPubnubHereNowSettings HereNowSettings = FPubnubHereNowSettings());

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void WhereNow(FString UserID, FOnWhereNowResponse WhereNowResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void SetState(FString ChannelName, FString StateJson, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void GetState(FString ChannelName, FString ChannelGroup, FString UserID, FOnGetStateResponse OnGetStateResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void Heartbeat(FString ChannelName, FString ChannelGroup);

#pragma endregion
	
private:

	//New threads to call all C-Core functions asynchronously
	
	//Thread for quick operations, generally everything except subscribe
	TObjectPtr<FPubnubFunctionThread> QuickActionThread = nullptr;
	//Thread for long operations that need to be called in a loop, mostly for subcribe
	TObjectPtr<FPubnubLoopingThread> LongpollThread = nullptr;

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

	//Returns FString from the pubnub_get response
	FString GetLastResponse(pubnub_t* context);
	
	//Returns FString from the pubnub_get_channel response
	FString GetLastChannelResponse(pubnub_t* context);


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
	bool CheckQuickActionThreadValidity();


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
	void AddChannelToGroup_priv(FString ChannelName, FString ChannelGroup);
	void RemoveChannelFromGroup_priv(FString ChannelName, FString ChannelGroup);
	void ListChannelsFromGroup_priv(FString ChannelGroup, FOnListChannelsFromGroupResponse OnListChannelsResponse);
	void RemoveChannelGroup_priv(FString ChannelGroup);
	void HereNow_priv(FString ChannelName, FOnHereNowResponse HereNowResponse, FPubnubHereNowSettings HereNowSettings = FPubnubHereNowSettings());
	void WhereNow_priv(FString UserID, FOnWhereNowResponse WhereNowResponse);
	void SetState_priv(FString ChannelName, FString StateJson, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());
	void GetState_priv(FString ChannelName, FString ChannelGroup, FString UserID, FOnGetStateResponse OnGetStateResponse);
	void Heartbeat_priv(FString ChannelName, FString ChannelGroup);

#pragma endregion
	
};

