#pragma once

#include "CoreMinimal.h"

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

