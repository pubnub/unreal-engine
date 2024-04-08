#pragma once

#include "CoreMinimal.h"
#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.generated.h"


USTRUCT(BlueprintType)
struct FPubnubPublishSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool StoreInHistory = true;
	//TODO: CipherKey is deprecated, decide what to do (hide variable or mark is as deprecated)
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString CipherKey = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool Replicate = true;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString MetaData = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) EPubnubPublishMethod PublishMethod = EPubnubPublishMethod::pubnubSendViaGET;
};

USTRUCT(BlueprintType)
struct FPubnubHereNowSettings
{
	GENERATED_BODY()

	//Comma-delimited list of channel group names. If NULL, will not be used.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString ChannelGroup = "";
	//If true will not give uuids associated with occupancy.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool DisableUUID = true;
	//If true (and if disable_uuds is false), will give associated state alongside uuid info.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool State = false;
};

USTRUCT(BlueprintType)
struct FPubnubSetStateSettings
{
	GENERATED_BODY()

	//The string with the channel name (or comma-delimited list of channel group names) to set state for.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString ChannelGroup = "";
	//The user_id of the user for which to set state for. If NULL, the current user_id of the p context will be used.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString UserID = "";
	//If set to true, you can set the state and make a heartbeat call at the same time via the /heartbeat endpoint.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool HeartBeat = false;
};