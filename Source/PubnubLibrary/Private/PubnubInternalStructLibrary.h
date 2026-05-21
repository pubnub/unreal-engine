// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"

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

/**
 * RAII guard around a non-blocking acquisition of an FCriticalSection.
 *
 * The guard is intentionally constructed in two phases (construct + TryLock) so it
 * can be declared at function scope by a macro, and only the TryLock() result needs
 * to drive control flow. If TryLock() succeeds the lock is automatically released
 * when the enclosing function returns; if it fails the destructor is a no-op.
 *
 * Used by PUBNUB_TRY_LOCK_MUTEX_* macros (PubnubInternalMacros.h) to guarantee that
 * the operation mutex is never leaked across early returns and is always released
 * after every code path that touches a Pubnub C-Core context (including the
 * post-pubnub_await result accessors such as pubnub_last_http_code /
 * pubnub_last_publish_result), so DeinitializeClient can safely free those
 * contexts under the same mutex.
 */
struct FPubnubOperationLockGuard
{
	explicit FPubnubOperationLockGuard(FCriticalSection& InMutex)
		: Mutex(InMutex)
		, bLocked(false)
	{
	}

	~FPubnubOperationLockGuard()
	{
		if (bLocked)
		{
			Mutex.Unlock();
		}
	}

	bool TryLock()
	{
		bLocked = Mutex.TryLock();
		return bLocked;
	}

	FPubnubOperationLockGuard(const FPubnubOperationLockGuard&) = delete;
	FPubnubOperationLockGuard& operator=(const FPubnubOperationLockGuard&) = delete;

private:
	FCriticalSection& Mutex;
	bool bLocked;
};
