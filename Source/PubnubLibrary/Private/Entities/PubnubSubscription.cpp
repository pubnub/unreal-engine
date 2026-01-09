// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubSubscription.h"
#include "PubnubSubsystem.h"
#include "Entities/PubnubBaseEntity.h"
#include "FunctionLibraries/PubnubUtilities.h"
#include "FunctionLibraries/PubnubInternalUtilities.h"


void UPubnubSubscriptionBase::BeginDestroy()
{
	CleanUpSubscription();
	
	Super::BeginDestroy();
}

FPubnubOperationResult UPubnubSubscription::Subscribe(FPubnubSubscriptionCursor Cursor)
{
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
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Subscribe]: This Subscription is invalid. Probably PubnubClient was deinitialized. Initialize it again and create new subscription."));
		return;
	}
	
	if(!CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Subscribe]: internal C-Core subscription set is invalid."));
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
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Unsubscribe]: This Subscription is invalid. Probably PubnubClient was deinitialized. Initialize it again and create new subscription."));
		return;
	}
	
	if(!CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Unsubscribe]: internal C-Core subscription set is invalid."));
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
	// Create callbacks for each Listener type

	// Messages and Presence
	pubnub_subscribe_message_callback_t CallbackMessages = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		auto SubscriptionWeak = TWeakObjectPtr<UPubnubSubscription>(static_cast<UPubnubSubscription*>(user_data));
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscription* S = SubscriptionWeak.Get(); IsValid(S))
			{
				// In C-Core there is no separate listener for Presence Events. They come together with published messages.
				// So we check if it's Presence Event here and choose equivalent delegates to call
				if(MessageData.Channel.Contains("-pnpres"))
				{
					S->OnPubnubPresenceEvent.Broadcast(MessageData);
					S->OnPubnubPresenceEventNative.Broadcast(MessageData);
				}
				else
				{
					S->OnPubnubMessage.Broadcast(MessageData);
					S->OnPubnubMessageNative.Broadcast(MessageData);
				}
				S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
			}
		});
	};

	// Signals
	pubnub_subscribe_message_callback_t CallbackSignals = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		auto SubscriptionWeak = TWeakObjectPtr<UPubnubSubscription>(static_cast<UPubnubSubscription*>(user_data));
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscription* S = SubscriptionWeak.Get(); IsValid(S))
			{
				S->OnPubnubSignal.Broadcast(MessageData);
				S->OnPubnubSignalNative.Broadcast(MessageData);
				S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
			}
		});
	};

	// Objects (App Context)
	pubnub_subscribe_message_callback_t CallbackObjects = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		auto SubscriptionWeak = TWeakObjectPtr<UPubnubSubscription>(static_cast<UPubnubSubscription*>(user_data));
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscription* S = SubscriptionWeak.Get(); IsValid(S))
			{
				S->OnPubnubObjectEvent.Broadcast(MessageData);
				S->OnPubnubObjectEventNative.Broadcast(MessageData);
				S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
			}
		});
	};

	// Message Actions
	pubnub_subscribe_message_callback_t CallbackMessageActions= +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		auto SubscriptionWeak = TWeakObjectPtr<UPubnubSubscription>(static_cast<UPubnubSubscription*>(user_data));
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscription* S = SubscriptionWeak.Get(); IsValid(S))
			{
				S->OnPubnubMessageAction.Broadcast(MessageData);
				S->OnPubnubMessageActionNative.Broadcast(MessageData);
				S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
			}
		});
	};

	// Register created callback in subscription
	UPubnubInternalUtilities::EEAddSubscriptionListenerOfType(CCoreSubscription, CallbackMessages, EPubnubListenerType::PLT_Message, this);
	UPubnubInternalUtilities::EEAddSubscriptionListenerOfType(CCoreSubscription, CallbackSignals, EPubnubListenerType::PLT_Signal, this);
	UPubnubInternalUtilities::EEAddSubscriptionListenerOfType(CCoreSubscription, CallbackObjects, EPubnubListenerType::PLT_Objects, this);
	UPubnubInternalUtilities::EEAddSubscriptionListenerOfType(CCoreSubscription, CallbackMessageActions, EPubnubListenerType::PLT_MessageAction, this);

	//Bind to OnPubnubClientDeinitialized so subscription is properly Cleaned up, when it's not needed
	PubnubClient->OnPubnubClientDeinitialized.AddDynamic(this, &UPubnubSubscription::CleanUpSubscription);

	//Now we are fully initialized
	IsInitialized = true;
}

void UPubnubSubscription::CleanUpSubscription()
{
	if(!IsInitialized) {return;}

	if(CCoreSubscription)
	{
		pubnub_subscription_free(&CCoreSubscription);
	}

	if(PubnubClient)
	{
		PubnubClient->OnPubnubClientDeinitialized.RemoveDynamic(this, &UPubnubSubscription::CleanUpSubscription);
	}

	IsInitialized = false;
}

FPubnubOperationResult UPubnubSubscriptionSet::Subscribe(FPubnubSubscriptionCursor Cursor)
{
	return FPubnubOperationResult();
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
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Subscribe]: This SubscriptionSet is invalid. Probably PubnubClient was deinitialized. Initialize it again and create new subscription."));
		return;
	}
	
	if(!CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Subscribe]: internal C-Core subscription set is invalid."));
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
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Unsubscribe]: This SubscriptionSet is invalid. Probably PubnubClient was deinitialized. Initialize it again and create new subscription."));
		return;
	}
	
	if(!CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Unsubscribe]: internal C-Core subscription set is invalid."));
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
	// Create callbacks for each Listener type

	// Messages and Presence
	pubnub_subscribe_message_callback_t CallbackMessages = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		auto SubscriptionWeak = TWeakObjectPtr<UPubnubSubscriptionSet>(static_cast<UPubnubSubscriptionSet*>(user_data));
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscriptionSet* S = SubscriptionWeak.Get(); IsValid(S))
			{
				// In C-Core there is no separate listener for Presence Events. They come together with published messages.
				// So we check if it's Presence Event here and choose equivalent delegates to call
				if(MessageData.Channel.Contains("-pnpres"))
				{
					S->OnPubnubPresenceEvent.Broadcast(MessageData);
					S->OnPubnubPresenceEventNative.Broadcast(MessageData);
				}
				else
				{
					S->OnPubnubMessage.Broadcast(MessageData);
					S->OnPubnubMessageNative.Broadcast(MessageData);
				}
				S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
			}
		});
	};

	// Signals
	pubnub_subscribe_message_callback_t CallbackSignals = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		auto SubscriptionWeak = TWeakObjectPtr<UPubnubSubscriptionSet>(static_cast<UPubnubSubscriptionSet*>(user_data));
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscriptionSet* S = SubscriptionWeak.Get(); IsValid(S))
			{
				S->OnPubnubSignal.Broadcast(MessageData);
				S->OnPubnubSignalNative.Broadcast(MessageData);
				S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
			}
		});
	};

	// Objects (App Context)
	pubnub_subscribe_message_callback_t CallbackObjects = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		auto SubscriptionWeak = TWeakObjectPtr<UPubnubSubscriptionSet>(static_cast<UPubnubSubscriptionSet*>(user_data));
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscriptionSet* S = SubscriptionWeak.Get(); IsValid(S))
			{
				S->OnPubnubObjectEvent.Broadcast(MessageData);
				S->OnPubnubObjectEventNative.Broadcast(MessageData);
				S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
			}
		});
	};

	// Message Actions
	pubnub_subscribe_message_callback_t CallbackMessageActions= +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		auto SubscriptionWeak = TWeakObjectPtr<UPubnubSubscriptionSet>(static_cast<UPubnubSubscriptionSet*>(user_data));
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, SubscriptionWeak]()
		{
			if (UPubnubSubscriptionSet* S = SubscriptionWeak.Get(); IsValid(S))
			{
				S->OnPubnubMessageAction.Broadcast(MessageData);
				S->OnPubnubMessageActionNative.Broadcast(MessageData);
				S->FOnPubnubAnyMessageType.Broadcast(MessageData);
				S->FOnPubnubAnyMessageTypeNative.Broadcast(MessageData);
			}
		});
	};

	// Register created callback in subscription
	UPubnubInternalUtilities::EEAddSubscriptionSetListenerOfType(CCoreSubscriptionSet, CallbackMessages, EPubnubListenerType::PLT_Message, this);
	UPubnubInternalUtilities::EEAddSubscriptionSetListenerOfType(CCoreSubscriptionSet, CallbackSignals, EPubnubListenerType::PLT_Signal, this);
	UPubnubInternalUtilities::EEAddSubscriptionSetListenerOfType(CCoreSubscriptionSet, CallbackObjects, EPubnubListenerType::PLT_Objects, this);
	UPubnubInternalUtilities::EEAddSubscriptionSetListenerOfType(CCoreSubscriptionSet, CallbackMessageActions, EPubnubListenerType::PLT_MessageAction, this);

	//Bind to OnPubnubClientDeinitialized so subscription is properly Cleaned up, when it's not needed
	PubnubClient->OnPubnubClientDeinitialized.AddDynamic(this, &UPubnubSubscriptionSet::CleanUpSubscription);

	//Now we are fully initialized
	IsInitialized = true;
}

void UPubnubSubscriptionSet::CleanUpSubscription()
{
	if(!IsInitialized) {return;}

	if(CCoreSubscriptionSet)
	{
		pubnub_subscription_set_free(&CCoreSubscriptionSet);
	}

	if(PubnubClient)
	{
		PubnubClient->OnPubnubClientDeinitialized.RemoveDynamic(this, &UPubnubSubscriptionSet::CleanUpSubscription);
	}

	IsInitialized = false;
}
