// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PubnubStructLibrary.h"
#include "PubnubInternalUtilities.generated.h"

struct pubnub_publish_options;

/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubInternalUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	static void PublishUESettingsToPubnubPublishOptions(FPubnubPublishSettings &PublishSettings, pubnub_publish_options &PubnubPublishOptions);
};
