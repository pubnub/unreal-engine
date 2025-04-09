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