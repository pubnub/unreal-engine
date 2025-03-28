// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "FunctionLibraries/PubnubUtilities.h"
#include "Runtime/Launch/Resources/Version.h"
#include "PubnubSubsystem.h"
#include "Json.h"

FString UPubnubUtilities::AddQuotesToString(const FString InString, bool SkipIfHasQuotes)
{
	if(InString.Left(1) != "\"" || InString.Right(1) != "\"" || !SkipIfHasQuotes)
	{
		return "\"" + InString + "\"";
	}

	return InString;
}

FString UPubnubUtilities::PubnubCharMemBlockToString(const pubnub_char_mem_block PnChar)
{
	if(!PnChar.ptr)
	{
		return "";
	}

#if ENGINE_MINOR_VERSION <= 3
	//This constructor is deprecated since 5.4
	return FString(PnChar.size, PnChar.ptr);
#else
	return FString::ConstructFromPtrSize(PnChar.ptr, PnChar.size);
#endif
}

pubnub_subscription_t* UPubnubUtilities::EEGetSubscriptionForChannel(pubnub_t* Context, FString Channel, FPubnubSubscribeSettings Options)
{
	pubnub_subscription_options_t PnOptions = pubnub_subscription_options_defopts();
	PnOptions.receive_presence_events = Options.ReceivePresenceEvents;

	pubnub_channel_t* PubnubChannel = pubnub_channel_alloc(Context, TCHAR_TO_ANSI(*Channel));
	
	pubnub_subscription_t* Subscription = pubnub_subscription_alloc((pubnub_entity_t*)PubnubChannel, &PnOptions);
	
	pubnub_entity_free((void**)&PubnubChannel);

	return Subscription;
}

pubnub_subscription_t* UPubnubUtilities::EEGetSubscriptionForChannelGroup(pubnub_t* Context, FString ChannelGroup, FPubnubSubscribeSettings Options)
{
	pubnub_subscription_options_t PnOptions = pubnub_subscription_options_defopts();
	PnOptions.receive_presence_events = Options.ReceivePresenceEvents;

	pubnub_channel_group_t* PubnubChannelGroup = pubnub_channel_group_alloc(Context, TCHAR_TO_ANSI(*ChannelGroup));
	
	pubnub_subscription_t* Subscription = pubnub_subscription_alloc((pubnub_entity_t*)PubnubChannelGroup, &PnOptions);
	
	pubnub_entity_free((void**)&PubnubChannelGroup);

	return Subscription;
}

bool UPubnubUtilities::EEAddListenerAndSubscribe(pubnub_subscription_t* Subscription, pubnub_subscribe_message_callback_t Callback, UPubnubSubsystem* PubnubSubsystem)
{
	if(!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("EEAddListenerAndSubscribe Failed, PubnubSubsystem is invalid"));
		return false;
	}
	
	enum pubnub_res AddListenerResult = pubnub_subscribe_add_subscription_listener(Subscription, PBSL_LISTENER_ON_MESSAGE, Callback, PubnubSubsystem);
	if(PNR_OK != AddListenerResult)
	{
		FString ResultString(pubnub_res_2_string(AddListenerResult));
		PubnubSubsystem->PubnubError("Failed to subscribe. Add_subscription_listener failed with error: " + ResultString);
		return false;
	}

	enum pubnub_res SubscribeResult = pubnub_subscribe_with_subscription(Subscription, nullptr);
	if(PNR_OK != SubscribeResult)
	{
		FString ResultString(pubnub_res_2_string(AddListenerResult));
		PubnubSubsystem->PubnubError("Failed to subscribe. Subscribe_with_subscription failed with error: " + ResultString);
		return false;
	}

	return true;
}

bool UPubnubUtilities::EERemoveListenerAndUnsubscribe(pubnub_subscription_t** SubscriptionPtr, pubnub_subscribe_message_callback_t Callback, UPubnubSubsystem* PubnubSubsystem)
{
	if(!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("EERemoveListenerAndUnsubscribe Failed, PubnubSubsystem is invalid"));
		return false;
	}

	if(!SubscriptionPtr)
	{
		PubnubSubsystem->PubnubError("Failed to unsubscribe. Passed subscription pointer is invalid");
		return false;
	}
	
	enum pubnub_res RemoveListenerResult =  pubnub_subscribe_remove_subscription_listener(*SubscriptionPtr, PBSL_LISTENER_ON_MESSAGE, Callback, PubnubSubsystem);
	if(PNR_OK != RemoveListenerResult)
	{
		FString ResultString(pubnub_res_2_string(RemoveListenerResult));
		PubnubSubsystem->PubnubError("Failed to subscribe. Remove_subscription_listener failed with error: " + ResultString);
		return false;
	}
	
	enum pubnub_res UnsubscribeResult = pubnub_unsubscribe_with_subscription(SubscriptionPtr);
	if(PNR_OK != UnsubscribeResult)
	{
		FString ResultString(pubnub_res_2_string(UnsubscribeResult));
		PubnubSubsystem->PubnubError("Failed to unsubscribe. Unsubscribe_with_subscription failed with error: " + ResultString);
		return false;
	}

	return true;
}
