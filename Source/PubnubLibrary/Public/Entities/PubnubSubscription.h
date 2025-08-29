// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubnubSubsystem.h"
#include "PubnubStructLibrary.h"
#include "PubnubSubscription.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPubnubMessage, FPubnubMessageData, Message);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPubnubMessageNative, const FPubnubMessageData& Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPubnubSignal, FPubnubMessageData, Message);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPubnubSignalNative, const FPubnubMessageData& Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPubnubPresenceEvent, FPubnubMessageData, Message);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPubnubPresenceEventNative, const FPubnubMessageData& Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPubnubObjectEvent, FPubnubMessageData, Message);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPubnubObjectEventNative, const FPubnubMessageData& Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPubnubMessageAction, FPubnubMessageData, Message);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPubnubMessageActionNative, const FPubnubMessageData& Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPubnubAnyMessageType, FPubnubMessageData, Message);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPubnubAnyMessageTypeNative, const FPubnubMessageData& Message);


/**
 *
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubSubscriptionBase : public UObject
{
	GENERATED_BODY()
	
	friend class UPubnubBaseEntity;
	
public:

	UPROPERTY(BlueprintAssignable, Category="Pubnub|Subscription")
	FOnPubnubMessage OnPubnubMessage;
	FOnPubnubMessageNative OnPubnubMessageNative;

	UPROPERTY(BlueprintAssignable, Category="Pubnub|Subscription")
	FOnPubnubSignal OnPubnubSignal;
	FOnPubnubSignalNative OnPubnubSignalNative;

	UPROPERTY(BlueprintAssignable, Category="Pubnub|Subscription")
	FOnPubnubPresenceEvent OnPubnubPresenceEvent;
	FOnPubnubPresenceEventNative OnPubnubPresenceEventNative;

	UPROPERTY(BlueprintAssignable, Category="Pubnub|Subscription")
	FOnPubnubObjectEvent OnPubnubObjectEvent;
	FOnPubnubObjectEventNative OnPubnubObjectEventNative;

	UPROPERTY(BlueprintAssignable, Category="Pubnub|Subscription")
	FOnPubnubMessageAction OnPubnubMessageAction;
	FOnPubnubMessageActionNative OnPubnubMessageActionNative;

	UPROPERTY(BlueprintAssignable, Category="Pubnub|Subscription")
	FOnPubnubAnyMessageType FOnPubnubAnyMessageType;
	FOnPubnubAnyMessageTypeNative FOnPubnubAnyMessageTypeNative;

	//virtual void Subscribe(FPubnubSubscriptionCursor Cursor){};
	//virtual void Unsubscribe(){};

	//Do proper clean up when object is being destroyed
	virtual void BeginDestroy() override;

protected:
	
	UPROPERTY()
	UPubnubSubsystem* PubnubSubsystem = nullptr;

	bool IsInitialized = false;
	virtual void CleanUpSubscription(){};
	
};


UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubSubscription: public UPubnubSubscriptionBase
{
	GENERATED_BODY()

	friend class UPubnubSubsystem;
	friend class UPubnubBaseEntity;
	friend class UPubnubSubscriptionSet;

public:
	
	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet", meta = (AutoCreateRefTerm = "OnSubscribeResponse"))
	virtual void Subscribe(FOnSubscribeOperationResponse OnSubscribeResponse, FPubnubSubscriptionCursor Cursor = FPubnubSubscriptionCursor());
	
	void Subscribe(FOnSubscribeOperationResponseNative NativeCallback = nullptr, FPubnubSubscriptionCursor Cursor = FPubnubSubscriptionCursor());
	void Subscribe(FPubnubSubscriptionCursor Cursor);

	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet", meta = (AutoCreateRefTerm = "OnUnsubscribeResponse"))
	void Unsubscribe(FOnSubscribeOperationResponse OnUnsubscribeResponse);

	void Unsubscribe(FOnSubscribeOperationResponseNative NativeCallback);

	UFUNCTION(BlueprintCallable, Category="Pubnub|Subscription")
	UPubnubSubscriptionSet* AddSubscription(UPubnubSubscription* Subscription);

private:

	pubnub_subscription_t* CCoreSubscription = nullptr;

	void InitSubscription(UPubnubSubsystem* InPubnubSubsystem, UPubnubBaseEntity* Entity, FPubnubSubscribeSettings InSubscribeSettings);
	void InitWithCCoreSubscription(UPubnubSubsystem* InPubnubSubsystem, pubnub_subscription_t* InCCoreSubscription);
	void InternalInit();

	UFUNCTION()
	virtual void CleanUpSubscription() override;
};


UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubSubscriptionSet: public UPubnubSubscriptionBase
{
	GENERATED_BODY()

	friend class UPubnubSubsystem;
	friend class UPubnubSubscription;

public:
	
	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet", meta = (AutoCreateRefTerm = "OnSubscribeResponse"))
	virtual void Subscribe(FOnSubscribeOperationResponse OnSubscribeResponse, FPubnubSubscriptionCursor Cursor = FPubnubSubscriptionCursor());
	
	void Subscribe(FOnSubscribeOperationResponseNative NativeCallback = nullptr, FPubnubSubscriptionCursor Cursor = FPubnubSubscriptionCursor());
	void Subscribe(FPubnubSubscriptionCursor Cursor);

	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet", meta = (AutoCreateRefTerm = "OnUnsubscribeResponse"))
	void Unsubscribe(FOnSubscribeOperationResponse OnUnsubscribeResponse);

	void Unsubscribe(FOnSubscribeOperationResponseNative NativeCallback);

	//This will subscribe automatically if set is subscribed
	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet")
	void AddSubscription(UPubnubSubscription* Subscription);

	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet")
	void RemoveSubscription(UPubnubSubscription* Subscription);

	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet")
	void AddSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet);

	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet")
	void RemoveSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|SubscriptionSet")
	TArray<UPubnubSubscription*> GetSubscriptions() { return Subscriptions;};
	
private:

	UPROPERTY()
	TArray<UPubnubSubscription*> Subscriptions;
	

	pubnub_subscription_set_t* CCoreSubscriptionSet = nullptr;

	void InitSubscriptionSet(UPubnubSubsystem* InPubnubSubsystem, TArray<FString> Channels, TArray<FString> ChannelGroups, FPubnubSubscribeSettings InSubscribeSettings);
	void InitWithSubscriptions(UPubnubSubsystem* InPubnubSubsystem, UPubnubSubscription* Subscription1, UPubnubSubscription* Subscription2);
	void InitWithCCoreSubscriptionSet(UPubnubSubsystem* InPubnubSubsystem, pubnub_subscription_set_t* InCCoreSubscriptionSet);
	void InternalInit();
	UFUNCTION()
	virtual void CleanUpSubscription() override;
};
