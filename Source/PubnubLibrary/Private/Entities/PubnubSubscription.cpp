// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubSubscription.h"
#include "PubnubSubsystem.h"
#include "Entities/PubnubBaseEntity.h"
#include "FunctionLibraries/PubnubUtilities.h"


void UPubnubSubscription::InitSubscription(UPubnubSubsystem* InPubnubSubsystem, UPubnubBaseEntity* Entity, FPubnubSubscribeSettings InSubscribeSettings)
{
	if(!Entity)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize subscription, Entity is invalid"));
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
}

void UPubnubSubscription::Subscribe(FPubnubSubscriptionCursor Cursor)
{
	if(!CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't subscribe, subscription is invalid"));
	}

	UPubnubUtilities::EESubscribeWithSubscription(CCoreSubscription, Cursor);
}

void UPubnubSubscription::Unsubscribe()
{
	if(!CCoreSubscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't unsubscribe, subscription is invalid"));
	}

	UPubnubUtilities::EEUnsubscribeWithSubscription(&CCoreSubscription);
}

void UPubnubSubscriptionSet::Subscribe(FPubnubSubscriptionCursor Cursor)
{
}

void UPubnubSubscriptionSet::Unsubscribe()
{
}

void UPubnubSubscriptionSet::InitSubscription(UPubnubSubsystem* InPubnubSubsystem, TArray<FString> Channels, TArray<FString> ChannelGroups, FPubnubSubscribeSettings InSubscribeSettings)
{
	if(Channels.IsEmpty() && ChannelGroups.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Can't initialize SubscriptionSet, at least one Channel or ChannelGroup is needed."));
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
}