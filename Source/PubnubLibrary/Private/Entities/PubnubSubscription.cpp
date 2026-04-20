// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubSubscription.h"
#include "PubnubSubsystem.h"
#include "Entities/PubnubBaseEntity.h"
#include "FunctionLibraries/PubnubUtilities.h"
#include "FunctionLibraries/PubnubInternalUtilities.h"
#include "PubnubInternalMacros.h"
#include "PubnubInternalStructLibrary.h"

void UPubnubSubscriptionBase::BeginDestroy()
{
	CleanUpSubscription();
	
	Super::BeginDestroy();
}

FPubnubOperationResult UPubnubSubscription::Subscribe(FPubnubSubscriptionCursor Cursor)
{
	PUBNUB_ENTITY_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	
	if(!CCoreSubscription)
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
	
	if(!CCoreSubscription)
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
	
	if(!CCoreSubscription)
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
	
	if(!CCoreSubscription)
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
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: This Subscription is invalid. Probably PubnubClient was deinitialized. Initialize it again and create new subscription."));
		return nullptr;
	}

	if(!CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: internal C-Core subscription is invalid."));
		return nullptr;
	}

	if(!Subscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: Can't add invalid subscription."));
		return nullptr;
	}

	if(!Subscription->CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: Provided Subscription's internal C-Core subscription is invalid."));
		return nullptr;
	}

	UPubnubSubscriptionSet* SubscriptionSet = NewObject<UPubnubSubscriptionSet>(this);
	SubscriptionSet->InitWithSubscriptions(PubnubClient, this, Subscription);
	
	return SubscriptionSet;
}

void UPubnubSubscription::InitSubscription(UPubnubClient* InPubnubClient, UPubnubBaseEntity* Entity, FPubnubSubscribeSettings InSubscribeSettings)
{
	if(!InPubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize subscription, PubnubClient is invalid."));
		return;
	}
	if(!Entity)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize subscription, Entity is invalid."));
		return;
	}
	PubnubClient = InPubnubClient;
	CCoreSubscription = UPubnubInternalUtilities::EEGetSubscriptionForEntity(InPubnubClient->ctx_ee, Entity->EntityID, Entity->EntityType, InSubscribeSettings);

	InternalInit();
}

void UPubnubSubscription::InitWithCCoreSubscription(UPubnubClient* InPubnubClient, pubnub_subscription_t* InCCoreSubscription)
{
	if(!InPubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize subscription, PubnubClient is invalid."));
		return;
	}
	if(!InCCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize subscription, InCCoreSubscription is invalid."));
		return;
	}
	PubnubClient = InPubnubClient;
	CCoreSubscription = InCCoreSubscription;

	InternalInit();
}

void UPubnubSubscription::InternalInit()
{
	// Set weak pointer to this object in the heap payload
	FPubnubInternalSubscriptionListenerUserData* UserData = new FPubnubInternalSubscriptionListenerUserData();
	UserData->WeakSubscription = this;
	ListenerUserData = UserData;

	// Create callbacks for each Listener type

	// Messages and Presence
	pubnub_subscribe_message_callback_t CallbackMessages = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		if(!user_data)
		{return;}
		
		FPubnubInternalSubscriptionListenerUserData* ListenerUserDataPtr = static_cast<FPubnubInternalSubscriptionListenerUserData*>(user_data);
		TWeakObjectPtr<UPubnubSubscription> SubscriptionWeak = ListenerUserDataPtr->WeakSubscription;
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscription* S = SubscriptionWeak.Get(); IsValid(S) && S->IsInitialized)
			{
				if(!S->PubnubClient)
				{return;}
				// In C-Core there is no separate listener for Presence Events. They come together with published messages.
				// So we check if it's Presence Event here and choose equivalent delegates to call
				// Check IsInitialized before each broadcast to prevent race conditions during destruction
				if(MessageData.Channel.Contains("-pnpres"))
				{
					// Subscription could be deinitialized from user's logic on any of these calls, so we need to check IsInitialized for every broadcast 
					if(S->IsInitialized)
					{
						S->OnPubnubPresenceEvent.Broadcast(MessageData);
					}
					if(S->IsInitialized)
					{
						S->OnPubnubPresenceEventNative.Broadcast(MessageData);
					}
				}
				else
				{
					if(S->IsInitialized)
					{
						S->OnPubnubMessage.Broadcast(MessageData);
					}
					if(S->IsInitialized)
					{
						S->OnPubnubMessageNative.Broadcast(MessageData);
					}
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
				}
			}
		});
	};

	// Signals
	pubnub_subscribe_message_callback_t CallbackSignals = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		if(!user_data)
		{return;}
		
		FPubnubInternalSubscriptionListenerUserData* ListenerUserDataPtr = static_cast<FPubnubInternalSubscriptionListenerUserData*>(user_data);
		TWeakObjectPtr<UPubnubSubscription> SubscriptionWeak = ListenerUserDataPtr->WeakSubscription;
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscription* S = SubscriptionWeak.Get(); IsValid(S) && S->IsInitialized)
			{
				if(!S->PubnubClient)
				{return;}
				// Subscription could be deinitialized from user's logic on any of these calls, so we need to check IsInitialized for every broadcast
				if(S->IsInitialized)
				{
					S->OnPubnubSignal.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->OnPubnubSignalNative.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
				}
			}
		});
	};

	// Objects (App Context)
	pubnub_subscribe_message_callback_t CallbackObjects = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		if(!user_data)
		{return;}
		
		FPubnubInternalSubscriptionListenerUserData* ListenerUserDataPtr = static_cast<FPubnubInternalSubscriptionListenerUserData*>(user_data);
		TWeakObjectPtr<UPubnubSubscription> SubscriptionWeak = ListenerUserDataPtr->WeakSubscription;
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscription* S = SubscriptionWeak.Get(); IsValid(S) && S->IsInitialized)
			{
				if(!S->PubnubClient)
				{return;}
				// Subscription could be deinitialized from user's logic on any of these calls, so we need to check IsInitialized for every broadcast
				if(S->IsInitialized)
				{
					S->OnPubnubObjectEvent.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->OnPubnubObjectEventNative.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
				}
			}
		});
	};

	// Message Actions
	pubnub_subscribe_message_callback_t CallbackMessageActions= +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		if(!user_data)
		{return;}
		
		FPubnubInternalSubscriptionListenerUserData* ListenerUserDataPtr = static_cast<FPubnubInternalSubscriptionListenerUserData*>(user_data);
		TWeakObjectPtr<UPubnubSubscription> SubscriptionWeak = ListenerUserDataPtr->WeakSubscription;
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscription* S = SubscriptionWeak.Get(); IsValid(S) && S->IsInitialized)
			{
				if(!S->PubnubClient)
				{return;}
				// Subscription could be deinitialized from user's logic on any of these calls, so we need to check IsInitialized for every broadcast
				if(S->IsInitialized)
				{
					S->OnPubnubMessageAction.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->OnPubnubMessageActionNative.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
				}
			}
		});
	};

	// Register created callback in subscription
	UPubnubInternalUtilities::EEAddSubscriptionListenerOfType(CCoreSubscription, CallbackMessages, EPubnubListenerType::PLT_Message, ListenerUserData);
	UPubnubInternalUtilities::EEAddSubscriptionListenerOfType(CCoreSubscription, CallbackSignals, EPubnubListenerType::PLT_Signal, ListenerUserData);
	UPubnubInternalUtilities::EEAddSubscriptionListenerOfType(CCoreSubscription, CallbackObjects, EPubnubListenerType::PLT_Objects, ListenerUserData);
	UPubnubInternalUtilities::EEAddSubscriptionListenerOfType(CCoreSubscription, CallbackMessageActions, EPubnubListenerType::PLT_MessageAction, ListenerUserData);

	// Bind to deinitialize start so subscription C-Core resources are released
	PubnubClient->OnClientDeinitializeStart.AddDynamic(this, &UPubnubSubscription::CleanUpSubscription);

	//Now we are fully initialized
	IsInitialized = true;
}

void UPubnubSubscription::CleanUpSubscription()
{
	if(!IsInitialized) {return;}

	// Set IsInitialized to false FIRST to prevent any pending async tasks from broadcasting
	IsInitialized = false;
	bIsSubscribed = false;

	// Clear all delegates to prevent broadcasting during destruction
	OnPubnubMessage.Clear();
	OnPubnubMessageNative.Clear();
	OnPubnubSignal.Clear();
	OnPubnubSignalNative.Clear();
	OnPubnubPresenceEvent.Clear();
	OnPubnubPresenceEventNative.Clear();
	OnPubnubObjectEvent.Clear();
	OnPubnubObjectEventNative.Clear();
	OnPubnubMessageAction.Clear();
	OnPubnubMessageActionNative.Clear();
	FOnPubnubAnyMessageType.Clear();
	FOnPubnubAnyMessageTypeNative.Clear();

	if(CCoreSubscription && IsValid(PubnubClient))
	{
		pubnub_subscription_free(&CCoreSubscription);
	}
	CCoreSubscription = nullptr;

	if(ListenerUserData)
	{
		delete static_cast<FPubnubInternalSubscriptionListenerUserData*>(ListenerUserData);
		ListenerUserData = nullptr;
	}

	if(PubnubClient)
	{
		PubnubClient->OnClientDeinitializeStart.RemoveDynamic(this, &UPubnubSubscription::CleanUpSubscription);
	}
}

FPubnubOperationResult UPubnubSubscriptionSet::Subscribe(FPubnubSubscriptionCursor Cursor)
{
	PUBNUB_ENTITY_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();

	if(!CCoreSubscriptionSet)
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
	
	if(!CCoreSubscriptionSet)
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
	
	if(!CCoreSubscriptionSet)
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
	
	if(!CCoreSubscriptionSet)
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
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: This SubscriptionSet is invalid. Probably PubnubClient was deinitialized. Initialize it again and create new subscription."));
		return;
	}

	if(!CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: internal C-Core subscription set is invalid."));
		return;
	}

	if(!Subscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: Can't add invalid subscription."));
		return;
	}

	if(!Subscription->CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: Provided Subscription's internal C-Core subscription is invalid."));
		return;
	}

	Subscriptions.Add(Subscription);

	pubnub_subscription_set_add(CCoreSubscriptionSet, Subscription->CCoreSubscription);
}

void UPubnubSubscriptionSet::RemoveSubscription(UPubnubSubscription* Subscription)
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscription]: This SubscriptionSet is invalid. Probably PubnubClient was deinitialized. Initialize it again and create new subscription."));
		return;
	}

	if(!CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscription]: internal C-Core subscription set is invalid."));
		return;
	}

	if(!Subscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscription]: Can't remove invalid subscription."));
		return;
	}

	if(!Subscription->CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscription]: Provided Subscription's internal C-Core subscription is invalid."));
		return;
	}

	Subscriptions.Remove(Subscription);

	pubnub_subscription_set_remove(CCoreSubscriptionSet, &Subscription->CCoreSubscription);
}

void UPubnubSubscriptionSet::AddSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet)
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscriptionSet]: This SubscriptionSet is invalid. Probably PubnubClient was deinitialized. Initialize it again and create new subscription."));
		return;
	}

	if(!CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscriptionSet]: internal C-Core subscription set is invalid."));
		return;
	}

	if(!SubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscriptionSet]: Can't add invalid subscription set."));
		return;
	}

	if(!SubscriptionSet->CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscriptionSet]: Provided Subscription Set's internal C-Core subscription set is invalid."));
		return;
	}

	pubnub_subscription_set_union(CCoreSubscriptionSet, SubscriptionSet->CCoreSubscriptionSet);
}

void UPubnubSubscriptionSet::RemoveSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet)
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscriptionSet]: This SubscriptionSet is invalid. Probably PubnubClient was deinitialized. Initialize it again and create new subscription."));
		return;
	}

	if(!CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscriptionSet]: internal C-Core subscription set is invalid."));
		return;
	}

	if(!SubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscriptionSet]: Can't remove invalid subscription set."));
		return;
	}

	if(!SubscriptionSet->CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscriptionSet]: Provided Subscription Set's internal C-Core subscription set is invalid."));
		return;
	}

	pubnub_subscription_set_subtract(CCoreSubscriptionSet, SubscriptionSet->CCoreSubscriptionSet);
}

void UPubnubSubscriptionSet::InitSubscriptionSet(UPubnubClient* InPubnubClient, TArray<FString> Channels, TArray<FString> ChannelGroups, FPubnubSubscribeSettings InSubscribeSettings)
{
	if(Channels.IsEmpty() && ChannelGroups.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize SubscriptionSet, at least one Channel or ChannelGroup is needed."));
		return;
	}
	PubnubClient = InPubnubClient;
	CCoreSubscriptionSet = UPubnubInternalUtilities::EEGetSubscriptionSetForEntities(InPubnubClient->ctx_ee, Channels, ChannelGroups, InSubscribeSettings);

	InternalInit();
}

void UPubnubSubscriptionSet::InitWithSubscriptions(UPubnubClient* InPubnubClient, UPubnubSubscription* Subscription1, UPubnubSubscription* Subscription2)
{
	if(!Subscription1 || !Subscription2 || !Subscription1->CCoreSubscription || !Subscription2->CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize SubscriptionSet, One of provided subscriptions is invalid."));
		return;
	}
	
	PubnubClient = InPubnubClient;
	
	CCoreSubscriptionSet = pubnub_subscription_set_alloc_with_subscriptions(Subscription1->CCoreSubscription, Subscription2->CCoreSubscription, nullptr);
	Subscriptions.Add(Subscription1);
	Subscriptions.Add(Subscription2);
	
	InternalInit();
}

void UPubnubSubscriptionSet::InitWithCCoreSubscriptionSet(UPubnubClient* InPubnubClient, pubnub_subscription_set_t* InCCoreSubscriptionSet)
{
	if(!InCCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize SubscriptionSet, InCCoreSubscriptionSet is invalid."));
		return;
	}
	PubnubClient = InPubnubClient;
	CCoreSubscriptionSet = InCCoreSubscriptionSet;

	InternalInit();
}

void UPubnubSubscriptionSet::InternalInit()
{
	// Set weak pointer to this object in the heap payload
	FPubnubInternalSubscriptionSetListenerUserData* UserData = new FPubnubInternalSubscriptionSetListenerUserData();
	UserData->WeakSubscriptionSet = this;
	ListenerUserData = UserData;

	// Create callbacks for each Listener type

	// Messages and Presence
	pubnub_subscribe_message_callback_t CallbackMessages = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		if(!user_data)
		{return;}
		
		FPubnubInternalSubscriptionSetListenerUserData* ListenerUserDataPtr = static_cast<FPubnubInternalSubscriptionSetListenerUserData*>(user_data);
		TWeakObjectPtr<UPubnubSubscriptionSet> SubscriptionWeak = ListenerUserDataPtr->WeakSubscriptionSet;
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscriptionSet* S = SubscriptionWeak.Get(); IsValid(S) && S->IsInitialized)
			{
				// In C-Core there is no separate listener for Presence Events. They come together with published messages.
				// So we check if it's Presence Event here and choose equivalent delegates to call
				// Check IsInitialized before each broadcast to prevent race conditions during destruction
				if(MessageData.Channel.Contains("-pnpres"))
				{
					// Subscription could be deinitialized from user's logic on any of these calls, so we need to check IsInitialized for every broadcast
					if(S->IsInitialized)
					{
						S->OnPubnubPresenceEvent.Broadcast(MessageData);
					}
					if(S->IsInitialized)
					{
						S->OnPubnubPresenceEventNative.Broadcast(MessageData);
					}
				}
				else
				{
					if(S->IsInitialized)
					{
						S->OnPubnubMessage.Broadcast(MessageData);
					}
					if(S->IsInitialized)
					{
						S->OnPubnubMessageNative.Broadcast(MessageData);
					}
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
				}
			}
		});
	};

	// Signals
	pubnub_subscribe_message_callback_t CallbackSignals = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		if(!user_data)
		{return;}
		
		FPubnubInternalSubscriptionSetListenerUserData* ListenerUserDataPtr = static_cast<FPubnubInternalSubscriptionSetListenerUserData*>(user_data);
		TWeakObjectPtr<UPubnubSubscriptionSet> SubscriptionWeak = ListenerUserDataPtr->WeakSubscriptionSet;
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscriptionSet* S = SubscriptionWeak.Get(); IsValid(S) && S->IsInitialized)
			{
				// Subscription could be deinitialized from user's logic on any of these calls, so we need to check IsInitialized for every broadcast
				if(S->IsInitialized)
				{
					S->OnPubnubSignal.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->OnPubnubSignalNative.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
				}
			}
		});
	};

	// Objects (App Context)
	pubnub_subscribe_message_callback_t CallbackObjects = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		if(!user_data)
		{return;}
		
		FPubnubInternalSubscriptionSetListenerUserData* ListenerUserDataPtr = static_cast<FPubnubInternalSubscriptionSetListenerUserData*>(user_data);
		TWeakObjectPtr<UPubnubSubscriptionSet> SubscriptionWeak = ListenerUserDataPtr->WeakSubscriptionSet;
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscriptionSet* S = SubscriptionWeak.Get(); IsValid(S) && S->IsInitialized)
			{
				// Subscription could be deinitialized from user's logic on any of these calls, so we need to check IsInitialized for every broadcast
				if(S->IsInitialized)
				{
					S->OnPubnubObjectEvent.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->OnPubnubObjectEventNative.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
				}
			}
		});
	};

	// Message Actions
	pubnub_subscribe_message_callback_t CallbackMessageActions= +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		if(!user_data)
		{return;}
		
		FPubnubInternalSubscriptionSetListenerUserData* ListenerUserDataPtr = static_cast<FPubnubInternalSubscriptionSetListenerUserData*>(user_data);
		TWeakObjectPtr<UPubnubSubscriptionSet> SubscriptionWeak = ListenerUserDataPtr->WeakSubscriptionSet;
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscriptionSet* S = SubscriptionWeak.Get(); IsValid(S) && S->IsInitialized)
			{
				// Subscription could be deinitialized from user's logic on any of these calls, so we need to check IsInitialized for every broadcast
				if(S->IsInitialized)
				{
					S->OnPubnubMessageAction.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->OnPubnubMessageActionNative.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				}
				if(S->IsInitialized)
				{
					S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
				}
			}
		});
	};

	// Register created callback in subscription
	UPubnubInternalUtilities::EEAddSubscriptionSetListenerOfType(CCoreSubscriptionSet, CallbackMessages, EPubnubListenerType::PLT_Message, ListenerUserData);
	UPubnubInternalUtilities::EEAddSubscriptionSetListenerOfType(CCoreSubscriptionSet, CallbackSignals, EPubnubListenerType::PLT_Signal, ListenerUserData);
	UPubnubInternalUtilities::EEAddSubscriptionSetListenerOfType(CCoreSubscriptionSet, CallbackObjects, EPubnubListenerType::PLT_Objects, ListenerUserData);
	UPubnubInternalUtilities::EEAddSubscriptionSetListenerOfType(CCoreSubscriptionSet, CallbackMessageActions, EPubnubListenerType::PLT_MessageAction, ListenerUserData);

	// Bind to deinitialize start so subscription C-Core resources are released
	PubnubClient->OnClientDeinitializeStart.AddDynamic(this, &UPubnubSubscriptionSet::CleanUpSubscription);

	//Now we are fully initialized
	IsInitialized = true;
}

void UPubnubSubscriptionSet::CleanUpSubscription()
{
	if(!IsInitialized) {return;}

	// Set IsInitialized to false FIRST to prevent any pending async tasks from broadcasting
	IsInitialized = false;
	bIsSubscribed = false;

	// Clear all delegates to prevent broadcasting during destruction
	// This must be done after setting IsInitialized = false so queued async tasks will skip broadcasting
	OnPubnubMessage.Clear();
	OnPubnubMessageNative.Clear();
	OnPubnubSignal.Clear();
	OnPubnubSignalNative.Clear();
	OnPubnubPresenceEvent.Clear();
	OnPubnubPresenceEventNative.Clear();
	OnPubnubObjectEvent.Clear();
	OnPubnubObjectEventNative.Clear();
	OnPubnubMessageAction.Clear();
	OnPubnubMessageActionNative.Clear();
	FOnPubnubAnyMessageType.Clear();
	FOnPubnubAnyMessageTypeNative.Clear();

	if(CCoreSubscriptionSet && IsValid(PubnubClient))
	{
		pubnub_subscription_set_free(&CCoreSubscriptionSet);
	}
	CCoreSubscriptionSet = nullptr;

	if(ListenerUserData)
	{
		delete static_cast<FPubnubInternalSubscriptionSetListenerUserData*>(ListenerUserData);
		ListenerUserData = nullptr;
	}

	if(PubnubClient)
	{
		PubnubClient->OnClientDeinitializeStart.RemoveDynamic(this, &UPubnubSubscriptionSet::CleanUpSubscription);
	}
}
