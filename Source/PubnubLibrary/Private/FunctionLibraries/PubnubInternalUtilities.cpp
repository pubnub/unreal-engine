// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "FunctionLibraries/PubnubInternalUtilities.h"
#include "PubnubClient.h"
#include "PubnubLibraryVersion.h"
#include "FunctionLibraries/PubnubUtilities.h"

FString UPubnubInternalUtilities::GetPubnubSdkVersionSuffix()
{
	return FString::Printf(
		TEXT("-Pubnub-C-core/%s/Unreal/%d.%d.%d"),
		ANSI_TO_TCHAR(PUBNUB_C_CORE_VERSION),
		PUBNUB_LIBRARY_VERSION_MAJOR,
		PUBNUB_LIBRARY_VERSION_MINOR,
		PUBNUB_LIBRARY_VERSION_PATCH
	);
}

void UPubnubInternalUtilities::PublishUESettingsToPubnubPublishOptions(const FPubnubPublishSettings &PublishSettings, pubnub_publish_options& PubnubPublishOptions)
{
	PubnubPublishOptions.store = PublishSettings.StoreInHistory;
	PubnubPublishOptions.replicate = PublishSettings.Replicate;
	PubnubPublishOptions.cipher_key = NULL;
	PubnubPublishOptions.ttl = PublishSettings.Ttl;
	PublishSettings.MetaData.IsEmpty() ? PubnubPublishOptions.meta = NULL : nullptr;
	PublishSettings.CustomMessageType.IsEmpty() ? PubnubPublishOptions.custom_message_type = NULL : nullptr;
	PubnubPublishOptions.method = (pubnub_method)(uint8)PublishSettings.PublishMethod;
}

void UPubnubInternalUtilities::HereNowUESettingsToPubnubHereNowOptions(const FPubnubListUsersFromChannelSettings& HereNowSettings, pubnub_here_now_options& PubnubHereNowOptions)
{
	PubnubHereNowOptions.disable_uuids = HereNowSettings.DisableUserID;
	PubnubHereNowOptions.state = HereNowSettings.State;
	HereNowSettings.ChannelGroup.IsEmpty() ? PubnubHereNowOptions.channel_group = NULL : nullptr;
	PubnubHereNowOptions.limit = HereNowSettings.Limit;
	PubnubHereNowOptions.offset = HereNowSettings.Offset;
}

void UPubnubInternalUtilities::SetStateUESettingsToPubnubSetStateOptions(const FPubnubSetStateSettings& SetStateSettings, pubnub_set_state_options& PubnubSetStateOptions)
{
	SetStateSettings.ChannelGroup.IsEmpty() ? PubnubSetStateOptions.channel_group = NULL : nullptr;
	SetStateSettings.UserID.IsEmpty() ? PubnubSetStateOptions.user_id = NULL : nullptr;
	PubnubSetStateOptions.heartbeat = SetStateSettings.HeartBeat;
}

void UPubnubInternalUtilities::FetchHistoryUESettingsToPbFetchHistoryOptions(const FPubnubFetchHistorySettings& FetchHistorySettings, pubnub_fetch_history_options& PubnubFetchHistoryOptions)
{
	PubnubFetchHistoryOptions.max_per_channel = FetchHistorySettings.MaxPerChannel;
	PubnubFetchHistoryOptions.reverse = FetchHistorySettings.Reverse;
	PubnubFetchHistoryOptions.include_meta = FetchHistorySettings.IncludeMeta;
	PubnubFetchHistoryOptions.include_message_type = FetchHistorySettings.IncludeMessageType;
	PubnubFetchHistoryOptions.include_user_id = FetchHistorySettings.IncludeUserID;
	PubnubFetchHistoryOptions.include_message_actions = FetchHistorySettings.IncludeMessageActions;
	PubnubFetchHistoryOptions.include_custom_message_type = FetchHistorySettings.IncludeCustomMessageType;
	FetchHistorySettings.Start.IsEmpty() ? PubnubFetchHistoryOptions.start = NULL : nullptr;
	FetchHistorySettings.End.IsEmpty() ? PubnubFetchHistoryOptions.end = NULL : nullptr;
}

pubnub_subscription_t* UPubnubInternalUtilities::EEGetSubscriptionForEntity(pubnub_t* Context, FString EntityID, EPubnubEntityType EntityType, FPubnubSubscribeSettings Options)
{
	pubnub_subscription_options_t PnOptions = pubnub_subscription_options_defopts();
	PnOptions.receive_presence_events = Options.ReceivePresenceEvents;

	FUTF8StringHolder EntityIDHolder(EntityID);
	pubnub_entity_t* PubnubEntity = nullptr;
	switch (EntityType)
	{
	case EPubnubEntityType::PEnT_Channel:
		PubnubEntity = reinterpret_cast<pubnub_entity_t*>(pubnub_channel_alloc(Context, EntityIDHolder.Get()));
		break;
	case EPubnubEntityType::PEnT_ChannelGroup:
		PubnubEntity = reinterpret_cast<pubnub_entity_t*>(pubnub_channel_group_alloc(Context, EntityIDHolder.Get()));
		break;
	case EPubnubEntityType::PEnT_ChannelMetadata:
		PubnubEntity = reinterpret_cast<pubnub_entity_t*>(pubnub_channel_metadata_alloc(Context, EntityIDHolder.Get()));
		break;
	case EPubnubEntityType::PEnT_UserMetadata:
		PubnubEntity = reinterpret_cast<pubnub_entity_t*>(pubnub_user_metadata_alloc(Context, EntityIDHolder.Get()));
		break;
	default:
		UE_LOG(PubnubLog, Error, TEXT("Unknown entity type: %d"), (int32)EntityType);
		return nullptr;
	}
	pubnub_subscription_t* Subscription = pubnub_subscription_alloc(PubnubEntity, &PnOptions);
	
	pubnub_entity_free(reinterpret_cast<void**>(&PubnubEntity));

	return Subscription;
}

pubnub_subscription_set_t* UPubnubInternalUtilities::EEGetSubscriptionSetForEntities(pubnub_t* Context, TArray<FString> Channels, TArray<FString> ChannelGroups, FPubnubSubscribeSettings Options)
{
	pubnub_subscription_options_t PnOptions = pubnub_subscription_options_defopts();
	PnOptions.receive_presence_events = Options.ReceivePresenceEvents;

	TArray<pubnub_entity_t*> PubnubEntities;
	PubnubEntities.Reserve(Channels.Num() + ChannelGroups.Num());

	for(FString Channel : Channels)
	{
		FUTF8StringHolder EntityIDHolder(Channel);
		PubnubEntities.Add(reinterpret_cast<pubnub_entity_t*>(pubnub_channel_alloc(Context, EntityIDHolder.Get())));
	}
	for(FString ChannelGroup : ChannelGroups)
	{
		FUTF8StringHolder EntityIDHolder(ChannelGroup);
		PubnubEntities.Add(reinterpret_cast<pubnub_entity_t*>(pubnub_channel_group_alloc(Context, EntityIDHolder.Get())));
	}

	pubnub_subscription_set_t* SubscriptionSet = pubnub_subscription_set_alloc_with_entities(PubnubEntities.GetData(), PubnubEntities.Num(), &PnOptions);

	for(pubnub_entity_t*& Entity : PubnubEntities)
	{
		pubnub_entity_free(reinterpret_cast<void**>(&Entity));
	}
	
	return SubscriptionSet;
}

bool UPubnubInternalUtilities::EEAddListenerAndSubscribe(pubnub_subscription_t* Subscription, pubnub_subscribe_message_callback_t Callback, UPubnubClient* PubnubClient)
{
	if(!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("EEAddListenerAndSubscribe Failed, PubnubClient is invalid"));
		return false;
	}

	EEAddSubscriptionListenersOfAllTypes(Subscription, Callback, PubnubClient);

	return EESubscribeWithSubscription(Subscription, FPubnubSubscriptionCursor());
}

bool UPubnubInternalUtilities::EERemoveListenerAndUnsubscribe(pubnub_subscription_t** SubscriptionPtr, pubnub_subscribe_message_callback_t Callback, UPubnubClient* PubnubClient)
{
	if(!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("EERemoveListenerAndUnsubscribe Failed, PubnubClient is invalid"));
		return false;
	}

	if(!SubscriptionPtr)
	{
		UE_LOG(PubnubLog, Error, TEXT("Failed to unsubscribe. Passed subscription pointer is invalid"));
		return false;
	}

	EERemoveSubscriptionListenersOfAllTypes(SubscriptionPtr, Callback, PubnubClient);

	return EEUnsubscribeWithSubscription(SubscriptionPtr);
}

bool UPubnubInternalUtilities::EESubscribeWithSubscription(pubnub_subscription_t* Subscription, FPubnubSubscriptionCursor Cursor)
{
	enum pubnub_res SubscribeResult;
	if(!Cursor.Timetoken.IsEmpty() || Cursor.Region != 0)
	{
		FUTF8StringHolder CursorTimetokenHolder(Cursor.Timetoken);
		pubnub_subscribe_cursor_t PubnubCursor =  pubnub_subscribe_cursor(CursorTimetokenHolder.Get());
		PubnubCursor.region = Cursor.Region;
		SubscribeResult = pubnub_subscribe_with_subscription(Subscription, &PubnubCursor);
	}
	else
	{
		SubscribeResult = pubnub_subscribe_with_subscription(Subscription, nullptr);
	}
	
	if(PNR_OK != SubscribeResult)
	{
		FString ResultString(pubnub_res_2_string(SubscribeResult));
		UE_LOG(PubnubLog, Error, TEXT("Failed to subscribe. Subscribe_with_subscription failed with error: %s"), *ResultString);
		return false;
	}

	return true;
}

bool UPubnubInternalUtilities::EEUnsubscribeWithSubscription(pubnub_subscription_t** SubscriptionPtr)
{
	enum pubnub_res UnsubscribeResult = pubnub_unsubscribe_with_subscription(SubscriptionPtr);
	if(PNR_OK != UnsubscribeResult)
	{
		FString ResultString(pubnub_res_2_string(UnsubscribeResult));
		UE_LOG(PubnubLog, Error, TEXT("Failed to unsubscribe. Unsubscribe_with_subscription failed with error: "), *ResultString);
		return false;
	}
	
	return true;
}

bool UPubnubInternalUtilities::EESubscribeWithSubscriptionSet(pubnub_subscription_set_t* SubscriptionSet, FPubnubSubscriptionCursor Cursor)
{
	enum pubnub_res SubscribeResult;
	if(!Cursor.Timetoken.IsEmpty() || Cursor.Region != 0)
	{
		FUTF8StringHolder CursorTimetokenHolder(Cursor.Timetoken);
		pubnub_subscribe_cursor_t PubnubCursor =  pubnub_subscribe_cursor(CursorTimetokenHolder.Get());
		PubnubCursor.region = Cursor.Region;
		SubscribeResult = pubnub_subscribe_with_subscription_set(SubscriptionSet, &PubnubCursor);
	}
	else
	{
		SubscribeResult = pubnub_subscribe_with_subscription_set(SubscriptionSet, nullptr);
	}
	
	if(PNR_OK != SubscribeResult)
	{
		FString ResultString(pubnub_res_2_string(SubscribeResult));
		UE_LOG(PubnubLog, Error, TEXT("Failed to subscribe. Subscribe_with_subscription_set failed with error: %s"), *ResultString);
		return false;
	}

	return true;
}

bool UPubnubInternalUtilities::EEUnsubscribeWithSubscriptionSet(pubnub_subscription_set_t** SubscriptionSetPtr)
{
	enum pubnub_res UnsubscribeResult = pubnub_unsubscribe_with_subscription_set(SubscriptionSetPtr);
	if(PNR_OK != UnsubscribeResult)
	{
		FString ResultString(pubnub_res_2_string(UnsubscribeResult));
		UE_LOG(PubnubLog, Error, TEXT("Failed to unsubscribe. Unsubscribe_with_subscription failed with error: "), *ResultString);
		return false;
	}
	
	return true;
}

bool UPubnubInternalUtilities::EEAddSubscriptionListenerOfType(pubnub_subscription_t* Subscription, pubnub_subscribe_message_callback_t Callback, EPubnubListenerType ListenerType, void* UserData)
{
	if(ListenerType == EPubnubListenerType::PLT_All)
	{
		EEAddSubscriptionListenersOfAllTypes(Subscription, Callback, UserData);
		return false;
	}
	
	pubnub_subscribe_listener_type PubnubListenerType = static_cast<pubnub_subscribe_listener_type>(static_cast<uint8>(ListenerType));
	enum pubnub_res AddMessageListenerResult = pubnub_subscribe_add_subscription_listener(Subscription, PubnubListenerType, Callback, UserData);

	if(PNR_OK != AddMessageListenerResult)
	{
		FString ResultString(pubnub_res_2_string(AddMessageListenerResult));
		UE_LOG(PubnubLog, Error, TEXT("Failed to add listener of type %s. Error: %s "), *StaticEnum<EPubnubListenerType>()->GetNameStringByValue(static_cast<int64>(ListenerType)), *ResultString);
		return false;
	}
	
	return true;
}

void UPubnubInternalUtilities::EEAddSubscriptionListenersOfAllTypes(pubnub_subscription_t* Subscription, pubnub_subscribe_message_callback_t Callback, void* UserData)
{
	for(EPubnubListenerType Type : TEnumRange<EPubnubListenerType>())
	{
		EEAddSubscriptionListenerOfType(Subscription, Callback, Type, UserData);
	}
}

bool UPubnubInternalUtilities::EERemoveSubscriptionListenerOfType(pubnub_subscription_t** SubscriptionPtr, pubnub_subscribe_message_callback_t Callback, EPubnubListenerType ListenerType, void* UserData)
{
	if(ListenerType == EPubnubListenerType::PLT_All)
	{
		EERemoveSubscriptionListenersOfAllTypes(SubscriptionPtr, Callback, UserData);
		return false;
	}

	pubnub_subscribe_listener_type PubnubListenerType = static_cast<pubnub_subscribe_listener_type>(static_cast<uint8>(ListenerType));
	enum pubnub_res RemoveMessageActionListenerResult =  pubnub_subscribe_remove_subscription_listener(*SubscriptionPtr, PubnubListenerType, Callback, UserData);
	if(PNR_OK != RemoveMessageActionListenerResult)
	{
		FString ResultString(pubnub_res_2_string(RemoveMessageActionListenerResult));
		UE_LOG(PubnubLog, Error, TEXT("Failed to remove listener of type %s. Error: %s "), *StaticEnum<EPubnubListenerType>()->GetNameStringByValue(static_cast<int64>(ListenerType)), *ResultString);
		return false;
	}

	return true;
}

void UPubnubInternalUtilities::EERemoveSubscriptionListenersOfAllTypes(pubnub_subscription_t** SubscriptionPtr, pubnub_subscribe_message_callback_t Callback, void* UserData)
{
	for(EPubnubListenerType Type : TEnumRange<EPubnubListenerType>())
	{
		EERemoveSubscriptionListenerOfType(SubscriptionPtr, Callback, Type, UserData);
	}
}

bool UPubnubInternalUtilities::EEAddSubscriptionSetListenerOfType(pubnub_subscription_set_t* SubscriptionSet, pubnub_subscribe_message_callback_t Callback, EPubnubListenerType ListenerType, void* UserData)
{
	if(ListenerType == EPubnubListenerType::PLT_All)
	{
		EEAddSubscriptionSetListenersOfAllTypes(SubscriptionSet, Callback, UserData);
		return false;
	}
	
	pubnub_subscribe_listener_type PubnubListenerType = static_cast<pubnub_subscribe_listener_type>(static_cast<uint8>(ListenerType));
	enum pubnub_res AddMessageListenerResult = pubnub_subscribe_add_subscription_set_listener(SubscriptionSet, PubnubListenerType, Callback, UserData);

	if(PNR_OK != AddMessageListenerResult)
	{
		FString ResultString(pubnub_res_2_string(AddMessageListenerResult));
		UE_LOG(PubnubLog, Error, TEXT("Failed to add listener of type %s. Error: %s "), *StaticEnum<EPubnubListenerType>()->GetNameStringByValue(static_cast<int64>(ListenerType)), *ResultString);
		return false;
	}
	
	return true;
}

void UPubnubInternalUtilities::EEAddSubscriptionSetListenersOfAllTypes(pubnub_subscription_set_t* SubscriptionSet, pubnub_subscribe_message_callback_t Callback, void* UserData)
{
	for(EPubnubListenerType Type : TEnumRange<EPubnubListenerType>())
	{
		EEAddSubscriptionSetListenerOfType(SubscriptionSet, Callback, Type, UserData);
	}
}

bool UPubnubInternalUtilities::EERemoveSubscriptionSetListenerOfType(pubnub_subscription_set_t** SubscriptionSetPtr, pubnub_subscribe_message_callback_t Callback, EPubnubListenerType ListenerType, void* UserData)
{
	if(ListenerType == EPubnubListenerType::PLT_All)
	{
		EERemoveSubscriptionSetListenersOfAllTypes(SubscriptionSetPtr, Callback, UserData);
		return false;
	}

	pubnub_subscribe_listener_type PubnubListenerType = static_cast<pubnub_subscribe_listener_type>(static_cast<uint8>(ListenerType));
	enum pubnub_res RemoveMessageActionListenerResult =  pubnub_subscribe_remove_subscription_set_listener(*SubscriptionSetPtr, PubnubListenerType, Callback, UserData);
	if(PNR_OK != RemoveMessageActionListenerResult)
	{
		FString ResultString(pubnub_res_2_string(RemoveMessageActionListenerResult));
		UE_LOG(PubnubLog, Error, TEXT("Failed to remove listener of type %s. Error: %s "), *StaticEnum<EPubnubListenerType>()->GetNameStringByValue(static_cast<int64>(ListenerType)), *ResultString);
		return false;
	}

	return true;
}

void UPubnubInternalUtilities::EERemoveSubscriptionSetListenersOfAllTypes(pubnub_subscription_set_t** SubscriptionSetPtr, pubnub_subscribe_message_callback_t Callback, void* UserData)
{
	for(EPubnubListenerType Type : TEnumRange<EPubnubListenerType>())
	{
		EERemoveSubscriptionSetListenerOfType(SubscriptionSetPtr, Callback, Type, UserData);
	}
}