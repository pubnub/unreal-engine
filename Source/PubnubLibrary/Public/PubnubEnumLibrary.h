// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubnubEnumLibrary.generated.h"

UENUM(BlueprintType)
enum class EPubnubPublishMethod : uint8
{
	PPM_SendViaGET				UMETA(DisplayName="SendViaGET"),
	PPM_SendViaPOST				UMETA(DisplayName="SendViaPOST"),
	PPM_UsePATCH				UMETA(DisplayName="UsePATCH"),
	PPM_SendViaPOSTwithGZIP		UMETA(DisplayName="SendViaPOSTwithGZIP"),
	PPM_UsePATCHwithGZIP		UMETA(DisplayName="UsePATCHwithGZIP"),
	PPM_UseDELETE				UMETA(DisplayName="UseDELETE")
};

UENUM(BlueprintType)
enum class EPubnubTribool : uint8
{
	PT_False		UMETA(DisplayName="False"),
	PT_True			UMETA(DisplayName="True"),
	PT_NotSet		UMETA(DisplayName="NotSet"),
};

UENUM(BlueprintType)
enum class EPubnubErrorType : uint8
{
	PET_Error		UMETA(DisplayName="Error"),
	PET_Warning		UMETA(DisplayName="Warning")
};

UENUM(BlueprintType)
enum class EPubnubMessageType : uint8
{
	/* Indicates that message was received as a signal */ 
	PMT_Signal,
	/* Indicates that message was published */ 
	PMT_Published,
	/* Indicates action on published message */
	PMT_Action,
	/* Message about Objects */
	PMT_Objects,
	/* Message about Files */
	PMT_Files,
};