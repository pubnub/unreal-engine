// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"
#include "FunctionLibraries/PubnubUtilities.h"

THIRD_PARTY_INCLUDES_START
#include "PubNub.h"
THIRD_PARTY_INCLUDES_END

class UPubnubSubscriptionBase;

/**
 * Heap payload registered as PubNub C-Core listener user_data for an entity subscription
 * or subscription set. The weak pointer stays safe if the UObject is destroyed before a
 * callback that was already queued on the C-Core thread.
 * Context is borrowed for the lifetime of the listener and is used only inside the callback
 * to copy event views into UE strings before the callback returns.
 */
struct FPubnubInternalEntityListenerUserData
{
	TWeakObjectPtr<UPubnubSubscriptionBase> WeakSubscription;
	pubnub_context_t* Context = nullptr;
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

/** One string field from a set-metadata JSON object. bSet is false when the field was absent. */
struct FMetadataStringField
{
	FString Value;
	bool bSet = false;
};

/** User metadata JSON split into the fields pubnub_set_uuid_metadata accepts. */
struct FParsedUserMetadataObject
{
	FMetadataStringField Name;
	FMetadataStringField ExternalId;
	FMetadataStringField ProfileUrl;
	FMetadataStringField Email;
	FMetadataStringField Status;
	FMetadataStringField Type;
	FString CustomJson;
	bool bHasCustom = false;
	TArray<FString> NullStringFields;
	TArray<FString> UnknownFields;
	FString Error;
};

/** Channel metadata JSON split into the fields pubnub_set_channel_metadata accepts. */
struct FParsedChannelMetadataObject
{
	FMetadataStringField Name;
	FMetadataStringField Description;
	FMetadataStringField Status;
	FMetadataStringField Type;
	FString CustomJson;
	bool bHasCustom = false;
	TArray<FString> NullStringFields;
	TArray<FString> UnknownFields;
	FString Error;
};

/** One App Context include token and the C-Core flag it maps to. */
struct FAppContextIncludeToken
{
	const TCHAR* Name = nullptr;
	uint32 Flag = 0;
};

/** One membership or channel-member item split into the fields the new C-Core set API accepts. */
struct FParsedRelationInput
{
	FString Id;
	FMetadataStringField Status;
	FMetadataStringField Type;
	FString CustomJson;
	bool bHasCustom = false;
};

/** Parsed set or remove JSON array. Error is set when parsing fails. */
struct FParsedRelationList
{
	TArray<FParsedRelationInput> Items;
	TArray<FString> NullStringFields;
	TArray<FString> UnknownFields;
	FString Error;
};

/** UTF-8 storage for pubnub_membership_input_t pointers. Holders must outlive the C-Core call. */
struct FMembershipInputBatch
{
	TArray<TUniquePtr<FUTF8StringHolder>> Holders;
	TArray<pubnub_membership_input_t> Items;

	const char* Hold(const FString& Value)
	{
		TUniquePtr<FUTF8StringHolder> Holder = MakeUnique<FUTF8StringHolder>(Value);
		const char* Pointer = Holder->Get();
		Holders.Add(MoveTemp(Holder));
		return Pointer;
	}
};

/** UTF-8 storage for pubnub_member_input_t pointers. Holders must outlive the C-Core call. */
struct FMemberInputBatch
{
	TArray<TUniquePtr<FUTF8StringHolder>> Holders;
	TArray<pubnub_member_input_t> Items;

	const char* Hold(const FString& Value)
	{
		TUniquePtr<FUTF8StringHolder> Holder = MakeUnique<FUTF8StringHolder>(Value);
		const char* Pointer = Holder->Get();
		Holders.Add(MoveTemp(Holder));
		return Pointer;
	}
};
