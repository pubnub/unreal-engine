// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PubnubStructLibrary.h"
#include "PubnubInternalUtilities.generated.h"

struct pubnub_publish_options;
struct pubnub_here_now_options;
struct pubnub_set_state_options;
struct pubnub_fetch_history_options;

/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubInternalUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	static void PublishUESettingsToPubnubPublishOptions(const FPubnubPublishSettings &PublishSettings, pubnub_publish_options &PubnubPublishOptions);
	static void HereNowUESettingsToPubnubHereNowOptions(const FPubnubListUsersFromChannelSettings &HereNowSettings, pubnub_here_now_options &PubnubHereNowOptions);
	static void SetStateUESettingsToPubnubSetStateOptions(const FPubnubSetStateSettings &SetStateSettings, pubnub_set_state_options &PubnubSetStateOptions);
	static void FetchHistoryUESettingsToPbFetchHistoryOptions(const FPubnubFetchHistorySettings &FetchHistorySettings, pubnub_fetch_history_options &PubnubFetchHistoryOptions);

};
