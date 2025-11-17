// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PubnubStructLibrary.h"
#include "PubnubEnumLibrary.h"
#include "UObject/Object.h"
#include "PubnubClient.generated.h"

class UPubnubSubsystem;
class FPubnubFunctionThread;
struct CCoreSubscriptionCallback;


struct pubnub_;
typedef struct pubnub_ pubnub_t;
enum pubnub_res;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPubnubOnSubscriptionStatusChanged, EPubnubSubscriptionStatus, Status, FPubnubSubscriptionStatusData, StatusData);
DECLARE_MULTICAST_DELEGATE_TwoParams(FPubnubOnSubscriptionStatusChangedNative, EPubnubSubscriptionStatus Status, const FPubnubSubscriptionStatusData& StatusData);


DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnPublishMessageResponse, FPubnubOperationResult, Result, FPubnubMessageData, PublishedMessage);
DECLARE_DELEGATE_TwoParams(FPubnubOnPublishMessageResponseNative, const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPubnubOnSubscribeOperationResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FPubnubOnSubscribeOperationResponseNative, const FPubnubOperationResult& Result);

/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubClient : public UObject
{
	GENERATED_BODY()

	friend class UPubnubSubsystem;

public:

	/**Listener to react for subscription status changed */
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FPubnubOnSubscriptionStatusChanged OnSubscriptionStatusChanged;

	/**Listener to react for subscription status changed , equivalent that accepts lambdas*/
	FPubnubOnSubscriptionStatusChangedNative OnSubscriptionStatusChangedNative;
	

	UFUNCTION(BlueprintCallable, Category="PubnubClient")
	int GetClientID() {return ClientID;};

	UFUNCTION(BlueprintCallable, Category="PubnubClient")
	void DestroyClient();


	/**
	 * Sets the user ID for the current session.
	 * 
	 * @param UserID The user ID to set.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|General")
	void SetUserID(FString UserID);

	/**
	 * Gets the current user ID.
	 * 
	 * @return The current user ID.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pubnub|General")
	FString GetUserID();

	/**
	 * Sets the secret key for the PubNub account. Uses SecretKey provided in plugin settings.
	 * Don't call it manually if "SetSecretKeyAutomatically" in plugin settings is set to true.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|AccessManager")
	void SetSecretKey();

	/**
	 * Publishes a message to a specified channel.
	 * 
	 * @param Channel The ID of the channel to publish the message to.
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param OnPublishMessageResponse Optional delegate to listen for the publish result.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Publish", meta = (AutoCreateRefTerm = "OnPublishMessageResponse"))
	void PublishMessage(FString Channel, FString Message, FPubnubOnPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

	/**
	 * Publishes a message to a specified channel.
	 * 
	 * @param Channel The ID of the channel to publish the message to.
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param NativeCallback Optional delegate to listen for the publish result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if publish result is not needed.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	void PublishMessage(FString Channel, FString Message, FPubnubOnPublishMessageResponseNative NativeCallback = nullptr, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

	/**
	 * Publishes a message to a specified channel. Overload without delegate to get publish result.
	 * 
	 * @param Channel The ID of the channel to publish the message to.
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	void PublishMessage(FString Channel, FString Message, FPubnubPublishSettings PublishSettings);




	

private:

	//Thread for all PubNub operations, this thread will queue all PubNub calls and trigger them one by one
	FPubnubFunctionThread* PubnubCallsThread = nullptr;

	//Pubnub context for the most of the pubnub operations
	pubnub_t *ctx_pub = nullptr;
	//Pubnub context for the event engine - subscribe operations
	pubnub_t *ctx_ee = nullptr;

#pragma region PUBNUB INIT

	void InitWithConfig(UPubnubSubsystem* InPubnubSubsystem, FPubnubConfig InConfig, int InClientID, FString InDebugName = "");
	void DeinitializeClient();
	
	UPROPERTY()
	TObjectPtr<UPubnubSubsystem> PubnubSubsystem = nullptr;
	int ClientID = -1;
	FString DebugName = "";
	bool IsInitialized = false;

#pragma endregion 

#pragma region PUBNUB CONFIG

	//Container for all configuration settings
	//TODO:: DO we event need to save config??
	UPROPERTY()
	FPubnubConfig PubnubConfig;

	void SavePubnubConfig(const FPubnubConfig &InConfig);
	
	//Containers for keys stored from settings
	static const int PublishKeySize = 42;
	static const int SecretKeySize = 54;
	char PublishKey[PublishKeySize + 1] = {};
	char SubscribeKey[PublishKeySize + 1] = {};
	char SecretKey[SecretKeySize + 1] = {};

#pragma endregion 

	//TODO:: Move these functions to the logger
	void PubnubError(FString ErrorMessage, EPubnubErrorType ErrorType = EPubnubErrorType::PET_Error);
	void PubnubResponseError(pubnub_res PubnubResponse, FString ErrorMessage);

	//Storage for global subscriptions (not from Entities)
	TMap<FString, CCoreSubscriptionCallback*> ChannelSubscriptions;
	TMap<FString, CCoreSubscriptionCallback*> ChannelGroupSubscriptions;

	//Array storing delegates for all queued subscription operations
	TArray<FPubnubOnSubscribeOperationResponseNative> SubscriptionResultDelegates;

	void OnCCoreSubscriptionStatusReceived(int StatusEnum, const void* StatusData);

	
	void InitPubnub_priv(const FPubnubConfig& Config);
};
