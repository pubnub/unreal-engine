// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Pubnub.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PubnubStructLibrary.h"
#include "PubnubInternalUtilities.generated.h"


class UPubnubClient;

/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubInternalUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/* STRUCT CONVERTERS */

	static void PublishUESettingsToPubnubPublishOptions(const FPubnubPublishSettings &PublishSettings, pubnub_publish_options &PubnubPublishOptions);
	static void HereNowUESettingsToPubnubHereNowOptions(const FPubnubListUsersFromChannelSettings &HereNowSettings, pubnub_here_now_options &PubnubHereNowOptions);
	static void SetStateUESettingsToPubnubSetStateOptions(const FPubnubSetStateSettings &SetStateSettings, pubnub_set_state_options &PubnubSetStateOptions);
	static void FetchHistoryUESettingsToPbFetchHistoryOptions(const FPubnubFetchHistorySettings &FetchHistorySettings, pubnub_fetch_history_options &PubnubFetchHistoryOptions);
	
	/* C-CORE EVENT ENGINE HELPERS */

	static pubnub_subscription_t* EEGetSubscriptionForEntity(pubnub_t* Context, FString EntityID, EPubnubEntityType EntityType, FPubnubSubscribeSettings Options);
	static pubnub_subscription_set_t* EEGetSubscriptionSetForEntities(pubnub_t* Context, TArray<FString> Channels, TArray<FString> ChannelGroups, FPubnubSubscribeSettings Options);
	static bool EEAddListenerAndSubscribe(pubnub_subscription_t* Subscription, pubnub_subscribe_message_callback_t Callback, UPubnubClient* PubnubClient);
	static bool EERemoveListenerAndUnsubscribe(pubnub_subscription_t** SubscriptionPtr, pubnub_subscribe_message_callback_t Callback, UPubnubClient* PubnubClient);
	static bool EESubscribeWithSubscription(pubnub_subscription_t* Subscription, FPubnubSubscriptionCursor Cursor);
	static bool EEUnsubscribeWithSubscription(pubnub_subscription_t** SubscriptionPtr);
	static bool EESubscribeWithSubscriptionSet(pubnub_subscription_set_t* SubscriptionSet, FPubnubSubscriptionCursor Cursor);
	static bool EEUnsubscribeWithSubscriptionSet(pubnub_subscription_set_t** SubscriptionSetPtr);
	static bool EEAddSubscriptionListenerOfType(pubnub_subscription_t* Subscription, pubnub_subscribe_message_callback_t Callback, EPubnubListenerType ListenerType, UObject* Caller);
	static void EEAddSubscriptionListenersOfAllTypes(pubnub_subscription_t* Subscription, pubnub_subscribe_message_callback_t Callback, UObject* Caller);
	static bool EERemoveSubscriptionListenerOfType(pubnub_subscription_t** SubscriptionPtr, pubnub_subscribe_message_callback_t Callback, EPubnubListenerType ListenerType, UObject* Caller);
	static void EERemoveSubscriptionListenersOfAllTypes(pubnub_subscription_t** SubscriptionPtr, pubnub_subscribe_message_callback_t Callback, UObject* Caller);
	static bool EEAddSubscriptionSetListenerOfType(pubnub_subscription_set_t* SubscriptionSet, pubnub_subscribe_message_callback_t Callback, EPubnubListenerType ListenerType, UObject* Caller);
	static void EEAddSubscriptionSetListenersOfAllTypes(pubnub_subscription_set_t* SubscriptionSet, pubnub_subscribe_message_callback_t Callback, UObject* Caller);
	static bool EERemoveSubscriptionSetListenerOfType(pubnub_subscription_set_t** SubscriptionSetPtr, pubnub_subscribe_message_callback_t Callback, EPubnubListenerType ListenerType, UObject* Caller);
	static void EERemoveSubscriptionSetListenersOfAllTypes(pubnub_subscription_set_t** SubscriptionSetPtr, pubnub_subscribe_message_callback_t Callback, UObject* Caller);
};
