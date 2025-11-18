// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "FunctionLibraries/PubnubInternalUtilities.h"
#include "PubNub.h"


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
