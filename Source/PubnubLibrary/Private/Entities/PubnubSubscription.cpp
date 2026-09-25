// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubSubscription.h"
#include "PubnubSubsystem.h"
#include "Entities/PubnubBaseEntity.h"
#include "FunctionLibraries/PubnubUtilities.h"
#include "FunctionLibraries/PubnubInternalUtilities.h"
#include "PubnubInternalMacros.h"
#include "PubnubInternalStructLibrary.h"
#include "PubNub.h"

void UPubnubSubscriptionBase::BeginDestroy()
{
	CleanUpSubscription();
	Super::BeginDestroy();
}

void UPubnubSubscriptionBase::DeliverSubscribeEvent(const FPubnubMessageData& MessageData)
{
	if (!IsInitialized)
	{
		return;
	}

	const bool bPresence = MessageData.Channel.EndsWith(TEXT("-pnpres"));
	switch (MessageData.MessageType)
	{
	case EPubnubMessageType::PMT_Signal:
		if (IsInitialized) { OnPubnubSignal.Broadcast(MessageData); }
		if (IsInitialized) { OnPubnubSignalNative.Broadcast(MessageData); }
		break;
	case EPubnubMessageType::PMT_Action:
		if (IsInitialized) { OnPubnubMessageAction.Broadcast(MessageData); }
		if (IsInitialized) { OnPubnubMessageActionNative.Broadcast(MessageData); }
		break;
	case EPubnubMessageType::PMT_Objects:
		if (IsInitialized) { OnPubnubObjectEvent.Broadcast(MessageData); }
		if (IsInitialized) { OnPubnubObjectEventNative.Broadcast(MessageData); }
		break;
	case EPubnubMessageType::PMT_Files:
		break;
	case EPubnubMessageType::PMT_Published:
	default:
		if (bPresence)
		{
			if (IsInitialized) { OnPubnubPresenceEvent.Broadcast(MessageData); }
			if (IsInitialized) { OnPubnubPresenceEventNative.Broadcast(MessageData); }
		}
		else
		{
			if (IsInitialized) { OnPubnubMessage.Broadcast(MessageData); }
			if (IsInitialized) { OnPubnubMessageNative.Broadcast(MessageData); }
		}
		break;
	}

	if (IsInitialized)
	{
		FOnPubnubAnyMessageType.Broadcast(MessageData);
	}
	if (IsInitialized)
	{
		FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
	}
}

FPubnubOperationResult UPubnubSubscription::Subscribe(FPubnubSubscriptionCursor Cursor)
{
	PUBNUB_ENTITY_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();

	if (!CCoreSubscription)
	{
		return FPubnubOperationResult({0, true, TEXT("Internal CCoreSubscription is invalid.")});
	}

	if (bIsSubscribed)
	{
		return FPubnubOperationResult({0, true, TEXT("Subscription is already subscribed.")});
	}

	return PubnubClient->SubscribeWithSubscription(this, Cursor);
}

void UPubnubSubscription::SubscribeAsync(FOnPubnubSubscribeOperationResponse OnSubscribeResponse, FPubnubSubscriptionCursor Cursor)
{
	FOnPubnubSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSubscribeResponse](FPubnubOperationResult Result)
	{
		OnSubscribeResponse.ExecuteIfBound(Result);
	});

	SubscribeAsync(NativeCallback, Cursor);
}

void UPubnubSubscription::SubscribeAsync(FOnPubnubSubscribeOperationResponseNative NativeCallback, FPubnubSubscriptionCursor Cursor)
{
	PUBNUB_ENTITY_ENSURE_CLIENT_INITIALIZED(NativeCallback);

	if (!CCoreSubscription)
	{
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(NativeCallback, TEXT("Internal CCoreSubscription is invalid."));
		return;
	}

	if (bIsSubscribed)
	{
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(NativeCallback, TEXT("Subscription is already subscribed."));
		return;
	}

	PubnubClient->SubscribeWithSubscriptionAsync(this, Cursor, NativeCallback);
}

void UPubnubSubscription::SubscribeAsync(FPubnubSubscriptionCursor Cursor)
{
	SubscribeAsync(nullptr, Cursor);
}

FPubnubOperationResult UPubnubSubscription::Unsubscribe()
{
	PUBNUB_ENTITY_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();

	if (!CCoreSubscription)
	{
		return FPubnubOperationResult({0, true, TEXT("Internal CCoreSubscription is invalid.")});
	}

	if (!bIsSubscribed)
	{
		return FPubnubOperationResult({0, true, TEXT("Subscription is not subscribed")});
	}

	return PubnubClient->UnsubscribeWithSubscription(this);
}

void UPubnubSubscription::UnsubscribeAsync(FOnPubnubSubscribeOperationResponse OnUnsubscribeResponse)
{
	FOnPubnubSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeResponse.ExecuteIfBound(Result);
	});

	UnsubscribeAsync(NativeCallback);
}

void UPubnubSubscription::UnsubscribeAsync(FOnPubnubSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENTITY_ENSURE_CLIENT_INITIALIZED(NativeCallback);

	if (!CCoreSubscription)
	{
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(NativeCallback, TEXT("Internal CCoreSubscription is invalid."));
		return;
	}

	if (!bIsSubscribed)
	{
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(NativeCallback, TEXT("Subscription is not subscribed."));
		return;
	}

	PubnubClient->UnsubscribeWithSubscriptionAsync(this, NativeCallback);
}

UPubnubSubscriptionSet* UPubnubSubscription::AddSubscription(UPubnubSubscription* Subscription)
{
	if (!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: This Subscription is invalid. Probably PubnubClient was deinitialized. Initialize it again and create new subscription."));
		return nullptr;
	}

	if (!CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: internal C-Core subscription is invalid."));
		return nullptr;
	}

	if (!Subscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: Can't add invalid subscription."));
		return nullptr;
	}

	if (!Subscription->CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: Provided Subscription's internal C-Core subscription is invalid."));
		return nullptr;
	}

	UPubnubSubscriptionSet* SubscriptionSet = UPubnubInternalUtilities::SafeNewObject<UPubnubSubscriptionSet>(this);
	SubscriptionSet->InitWithSubscriptions(PubnubClient, this, Subscription);
	return SubscriptionSet;
}

void UPubnubSubscription::InitSubscription(UPubnubClient* InPubnubClient, UPubnubBaseEntity* Entity, FPubnubSubscribeSettings InSubscribeSettings)
{
	if (!InPubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize subscription, PubnubClient is invalid."));
		return;
	}
	if (!Entity)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize subscription, Entity is invalid."));
		return;
	}

	PubnubClient = InPubnubClient;
	CCoreSubscription = UPubnubInternalUtilities::CreateCCoreSubscription(InPubnubClient->pubnub_context, Entity->EntityID, Entity->EntityType, InSubscribeSettings);
	if (!CCoreSubscription)
	{
		return;
	}

	InternalInit();
	if (IsInitialized)
	{
		InPubnubClient->RegisterManagedSubscription(CCoreSubscription, this);
	}
}

void UPubnubSubscription::InitWithCCoreSubscription(UPubnubClient* InPubnubClient, pubnub_subscription_t InCCoreSubscription)
{
	if (!InPubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize subscription, PubnubClient is invalid."));
		return;
	}
	if (!InCCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize subscription, InCCoreSubscription is invalid."));
		return;
	}

	PubnubClient = InPubnubClient;
	CCoreSubscription = InCCoreSubscription;
	InternalInit();
	if (IsInitialized)
	{
		InPubnubClient->RegisterManagedSubscription(CCoreSubscription, this);
	}
}

void UPubnubSubscription::InternalInit()
{
	ListenerUserData = UPubnubInternalUtilities::CreateEntityListenerUserData(this, PubnubClient ? PubnubClient->pubnub_context : nullptr);
	ListenerHandle = UPubnubInternalUtilities::AddEntitySubscriptionListener(CCoreSubscription, ListenerUserData);
	if (ListenerHandle == PUBNUB_LISTENER_HANDLE_INVALID)
	{
		UE_LOG(PubnubLog, Error, TEXT("Failed to register subscription listener."));
		UPubnubInternalUtilities::DestroyEntityListenerUserData(ListenerUserData);
		ListenerUserData = nullptr;
		pubnub_subscription_destroy(CCoreSubscription);
		CCoreSubscription = nullptr;
		return;
	}

	PubnubClient->OnClientDeinitializeStart.AddDynamic(this, &UPubnubSubscription::CleanUpSubscription);
	IsInitialized = true;
}

void UPubnubSubscription::RemoveRegisteredListeners()
{
	UPubnubInternalUtilities::RemoveEntitySubscriptionListener(CCoreSubscription, ListenerHandle);
	ListenerHandle = PUBNUB_LISTENER_HANDLE_INVALID;
}

void UPubnubSubscription::CleanUpSubscription()
{
	if (!IsInitialized)
	{
		return;
	}

	IsInitialized = false;
	bIsSubscribed = false;
	UPubnubInternalUtilities::ClearSubscriptionDelegates(this);

	if (IsValid(PubnubClient))
	{
		RemoveRegisteredListeners();
	}

	if (IsValid(PubnubClient) && CCoreSubscription)
	{
		PubnubClient->UnregisterManagedSubscription(CCoreSubscription);
	}

	if (CCoreSubscription && IsValid(PubnubClient))
	{
		pubnub_subscription_destroy(CCoreSubscription);
	}
	CCoreSubscription = nullptr;

	UPubnubInternalUtilities::DestroyEntityListenerUserData(ListenerUserData);
	ListenerUserData = nullptr;

	if (PubnubClient)
	{
		PubnubClient->OnClientDeinitializeStart.RemoveDynamic(this, &UPubnubSubscription::CleanUpSubscription);
	}
}

FPubnubOperationResult UPubnubSubscriptionSet::Subscribe(FPubnubSubscriptionCursor Cursor)
{
	PUBNUB_ENTITY_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();

	if (!CCoreSubscriptionSet)
	{
		return FPubnubOperationResult({0, true, TEXT("CCoreSubscriptionSet is invalid.")});
	}

	if (bIsSubscribed)
	{
		return FPubnubOperationResult({0, true, TEXT("SubscriptionSet is already subscribed.")});
	}

	return PubnubClient->SubscribeWithSubscriptionSet(this, Cursor);
}

void UPubnubSubscriptionSet::SubscribeAsync(FOnPubnubSubscribeOperationResponse OnSubscribeResponse, FPubnubSubscriptionCursor Cursor)
{
	FOnPubnubSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSubscribeResponse](FPubnubOperationResult Result)
	{
		OnSubscribeResponse.ExecuteIfBound(Result);
	});

	SubscribeAsync(NativeCallback, Cursor);
}

void UPubnubSubscriptionSet::SubscribeAsync(FOnPubnubSubscribeOperationResponseNative NativeCallback, FPubnubSubscriptionCursor Cursor)
{
	PUBNUB_ENTITY_ENSURE_CLIENT_INITIALIZED(NativeCallback);

	if (!CCoreSubscriptionSet)
	{
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(NativeCallback, TEXT("Internal CCoreSubscriptionSet is invalid."));
		return;
	}

	if (bIsSubscribed)
	{
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(NativeCallback, TEXT("SubscriptionSet is already subscribed."));
		return;
	}

	PubnubClient->SubscribeWithSubscriptionSetAsync(this, Cursor, NativeCallback);
}

void UPubnubSubscriptionSet::SubscribeAsync(FPubnubSubscriptionCursor Cursor)
{
	SubscribeAsync(nullptr, Cursor);
}

FPubnubOperationResult UPubnubSubscriptionSet::Unsubscribe()
{
	PUBNUB_ENTITY_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();

	if (!CCoreSubscriptionSet)
	{
		return FPubnubOperationResult({0, true, TEXT("Internal CCoreSubscriptionSet is invalid.")});
	}

	if (!bIsSubscribed)
	{
		return FPubnubOperationResult({0, true, TEXT("SubscriptionSet is not subscribed")});
	}

	return PubnubClient->UnsubscribeWithSubscriptionSet(this);
}

void UPubnubSubscriptionSet::UnsubscribeAsync(FOnPubnubSubscribeOperationResponse OnUnsubscribeResponse)
{
	FOnPubnubSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeResponse.ExecuteIfBound(Result);
	});

	UnsubscribeAsync(NativeCallback);
}

void UPubnubSubscriptionSet::UnsubscribeAsync(FOnPubnubSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENTITY_ENSURE_CLIENT_INITIALIZED(NativeCallback);

	if (!CCoreSubscriptionSet)
	{
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(NativeCallback, TEXT("Internal CCoreSubscriptionSet is invalid."));
		return;
	}

	if (!bIsSubscribed)
	{
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(NativeCallback, TEXT("SubscriptionSet is not subscribed."));
		return;
	}

	PubnubClient->UnsubscribeWithSubscriptionSetAsync(this, NativeCallback);
}

void UPubnubSubscriptionSet::AddSubscription(UPubnubSubscription* Subscription)
{
	if (!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: This SubscriptionSet is invalid. Probably PubnubClient was deinitialized. Initialize it again and create new subscription."));
		return;
	}

	if (!CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: internal C-Core subscription set is invalid."));
		return;
	}

	if (!Subscription || !Subscription->CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: Provided Subscription's internal C-Core subscription is invalid."));
		return;
	}

	const pubnub_res_t AddResult = pubnub_subscription_set_add_subscription(CCoreSubscriptionSet, Subscription->CCoreSubscription);
	if (AddResult != PUBNUB_OK)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: failed to add subscription. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(AddResult)));
		return;
	}

	Subscriptions.AddUnique(Subscription);
}

void UPubnubSubscriptionSet::RemoveSubscription(UPubnubSubscription* Subscription)
{
	if (!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscription]: This SubscriptionSet is invalid. Probably PubnubClient was deinitialized. Initialize it again and create new subscription."));
		return;
	}

	if (!CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscription]: internal C-Core subscription set is invalid."));
		return;
	}

	if (!Subscription || !Subscription->CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscription]: Provided Subscription's internal C-Core subscription is invalid."));
		return;
	}

	const pubnub_res_t RemoveResult = pubnub_subscription_set_remove_subscription(CCoreSubscriptionSet, Subscription->CCoreSubscription);
	if (RemoveResult != PUBNUB_OK)
	{
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscription]: failed to remove subscription. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(RemoveResult)));
		return;
	}

	Subscriptions.Remove(Subscription);
}

void UPubnubSubscriptionSet::AddSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet)
{
	if (!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscriptionSet]: This SubscriptionSet is invalid. Probably PubnubClient was deinitialized. Initialize it again and create new subscription."));
		return;
	}

	if (!CCoreSubscriptionSet || !SubscriptionSet || !SubscriptionSet->CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscriptionSet]: internal C-Core subscription set is invalid."));
		return;
	}

	const pubnub_res_t AddResult = pubnub_subscription_set_add_subscription_set(CCoreSubscriptionSet, SubscriptionSet->CCoreSubscriptionSet);
	if (AddResult != PUBNUB_OK)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscriptionSet]: failed to merge subscription set. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(AddResult)));
		return;
	}

	for (UPubnubSubscription* Subscription : SubscriptionSet->Subscriptions)
	{
		Subscriptions.AddUnique(Subscription);
	}
}

void UPubnubSubscriptionSet::RemoveSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet)
{
	if (!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscriptionSet]: This SubscriptionSet is invalid. Probably PubnubClient was deinitialized. Initialize it again and create new subscription."));
		return;
	}

	if (!CCoreSubscriptionSet || !SubscriptionSet || !SubscriptionSet->CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscriptionSet]: internal C-Core subscription set is invalid."));
		return;
	}

	const pubnub_res_t RemoveResult = pubnub_subscription_set_remove_subscription_set(CCoreSubscriptionSet, SubscriptionSet->CCoreSubscriptionSet);
	if (RemoveResult != PUBNUB_OK)
	{
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscriptionSet]: failed to subtract subscription set. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(RemoveResult)));
		return;
	}

	for (UPubnubSubscription* Subscription : SubscriptionSet->Subscriptions)
	{
		Subscriptions.Remove(Subscription);
	}
}

void UPubnubSubscriptionSet::InitSubscriptionSet(UPubnubClient* InPubnubClient, TArray<FString> Channels, TArray<FString> ChannelGroups, FPubnubSubscribeSettings InSubscribeSettings)
{
	InitSubscriptionSet(InPubnubClient, Channels, ChannelGroups, TArray<FString>(), TArray<FString>(), InSubscribeSettings);
}

void UPubnubSubscriptionSet::InitSubscriptionSet(UPubnubClient* InPubnubClient, TArray<FString> Channels, TArray<FString> ChannelGroups, TArray<FString> ChannelMetadataIds, TArray<FString> UserMetadataIds, FPubnubSubscribeSettings InSubscribeSettings)
{
	if (!InPubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize SubscriptionSet, PubnubClient is invalid."));
		return;
	}
	if (Channels.IsEmpty() && ChannelGroups.IsEmpty() && ChannelMetadataIds.IsEmpty() && UserMetadataIds.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize SubscriptionSet, at least one entity is needed."));
		return;
	}

	pubnub_subscription_set_t SubscriptionSet = UPubnubInternalUtilities::CreateCCoreSubscriptionSet(InPubnubClient->pubnub_context);
	if (!SubscriptionSet)
	{
		return;
	}

	auto AddIds = [&](const TArray<FString>& Ids, EPubnubEntityType EntityType) -> bool
	{
		for (const FString& Id : Ids)
		{
			if (!UPubnubInternalUtilities::AddEntityToCCoreSubscriptionSet(InPubnubClient->pubnub_context, SubscriptionSet, Id, EntityType, InSubscribeSettings))
			{
				return false;
			}
		}
		return true;
	};

	const bool bAdded = AddIds(Channels, EPubnubEntityType::PEnT_Channel)
		&& AddIds(ChannelGroups, EPubnubEntityType::PEnT_ChannelGroup)
		&& AddIds(ChannelMetadataIds, EPubnubEntityType::PEnT_ChannelMetadata)
		&& AddIds(UserMetadataIds, EPubnubEntityType::PEnT_UserMetadata);
	if (!bAdded)
	{
		pubnub_subscription_set_destroy(SubscriptionSet);
		return;
	}

	PubnubClient = InPubnubClient;
	CCoreSubscriptionSet = SubscriptionSet;
	InternalInit();
	if (IsInitialized)
	{
		InPubnubClient->RegisterManagedSubscriptionSet(CCoreSubscriptionSet, this);
	}
}

void UPubnubSubscriptionSet::InitWithSubscriptions(UPubnubClient* InPubnubClient, UPubnubSubscription* Subscription1, UPubnubSubscription* Subscription2)
{
	if (!InPubnubClient || !Subscription1 || !Subscription2 || !Subscription1->CCoreSubscription || !Subscription2->CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize SubscriptionSet, One of provided subscriptions is invalid."));
		return;
	}

	pubnub_subscription_set_t SubscriptionSet = UPubnubInternalUtilities::CreateCCoreSubscriptionSet(InPubnubClient->pubnub_context);
	if (!SubscriptionSet)
	{
		return;
	}

	const pubnub_res_t FirstAdd = pubnub_subscription_set_add_subscription(SubscriptionSet, Subscription1->CCoreSubscription);
	const pubnub_res_t SecondAdd = FirstAdd == PUBNUB_OK
		? pubnub_subscription_set_add_subscription(SubscriptionSet, Subscription2->CCoreSubscription)
		: FirstAdd;
	if (SecondAdd != PUBNUB_OK)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize SubscriptionSet. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(SecondAdd)));
		pubnub_subscription_set_destroy(SubscriptionSet);
		return;
	}

	PubnubClient = InPubnubClient;
	CCoreSubscriptionSet = SubscriptionSet;
	Subscriptions.Add(Subscription1);
	Subscriptions.Add(Subscription2);
	InternalInit();
	if (IsInitialized)
	{
		InPubnubClient->RegisterManagedSubscriptionSet(CCoreSubscriptionSet, this);
	}
}

void UPubnubSubscriptionSet::InitWithCCoreSubscriptionSet(UPubnubClient* InPubnubClient, pubnub_subscription_set_t InCCoreSubscriptionSet)
{
	if (!InPubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize SubscriptionSet, PubnubClient is invalid."));
		return;
	}
	if (!InCCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize SubscriptionSet, InCCoreSubscriptionSet is invalid."));
		return;
	}

	PubnubClient = InPubnubClient;
	CCoreSubscriptionSet = InCCoreSubscriptionSet;
	InternalInit();
	if (IsInitialized)
	{
		InPubnubClient->RegisterManagedSubscriptionSet(CCoreSubscriptionSet, this);
	}
}

void UPubnubSubscriptionSet::InternalInit()
{
	ListenerUserData = UPubnubInternalUtilities::CreateEntityListenerUserData(this, PubnubClient ? PubnubClient->pubnub_context : nullptr);
	ListenerHandle = UPubnubInternalUtilities::AddEntitySubscriptionSetListener(CCoreSubscriptionSet, ListenerUserData);
	if (ListenerHandle == PUBNUB_LISTENER_HANDLE_INVALID)
	{
		UE_LOG(PubnubLog, Error, TEXT("Failed to register subscription set listener."));
		UPubnubInternalUtilities::DestroyEntityListenerUserData(ListenerUserData);
		ListenerUserData = nullptr;
		pubnub_subscription_set_destroy(CCoreSubscriptionSet);
		CCoreSubscriptionSet = nullptr;
		return;
	}

	PubnubClient->OnClientDeinitializeStart.AddDynamic(this, &UPubnubSubscriptionSet::CleanUpSubscription);
	IsInitialized = true;
}

void UPubnubSubscriptionSet::RemoveRegisteredListeners()
{
	UPubnubInternalUtilities::RemoveEntitySubscriptionSetListener(CCoreSubscriptionSet, ListenerHandle);
	ListenerHandle = PUBNUB_LISTENER_HANDLE_INVALID;
}

void UPubnubSubscriptionSet::CleanUpSubscription()
{
	if (!IsInitialized)
	{
		return;
	}

	IsInitialized = false;
	bIsSubscribed = false;
	UPubnubInternalUtilities::ClearSubscriptionDelegates(this);

	if (IsValid(PubnubClient))
	{
		RemoveRegisteredListeners();
	}

	if (IsValid(PubnubClient) && CCoreSubscriptionSet)
	{
		PubnubClient->UnregisterManagedSubscriptionSet(CCoreSubscriptionSet);
	}

	if (CCoreSubscriptionSet && IsValid(PubnubClient))
	{
		pubnub_subscription_set_destroy(CCoreSubscriptionSet);
	}
	CCoreSubscriptionSet = nullptr;

	UPubnubInternalUtilities::DestroyEntityListenerUserData(ListenerUserData);
	ListenerUserData = nullptr;

	if (PubnubClient)
	{
		PubnubClient->OnClientDeinitializeStart.RemoveDynamic(this, &UPubnubSubscriptionSet::CleanUpSubscription);
	}
}
