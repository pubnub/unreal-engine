// Copyright 2025 PubNub Inc. All Rights Reserved.

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
	/* Message about Files - Files are not supported yet*/
	PMT_Files,
};

UENUM(BlueprintType)
enum class EPubnubListenerType : uint8
{
	PLT_Message					UMETA(DisplayName="Message"),
	PLT_Signal					UMETA(DisplayName="Signal"),
	PLT_MessageAction			UMETA(DisplayName="MessageAction"),
	PLT_Objects					UMETA(DisplayName="Objects"),
	/* Files are not supported yet */
	PLT_Files					UMETA(DisplayName="Files"),
	PLT_All						UMETA(DisplayName="All"),

	Count
};
//Skip All in count (and files until not supported)
ENUM_RANGE_BY_FIRST_AND_LAST(EPubnubListenerType, EPubnubListenerType::PLT_Message, EPubnubListenerType::PLT_Objects);

UENUM(BlueprintType)
enum class EPubnubMembershipSortType : uint8
{
	PMST_ChannelID				UMETA(DisplayName="ChannelID"),
	PMST_ChannelName			UMETA(DisplayName="ChannelName"),
	PMST_ChannelUpdated			UMETA(DisplayName="ChannelUpdated"),
	PMST_ChannelStatus			UMETA(DisplayName="ChannelStatus"),
	PMST_ChannelType			UMETA(DisplayName="ChannelType"),
	PMST_Updated				UMETA(DisplayName="Updated"),
	PMST_Status					UMETA(DisplayName="Status"),
	PMST_Type					UMETA(DisplayName="Type")
};

UENUM(BlueprintType)
enum class EPubnubMemberSortType : uint8
{
	PMeST_UserID				UMETA(DisplayName="UserID"),
	PMeST_UserName				UMETA(DisplayName="UserName"),
	PMeST_UserUpdated			UMETA(DisplayName="UserUpdated"),
	PMeST_UserStatus			UMETA(DisplayName="UserStatus"),
	PMeST_UserType				UMETA(DisplayName="UserType"),
	PMeST_Updated				UMETA(DisplayName="Updated"),
	PMeST_Status				UMETA(DisplayName="Status"),
	PMeST_Type					UMETA(DisplayName="Type")
};

UENUM(BlueprintType)
enum class EPubnubGetAllSortType : uint8
{
	PGAST_ID				UMETA(DisplayName="UserID"),
	PGAST_Name				UMETA(DisplayName="UserName"),
	PGAST_Updated			UMETA(DisplayName="UserUpdated"),
	PGAST_Status			UMETA(DisplayName="UserStatus"),
	PGAST_Type				UMETA(DisplayName="UserType")
};

UENUM(BlueprintType)
enum class EPubnubSubscriptionStatus : uint8
{
	PSS_Connected						UMETA(DisplayName="Connected"),
	PSS_ConnectionError					UMETA(DisplayName="ConnectionError"),
	PSS_DisconnectedUnexpectedly		UMETA(DisplayName="DisconnectedUnexpectedly"),
	PSS_Disconnected					UMETA(DisplayName="Disconnected"),
	PSS_SubscriptionChanged				UMETA(DisplayName="SubscriptionChanged")
};


UENUM(BlueprintType)
enum class EPubnubEntityType : uint8
{
	PEnT_Channel				UMETA(DisplayName="Channel"),
	PEnT_ChannelGroup			UMETA(DisplayName="ChannelGroup"),
	PEnT_ChannelMetadata		UMETA(DisplayName="ChannelMetadata"),
	PEnT_UserMetadata			UMETA(DisplayName="UserMetadata"),
};