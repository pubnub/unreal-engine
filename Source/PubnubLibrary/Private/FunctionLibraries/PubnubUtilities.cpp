// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "FunctionLibraries/PubnubUtilities.h"
#include "Runtime/Launch/Resources/Version.h"
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

FString UPubnubUtilities::MembershipIncludeToString(const FPubnubMembershipInclude& MembershipInclude)
{
	FString FinalString = "";
	if(MembershipInclude.IncludeCustom)			{FinalString.Append("custom,");}
	if(MembershipInclude.IncludeStatus)			{FinalString.Append("status,");}
	if(MembershipInclude.IncludeType)			{FinalString.Append("type,");}
	if(MembershipInclude.IncludeChannel)		{FinalString.Append("channel,");}
	if(MembershipInclude.IncludeChannelCustom)	{FinalString.Append("channel.custom,");}
	if(MembershipInclude.IncludeChannelStatus)	{FinalString.Append("channel.status,");}
	if(MembershipInclude.IncludeChannelType)	{FinalString.Append("channel.type,");}
	//Total count is passed as a separate parameter, so it's not included directly in the final string

	//If there was any include remove the last comma
	if(!FinalString.IsEmpty())
	{
		FinalString.RemoveAt(FinalString.Len() - 1);
	}

	return FinalString;
}

FString UPubnubUtilities::MemberIncludeToString(const FPubnubMemberInclude& MemberInclude)
{
	FString FinalString = "";
	if(MemberInclude.IncludeCustom)			{FinalString.Append("custom,");}
	if(MemberInclude.IncludeStatus)			{FinalString.Append("status,");}
	if(MemberInclude.IncludeType)			{FinalString.Append("type,");}
	if(MemberInclude.IncludeUser)			{FinalString.Append("user,");}
	if(MemberInclude.IncludeUserCustom)		{FinalString.Append("user.custom,");}
	if(MemberInclude.IncludeUserStatus)		{FinalString.Append("user.status,");}
	if(MemberInclude.IncludeUserType)		{FinalString.Append("user.type,");}
	//Total count is passed as a separate parameter, so it's not included directly in the final string

	//If there was any include remove the last comma
	if(!FinalString.IsEmpty())
	{
		FinalString.RemoveAt(FinalString.Len() - 1);
	}

	return FinalString;
}

FString UPubnubUtilities::GetAllIncludeToString(const FPubnubGetAllInclude& GetAllInclude)
{
	FString FinalString = "";
	if(GetAllInclude.IncludeCustom)			{FinalString.Append("custom,");}
	if(GetAllInclude.IncludeStatus)			{FinalString.Append("status,");}
	if(GetAllInclude.IncludeType)			{FinalString.Append("type,");}
	//Total count is passed as a separate parameter, so it's not included directly in the final string

	//If there was any include remove the last comma
	if(!FinalString.IsEmpty())
	{
		FinalString.RemoveAt(FinalString.Len() - 1);
	}

	return FinalString;
}

FString UPubnubUtilities::MembershipSortTypeToString(const EPubnubMembershipSortType SortType)
{
	switch (SortType)
	{
	case EPubnubMembershipSortType::PMST_ChannelID:
		return "channel.id";
	case EPubnubMembershipSortType::PMST_ChannelName:
		return "channel.name";
	case EPubnubMembershipSortType::PMST_ChannelUpdated:
		return "channel.updated";
	case EPubnubMembershipSortType::PMST_ChannelStatus:
		return "channel.status";
	case EPubnubMembershipSortType::PMST_ChannelType:
		return "channel.type";
	case EPubnubMembershipSortType::PMST_Updated:
		return "updated";
	case EPubnubMembershipSortType::PMST_Status:
		return "status";
	case EPubnubMembershipSortType::PMST_Type:
		return "type";
	}
	return "";
}

FString UPubnubUtilities::MemberSortTypeToString(const EPubnubMemberSortType SortType)
{
	switch (SortType)
	{
	case EPubnubMemberSortType::PMeST_UserID:
		return "user.id";
	case EPubnubMemberSortType::PMeST_UserName:
		return "user.name";
	case EPubnubMemberSortType::PMeST_UserUpdated:
		return "user.updated";
	case EPubnubMemberSortType::PMeST_UserStatus:
		return "user.status";
	case EPubnubMemberSortType::PMeST_UserType:
		return "user.type";
	case EPubnubMemberSortType::PMeST_Updated:
		return "updated";
	case EPubnubMemberSortType::PMeST_Status:
		return "status";
	case EPubnubMemberSortType::PMeST_Type:
		return "type";
	}
	return "";
}

FString UPubnubUtilities::GetAllSortTypeToString(const EPubnubGetAllSortType SortType)
{
	switch (SortType)
	{
	case EPubnubGetAllSortType::PGAST_ID:
		return "id";
	case EPubnubGetAllSortType::PGAST_Name:
		return "name";
	case EPubnubGetAllSortType::PGAST_Updated:
		return "updated";
	case EPubnubGetAllSortType::PGAST_Status:
		return "status";
	case EPubnubGetAllSortType::PGAST_Type:
		return "type";
	}
	return "";
}

FString UPubnubUtilities::MembershipSortToString(const FPubnubMembershipSort& MemberInclude)
{
	FString FinalString = "";
	//Form comma separated string of sorts
	for(auto SingleSort : MemberInclude.MembershipSort)
	{
		if(!FinalString.IsEmpty()) {FinalString.Append(",");}
		FinalString.Append(MembershipSortTypeToString(SingleSort.SortType));
		//Default sort is ascending, so we only specify order when it's descending
		if(SingleSort.SortOrder)
		{
			FinalString.Append(":desc");
		}
	}

	return FinalString;
}

FString UPubnubUtilities::MemberSortToString(const FPubnubMemberSort& MemberInclude)
{
	FString FinalString = "";
	//Form comma separated string of sorts
	for(auto SingleSort : MemberInclude.MemberSort)
	{
		if(!FinalString.IsEmpty()) {FinalString.Append(",");}
		FinalString.Append(MemberSortTypeToString(SingleSort.SortType));
		//Default sort is ascending, so we only specify order when it's descending
		if(SingleSort.SortOrder)
		{
			FinalString.Append(":desc");
		}
	}

	return FinalString;
}

FString UPubnubUtilities::GetAllSortToString(const FPubnubGetAllSort& GetAllInclude)
{
	FString FinalString = "";
	//Form comma separated string of sorts
	for(auto SingleSort : GetAllInclude.GetAllSort)
	{
		if(!FinalString.IsEmpty()) {FinalString.Append(",");}
		FinalString.Append(GetAllSortTypeToString(SingleSort.SortType));
		//Default sort is ascending, so we only specify order when it's descending
		if(SingleSort.SortOrder)
		{
			FinalString.Append(":desc");
		}
	}

	return FinalString;
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