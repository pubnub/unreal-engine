// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "FunctionLibraries/PubnubInternalUtilities.h"
#include "Entities/PubnubSubscription.h"
#include "PubnubClient.h"
#include "PubnubLibraryVersion.h"
#include "PubnubInternalStructLibrary.h"
#include "FunctionLibraries/PubnubUtilities.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

#include "PubNub.h"

FString UPubnubInternalUtilities::GetOperatingSystemString()
{
#if PLATFORM_WINDOWS
	return FString(TEXT("Windows"));
#elif PLATFORM_MAC
	return FString(TEXT("MacOS"));
#elif PLATFORM_ANDROID
	return FString(TEXT("Android"));
#elif PLATFORM_IOS
	return FString(TEXT("iOS"));
#elif PLATFORM_LINUX
	return FString(TEXT("Linux"));
#else
	return FString(TEXT("Unknown"));
#endif
}

FString UPubnubInternalUtilities::GetPubnubSdkVersionSuffix()
{
	return FString::Printf(
		TEXT("%s-Pubnub-C-core/%s/Unreal/%d.%d.%d"),
		*GetOperatingSystemString(),
		ANSI_TO_TCHAR(PUBNUB_C_CORE_VERSION),
		PUBNUB_LIBRARY_VERSION_MAJOR,
		PUBNUB_LIBRARY_VERSION_MINOR,
		PUBNUB_LIBRARY_VERSION_PATCH
	);
}

FString UPubnubInternalUtilities::PubnubStringViewToString(pubnub_string_view_t StringView)
{
	if (!StringView.ptr || StringView.len == 0)
	{
		return FString();
	}
	FUTF8ToTCHAR Converter(StringView.ptr, static_cast<int32>(StringView.len));
	return FString(Converter.Length(), Converter.Get());
}

void UPubnubInternalUtilities::PublishUESettingsToPubnubPublishOptions(const FPubnubPublishSettings &PublishSettings, pubnub_publish_opts_t& PubnubPublishOptions)
{
	PublishSettings.StoreInHistory ? PubnubPublishOptions.store = PUBNUB_PUBLISH_STORE_YES : PubnubPublishOptions.store = PUBNUB_PUBLISH_STORE_NO;
	PubnubPublishOptions.ttl = PublishSettings.Ttl;
	PubnubPublishOptions.method = static_cast<pubnub_publish_method_t>(static_cast<uint8>(PublishSettings.PublishMethod));
}

FString UPubnubInternalUtilities::PubnubJsonValueToFString(pubnub_serialization_provider_t* Serial, const pubnub_json_value_t* Value)
{
	if (!Serial || !Value)
	{
		return FString();
	}

	if (Serial->value_type && Serial->value_as_string && Serial->value_type(Value) == PUBNUB_JSON_STRING)
	{
		size_t Length = 0;
		const char* StringPtr = Serial->value_as_string(Value, &Length);
		if (StringPtr)
		{
			pubnub_string_view_t View;
			View.ptr = StringPtr;
			View.len = Length;
			return PubnubStringViewToString(View);
		}
	}

	if (!Serial->serialize)
	{
		return FString();
	}

	// Payload cannot exceed the C-Core response buffer; serialize into a matching heap buffer.
	TArray<uint8> Buffer;
	size_t Capacity = static_cast<size_t>(PUBNUB_CFG_RESPONSE_BUFFER_SIZE) + 1;
	for (int32 Attempt = 0; Attempt < 3; ++Attempt)
	{
		Buffer.SetNumUninitialized(static_cast<int32>(Capacity));
		size_t OutLength = 0;
		const pubnub_res_t SerializeResult = Serial->serialize(Serial, Value, Buffer.GetData(), Buffer.Num() - 1, &OutLength);
		if (SerializeResult == PUBNUB_OK)
		{
			Buffer[OutLength] = 0;
			return FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(Buffer.GetData())));
		}
		Capacity *= 4;
	}

	return FString();
}

FString UPubnubInternalUtilities::PubnubJsonFieldToString(pubnub_serialization_provider_t* Serial, const pubnub_json_value_t* Value)
{
	if (!Serial || !Value || !Serial->value_type || Serial->value_type(Value) == PUBNUB_JSON_NULL)
	{
		return FString();
	}
	return PubnubJsonValueToFString(Serial, Value);
}

FString UPubnubInternalUtilities::PubnubJsonObjectStringField(pubnub_serialization_provider_t* Serial, const pubnub_json_value_t* Object, const char* Key)
{
	if (!Serial || !Object || !Key || !Serial->object_get || !Serial->value_type || !Serial->value_as_string)
	{
		return FString();
	}

	const size_t KeyLen = static_cast<size_t>(FCStringAnsi::Strlen(Key));
	const pubnub_json_value_t* Field = Serial->object_get(Object, Key, KeyLen);
	if (!Field || Serial->value_type(Field) != PUBNUB_JSON_STRING)
	{
		return FString();
	}

	size_t Length = 0;
	const char* StringPtr = Serial->value_as_string(Field, &Length);
	if (!StringPtr)
	{
		return FString();
	}

	pubnub_string_view_t View;
	View.ptr = StringPtr;
	View.len = Length;
	return PubnubStringViewToString(View);
}

int UPubnubInternalUtilities::MessageCountToInt(uint32 Count)
{
	return Count > static_cast<uint32>(MAX_int32) ? MAX_int32 : static_cast<int>(Count);
}

void UPubnubInternalUtilities::AppendHistoryMessageActions(pubnub_serialization_provider_t* Serial, const pubnub_json_value_t* Actions, const FString& MessageTimetoken, TArray<FPubnubMessageActionData>& OutActions)
{
	if (!Serial || !Actions || !Serial->value_type || !Serial->object_iter_init || !Serial->object_iter_next || !Serial->array_get || !Serial->array_size)
	{
		return;
	}
	if (Serial->value_type(Actions) != PUBNUB_JSON_OBJECT)
	{
		return;
	}

	pubnub_json_iter_t TypeIter;
	if (Serial->object_iter_init(Actions, &TypeIter) == 0)
	{
		return;
	}

	const char* TypeKey = nullptr;
	size_t TypeKeyLen = 0;
	pubnub_json_value_t* TypeValue = nullptr;
	while (Serial->object_iter_next(&TypeIter, &TypeKey, &TypeKeyLen, &TypeValue))
	{
		if (!TypeValue || Serial->value_type(TypeValue) != PUBNUB_JSON_OBJECT)
		{
			continue;
		}

		pubnub_string_view_t TypeView;
		TypeView.ptr = TypeKey;
		TypeView.len = TypeKeyLen;
		const FString ActionType = PubnubStringViewToString(TypeView);

		pubnub_json_iter_t ValueIter;
		if (Serial->object_iter_init(TypeValue, &ValueIter) == 0)
		{
			continue;
		}

		const char* ValueKey = nullptr;
		size_t ValueKeyLen = 0;
		pubnub_json_value_t* ValueNode = nullptr;
		while (Serial->object_iter_next(&ValueIter, &ValueKey, &ValueKeyLen, &ValueNode))
		{
			if (!ValueNode || Serial->value_type(ValueNode) != PUBNUB_JSON_ARRAY)
			{
				continue;
			}

			pubnub_string_view_t ValueView;
			ValueView.ptr = ValueKey;
			ValueView.len = ValueKeyLen;
			const FString ActionValue = PubnubStringViewToString(ValueView);
			const size_t ActionCount = Serial->array_size(ValueNode);
			for (size_t ActionIndex = 0; ActionIndex < ActionCount; ++ActionIndex)
			{
				const pubnub_json_value_t* ActionNode = Serial->array_get(ValueNode, ActionIndex);
				if (!ActionNode || Serial->value_type(ActionNode) != PUBNUB_JSON_OBJECT)
				{
					continue;
				}

				FPubnubMessageActionData Action;
				Action.Type = ActionType;
				Action.Value = ActionValue;
				Action.MessageTimetoken = MessageTimetoken;
				Action.UserID = PubnubJsonObjectStringField(Serial, ActionNode, "uuid");
				Action.ActionTimetoken = PubnubJsonObjectStringField(Serial, ActionNode, "actionTimetoken");
				OutActions.Add(MoveTemp(Action));
			}
		}
	}
}

EPubnubMessageType UPubnubInternalUtilities::MessageTypeFromPubnubSubscribeEvent(int EventType)
{
	switch (EventType)
	{
	case PUBNUB_SUBSCRIBE_SIGNAL: return EPubnubMessageType::PMT_Signal;
	case PUBNUB_SUBSCRIBE_MESSAGE_ACTION: return EPubnubMessageType::PMT_Action;
	case PUBNUB_SUBSCRIBE_APP_CONTEXT: return EPubnubMessageType::PMT_Objects;
	case PUBNUB_SUBSCRIBE_FILE: return EPubnubMessageType::PMT_Files;
	case PUBNUB_SUBSCRIBE_MESSAGE:
	case PUBNUB_SUBSCRIBE_PRESENCE:
	default: return EPubnubMessageType::PMT_Published;
	}
}

EPubnubSubscriptionStatus UPubnubInternalUtilities::SubscriptionStatusFromPubnubSubscribeStatus(int Status)
{
	// C-Core and UE enum ordering differ: DISCONNECTED and CONNECTION_ERROR are swapped.
	switch (Status)
	{
	case PUBNUB_SUBSCRIBE_STATUS_CONNECTED:                 return EPubnubSubscriptionStatus::PSS_Connected;
	case PUBNUB_SUBSCRIBE_STATUS_DISCONNECTED:              return EPubnubSubscriptionStatus::PSS_Disconnected;
	case PUBNUB_SUBSCRIBE_STATUS_DISCONNECTED_UNEXPECTEDLY: return EPubnubSubscriptionStatus::PSS_DisconnectedUnexpectedly;
	case PUBNUB_SUBSCRIBE_STATUS_CONNECTION_ERROR:          return EPubnubSubscriptionStatus::PSS_ConnectionError;
	case PUBNUB_SUBSCRIBE_STATUS_SUBSCRIPTION_CHANGED:      return EPubnubSubscriptionStatus::PSS_SubscriptionChanged;
	default:                                                return EPubnubSubscriptionStatus::PSS_Disconnected;
	}
}

pubnub_subscription_t UPubnubInternalUtilities::CreateCCoreSubscription(pubnub_context_t* Context, const FString& EntityID, EPubnubEntityType EntityType, FPubnubSubscribeSettings Options)
{
	if (!Context || EntityID.IsEmpty())
	{
		return nullptr;
	}

	FUTF8StringHolder EntityIDHolder(EntityID);
	pubnub_entity_t Entity = nullptr;
	switch (EntityType)
	{
	case EPubnubEntityType::PEnT_Channel:
		Entity = pubnub_channel(Context, EntityIDHolder.Get());
		break;
	case EPubnubEntityType::PEnT_ChannelGroup:
		Entity = pubnub_channel_group(Context, EntityIDHolder.Get());
		break;
	case EPubnubEntityType::PEnT_ChannelMetadata:
		Entity = pubnub_channel_metadata(Context, EntityIDHolder.Get());
		break;
	case EPubnubEntityType::PEnT_UserMetadata:
		Entity = pubnub_user_metadata(Context, EntityIDHolder.Get());
		break;
	default:
		UE_LOG(PubnubLog, Error, TEXT("Unknown entity type: %d"), static_cast<int32>(EntityType));
		return nullptr;
	}

	if (!Entity)
	{
		UE_LOG(PubnubLog, Error, TEXT("Failed to create C-Core entity for '%s'."), *EntityID);
		return nullptr;
	}

	pubnub_subscription_opts_t SubscriptionOptions = PUBNUB_SUBSCRIPTION_OPTS_INIT;
	SubscriptionOptions.with_presence = Options.ReceivePresenceEvents ? 1 : 0;

	pubnub_subscription_t Subscription = pubnub_subscription_create(Entity, &SubscriptionOptions);
	pubnub_entity_destroy(Entity);

	if (!Subscription)
	{
		UE_LOG(PubnubLog, Error, TEXT("Failed to create C-Core subscription for '%s'."), *EntityID);
	}

	return Subscription;
}

void UPubnubInternalUtilities::ListUserSubscribedChannelsFromFuture(pubnub_future_t Future, FPubnubListUsersSubscribedChannelsResult& OutResult)
{
	const pubnub_where_now_result_t WhereNowResult = pubnub_where_now_result(Future);
	OutResult.Channels.Reset();
	OutResult.Channels.Reserve(static_cast<int32>(WhereNowResult.channel_count));
	for (uint32_t ChannelIndex = 0; ChannelIndex < WhereNowResult.channel_count; ++ChannelIndex)
	{
		const pubnub_string_view_t ChannelName = pubnub_where_now_result_channel_at(Future, ChannelIndex);
		OutResult.Channels.Add(PubnubStringViewToString(ChannelName));
	}
}

void UPubnubInternalUtilities::GetStateFromFuture(pubnub_future_t Future, FPubnubGetStateResult& OutResult)
{
	pubnub_serialization_provider_t* Serial = pubnub_serialization(Future.ctx);
	const pubnub_get_state_result_t Summary = pubnub_get_state_result(Future);
	OutResult.States.Reset();
	OutResult.States.Reserve(static_cast<int32>(Summary.channel_count));

	for (uint32_t ChannelIndex = 0; ChannelIndex < Summary.channel_count; ++ChannelIndex)
	{
		const pubnub_get_state_channel_result_t Entry = pubnub_get_state_result_channel_at(Future, ChannelIndex);
		FPubnubUserStateOnChannel UserState;
		UserState.Channel = PubnubStringViewToString(Entry.channel);
		if (UserState.Channel.IsEmpty())
		{
			continue;
		}
		UserState.State = PubnubJsonValueToFString(Serial, Entry.state);
		OutResult.States.Add(MoveTemp(UserState));
	}
}

void UPubnubInternalUtilities::ListChannelsFromGroupFromFuture(pubnub_future_t Future, FPubnubListChannelsFromGroupResult& OutResult)
{
	const pubnub_channel_group_list_result_t List = pubnub_channel_group_list_result(Future);
	OutResult.Channels.Reset();
	OutResult.Channels.Reserve(static_cast<int32>(List.count));
	for (uint32_t ChannelIndex = 0; ChannelIndex < List.count; ++ChannelIndex)
	{
		const pubnub_string_view_t ChannelName = pubnub_channel_group_list_result_channel_at(Future, ChannelIndex);
		OutResult.Channels.Add(PubnubStringViewToString(ChannelName));
	}
}

void UPubnubInternalUtilities::ListUsersFromChannelFromFuture(pubnub_future_t Future, FPubnubListUsersFromChannelResult& OutResult)
{
	const pubnub_here_now_result_t Summary = pubnub_here_now_result(Future);
	OutResult.TotalOccupancy = static_cast<int>(Summary.total_occupancy);
	OutResult.TotalChannels = static_cast<int>(Summary.total_channels);
	OutResult.Channels.Reset();
	OutResult.Channels.Reserve(static_cast<int32>(Summary.channel_count));

	for (uint32_t ChannelIndex = 0; ChannelIndex < Summary.channel_count; ++ChannelIndex)
	{
		const pubnub_here_now_channel_result_t ChannelResult = pubnub_here_now_result_channel_at(Future, ChannelIndex);
		FPubnubUsersFromChannel ChannelUsers;
		ChannelUsers.Channel = PubnubStringViewToString(ChannelResult.name);
		ChannelUsers.Occupancy = static_cast<int>(ChannelResult.occupancy);
		ChannelUsers.Users.Reserve(static_cast<int32>(ChannelResult.occupant_count));

		for (uint32_t OccupantIndex = 0; OccupantIndex < ChannelResult.occupant_count; ++OccupantIndex)
		{
			const pubnub_here_now_occupant_result_t Occupant = pubnub_here_now_result_occupant_at(Future, ChannelIndex, OccupantIndex);
			FPubnubUserFromChannel User;
			User.UserID = PubnubStringViewToString(Occupant.uuid);
			if (User.UserID.IsEmpty())
			{
				continue;
			}
			User.State = PubnubStringViewToString(Occupant.state);
			ChannelUsers.Users.Add(MoveTemp(User));
		}

		OutResult.Channels.Add(MoveTemp(ChannelUsers));
	}
}

FPubnubMessageData UPubnubInternalUtilities::UEMessageFromSubscribeEvent(pubnub_context_t* Context, const pubnub_subscribe_event_t* Event)
{
	FPubnubMessageData MessageData;
	if (!Context || !Event)
	{
		return MessageData;
	}

	MessageData.Channel = PubnubStringViewToString(Event->channel);
	// Presence events are delivered on the base channel; keep the old UE -pnpres convention
	// so existing listeners can still detect presence from Channel.
	if (Event->type == PUBNUB_SUBSCRIBE_PRESENCE && !MessageData.Channel.EndsWith(TEXT("-pnpres")))
	{
		MessageData.Channel.Append(TEXT("-pnpres"));
	}

	MessageData.UserID = PubnubStringViewToString(Event->publisher);
	MessageData.Timetoken = PubnubStringViewToString(Event->timetoken);
	MessageData.CustomMessageType = PubnubStringViewToString(Event->custom_message_type);
	MessageData.MatchOrGroup = PubnubStringViewToString(Event->subscription);
	MessageData.MessageType = MessageTypeFromPubnubSubscribeEvent(Event->type);
	MessageData.flags = static_cast<int>(Event->flags);

	pubnub_serialization_provider_t* Serial = pubnub_serialization(Context);
	MessageData.Message = PubnubJsonValueToFString(Serial, Event->payload);
	MessageData.Metadata = PubnubJsonValueToFString(Serial, Event->user_metadata);

	return MessageData;
}

void UPubnubInternalUtilities::FetchHistoryFromFuture(pubnub_future_t Future, FPubnubFetchHistoryResult& OutResult)
{
	pubnub_serialization_provider_t* Serial = pubnub_serialization(Future.ctx);
	const pubnub_fetch_messages_result_t Summary = pubnub_fetch_messages_result(Future);
	OutResult.Messages.Reset();

	// FetchHistory is a single-channel API. The previous JSON parser kept only the first channel.
	if (Summary.channel_count == 0)
	{
		return;
	}

	const pubnub_fetch_messages_channel_result_t ChannelResult = pubnub_fetch_messages_result_channel_at(Future, 0);
	const FString ChannelName = PubnubStringViewToString(ChannelResult.name);
	OutResult.Messages.Reserve(static_cast<int32>(ChannelResult.message_count));

	for (uint32 MessageIndex = 0; MessageIndex < ChannelResult.message_count; ++MessageIndex)
	{
		// Copy payload before the next message_at call. A decrypted node is invalidated by the next access.
		const pubnub_history_message_result_t Message = pubnub_fetch_messages_result_message_at(Future, 0, MessageIndex);
		FPubnubHistoryMessageData HistoryMessage;
		HistoryMessage.Channel = ChannelName;
		HistoryMessage.Message = PubnubJsonFieldToString(Serial, Message.message);
		HistoryMessage.UserID = PubnubStringViewToString(Message.uuid);
		HistoryMessage.Timetoken = PubnubStringViewToString(Message.timetoken);
		HistoryMessage.Meta = PubnubJsonFieldToString(Serial, Message.meta);
		HistoryMessage.CustomMessageType = PubnubStringViewToString(Message.custom_message_type);

		// C-Core maps JSON null message_type to PUBNUB_EVENT_TYPE_MESSAGE. The previous parser
		// left MessageType empty for null and for numeric 0. Only other numeric types are written.
		if (Message.event_type != PUBNUB_EVENT_TYPE_UNKNOWN
			&& Message.event_type != PUBNUB_EVENT_TYPE_MESSAGE
			&& Message.event_type != PUBNUB_EVENT_TYPE_PRESENCE)
		{
			HistoryMessage.MessageType = FString::FromInt(static_cast<int32>(Message.event_type));
		}

		const pubnub_json_value_t* Actions = pubnub_fetch_messages_result_actions_at(Future, 0, MessageIndex);
		AppendHistoryMessageActions(Serial, Actions, HistoryMessage.Timetoken, HistoryMessage.MessageActions);
		OutResult.Messages.Add(MoveTemp(HistoryMessage));
	}
}

void UPubnubInternalUtilities::MessageCountsFromFuture(pubnub_future_t Future, const FString& Channel, FPubnubMessageCountsResult& OutResult)
{
	const pubnub_message_counts_result_t Summary = pubnub_message_counts_result(Future);
	int SoleCount = 0;
	bool bFoundName = false;

	for (uint32 ChannelIndex = 0; ChannelIndex < Summary.channel_count; ++ChannelIndex)
	{
		const pubnub_message_counts_channel_result_t Entry = pubnub_message_counts_result_channel_at(Future, ChannelIndex);
		const int Count = MessageCountToInt(Entry.count);
		if (Summary.channel_count == 1)
		{
			SoleCount = Count;
		}
		if (PubnubStringViewToString(Entry.name) == Channel)
		{
			OutResult.MessageCounts = Count;
			bFoundName = true;
			break;
		}
	}

	if (!bFoundName)
	{
		OutResult.MessageCounts = SoleCount;
	}
}

void UPubnubInternalUtilities::MessageCountsMultipleFromFuture(pubnub_future_t Future, const TArray<FString>& Channels, FPubnubMessageCountsMultipleResult& OutResult)
{
	OutResult.MessageCountsPerChannel.Reset();
	for (const FString& RequestedChannel : Channels)
	{
		OutResult.MessageCountsPerChannel.FindOrAdd(RequestedChannel) = 0;
	}

	const pubnub_message_counts_result_t Summary = pubnub_message_counts_result(Future);
	for (uint32 ChannelIndex = 0; ChannelIndex < Summary.channel_count; ++ChannelIndex)
	{
		const pubnub_message_counts_channel_result_t Entry = pubnub_message_counts_result_channel_at(Future, ChannelIndex);
		const FString Name = PubnubStringViewToString(Entry.name);
		if (Name.IsEmpty())
		{
			continue;
		}
		OutResult.MessageCountsPerChannel.FindOrAdd(Name) = MessageCountToInt(Entry.count);
	}
}

static FPubnubMessageActionData MessageActionFromCCore(const pubnub_message_action_t& Action)
{
	FPubnubMessageActionData Data;
	Data.Type = UPubnubInternalUtilities::PubnubStringViewToString(Action.type);
	Data.Value = UPubnubInternalUtilities::PubnubStringViewToString(Action.value);
	Data.UserID = UPubnubInternalUtilities::PubnubStringViewToString(Action.uuid);
	Data.ActionTimetoken = UPubnubInternalUtilities::PubnubStringViewToString(Action.action_timetoken);
	Data.MessageTimetoken = UPubnubInternalUtilities::PubnubStringViewToString(Action.message_timetoken);
	return Data;
}

void UPubnubInternalUtilities::AddMessageActionFromFuture(pubnub_future_t Future, FPubnubAddMessageActionResult& OutResult)
{
	const pubnub_add_message_action_result_t Result = pubnub_add_message_action_result(Future);
	OutResult.MessageActionData = MessageActionFromCCore(Result.action);
}

void UPubnubInternalUtilities::GetMessageActionsFromFuture(pubnub_future_t Future, FPubnubGetMessageActionsResult& OutResult)
{
	const pubnub_get_message_actions_result_t Summary = pubnub_get_message_actions_result(Future);
	OutResult.MessageActions.Reset();
	OutResult.MessageActions.Reserve(static_cast<int32>(Summary.count));

	for (uint32 ActionIndex = 0; ActionIndex < Summary.count; ++ActionIndex)
	{
		const pubnub_message_action_t Action = pubnub_get_message_actions_result_action_at(Future, ActionIndex);
		OutResult.MessageActions.Add(MessageActionFromCCore(Action));
	}
}

FString UPubnubInternalUtilities::AppContextJsonObjectToString(pubnub_serialization_provider_t* Serial, const pubnub_json_value_t* Value)
{
	if (!Serial || !Value || !Serial->value_type || Serial->value_type(Value) != PUBNUB_JSON_OBJECT)
	{
		return FString();
	}
	return PubnubJsonValueToFString(Serial, Value);
}

FPubnubUserData UPubnubInternalUtilities::UserDataFromUuidMetadata(pubnub_serialization_provider_t* Serial, const pubnub_uuid_metadata_t& Metadata)
{
	FPubnubUserData UserData;
	UserData.UserID = PubnubStringViewToString(Metadata.id);
	UserData.UserName = PubnubStringViewToString(Metadata.name);
	UserData.ExternalID = PubnubStringViewToString(Metadata.external_id);
	UserData.ProfileUrl = PubnubStringViewToString(Metadata.profile_url);
	UserData.Email = PubnubStringViewToString(Metadata.email);
	UserData.Status = PubnubStringViewToString(Metadata.status);
	UserData.Type = PubnubStringViewToString(Metadata.type);
	UserData.Updated = PubnubStringViewToString(Metadata.updated);
	UserData.ETag = PubnubStringViewToString(Metadata.etag);
	UserData.Custom = AppContextJsonObjectToString(Serial, Metadata.custom);
	return UserData;
}

FPubnubChannelData UPubnubInternalUtilities::ChannelDataFromChannelMetadata(pubnub_serialization_provider_t* Serial, const pubnub_channel_metadata_t& Metadata)
{
	FPubnubChannelData ChannelData;
	ChannelData.ChannelID = PubnubStringViewToString(Metadata.id);
	ChannelData.ChannelName = PubnubStringViewToString(Metadata.name);
	ChannelData.Description = PubnubStringViewToString(Metadata.description);
	ChannelData.Status = PubnubStringViewToString(Metadata.status);
	ChannelData.Type = PubnubStringViewToString(Metadata.type);
	ChannelData.Updated = PubnubStringViewToString(Metadata.updated);
	ChannelData.ETag = PubnubStringViewToString(Metadata.etag);
	ChannelData.Custom = AppContextJsonObjectToString(Serial, Metadata.custom);
	return ChannelData;
}

void UPubnubInternalUtilities::CopyAppContextPage(const pubnub_app_context_page_t& Page, FPubnubPage& OutPage, int& OutTotalCount)
{
	OutPage.Next = PubnubStringViewToString(Page.next);
	OutPage.Prev = PubnubStringViewToString(Page.prev);
	OutTotalCount = MessageCountToInt(Page.total_count);
}

uint32 UPubnubInternalUtilities::AppContextIncludeMaskFromString(const FString& Include, FString& OutUnknownTokens)
{
	static const FAppContextIncludeToken IncludeTokens[] = {
		{TEXT("custom"), PUBNUB_APP_CONTEXT_INCLUDE_CUSTOM},
		{TEXT("type"), PUBNUB_APP_CONTEXT_INCLUDE_TYPE},
		{TEXT("status"), PUBNUB_APP_CONTEXT_INCLUDE_STATUS},
		{TEXT("uuid"), PUBNUB_APP_CONTEXT_INCLUDE_UUID},
		{TEXT("uuid.custom"), PUBNUB_APP_CONTEXT_INCLUDE_UUID_CUSTOM},
		{TEXT("uuid.type"), PUBNUB_APP_CONTEXT_INCLUDE_UUID_TYPE},
		{TEXT("uuid.status"), PUBNUB_APP_CONTEXT_INCLUDE_UUID_STATUS},
		{TEXT("channel"), PUBNUB_APP_CONTEXT_INCLUDE_CHANNEL},
		{TEXT("channel.custom"), PUBNUB_APP_CONTEXT_INCLUDE_CHANNEL_CUSTOM},
		{TEXT("channel.type"), PUBNUB_APP_CONTEXT_INCLUDE_CHANNEL_TYPE},
		{TEXT("channel.status"), PUBNUB_APP_CONTEXT_INCLUDE_CHANNEL_STATUS},
	};

	uint32 Mask = 0;
	OutUnknownTokens.Reset();
	if (Include.IsEmpty())
	{
		return Mask;
	}

	TArray<FString> Tokens;
	Include.ParseIntoArray(Tokens, TEXT(","), true);
	for (const FString& RawToken : Tokens)
	{
		const FString Token = RawToken.TrimStartAndEnd();
		if (Token.IsEmpty())
		{
			continue;
		}

		const FAppContextIncludeToken* Match = nullptr;
		for (const FAppContextIncludeToken& Candidate : IncludeTokens)
		{
			if (Token.Equals(Candidate.Name, ESearchCase::CaseSensitive))
			{
				Match = &Candidate;
				break;
			}
		}

		if (Match)
		{
			Mask |= Match->Flag;
		}
		else
		{
			if (!OutUnknownTokens.IsEmpty())
			{
				OutUnknownTokens.Append(TEXT(","));
			}
			OutUnknownTokens.Append(Token);
		}
	}

	return Mask;
}

void UPubnubInternalUtilities::GetAllUserMetadataFromFuture(pubnub_future_t Future, FPubnubGetAllUserMetadataResult& OutResult)
{
	pubnub_serialization_provider_t* Serial = pubnub_serialization(Future.ctx);
	const pubnub_app_context_page_t Page = pubnub_get_all_uuid_metadata_result(Future);
	CopyAppContextPage(Page, OutResult.Page, OutResult.TotalCount);

	OutResult.UsersData.Reset();
	OutResult.UsersData.Reserve(static_cast<int32>(Page.count));
	for (uint32_t Index = 0; Index < Page.count; ++Index)
	{
		const pubnub_uuid_metadata_t Metadata = pubnub_get_all_uuid_metadata_result_uuid_at(Future, Index);
		OutResult.UsersData.Add(UserDataFromUuidMetadata(Serial, Metadata));
	}
}

void UPubnubInternalUtilities::GetAllChannelMetadataFromFuture(pubnub_future_t Future, FPubnubGetAllChannelMetadataResult& OutResult)
{
	pubnub_serialization_provider_t* Serial = pubnub_serialization(Future.ctx);
	const pubnub_app_context_page_t Page = pubnub_get_all_channel_metadata_result(Future);
	CopyAppContextPage(Page, OutResult.Page, OutResult.TotalCount);

	OutResult.ChannelsData.Reset();
	OutResult.ChannelsData.Reserve(static_cast<int32>(Page.count));
	for (uint32_t Index = 0; Index < Page.count; ++Index)
	{
		const pubnub_channel_metadata_t Metadata = pubnub_get_all_channel_metadata_result_channel_at(Future, Index);
		OutResult.ChannelsData.Add(ChannelDataFromChannelMetadata(Serial, Metadata));
	}
}

FPubnubUserData UPubnubInternalUtilities::UserDataFromGetUuidMetadataFuture(pubnub_future_t Future)
{
	return UserDataFromUuidMetadata(pubnub_serialization(Future.ctx), pubnub_get_uuid_metadata_result(Future));
}

FPubnubUserData UPubnubInternalUtilities::UserDataFromSetUuidMetadataFuture(pubnub_future_t Future)
{
	return UserDataFromUuidMetadata(pubnub_serialization(Future.ctx), pubnub_set_uuid_metadata_result(Future));
}

FPubnubChannelData UPubnubInternalUtilities::ChannelDataFromGetChannelMetadataFuture(pubnub_future_t Future)
{
	return ChannelDataFromChannelMetadata(pubnub_serialization(Future.ctx), pubnub_get_channel_metadata_result(Future));
}

FPubnubChannelData UPubnubInternalUtilities::ChannelDataFromSetChannelMetadataFuture(pubnub_future_t Future)
{
	return ChannelDataFromChannelMetadata(pubnub_serialization(Future.ctx), pubnub_set_channel_metadata_result(Future));
}

uint32 UPubnubInternalUtilities::MetadataListIncludeMask(const FString& Include, EPubnubTribool Count, FString& OutUnknownTokens)
{
	uint32 Mask = AppContextIncludeMaskFromString(Include, OutUnknownTokens);
	if (Count == EPubnubTribool::PT_True)
	{
		Mask |= PUBNUB_APP_CONTEXT_INCLUDE_TOTAL_COUNT;
	}
	return Mask;
}

bool UPubnubInternalUtilities::ReadOptionalMetadataString(const TSharedPtr<FJsonObject>& Object, const TCHAR* Key, FMetadataStringField& Out, TArray<FString>& NullFields, FString& Error)
{
	if (!Object->HasField(Key))
	{
		return true;
	}

	const TSharedPtr<FJsonValue> Field = Object->TryGetField(Key);
	if (!Field.IsValid() || Field->Type == EJson::Null)
	{
		NullFields.Add(Key);
		return true;
	}
	if (Field->Type != EJson::String)
	{
		Error = FString::Printf(TEXT("%s must be a JSON string or null."), Key);
		return false;
	}

	Out.Value = Field->AsString();
	Out.bSet = true;
	return true;
}

bool UPubnubInternalUtilities::ReadMetadataCustom(const TSharedPtr<FJsonObject>& Object, FString& OutJson, bool& bHasCustom, FString& Error)
{
	if (!Object->HasField(TEXT("custom")))
	{
		return true;
	}

	const TSharedPtr<FJsonValue> Field = Object->TryGetField(TEXT("custom"));
	if (!Field.IsValid() || Field->Type == EJson::Null)
	{
		// Raw JSON null is accepted by pubnub_set_*_metadata custom and clears the object.
		OutJson = TEXT("null");
		bHasCustom = true;
		return true;
	}
	if (Field->Type != EJson::Object)
	{
		Error = TEXT("custom must be a JSON object or null.");
		return false;
	}

	OutJson = UPubnubJsonUtilities::JsonObjectToString(Field->AsObject());
	bHasCustom = !OutJson.IsEmpty();
	return true;
}

void UPubnubInternalUtilities::CollectUnknownMetadataFields(const TSharedPtr<FJsonObject>& Object, const TArray<FString>& KnownKeys, TArray<FString>& OutUnknown)
{
	for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Object->Values)
	{
		if (!KnownKeys.Contains(Pair.Key))
		{
			OutUnknown.Add(Pair.Key);
		}
	}
}

bool UPubnubInternalUtilities::ParseUserMetadataObject(const FString& Json, FParsedUserMetadataObject& Out)
{
	TSharedPtr<FJsonObject> Object;
	if (!UPubnubJsonUtilities::StringToJsonObject(Json, Object) || !Object.IsValid())
	{
		Out.Error = TEXT("UserMetadataObj has to be a correct Json Object. Operation aborted.");
		return false;
	}

	if (!ReadOptionalMetadataString(Object, TEXT("name"), Out.Name, Out.NullStringFields, Out.Error)
		|| !ReadOptionalMetadataString(Object, TEXT("externalId"), Out.ExternalId, Out.NullStringFields, Out.Error)
		|| !ReadOptionalMetadataString(Object, TEXT("profileUrl"), Out.ProfileUrl, Out.NullStringFields, Out.Error)
		|| !ReadOptionalMetadataString(Object, TEXT("email"), Out.Email, Out.NullStringFields, Out.Error)
		|| !ReadOptionalMetadataString(Object, TEXT("status"), Out.Status, Out.NullStringFields, Out.Error)
		|| !ReadOptionalMetadataString(Object, TEXT("type"), Out.Type, Out.NullStringFields, Out.Error)
		|| !ReadMetadataCustom(Object, Out.CustomJson, Out.bHasCustom, Out.Error))
	{
		return false;
	}

	CollectUnknownMetadataFields(Object, {
		TEXT("id"), TEXT("name"), TEXT("externalId"), TEXT("profileUrl"), TEXT("email"),
		TEXT("custom"), TEXT("status"), TEXT("type"), TEXT("updated"), TEXT("eTag")
	}, Out.UnknownFields);
	return true;
}

bool UPubnubInternalUtilities::ParseChannelMetadataObject(const FString& Json, FParsedChannelMetadataObject& Out)
{
	TSharedPtr<FJsonObject> Object;
	if (!UPubnubJsonUtilities::StringToJsonObject(Json, Object) || !Object.IsValid())
	{
		Out.Error = TEXT("ChannelMetadataObj has to be a correct Json Object. Operation aborted.");
		return false;
	}

	if (!ReadOptionalMetadataString(Object, TEXT("name"), Out.Name, Out.NullStringFields, Out.Error)
		|| !ReadOptionalMetadataString(Object, TEXT("description"), Out.Description, Out.NullStringFields, Out.Error)
		|| !ReadOptionalMetadataString(Object, TEXT("status"), Out.Status, Out.NullStringFields, Out.Error)
		|| !ReadOptionalMetadataString(Object, TEXT("type"), Out.Type, Out.NullStringFields, Out.Error)
		|| !ReadMetadataCustom(Object, Out.CustomJson, Out.bHasCustom, Out.Error))
	{
		return false;
	}

	CollectUnknownMetadataFields(Object, {
		TEXT("id"), TEXT("name"), TEXT("description"), TEXT("custom"),
		TEXT("status"), TEXT("type"), TEXT("updated"), TEXT("eTag")
	}, Out.UnknownFields);
	return true;
}

FString UPubnubInternalUtilities::MetadataNullFieldError(const TArray<FString>& Fields)
{
	return FString::Printf(
		TEXT("Cannot clear metadata fields with JSON null (%s). The new C-Core set API can omit a field or set a string, but it cannot send JSON null for string fields."),
		*FString::Join(Fields, TEXT(", ")));
}

const char* UPubnubInternalUtilities::OptionalMetadataString(const FMetadataStringField& Field, const FUTF8StringHolder& Holder)
{
	return Field.bSet ? Holder.Get() : nullptr;
}

FPubnubMembershipData UPubnubInternalUtilities::MembershipDataFromMembership(pubnub_serialization_provider_t* Serial, const pubnub_membership_t& Membership)
{
	FPubnubMembershipData MembershipData;
	MembershipData.Channel = ChannelDataFromChannelMetadata(Serial, Membership.channel);
	MembershipData.Status = PubnubStringViewToString(Membership.status);
	MembershipData.Type = PubnubStringViewToString(Membership.type);
	MembershipData.Custom = AppContextJsonObjectToString(Serial, Membership.custom);
	MembershipData.Updated = PubnubStringViewToString(Membership.updated);
	MembershipData.ETag = PubnubStringViewToString(Membership.etag);
	return MembershipData;
}

FPubnubChannelMemberData UPubnubInternalUtilities::ChannelMemberDataFromMember(pubnub_serialization_provider_t* Serial, const pubnub_member_t& Member)
{
	FPubnubChannelMemberData MemberData;
	MemberData.User = UserDataFromUuidMetadata(Serial, Member.uuid);
	MemberData.Status = PubnubStringViewToString(Member.status);
	MemberData.Type = PubnubStringViewToString(Member.type);
	MemberData.Custom = AppContextJsonObjectToString(Serial, Member.custom);
	MemberData.Updated = PubnubStringViewToString(Member.updated);
	MemberData.ETag = PubnubStringViewToString(Member.etag);
	return MemberData;
}

void UPubnubInternalUtilities::GetMembershipsFromFuture(pubnub_future_t Future, FPubnubMembershipsResult& OutResult)
{
	pubnub_serialization_provider_t* Serial = pubnub_serialization(Future.ctx);
	const pubnub_app_context_page_t Page = pubnub_get_memberships_result(Future);
	CopyAppContextPage(Page, OutResult.Page, OutResult.TotalCount);

	OutResult.MembershipsData.Reset();
	OutResult.MembershipsData.Reserve(static_cast<int32>(Page.count));
	for (uint32_t Index = 0; Index < Page.count; ++Index)
	{
		const pubnub_membership_t Membership = pubnub_get_memberships_result_membership_at(Future, Index);
		OutResult.MembershipsData.Add(MembershipDataFromMembership(Serial, Membership));
	}
}

void UPubnubInternalUtilities::SetMembershipsFromFuture(pubnub_future_t Future, FPubnubMembershipsResult& OutResult)
{
	pubnub_serialization_provider_t* Serial = pubnub_serialization(Future.ctx);
	const pubnub_app_context_page_t Page = pubnub_set_memberships_result(Future);
	CopyAppContextPage(Page, OutResult.Page, OutResult.TotalCount);

	OutResult.MembershipsData.Reset();
	OutResult.MembershipsData.Reserve(static_cast<int32>(Page.count));
	for (uint32_t Index = 0; Index < Page.count; ++Index)
	{
		const pubnub_membership_t Membership = pubnub_set_memberships_result_membership_at(Future, Index);
		OutResult.MembershipsData.Add(MembershipDataFromMembership(Serial, Membership));
	}
}

void UPubnubInternalUtilities::GetChannelMembersFromFuture(pubnub_future_t Future, FPubnubChannelMembersResult& OutResult)
{
	pubnub_serialization_provider_t* Serial = pubnub_serialization(Future.ctx);
	const pubnub_app_context_page_t Page = pubnub_get_channel_members_result(Future);
	CopyAppContextPage(Page, OutResult.Page, OutResult.TotalCount);

	OutResult.MembersData.Reset();
	OutResult.MembersData.Reserve(static_cast<int32>(Page.count));
	for (uint32_t Index = 0; Index < Page.count; ++Index)
	{
		const pubnub_member_t Member = pubnub_get_channel_members_result_member_at(Future, Index);
		OutResult.MembersData.Add(ChannelMemberDataFromMember(Serial, Member));
	}
}

void UPubnubInternalUtilities::SetChannelMembersFromFuture(pubnub_future_t Future, FPubnubChannelMembersResult& OutResult)
{
	pubnub_serialization_provider_t* Serial = pubnub_serialization(Future.ctx);
	const pubnub_app_context_page_t Page = pubnub_set_channel_members_result(Future);
	CopyAppContextPage(Page, OutResult.Page, OutResult.TotalCount);

	OutResult.MembersData.Reset();
	OutResult.MembersData.Reserve(static_cast<int32>(Page.count));
	for (uint32_t Index = 0; Index < Page.count; ++Index)
	{
		const pubnub_member_t Member = pubnub_set_channel_members_result_member_at(Future, Index);
		OutResult.MembersData.Add(ChannelMemberDataFromMember(Serial, Member));
	}
}

bool UPubnubInternalUtilities::ParseRelationSetList(const FString& Json, const TCHAR* IdObjectKey, const TCHAR* JsonName, FParsedRelationList& Out)
{
	TArray<TSharedPtr<FJsonValue>> Items;
	if (!UPubnubJsonUtilities::StringToJsonArray(Json, Items))
	{
		Out.Error = FString::Printf(TEXT("%s has to be a correct Json Object. Operation aborted."), JsonName);
		return false;
	}
	if (Items.Num() == 0)
	{
		Out.Error = FString::Printf(TEXT("%s must contain at least one item. The new C-Core rejects an empty set or remove list."), JsonName);
		return false;
	}

	Out.Items.Reserve(Items.Num());
	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		const TSharedPtr<FJsonObject> Item = Items[Index].IsValid() ? Items[Index]->AsObject() : nullptr;
		if (!Item.IsValid())
		{
			Out.Error = FString::Printf(TEXT("%s item %d must be a JSON object."), JsonName, Index);
			return false;
		}

		const TSharedPtr<FJsonObject>* IdObject = nullptr;
		if (!Item->TryGetObjectField(IdObjectKey, IdObject) || !IdObject || !IdObject->IsValid())
		{
			Out.Error = FString::Printf(TEXT("%s item %d is missing %s.id."), JsonName, Index, IdObjectKey);
			return false;
		}

		FParsedRelationInput Parsed;
		if (!(*IdObject)->TryGetStringField(TEXT("id"), Parsed.Id) || Parsed.Id.IsEmpty())
		{
			Out.Error = FString::Printf(TEXT("%s item %d is missing %s.id."), JsonName, Index, IdObjectKey);
			return false;
		}

		TArray<FString> ItemNullFields;
		if (!ReadOptionalMetadataString(Item, TEXT("status"), Parsed.Status, ItemNullFields, Out.Error)
			|| !ReadOptionalMetadataString(Item, TEXT("type"), Parsed.Type, ItemNullFields, Out.Error)
			|| !ReadMetadataCustom(Item, Parsed.CustomJson, Parsed.bHasCustom, Out.Error))
		{
			Out.Error = FString::Printf(TEXT("%s item %d: %s"), JsonName, Index, *Out.Error);
			return false;
		}
		for (const FString& NullField : ItemNullFields)
		{
			Out.NullStringFields.AddUnique(NullField);
		}

		TArray<FString> KnownItemKeys = {FString(IdObjectKey), TEXT("custom"), TEXT("status"), TEXT("type")};
		CollectUnknownMetadataFields(Item, KnownItemKeys, Out.UnknownFields);
		TArray<FString> IdUnknown;
		CollectUnknownMetadataFields(*IdObject, {TEXT("id")}, IdUnknown);
		for (const FString& Field : IdUnknown)
		{
			Out.UnknownFields.Add(FString::Printf(TEXT("%s.%s"), IdObjectKey, *Field));
		}
		Out.Items.Add(MoveTemp(Parsed));
	}

	return true;
}

bool UPubnubInternalUtilities::ParseRelationRemoveList(const FString& Json, const TCHAR* IdObjectKey, const TCHAR* JsonName, FParsedRelationList& Out)
{
	TArray<TSharedPtr<FJsonValue>> Items;
	if (!UPubnubJsonUtilities::StringToJsonArray(Json, Items))
	{
		Out.Error = FString::Printf(TEXT("%s has to be a correct Json Object. Operation aborted."), JsonName);
		return false;
	}
	if (Items.Num() == 0)
	{
		Out.Error = FString::Printf(TEXT("%s must contain at least one item. The new C-Core rejects an empty set or remove list."), JsonName);
		return false;
	}

	Out.Items.Reserve(Items.Num());
	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		const TSharedPtr<FJsonObject> Item = Items[Index].IsValid() ? Items[Index]->AsObject() : nullptr;
		if (!Item.IsValid())
		{
			Out.Error = FString::Printf(TEXT("%s item %d must be a JSON object."), JsonName, Index);
			return false;
		}

		const TSharedPtr<FJsonObject>* IdObject = nullptr;
		if (!Item->TryGetObjectField(IdObjectKey, IdObject) || !IdObject || !IdObject->IsValid())
		{
			Out.Error = FString::Printf(TEXT("%s item %d is missing %s.id."), JsonName, Index, IdObjectKey);
			return false;
		}

		FParsedRelationInput Parsed;
		if (!(*IdObject)->TryGetStringField(TEXT("id"), Parsed.Id) || Parsed.Id.IsEmpty())
		{
			Out.Error = FString::Printf(TEXT("%s item %d is missing %s.id."), JsonName, Index, IdObjectKey);
			return false;
		}

		TArray<FString> KnownItemKeys = {FString(IdObjectKey)};
		CollectUnknownMetadataFields(Item, KnownItemKeys, Out.UnknownFields);
		TArray<FString> IdUnknown;
		CollectUnknownMetadataFields(*IdObject, {TEXT("id")}, IdUnknown);
		for (const FString& Field : IdUnknown)
		{
			Out.UnknownFields.Add(FString::Printf(TEXT("%s.%s"), IdObjectKey, *Field));
		}
		Out.Items.Add(MoveTemp(Parsed));
	}

	return true;
}

bool UPubnubInternalUtilities::ParseMembershipSetList(const FString& Json, FParsedRelationList& Out)
{
	return ParseRelationSetList(Json, TEXT("channel"), TEXT("SetObj"), Out);
}

bool UPubnubInternalUtilities::ParseMembershipRemoveList(const FString& Json, FParsedRelationList& Out)
{
	return ParseRelationRemoveList(Json, TEXT("channel"), TEXT("RemoveObj"), Out);
}

bool UPubnubInternalUtilities::ParseMemberSetList(const FString& Json, FParsedRelationList& Out)
{
	return ParseRelationSetList(Json, TEXT("uuid"), TEXT("SetObj"), Out);
}

bool UPubnubInternalUtilities::ParseMemberRemoveList(const FString& Json, FParsedRelationList& Out)
{
	return ParseRelationRemoveList(Json, TEXT("uuid"), TEXT("RemoveObj"), Out);
}

void UPubnubInternalUtilities::BuildMembershipInputs(const FParsedRelationList& Parsed, FMembershipInputBatch& Out)
{
	const TArray<FParsedRelationInput>& Items = Parsed.Items;
	Out.Items.Reset();
	Out.Items.Reserve(Items.Num());
	for (const FParsedRelationInput& Item : Items)
	{
		pubnub_membership_input_t Input = {};
		Input.channel_id = Out.Hold(Item.Id);
		Input.status = Item.Status.bSet ? Out.Hold(Item.Status.Value) : nullptr;
		Input.type = Item.Type.bSet ? Out.Hold(Item.Type.Value) : nullptr;
		if (Item.bHasCustom)
		{
			Input.custom = Out.Hold(Item.CustomJson);
		}
		Out.Items.Add(Input);
	}
}

void UPubnubInternalUtilities::BuildMemberInputs(const FParsedRelationList& Parsed, FMemberInputBatch& Out)
{
	const TArray<FParsedRelationInput>& Items = Parsed.Items;
	Out.Items.Reset();
	Out.Items.Reserve(Items.Num());
	for (const FParsedRelationInput& Item : Items)
	{
		pubnub_member_input_t Input = {};
		Input.uuid_id = Out.Hold(Item.Id);
		Input.status = Item.Status.bSet ? Out.Hold(Item.Status.Value) : nullptr;
		Input.type = Item.Type.bSet ? Out.Hold(Item.Type.Value) : nullptr;
		if (Item.bHasCustom)
		{
			Input.custom = Out.Hold(Item.CustomJson);
		}
		Out.Items.Add(Input);
	}
}

template<typename HandleType, typename QueryType>
void UPubnubInternalUtilities::ListCCoreHandles(QueryType Query, TArray<HandleType>& Out)
{
	Out.Reset();
	TArray<HandleType> Buffer;
	Buffer.SetNum(64);
	size_t Count = 0;
	pubnub_res_t Result = Query(Buffer.GetData(), static_cast<size_t>(Buffer.Num()), &Count);
	if (Result == PUBNUB_ERR_BUFFER_TOO_SMALL)
	{
		Buffer.SetNum(static_cast<int32>(Count));
		Result = Query(Buffer.GetData(), static_cast<size_t>(Buffer.Num()), &Count);
	}
	if (Result != PUBNUB_OK)
	{
		UE_LOG(PubnubLog, Error, TEXT("Failed to list C-Core subscriptions. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(Result)));
		Out.Reset();
		return;
	}
	Buffer.SetNum(static_cast<int32>(Count));
	Out = MoveTemp(Buffer);
}

void UPubnubInternalUtilities::FillEntityDataListener(pubnub_subscribe_listener_t* Listener, void* UserData)
{
	if (!Listener)
	{
		return;
	}

	*Listener = {};
	Listener->on_message = +[](const pubnub_subscribe_event_t* Event, void* Data)
	{
		UPubnubInternalUtilities::OnEntitySubscribeEvent(Event, Data);
	};
	Listener->on_signal = +[](const pubnub_subscribe_event_t* Event, void* Data)
	{
		UPubnubInternalUtilities::OnEntitySubscribeEvent(Event, Data);
	};
	Listener->on_presence = +[](const pubnub_subscribe_event_t* Event, void* Data)
	{
		UPubnubInternalUtilities::OnEntitySubscribeEvent(Event, Data);
	};
	Listener->on_message_action = +[](const pubnub_subscribe_event_t* Event, void* Data)
	{
		UPubnubInternalUtilities::OnEntitySubscribeEvent(Event, Data);
	};
	Listener->on_app_context = +[](const pubnub_subscribe_event_t* Event, void* Data)
	{
		UPubnubInternalUtilities::OnEntitySubscribeEvent(Event, Data);
	};
	Listener->on_file = +[](const pubnub_subscribe_event_t* Event, void* Data)
	{
		UPubnubInternalUtilities::OnEntitySubscribeEvent(Event, Data);
	};
	Listener->user_data = UserData;
}

int32 UPubnubInternalUtilities::RestoreSubscribeTimetoken(pubnub_context_t* Context, const FString& Timetoken)
{
	FUTF8StringHolder TimetokenHolder(Timetoken);
	pubnub_timetoken_t Cursor;
	Cursor.ptr = TimetokenHolder.Get();
	Cursor.len = static_cast<size_t>(TimetokenHolder.Converter.Length());
	return pubnub_subscribe_restore(Context, Cursor);
}

pubnub_subscription_set_t UPubnubInternalUtilities::CreateCCoreSubscriptionSet(pubnub_context_t* Context)
{
	if (!Context)
	{
		return nullptr;
	}

	pubnub_subscription_set_t SubscriptionSet = pubnub_subscription_set_create(Context);
	if (!SubscriptionSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("Failed to create C-Core subscription set."));
	}
	return SubscriptionSet;
}

bool UPubnubInternalUtilities::AddEntityToCCoreSubscriptionSet(pubnub_context_t* Context, pubnub_subscription_set_t SubscriptionSet, const FString& EntityID, EPubnubEntityType EntityType, FPubnubSubscribeSettings Options)
{
	if (!Context || !SubscriptionSet || EntityID.IsEmpty())
	{
		return false;
	}

	pubnub_subscription_t Subscription = CreateCCoreSubscription(Context, EntityID, EntityType, Options);
	if (!Subscription)
	{
		return false;
	}

	const pubnub_res_t AddResult = pubnub_subscription_set_add_subscription(SubscriptionSet, Subscription);
	pubnub_subscription_destroy(Subscription);
	if (AddResult != PUBNUB_OK)
	{
		UE_LOG(PubnubLog, Error, TEXT("Failed to add '%s' to subscription set. Error: %s"), *EntityID, UTF8_TO_TCHAR(pubnub_res_str(AddResult)));
		return false;
	}
	return true;
}

int32 UPubnubInternalUtilities::ActivateCCoreSubscription(pubnub_context_t* Context, pubnub_subscription_t Subscription, const FPubnubSubscriptionCursor& Cursor)
{
	if (!Context || !Subscription)
	{
		return PUBNUB_ERR_INVALID_ARGUMENT;
	}
	if (Cursor.Region != 0)
	{
		UE_LOG(PubnubLog, Warning, TEXT("Subscription cursor Region is ignored. The new C-Core resume cursor is a timetoken only."));
	}

	const pubnub_res_t SubscribeResult = pubnub_subscription_subscribe(Subscription);
	if (SubscribeResult != PUBNUB_OK || Cursor.Timetoken.IsEmpty())
	{
		return SubscribeResult;
	}

	const int32 RestoreResult = RestoreSubscribeTimetoken(Context, Cursor.Timetoken);
	if (RestoreResult != PUBNUB_OK)
	{
		pubnub_subscription_unsubscribe(Subscription);
	}
	return RestoreResult;
}

int32 UPubnubInternalUtilities::ActivateCCoreSubscriptionSet(pubnub_context_t* Context, pubnub_subscription_set_t SubscriptionSet, const FPubnubSubscriptionCursor& Cursor)
{
	if (!Context || !SubscriptionSet)
	{
		return PUBNUB_ERR_INVALID_ARGUMENT;
	}
	if (Cursor.Region != 0)
	{
		UE_LOG(PubnubLog, Warning, TEXT("Subscription cursor Region is ignored. The new C-Core resume cursor is a timetoken only."));
	}

	const pubnub_res_t SubscribeResult = pubnub_subscription_set_subscribe(SubscriptionSet);
	if (SubscribeResult != PUBNUB_OK || Cursor.Timetoken.IsEmpty())
	{
		return SubscribeResult;
	}

	const int32 RestoreResult = RestoreSubscribeTimetoken(Context, Cursor.Timetoken);
	if (RestoreResult != PUBNUB_OK)
	{
		pubnub_subscription_set_unsubscribe(SubscriptionSet);
	}
	return RestoreResult;
}

pubnub_listener_handle_t UPubnubInternalUtilities::AddEntitySubscriptionListener(pubnub_subscription_t Subscription, void* UserData)
{
	if (!Subscription || !UserData)
	{
		return PUBNUB_LISTENER_HANDLE_INVALID;
	}
	pubnub_subscribe_listener_t Listener = {};
	FillEntityDataListener(&Listener, UserData);
	return pubnub_subscription_add_listener(Subscription, &Listener);
}

pubnub_listener_handle_t UPubnubInternalUtilities::AddEntitySubscriptionSetListener(pubnub_subscription_set_t SubscriptionSet, void* UserData)
{
	if (!SubscriptionSet || !UserData)
	{
		return PUBNUB_LISTENER_HANDLE_INVALID;
	}
	pubnub_subscribe_listener_t Listener = {};
	FillEntityDataListener(&Listener, UserData);
	return pubnub_subscription_set_add_listener(SubscriptionSet, &Listener);
}

void UPubnubInternalUtilities::RemoveEntitySubscriptionListener(pubnub_subscription_t Subscription, pubnub_listener_handle_t Handle)
{
	if (!Subscription || Handle == PUBNUB_LISTENER_HANDLE_INVALID)
	{
		return;
	}
	pubnub_subscription_remove_listener(Subscription, Handle);
}

void UPubnubInternalUtilities::RemoveEntitySubscriptionSetListener(pubnub_subscription_set_t SubscriptionSet, pubnub_listener_handle_t Handle)
{
	if (!SubscriptionSet || Handle == PUBNUB_LISTENER_HANDLE_INVALID)
	{
		return;
	}
	pubnub_subscription_set_remove_listener(SubscriptionSet, Handle);
}

void UPubnubInternalUtilities::ListActiveCCoreSubscriptions(pubnub_context_t* Context, TArray<pubnub_subscription_t>& Out)
{
	if (!Context)
	{
		Out.Reset();
		return;
	}
	ListCCoreHandles<pubnub_subscription_t>(
		[Context](pubnub_subscription_t* Buffer, size_t MaxCount, size_t* OutCount)
		{
			return pubnub_subscriptions(Context, Buffer, MaxCount, OutCount);
		},
		Out);
}

void UPubnubInternalUtilities::ListActiveCCoreSubscriptionSets(pubnub_context_t* Context, TArray<pubnub_subscription_set_t>& Out)
{
	if (!Context)
	{
		Out.Reset();
		return;
	}
	ListCCoreHandles<pubnub_subscription_set_t>(
		[Context](pubnub_subscription_set_t* Buffer, size_t MaxCount, size_t* OutCount)
		{
			return pubnub_subscription_sets(Context, Buffer, MaxCount, OutCount);
		},
		Out);
}

void UPubnubInternalUtilities::ListCCoreSetSubscriptions(pubnub_subscription_set_t SubscriptionSet, TArray<pubnub_subscription_t>& Out)
{
	if (!SubscriptionSet)
	{
		Out.Reset();
		return;
	}
	ListCCoreHandles<pubnub_subscription_t>(
		[SubscriptionSet](pubnub_subscription_t* Buffer, size_t MaxCount, size_t* OutCount)
		{
			return pubnub_subscription_set_subscriptions(SubscriptionSet, Buffer, MaxCount, OutCount);
		},
		Out);
}

void* UPubnubInternalUtilities::CreateEntityListenerUserData(UPubnubSubscriptionBase* Subscription, pubnub_context_t* Context)
{
	FPubnubInternalEntityListenerUserData* UserData = new FPubnubInternalEntityListenerUserData();
	UserData->WeakSubscription = Subscription;
	UserData->Context = Context;
	return UserData;
}

void UPubnubInternalUtilities::DestroyEntityListenerUserData(void* UserData)
{
	delete static_cast<FPubnubInternalEntityListenerUserData*>(UserData);
}

void UPubnubInternalUtilities::ClearSubscriptionDelegates(UPubnubSubscriptionBase* Subscription)
{
	if (!Subscription)
	{
		return;
	}

	Subscription->OnPubnubMessage.Clear();
	Subscription->OnPubnubMessageNative.Clear();
	Subscription->OnPubnubSignal.Clear();
	Subscription->OnPubnubSignalNative.Clear();
	Subscription->OnPubnubPresenceEvent.Clear();
	Subscription->OnPubnubPresenceEventNative.Clear();
	Subscription->OnPubnubObjectEvent.Clear();
	Subscription->OnPubnubObjectEventNative.Clear();
	Subscription->OnPubnubMessageAction.Clear();
	Subscription->OnPubnubMessageActionNative.Clear();
	Subscription->FOnPubnubAnyMessageType.Clear();
	Subscription->FOnPubnubAnyMessageTypeNative.Clear();
}

void UPubnubInternalUtilities::OnEntitySubscribeEvent(const pubnub_subscribe_event_t* Event, void* UserData)
{
	if (!Event || !UserData)
	{
		return;
	}

	FPubnubInternalEntityListenerUserData* ListenerData = static_cast<FPubnubInternalEntityListenerUserData*>(UserData);
	if (!ListenerData->Context)
	{
		return;
	}

	const TWeakObjectPtr<UPubnubSubscriptionBase> WeakSubscription = ListenerData->WeakSubscription;
	const FPubnubMessageData MessageData = UEMessageFromSubscribeEvent(ListenerData->Context, Event);
	AsyncTask(ENamedThreads::GameThread, [MessageData, WeakSubscription]()
	{
		UPubnubSubscriptionBase* Subscription = WeakSubscription.Get();
		if (!IsValid(Subscription))
		{
			return;
		}
		Subscription->DeliverSubscribeEvent(MessageData);
	});
}

