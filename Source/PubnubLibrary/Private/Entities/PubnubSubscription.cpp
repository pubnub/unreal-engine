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

void UPubnubSubscription::Subscribe(FPubnubSubscriptionCursor Cursor)
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't subscribe, This Subscription is invalid. Probably PubnubSubsystem was deinitialized. Initialize it again and create new subscription"));
	}
	
	if(!CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't subscribe, internal C-Core subscription is invalid"));
	}

	UPubnubUtilities::EESubscribeWithSubscription(CCoreSubscription, Cursor);
}

void UPubnubSubscription::Unsubscribe()
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't unsubscribe, This Subscription is invalid. Probably PubnubSubsystem was deinitialized. Initialize it again and create new subscription"));
	}
	
	if(!CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't unsubscribe, internal C-Core subscription is invalid"));
	}

	UPubnubUtilities::EEUnsubscribeWithSubscription(&CCoreSubscription);
}

void UPubnubSubscription::InitSubscription(UPubnubSubsystem* InPubnubSubsystem, UPubnubBaseEntity* Entity, FPubnubSubscribeSettings InSubscribeSettings)
{
	if(!Entity)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize subscription, Entity is invalid"));
		return;
	}
	PubnubSubsystem = InPubnubSubsystem;
	CCoreSubscription = UPubnubUtilities::EEGetSubscriptionForEntity(InPubnubSubsystem->ctx_ee, Entity->EntityID, Entity->EntityType, InSubscribeSettings);

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
				S->OnPubnubMessage.Broadcast(MessageData);
				S->OnPubnubMessageNative.Broadcast(MessageData);
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

void UPubnubSubscriptionSet::Subscribe(FPubnubSubscriptionCursor Cursor)
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't subscribe, This SubscriptionSet is invalid. Probably PubnubSubsystem was deinitialized. Initialize it again and create new subscription"));
	}
	
	if(!CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't subscribe, internal C-Core subscription set is invalid"));
	}

	UPubnubUtilities::EESubscribeWithSubscriptionSet(CCoreSubscriptionSet, Cursor);
}

void UPubnubSubscriptionSet::Unsubscribe()
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't unsubscribe, This SubscriptionSet is invalid. Probably PubnubSubsystem was deinitialized. Initialize it again and create new subscription"));
	}
	
	if(!CCoreSubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't unsubscribe, internal C-Core subscription set is invalid"));
	}

	UPubnubUtilities::EEUnsubscribeWithSubscriptionSet(&CCoreSubscriptionSet);
}

void UPubnubSubscriptionSet::InitSubscription(UPubnubSubsystem* InPubnubSubsystem, TArray<FString> Channels, TArray<FString> ChannelGroups, FPubnubSubscribeSettings InSubscribeSettings)
{
	if(Channels.IsEmpty() && ChannelGroups.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize SubscriptionSet, at least one Channel or ChannelGroup is needed."));
		return;
	}
	PubnubSubsystem = InPubnubSubsystem;
	CCoreSubscriptionSet = UPubnubUtilities::EEGetSubscriptionSetForEntities(InPubnubSubsystem->ctx_ee, Channels, ChannelGroups, InSubscribeSettings);

	
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
				S->OnPubnubMessage.Broadcast(MessageData);
				S->OnPubnubMessageNative.Broadcast(MessageData);
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
