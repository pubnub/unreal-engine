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

UENUM(BlueprintType)
enum class EPubnubTribool : uint8
{
	pbccFalse,
	pbccTrue,
	pbccNotSet
};

UENUM(BlueprintType)
enum class EPubnubActionType : uint8
{
	//Note in ChatSDK this is called "reactions". In C-Core this will be send as "reaction"
    pbactypReaction,
    pbactypReceipt,
    pbactypCustom,
	pbactypEdited,
	pbactypDeleted
};

UENUM(BlueprintType)
enum class EPubnubErrorType : uint8
{
	PET_Error		UMETA(DisplayName="Error"),
	PET_Warning		UMETA(DisplayName="Warning")
};
