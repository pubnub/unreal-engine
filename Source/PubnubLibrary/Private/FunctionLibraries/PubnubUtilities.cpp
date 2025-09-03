// Copyright 2025 PubNub Inc. All Rights Reserved.


#include "FunctionLibraries/PubnubUtilities.h"
#include "Config/PubnubSettings.h"
#include "Entities/PubnubSubscription.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "Kismet/KismetMathLibrary.h"


FPubnubConfig UPubnubUtilities::PubnubConfigFromPluginSettings(UPubnubSettings* PubnubSettings)
{
	FPubnubConfig Config;
	Config.PublishKey = PubnubSettings->PublishKey;
	Config.SubscribeKey = PubnubSettings->SubscribeKey;
	Config.SecretKey = PubnubSettings->SecretKey;
	Config.SetSecretKeyAutomatically = PubnubSettings->SetSecretKeyAutomatically;

	return Config;
}

FString UPubnubUtilities::PubnubGetLastServerHttpResponse(pubnub_t* Context)
{
	pubnub_char_mem_block LastServerResponse;
	pubnub_last_http_response_body(Context, &LastServerResponse);
	return PubnubCharMemBlockToString(LastServerResponse);
}

int UPubnubUtilities::RoundLimitForPubnubFunctions(int ProvidedLimit)
{
	return UKismetMathLibrary::Clamp(ProvidedLimit, 0, PUBNUB_MAX_LIMIT);
}

FString UPubnubUtilities::AddQuotesToString(const FString InString, bool SkipIfHasQuotes)
{
	if(InString.Left(1) != "\"" || InString.Right(1) != "\"" || !SkipIfHasQuotes)
	{
		return "\"" + InString + "\"";
	}

	return InString;
}

FString UPubnubUtilities::RemoveOuterQuotesFromString(const FString InString)
{
	FString FinalString = InString;
	if(FinalString.Left(1) == "\"")
	{
		FinalString.RightChopInline(1);
	}
	if(FinalString.Right(1) == "\"")
	{
		FinalString.LeftChopInline(1);
	}

	return FinalString;
}

FString UPubnubUtilities::PubnubCharMemBlockToString(const pubnub_char_mem_block PnChar)
{
	if (!PnChar.ptr || PnChar.size == 0)
	{
		return FString();
	}
	
	FUTF8ToTCHAR Converter(reinterpret_cast<const ANSICHAR*>(PnChar.ptr), PnChar.size);
	return FString(Converter.Length(), Converter.Get());
}

bool UPubnubUtilities::SafeCopyFStringToCharBuffer(char* Destination, int DestSize, const FString& Source, const TCHAR* KeyName)
{
	if (!Destination || DestSize <= 0)
	{
		UE_LOG(PubnubLog, Error, TEXT("SafeCopyFStringToCharBuffer: Invalid destination buffer for %s"), KeyName);
		return false;
	}

	if (Source.IsEmpty())
	{
		Destination[0] = '\0';
		return true;
	}

	// Use UTF8 conversion with proper lifetime management (same pattern as FUTF8StringHolder)
	FTCHARToUTF8 Converter(*Source);
	const char* Utf8Source = Converter.Get();
	
	if (!Utf8Source)
	{
		UE_LOG(PubnubLog, Error, TEXT("SafeCopyFStringToCharBuffer: Failed to convert %s to UTF8"), KeyName);
		Destination[0] = '\0';
		return false;
	}

	// Get source length
	const int SourceLen = Converter.Length();
	
	// Check if source fits in destination (leaving space for null terminator)
	if (SourceLen >= DestSize)
	{
		UE_LOG(PubnubLog, Warning, TEXT("SafeCopyFStringToCharBuffer: %s is too long (%d chars), truncating to %d chars"), 
			KeyName, SourceLen, DestSize - 1);
	}

	// Copy with bounds checking - copy at most (DestSize - 1) characters
	const int CopyLen = FMath::Min(SourceLen, DestSize - 1);
	FMemory::Memcpy(Destination, Utf8Source, CopyLen);
	Destination[CopyLen] = '\0';

	return true;
}

FString UPubnubUtilities::GetNameFromFunctionMacro(FString FunctionName)
{
	if(FunctionName.IsEmpty()) {return "";}
	int Index = -1;
	FunctionName.FindLastChar(TEXT(':'), Index);
	//Leave Class name, just take function name
	FString FinalFunctionName = FunctionName.Mid(Index + 1);
	//Remove "_priv" from function name
	FinalFunctionName.ReplaceInline(TEXT("_priv"), TEXT(""));
	return FinalFunctionName;
}

FPubnubMessageData UPubnubUtilities::UEMessageFromPubnubMessage(pubnub_v2_message PubnubMessage)
{
	FPubnubMessageData MessageData;
	MessageData.Message = PubnubCharMemBlockToString(PubnubMessage.payload);

	//If message was just a string, we need to deserialize it
	if(!UPubnubJsonUtilities::IsCorrectJsonString(MessageData.Message, false))
	{
		MessageData.Message = UPubnubJsonUtilities::DeserializeString(MessageData.Message);
	}
	
	MessageData.Channel = PubnubCharMemBlockToString(PubnubMessage.channel);
	MessageData.UserID = PubnubCharMemBlockToString(PubnubMessage.publisher);
	MessageData.Timetoken = PubnubCharMemBlockToString(PubnubMessage.tt);
	MessageData.Metadata = PubnubCharMemBlockToString(PubnubMessage.metadata);
	MessageData.MessageType = (EPubnubMessageType)(PubnubMessage.message_type);
	MessageData.CustomMessageType = PubnubCharMemBlockToString(PubnubMessage.custom_message_type);
	MessageData.MatchOrGroup = PubnubCharMemBlockToString(PubnubMessage.match_or_group);
	return MessageData;
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
	if(MemberInclude.IncludeUser)			{FinalString.Append("uuid,");}
	if(MemberInclude.IncludeUserCustom)		{FinalString.Append("uuid.custom,");}
	if(MemberInclude.IncludeUserStatus)		{FinalString.Append("uuid.status,");}
	if(MemberInclude.IncludeUserType)		{FinalString.Append("uuid.type,");}
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

FString UPubnubUtilities::GetMetadataIncludeToString(const FPubnubGetMetadataInclude& GetMetadataInclude)
{
	FString FinalString = "";
	if(GetMetadataInclude.IncludeCustom)			{FinalString.Append("custom,");}
	if(GetMetadataInclude.IncludeStatus)			{FinalString.Append("status,");}
	if(GetMetadataInclude.IncludeType)			{FinalString.Append("type,");}

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
		return "uuid.id";
	case EPubnubMemberSortType::PMeST_UserName:
		return "uuid.name";
	case EPubnubMemberSortType::PMeST_UserUpdated:
		return "uuid.updated";
	case EPubnubMemberSortType::PMeST_UserStatus:
		return "uuid.status";
	case EPubnubMemberSortType::PMeST_UserType:
		return "uuid.type";
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

pubnub_subscription_t* UPubnubUtilities::EEGetSubscriptionForEntity(pubnub_t* Context, FString EntityID, EPubnubEntityType EntityType, FPubnubSubscribeSettings Options)
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
	
	pubnub_entity_free((void**)&PubnubEntity);

	return Subscription;
}

pubnub_subscription_set_t* UPubnubUtilities::EEGetSubscriptionSetForEntities(pubnub_t* Context, TArray<FString> Channels, TArray<FString> ChannelGroups, FPubnubSubscribeSettings Options)
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
		PubnubEntities.Add(reinterpret_cast<pubnub_entity_t*>(pubnub_channel_alloc(Context, EntityIDHolder.Get())));
	}

	pubnub_subscription_set_t* SubscriptionSet = pubnub_subscription_set_alloc_with_entities(PubnubEntities.GetData(), PubnubEntities.Num(), &PnOptions);

	for(pubnub_entity_t*& Entity : PubnubEntities)
	{
		pubnub_entity_free((void**)&Entity);
	}
	
	return SubscriptionSet;
}

bool UPubnubUtilities::EEAddListenerAndSubscribe(pubnub_subscription_t* Subscription, pubnub_subscribe_message_callback_t Callback, UPubnubSubsystem* PubnubSubsystem)
{
	if(!PubnubSubsystem)
	{
		UE_LOG(PubnubLog, Error, TEXT("EEAddListenerAndSubscribe Failed, PubnubSubsystem is invalid"));
		return false;
	}

	EEAddSubscriptionListenersOfAllTypes(Subscription, Callback, PubnubSubsystem);

	return EESubscribeWithSubscription(Subscription, FPubnubSubscriptionCursor());
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

	EERemoveSubscriptionListenersOfAllTypes(SubscriptionPtr, Callback, PubnubSubsystem);

	return EEUnsubscribeWithSubscription(SubscriptionPtr);
}

bool UPubnubUtilities::EESubscribeWithSubscription(pubnub_subscription_t* Subscription, FPubnubSubscriptionCursor Cursor)
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

bool UPubnubUtilities::EEUnsubscribeWithSubscription(pubnub_subscription_t** SubscriptionPtr)
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

bool UPubnubUtilities::EESubscribeWithSubscriptionSet(pubnub_subscription_set_t* SubscriptionSet, FPubnubSubscriptionCursor Cursor)
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

bool UPubnubUtilities::EEUnsubscribeWithSubscriptionSet(pubnub_subscription_set_t** SubscriptionSetPtr)
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

bool UPubnubUtilities::EEAddSubscriptionListenerOfType(pubnub_subscription_t* Subscription, pubnub_subscribe_message_callback_t Callback, EPubnubListenerType ListenerType, UObject* Caller)
{
	if(ListenerType == EPubnubListenerType::PLT_All)
	{
		EEAddSubscriptionListenersOfAllTypes(Subscription, Callback, Caller);
		return false;
	}
	
	pubnub_subscribe_listener_type PubnubListenerType = static_cast<pubnub_subscribe_listener_type>(static_cast<uint8>(ListenerType));
	enum pubnub_res AddMessageListenerResult = pubnub_subscribe_add_subscription_listener(Subscription, PubnubListenerType, Callback, Caller);

	if(PNR_OK != AddMessageListenerResult)
	{
		FString ResultString(pubnub_res_2_string(AddMessageListenerResult));
		UE_LOG(PubnubLog, Error, TEXT("Failed to add listener of type %s. Error: %s "), *StaticEnum<EPubnubListenerType>()->GetNameStringByValue(static_cast<int64>(ListenerType)), *ResultString);
		return false;
	}
	
	return true;
}

void UPubnubUtilities::EEAddSubscriptionListenersOfAllTypes(pubnub_subscription_t* Subscription, pubnub_subscribe_message_callback_t Callback, UObject* Caller)
{
	for(EPubnubListenerType Type : TEnumRange<EPubnubListenerType>())
	{
		EEAddSubscriptionListenerOfType(Subscription, Callback, Type, Caller);
	}
}

bool UPubnubUtilities::EERemoveSubscriptionListenerOfType(pubnub_subscription_t** SubscriptionPtr, pubnub_subscribe_message_callback_t Callback, EPubnubListenerType ListenerType, UObject* Caller)
{
	if(ListenerType == EPubnubListenerType::PLT_All)
	{
		EERemoveSubscriptionListenersOfAllTypes(SubscriptionPtr, Callback, Caller);
		return false;
	}

	pubnub_subscribe_listener_type PubnubListenerType = static_cast<pubnub_subscribe_listener_type>(static_cast<uint8>(ListenerType));
	enum pubnub_res RemoveMessageActionListenerResult =  pubnub_subscribe_remove_subscription_listener(*SubscriptionPtr, PubnubListenerType, Callback, Caller);
	if(PNR_OK != RemoveMessageActionListenerResult)
	{
		FString ResultString(pubnub_res_2_string(RemoveMessageActionListenerResult));
		UE_LOG(PubnubLog, Error, TEXT("Failed to remove listener of type %s. Error: %s "), *StaticEnum<EPubnubListenerType>()->GetNameStringByValue(static_cast<int64>(ListenerType)), *ResultString);
		return false;
	}

	return true;
}

void UPubnubUtilities::EERemoveSubscriptionListenersOfAllTypes(pubnub_subscription_t** SubscriptionPtr, pubnub_subscribe_message_callback_t Callback, UObject* Caller)
{
	for(EPubnubListenerType Type : TEnumRange<EPubnubListenerType>())
	{
		EERemoveSubscriptionListenerOfType(SubscriptionPtr, Callback, Type, Caller);
	}
}

bool UPubnubUtilities::EEAddSubscriptionSetListenerOfType(pubnub_subscription_set_t* SubscriptionSet, pubnub_subscribe_message_callback_t Callback, EPubnubListenerType ListenerType, UObject* Caller)
{
	if(ListenerType == EPubnubListenerType::PLT_All)
	{
		EEAddSubscriptionSetListenersOfAllTypes(SubscriptionSet, Callback, Caller);
		return false;
	}
	
	pubnub_subscribe_listener_type PubnubListenerType = static_cast<pubnub_subscribe_listener_type>(static_cast<uint8>(ListenerType));
	enum pubnub_res AddMessageListenerResult = pubnub_subscribe_add_subscription_set_listener(SubscriptionSet, PubnubListenerType, Callback, Caller);

	if(PNR_OK != AddMessageListenerResult)
	{
		FString ResultString(pubnub_res_2_string(AddMessageListenerResult));
		UE_LOG(PubnubLog, Error, TEXT("Failed to add listener of type %s. Error: %s "), *StaticEnum<EPubnubListenerType>()->GetNameStringByValue(static_cast<int64>(ListenerType)), *ResultString);
		return false;
	}
	
	return true;
}

void UPubnubUtilities::EEAddSubscriptionSetListenersOfAllTypes(pubnub_subscription_set_t* SubscriptionSet, pubnub_subscribe_message_callback_t Callback, UObject* Caller)
{
	for(EPubnubListenerType Type : TEnumRange<EPubnubListenerType>())
	{
		EEAddSubscriptionSetListenerOfType(SubscriptionSet, Callback, Type, Caller);
	}
}

bool UPubnubUtilities::EERemoveSubscriptionSetListenerOfType(pubnub_subscription_set_t** SubscriptionSetPtr, pubnub_subscribe_message_callback_t Callback, EPubnubListenerType ListenerType, UObject* Caller)
{
	if(ListenerType == EPubnubListenerType::PLT_All)
	{
		EERemoveSubscriptionSetListenersOfAllTypes(SubscriptionSetPtr, Callback, Caller);
		return false;
	}

	pubnub_subscribe_listener_type PubnubListenerType = static_cast<pubnub_subscribe_listener_type>(static_cast<uint8>(ListenerType));
	enum pubnub_res RemoveMessageActionListenerResult =  pubnub_subscribe_remove_subscription_set_listener(*SubscriptionSetPtr, PubnubListenerType, Callback, Caller);
	if(PNR_OK != RemoveMessageActionListenerResult)
	{
		FString ResultString(pubnub_res_2_string(RemoveMessageActionListenerResult));
		UE_LOG(PubnubLog, Error, TEXT("Failed to remove listener of type %s. Error: %s "), *StaticEnum<EPubnubListenerType>()->GetNameStringByValue(static_cast<int64>(ListenerType)), *ResultString);
		return false;
	}

	return true;
}

void UPubnubUtilities::EERemoveSubscriptionSetListenersOfAllTypes(pubnub_subscription_set_t** SubscriptionSetPtr, pubnub_subscribe_message_callback_t Callback, UObject* Caller)
{
	for(EPubnubListenerType Type : TEnumRange<EPubnubListenerType>())
	{
		EERemoveSubscriptionSetListenerOfType(SubscriptionSetPtr, Callback, Type, Caller);
	}
}

