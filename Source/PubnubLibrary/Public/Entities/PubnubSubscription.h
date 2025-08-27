// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
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

	virtual void Subscribe(FPubnubSubscriptionCursor Cursor){};
	virtual void Unsubscribe(){};

protected:
	
	UPROPERTY()
	UPubnubSubsystem* PubnubSubsystem = nullptr;
	
};


UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubSubscription: public UPubnubSubscriptionBase
{
	GENERATED_BODY()

	friend class UPubnubBaseEntity;

public:
	
	UFUNCTION(BlueprintCallable, Category="Pubnub|Subscription")
	virtual void Subscribe(FPubnubSubscriptionCursor Cursor = FPubnubSubscriptionCursor()) override;

	UFUNCTION(BlueprintCallable, Category="Pubnub|Subscription")
	virtual void Unsubscribe() override;

private:

	pubnub_subscription_t* CCoreSubscription = nullptr;

	void InitSubscription(UPubnubSubsystem* InPubnubSubsystem, UPubnubBaseEntity* Entity, FPubnubSubscribeSettings InSubscribeSettings);
	
};


UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubSubscriptionSet: public UPubnubSubscriptionBase
{
	GENERATED_BODY()

	
	
};