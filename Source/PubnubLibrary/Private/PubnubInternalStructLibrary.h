// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

THIRD_PARTY_INCLUDES_START
#include "PubNub.h"
THIRD_PARTY_INCLUDES_END

class UPubnubSubscription;
class UPubnubSubscriptionSet;

/**
 * Internal structs for the Pubnub library.
 *
 * Heap payloads registered as PubNub C-Core listener user_data: they carry Unreal-side context
 * for native callbacks that may run off the game thread or out of sync with UObject lifetime.
 * Using TWeakObjectPtr here avoids passing a raw UObject*, which native code must not assume
 * remains safe to dereference whenever a callback fires.
 *
 * Each callback pointer must match registration so we can unregister before ListenerUserData
 * is freed; otherwise PubNub can still invoke the listener during unsubscribe / subscription_free.
 */
struct FPubnubInternalSubscriptionListenerUserData
{
	TWeakObjectPtr<UPubnubSubscription> WeakSubscription;

	pubnub_subscribe_message_callback_t MessageCb = nullptr;
	pubnub_subscribe_message_callback_t SignalCb = nullptr;
	pubnub_subscribe_message_callback_t MessageActionCb = nullptr;
	pubnub_subscribe_message_callback_t ObjectsCb = nullptr;
};

struct FPubnubInternalSubscriptionSetListenerUserData
{
	TWeakObjectPtr<UPubnubSubscriptionSet> WeakSubscriptionSet;

	pubnub_subscribe_message_callback_t MessageCb = nullptr;
	pubnub_subscribe_message_callback_t SignalCb = nullptr;
	pubnub_subscribe_message_callback_t MessageActionCb = nullptr;
	pubnub_subscribe_message_callback_t ObjectsCb = nullptr;
};
