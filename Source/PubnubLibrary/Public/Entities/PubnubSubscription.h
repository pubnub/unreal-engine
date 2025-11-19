// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubnubClient.h"
#include "PubnubStructLibrary.h"
#include "PubnubSubscription.generated.h"


class UPubnubClient;

struct pubnub_subscription;
typedef struct pubnub_subscription pubnub_subscription_t;
struct pubnub_subscription_set;
typedef struct pubnub_subscription_set pubnub_subscription_set_t;

// Blueprint-compatible delegate for handling published messages
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPubnubMessage, FPubnubMessageData, Message);
// Native C++ delegate for handling published messages (can accept lambdas)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPubnubMessageNative, const FPubnubMessageData& Message);
// Blueprint-compatible delegate for handling signals
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPubnubSignal, FPubnubMessageData, Message);
// Native C++ delegate for handling signals (can accept lambdas)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPubnubSignalNative, const FPubnubMessageData& Message);
// Blueprint-compatible delegate for handling presence events (join/leave/timeout)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPubnubPresenceEvent, FPubnubMessageData, Message);
// Native C++ delegate for handling presence events (can accept lambdas)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPubnubPresenceEventNative, const FPubnubMessageData& Message);
// Blueprint-compatible delegate for handling App Context object events (user/channel metadata changes)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPubnubObjectEvent, FPubnubMessageData, Message);
// Native C++ delegate for handling App Context object events (can accept lambdas)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPubnubObjectEventNative, const FPubnubMessageData& Message);
// Blueprint-compatible delegate for handling message action events (add/remove reactions)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPubnubMessageAction, FPubnubMessageData, Message);
// Native C++ delegate for handling message action events (can accept lambdas)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPubnubMessageActionNative, const FPubnubMessageData& Message);
// Blueprint-compatible delegate that fires for any type of PubNub message/event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPubnubAnyMessageType, FPubnubMessageData, Message);
// Native C++ delegate that fires for any type of PubNub message/event (can accept lambdas)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPubnubAnyMessageTypeNative, const FPubnubMessageData& Message);


/**
 * Base class for PubNub subscriptions in Unreal Engine.
 * 
 * This abstract class provides the foundation for all PubNub subscription types,
 * containing event delegates for handling different types of real-time messages
 * and events from PubNub channels. It manages the lifecycle of subscriptions
 * and provides both Blueprint-compatible and native C++ event handling.
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubSubscriptionBase : public UObject
{
	GENERATED_BODY()
	
	friend class UPubnubBaseEntity;
	
public:

	/** Event fired when a regular message is received on subscribed channels/channel groups. */
	UPROPERTY(BlueprintAssignable, Category="Pubnub|Subscription")
	FOnPubnubMessage OnPubnubMessage;
	/** Native C++ version of OnPubnubMessage that can accept lambda functions. */
	FOnPubnubMessageNative OnPubnubMessageNative;

	/** Event fired when a signal is received on subscribed channels/channel groups. */
	UPROPERTY(BlueprintAssignable, Category="Pubnub|Subscription")
	FOnPubnubSignal OnPubnubSignal;
	/** Native C++ version of OnPubnubSignal that can accept lambda functions. */
	FOnPubnubSignalNative OnPubnubSignalNative;

	/** Event fired when a presence event occurs (user join/leave/timeout) on subscribed channels. */
	UPROPERTY(BlueprintAssignable, Category="Pubnub|Subscription")
	FOnPubnubPresenceEvent OnPubnubPresenceEvent;
	/** Native C++ version of OnPubnubPresenceEvent that can accept lambda functions. */
	FOnPubnubPresenceEventNative OnPubnubPresenceEventNative;

	/** Event fired when App Context object events occur (user/channel metadata changes). */
	UPROPERTY(BlueprintAssignable, Category="Pubnub|Subscription")
	FOnPubnubObjectEvent OnPubnubObjectEvent;
	/** Native C++ version of OnPubnubObjectEvent that can accept lambda functions. */
	FOnPubnubObjectEventNative OnPubnubObjectEventNative;

	/** Event fired when message action events occur (reactions added/removed). */
	UPROPERTY(BlueprintAssignable, Category="Pubnub|Subscription")
	FOnPubnubMessageAction OnPubnubMessageAction;
	/** Native C++ version of OnPubnubMessageAction that can accept lambda functions. */
	FOnPubnubMessageActionNative OnPubnubMessageActionNative;

	/** Universal event that fires for any type of PubNub message or event received. */
	UPROPERTY(BlueprintAssignable, Category="Pubnub|Subscription")
	FOnPubnubAnyMessageType FOnPubnubAnyMessageType;
	/** Native C++ version of FOnPubnubAnyMessageType that can accept lambda functions. */
	FOnPubnubAnyMessageTypeNative FOnPubnubAnyMessageTypeNative;

	/**
	 * Called when this subscription object is being destroyed.
	 * 
	 * Ensures proper cleanup of subscription resources and unregisters
	 * from PubNub services to prevent memory leaks and dangling references.
	 */
	virtual void BeginDestroy() override;

protected:
	
	UPROPERTY()
	TObjectPtr<UPubnubClient> PubnubClient = nullptr;

	bool IsInitialized = false;
	virtual void CleanUpSubscription(){};
	
};


/**
 * Represents a subscription to a single PubNub entity (channel or channel group).
 * 
 * This class manages subscription state for a specific PubNub entity and provides
 * methods for subscribing, unsubscribing, and combining with other subscriptions.
 * It handles real-time message delivery and event notifications from the subscribed
 * entity through inherited event delegates.
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubSubscription: public UPubnubSubscriptionBase
{
	GENERATED_BODY()
	
	friend class UPubnubClient;
	friend class UPubnubBaseEntity;
	friend class UPubnubSubscriptionSet;

public:
	
	/**
	 * Subscribes to the entity associated with this subscription.
	 * 
	 * Initiates real-time message delivery from the subscribed entity.
	 * Events will be delivered through the inherited event delegates.
	 * 
	 * @param OnSubscribeResponse Callback function to handle the subscription result.
	 * @param Cursor Optional cursor to resume subscription from a specific point in time.
	 */
	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet", meta = (AutoCreateRefTerm = "OnSubscribeResponse"))
	virtual void Subscribe(FOnPubnubSubscribeOperationResponse OnSubscribeResponse, FPubnubSubscriptionCursor Cursor = FPubnubSubscriptionCursor());
	
	/**
	 * Subscribes to the entity associated with this subscription (native version).
	 * 
	 * @param NativeCallback Optional native callback that can accept lambda functions.
	 * @param Cursor Optional cursor to resume subscription from a specific point in time.
	 */
	void Subscribe(FOnPubnubSubscribeOperationResponseNative NativeCallback = nullptr, FPubnubSubscriptionCursor Cursor = FPubnubSubscriptionCursor());
	
	/**
	 * Subscribes to the entity associated with this subscription without a callback.
	 * 
	 * @param Cursor Optional cursor to resume subscription from a specific point in time.
	 */
	void Subscribe(FPubnubSubscriptionCursor Cursor);

	/**
	 * Unsubscribes from the entity associated with this subscription.
	 * 
	 * Stops real-time message delivery from the subscribed entity.
	 * The subscription object remains valid and can be reused for future subscriptions.
	 * 
	 * @param OnUnsubscribeResponse Callback function to handle the unsubscription result.
	 */
	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet", meta = (AutoCreateRefTerm = "OnUnsubscribeResponse"))
	void Unsubscribe(FOnPubnubSubscribeOperationResponse OnUnsubscribeResponse);

	/**
	 * Unsubscribes from the entity associated with this subscription (native version).
	 * 
	 * @param NativeCallback Optional native callback that can accept lambda functions.
	 */
	void Unsubscribe(FOnPubnubSubscribeOperationResponseNative NativeCallback);

	/**
	 * Combines this subscription with another subscription to create a subscription set.
	 * 
	 * Creates a new subscription set containing both this subscription and the provided
	 * subscription, allowing you to manage multiple subscriptions as a single unit.
	 * 
	 * @param Subscription Another subscription to combine with this one.
	 * @return A new subscription set containing both subscriptions.
	 */
	UFUNCTION(BlueprintCallable, Category="Pubnub|Subscription")
	UPubnubSubscriptionSet* AddSubscription(UPubnubSubscription* Subscription);

private:

	pubnub_subscription_t* CCoreSubscription = nullptr;

	void InitSubscription(UPubnubClient* InPubnubClient, UPubnubBaseEntity* Entity, FPubnubSubscribeSettings InSubscribeSettings);
	void InitWithCCoreSubscription(UPubnubClient* InPubnubClient, pubnub_subscription_t* InCCoreSubscription);
	void InternalInit();

	UFUNCTION()
	virtual void CleanUpSubscription() override;
};


/**
 * Represents a collection of PubNub subscriptions that can be managed as a single unit.
 * 
 * This class allows you to group multiple individual subscriptions together and perform
 * operations like subscribe/unsubscribe on all of them simultaneously. It provides
 * efficient management of multiple entity subscriptions and inherits all event handling
 * capabilities from the base subscription class.
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubSubscriptionSet: public UPubnubSubscriptionBase
{
	GENERATED_BODY()
	
	friend class UPubnubClient;
	friend class UPubnubSubscription;

public:
	
	/**
	 * Subscribes to all entities in this subscription set.
	 * 
	 * Initiates real-time message delivery from all subscriptions in this set.
	 * Events will be delivered through the inherited event delegates.
	 * 
	 * @param OnSubscribeResponse Callback function to handle the subscription result.
	 * @param Cursor Optional cursor to resume subscription from a specific point in time.
	 */
	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet", meta = (AutoCreateRefTerm = "OnSubscribeResponse"))
	virtual void Subscribe(FOnPubnubSubscribeOperationResponse OnSubscribeResponse, FPubnubSubscriptionCursor Cursor = FPubnubSubscriptionCursor());
	
	/**
	 * Subscribes to all entities in this subscription set (native version).
	 * 
	 * @param NativeCallback Optional native callback that can accept lambda functions.
	 * @param Cursor Optional cursor to resume subscription from a specific point in time.
	 */
	void Subscribe(FOnPubnubSubscribeOperationResponseNative NativeCallback = nullptr, FPubnubSubscriptionCursor Cursor = FPubnubSubscriptionCursor());
	
	/**
	 * Subscribes to all entities in this subscription set without a callback.
	 * 
	 * @param Cursor Optional cursor to resume subscription from a specific point in time.
	 */
	void Subscribe(FPubnubSubscriptionCursor Cursor);

	/**
	 * Unsubscribes from all entities in this subscription set.
	 * 
	 * Stops real-time message delivery from all subscriptions in this set.
	 * The subscription set remains valid and can be reused for future subscriptions.
	 * 
	 * @param OnUnsubscribeResponse Callback function to handle the unsubscription result.
	 */
	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet", meta = (AutoCreateRefTerm = "OnUnsubscribeResponse"))
	void Unsubscribe(FOnPubnubSubscribeOperationResponse OnUnsubscribeResponse);

	/**
	 * Unsubscribes from all entities in this subscription set (native version).
	 * 
	 * @param NativeCallback Optional native callback that can accept lambda functions.
	 */
	void Unsubscribe(FOnPubnubSubscribeOperationResponseNative NativeCallback);

	/**
	 * Adds a subscription to this subscription set.
	 * 
	 * If this subscription set is currently subscribed, the added subscription
	 * will automatically be subscribed as well.
	 * 
	 * @param Subscription The subscription to add to this set.
	 */
	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet")
	void AddSubscription(UPubnubSubscription* Subscription);

	/**
	 * Removes a subscription from this subscription set.
	 * 
	 * The removed subscription will be unsubscribed if the set is currently subscribed.
	 * 
	 * @param Subscription The subscription to remove from this set.
	 */
	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet")
	void RemoveSubscription(UPubnubSubscription* Subscription);

	/**
	 * Merges another subscription set into this one.
	 * 
	 * All subscriptions from the other set will be added to this set.
	 * 
	 * @param SubscriptionSet The subscription set to merge into this one.
	 */
	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet")
	void AddSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet);

	/**
	 * Removes all subscriptions that are also present in another subscription set.
	 * 
	 * @param SubscriptionSet The subscription set containing subscriptions to remove.
	 */
	UFUNCTION(BlueprintCallable, Category="Pubnub|SubscriptionSet")
	void RemoveSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet);

	/**
	 * Gets all individual subscriptions contained in this subscription set.
	 * 
	 * @return Array of all subscriptions in this set.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|SubscriptionSet")
	TArray<UPubnubSubscription*> GetSubscriptions() { return Subscriptions;};
	
private:

	UPROPERTY()
	TArray<UPubnubSubscription*> Subscriptions;
	

	pubnub_subscription_set_t* CCoreSubscriptionSet = nullptr;

	void InitSubscriptionSet(UPubnubClient* InPubnubClient, TArray<FString> Channels, TArray<FString> ChannelGroups, FPubnubSubscribeSettings InSubscribeSettings);
	void InitWithSubscriptions(UPubnubClient* InPubnubClient, UPubnubSubscription* Subscription1, UPubnubSubscription* Subscription2);
	void InitWithCCoreSubscriptionSet(UPubnubClient* InPubnubClient, pubnub_subscription_set_t* InCCoreSubscriptionSet);
	void InternalInit();
	UFUNCTION()
	virtual void CleanUpSubscription() override;
};
