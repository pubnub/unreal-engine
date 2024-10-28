// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubnubEnumLibrary.generated.h"

UENUM(BlueprintType)
enum class EPubnubPublishMethod : uint8
{
	pubnubSendViaGET,
	pubnubSendViaPOST,
	pubnubUsePATCH,
	pubnubSendViaPOSTwithGZIP,
	pubnubUsePATCHwithGZIP,
	pubnubUseDELETE
};

UENUM(BlueprintType)
enum class EPubnubTribool : uint8
{
	pbccFalse,
	pbccTrue,
	pbccNotSet
};

UENUM(BlueprintType)
enum class EPubnubErrorType : uint8
{
	PET_Error		UMETA(DisplayName="Error"),
	PET_Warning		UMETA(DisplayName="Warning")
};
