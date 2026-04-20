// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entities/PubnubSubscription.h"

/**
 * Internal structs for the Pubnub library (not Blueprint-facing).
 *
 * Heap payloads registered as PubNub C-Core listener user_data: they carry Unreal-side context
 * for native callbacks that may run off the game thread or out of sync with UObject lifetime.
 * Using TWeakObjectPtr here avoids passing a raw UObject*, which native code must not assume
 * remains safe to dereference whenever a callback fires.
 */
struct FPubnubInternalSubscriptionListenerUserData
{
	TWeakObjectPtr<UPubnubSubscription> WeakSubscription;
};

struct FPubnubInternalSubscriptionSetListenerUserData
{
	TWeakObjectPtr<UPubnubSubscriptionSet> WeakSubscriptionSet;
};
