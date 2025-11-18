// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "FunctionLibraries/PubnubInternalUtilities.h"
#include "PubNub.h"


void UPubnubInternalUtilities::PublishUESettingsToPubnubPublishOptions(FPubnubPublishSettings &PublishSettings, pubnub_publish_options& PubnubPublishOptions)
{
	PubnubPublishOptions.store = PublishSettings.StoreInHistory;
	PubnubPublishOptions.replicate = PublishSettings.Replicate;
	PubnubPublishOptions.cipher_key = NULL;
	PubnubPublishOptions.ttl = PublishSettings.Ttl;
	PublishSettings.MetaData.IsEmpty() ? PubnubPublishOptions.meta = NULL : nullptr;
	PublishSettings.CustomMessageType.IsEmpty() ? PubnubPublishOptions.custom_message_type = NULL : nullptr;
	PubnubPublishOptions.method = (pubnub_method)(uint8)PublishSettings.PublishMethod;
}