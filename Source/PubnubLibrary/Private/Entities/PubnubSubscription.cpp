// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubSubscription.h"
#include "PubnubSubsystem.h"
#include "Entities/PubnubBaseEntity.h"
#include "FunctionLibraries/PubnubUtilities.h"


void UPubnubSubscriptionBase::BeginDestroy()
{
	CleanUpSubscription();
	
	Super::BeginDestroy();
}

void UPubnubSubscription::Subscribe(FOnSubscribeOperationResponse OnSubscribeResponse, FPubnubSubscriptionCursor Cursor)
{
	FOnSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSubscribeResponse](FPubnubOperationResult Result)
	{
		OnSubscribeResponse.ExecuteIfBound(Result);
	});

	Subscribe(NativeCallback, Cursor);
}

void UPubnubSubscription::Subscribe(FOnSubscribeOperationResponseNative NativeCallback, FPubnubSubscriptionCursor Cursor)
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Subscribe]: This Subscription is invalid. Probably PubnubSubsystem was deinitialized. Initialize it again and create new subscription."));
		return;
	}
	
	if(!CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Subscribe]: internal C-Core subscription set is invalid."));
		return;
	}
	
	PubnubSubsystem->SubscribeWithSubscription(this, Cursor, NativeCallback);
}

void UPubnubSubscription::Subscribe(FPubnubSubscriptionCursor Cursor)
{
	Subscribe(nullptr, Cursor);
}

void UPubnubSubscription::Unsubscribe(FOnSubscribeOperationResponse OnUnsubscribeResponse)
{
	FOnSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeResponse.ExecuteIfBound(Result);
	});

	Unsubscribe(NativeCallback);
}

void UPubnubSubscription::Unsubscribe(FOnSubscribeOperationResponseNative NativeCallback)
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Unsubscribe]: This Subscription is invalid. Probably PubnubSubsystem was deinitialized. Initialize it again and create new subscription."));
		return;
	}
	
	if(!CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Unsubscribe]: internal C-Core subscription set is invalid."));
		return;
	}

	PubnubSubsystem->UnsubscribeWithSubscription(this, NativeCallback);
}

UPubnubSubscriptionSet* UPubnubSubscription::AddSubscription(UPubnubSubscription* Subscription)
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: This Subscription is invalid. Probably PubnubSubsystem was deinitialized. Initialize it again and create new subscription."));
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
	SubscriptionSet->InitWithSubscriptions(PubnubSubsystem, this, Subscription);
	
	return SubscriptionSet;
}

void UPubnubSubscription::InitSubscription(UPubnubSubsystem* InPubnubSubsystem, UPubnubBaseEntity* Entity, FPubnubSubscribeSettings InSubscribeSettings)
{
	if(!InPubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize subscription, PubnubSubsystem is invalid."));
		return;
	}
	if(!Entity)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize subscription, Entity is invalid."));
		return;
	}
	PubnubSubsystem = InPubnubSubsystem;
	CCoreSubscription = UPubnubUtilities::EEGetSubscriptionForEntity(InPubnubSubsystem->ctx_ee, Entity->EntityID, Entity->EntityType, InSubscribeSettings);

	InternalInit();
}

void UPubnubSubscription::InitWithCCoreSubscription(UPubnubSubsystem* InPubnubSubsystem, pubnub_subscription_t* InCCoreSubscription)
{
	if(!InPubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize subscription, PubnubSubsystem is invalid."));
		return;
	}
	if(!InCCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize subscription, InCCoreSubscription is invalid."));
		return;
	}
	PubnubSubsystem = InPubnubSubsystem;
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
	UPubnubUtilities::EEAddSubscriptionListenerOfType(CCoreSubscription, CallbackMessages, EPubnubListenerType::PLT_Message, this);
	UPubnubUtilities::EEAddSubscriptionListenerOfType(CCoreSubscription, CallbackSignals, EPubnubListenerType::PLT_Signal, this);
	UPubnubUtilities::EEAddSubscriptionListenerOfType(CCoreSubscription, CallbackObjects, EPubnubListenerType::PLT_Objects, this);
	UPubnubUtilities::EEAddSubscriptionListenerOfType(CCoreSubscription, CallbackMessageActions, EPubnubListenerType::PLT_MessageAction, this);

	//Bind to OnPubnubSubsystemDeinitialized so subscription is properly Cleaned up, when it's not needed
	PubnubSubsystem->OnPubnubSubsystemDeinitialized.AddDynamic(this, &UPubnubSubscription::CleanUpSubscription);

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

	if(PubnubSubsystem)
	{
		PubnubSubsystem->OnPubnubSubsystemDeinitialized.RemoveDynamic(this, &UPubnubSubscription::CleanUpSubscription);
	}

	IsInitialized = false;
}

void UPubnubSubscriptionSet::Subscribe(FOnSubscribeOperationResponse OnSubscribeResponse, FPubnubSubscriptionCursor Cursor)
{
	FOnSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSubscribeResponse](FPubnubOperationResult Result)
	{
		OnSubscribeResponse.ExecuteIfBound(Result);
	});

	Subscribe(NativeCallback, Cursor);
}

void UPubnubSubscriptionSet::Subscribe(FOnSubscribeOperationResponseNative NativeCallback, FPubnubSubscriptionCursor Cursor)
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Subscribe]: This SubscriptionSet is invalid. Probably PubnubSubsystem was deinitialized. Initialize it again and create new subscription."));
		return;
	}
	
	if(!CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Subscribe]: internal C-Core subscription set is invalid."));
		return;
	}
	
	PubnubSubsystem->SubscribeWithSubscriptionSet(this, Cursor, NativeCallback);
}

void UPubnubSubscriptionSet::Subscribe(FPubnubSubscriptionCursor Cursor)
{
	Subscribe(nullptr, Cursor);
}

void UPubnubSubscriptionSet::Unsubscribe(FOnSubscribeOperationResponse OnUnsubscribeResponse)
{
	FOnSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeResponse.ExecuteIfBound(Result);
	});

	Unsubscribe(NativeCallback);
}

void UPubnubSubscriptionSet::Unsubscribe(FOnSubscribeOperationResponseNative NativeCallback)
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Unsubscribe]: This SubscriptionSet is invalid. Probably PubnubSubsystem was deinitialized. Initialize it again and create new subscription."));
		return;
	}
	
	if(!CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("[Unsubscribe]: internal C-Core subscription set is invalid."));
		return;
	}

	PubnubSubsystem->UnsubscribeWithSubscriptionSet(this, NativeCallback);
}

void UPubnubSubscriptionSet::AddSubscription(UPubnubSubscription* Subscription)
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscription]: This SubscriptionSet is invalid. Probably PubnubSubsystem was deinitialized. Initialize it again and create new subscription."));
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
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscription]: This SubscriptionSet is invalid. Probably PubnubSubsystem was deinitialized. Initialize it again and create new subscription."));
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
		UE_LOG(PubnubLog, Error, TEXT("[AddSubscriptionSet]: This SubscriptionSet is invalid. Probably PubnubSubsystem was deinitialized. Initialize it again and create new subscription."));
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
		UE_LOG(PubnubLog, Error, TEXT("[RemoveSubscriptionSet]: This SubscriptionSet is invalid. Probably PubnubSubsystem was deinitialized. Initialize it again and create new subscription."));
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

void UPubnubSubscriptionSet::InitSubscriptionSet(UPubnubSubsystem* InPubnubSubsystem, TArray<FString> Channels, TArray<FString> ChannelGroups, FPubnubSubscribeSettings InSubscribeSettings)
{
	if(Channels.IsEmpty() && ChannelGroups.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize SubscriptionSet, at least one Channel or ChannelGroup is needed."));
		return;
	}
	PubnubSubsystem = InPubnubSubsystem;
	CCoreSubscriptionSet = UPubnubUtilities::EEGetSubscriptionSetForEntities(InPubnubSubsystem->ctx_ee, Channels, ChannelGroups, InSubscribeSettings);

	InternalInit();
}

void UPubnubSubscriptionSet::InitWithSubscriptions(UPubnubSubsystem* InPubnubSubsystem, UPubnubSubscription* Subscription1, UPubnubSubscription* Subscription2)
{
	if(!Subscription1 || !Subscription2 || !Subscription1->CCoreSubscription || !Subscription2->CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize SubscriptionSet, One of provided subscriptions is invalid."));
		return;
	}
	
	PubnubSubsystem = InPubnubSubsystem;
	
	CCoreSubscriptionSet = pubnub_subscription_set_alloc_with_subscriptions(Subscription1->CCoreSubscription, Subscription2->CCoreSubscription, nullptr);
	Subscriptions.Add(Subscription1);
	Subscriptions.Add(Subscription2);
	
	InternalInit();
}

void UPubnubSubscriptionSet::InitWithCCoreSubscriptionSet(UPubnubSubsystem* InPubnubSubsystem, pubnub_subscription_set_t* InCCoreSubscriptionSet)
{
	if(!InCCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize SubscriptionSet, InCCoreSubscriptionSet is invalid."));
		return;
	}
	PubnubSubsystem = InPubnubSubsystem;
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
	UPubnubUtilities::EEAddSubscriptionSetListenerOfType(CCoreSubscriptionSet, CallbackMessages, EPubnubListenerType::PLT_Message, this);
	UPubnubUtilities::EEAddSubscriptionSetListenerOfType(CCoreSubscriptionSet, CallbackSignals, EPubnubListenerType::PLT_Signal, this);
	UPubnubUtilities::EEAddSubscriptionSetListenerOfType(CCoreSubscriptionSet, CallbackObjects, EPubnubListenerType::PLT_Objects, this);
	UPubnubUtilities::EEAddSubscriptionSetListenerOfType(CCoreSubscriptionSet, CallbackMessageActions, EPubnubListenerType::PLT_MessageAction, this);

	//Bind to OnPubnubSubsystemDeinitialized so subscription is properly Cleaned up, when it's not needed
	PubnubSubsystem->OnPubnubSubsystemDeinitialized.AddDynamic(this, &UPubnubSubscriptionSet::CleanUpSubscription);

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

	if(PubnubSubsystem)
	{
		PubnubSubsystem->OnPubnubSubsystemDeinitialized.RemoveDynamic(this, &UPubnubSubscriptionSet::CleanUpSubscription);
	}

	IsInitialized = false;
}
