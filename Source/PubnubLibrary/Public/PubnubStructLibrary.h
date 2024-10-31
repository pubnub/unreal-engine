// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.generated.h"


USTRUCT(BlueprintType)
struct FPubnubPublishSettings
{
	GENERATED_BODY()

	//If true, the message is stored in history. If false, the message is not stored in history.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool StoreInHistory = true;
	//TODO: CipherKey is deprecated, decide what to do (hide variable or mark is as deprecated)
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString CipherKey = "";
	//If true, the message is replicated, thus will be received by all subscribers. If false, the message is not replicated
	//and will be delivered only to Function event handlers. Setting false here and false on store is referred to as a Fire (instead of a publish).
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Replicate = true;
	//An optional JSON object, used to send additional (meta) data about the message, which can be used for stream filtering.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString MetaData = "";
	//Defines the method by which publish transaction will be performed. Can be HTTP GET or POST. If using POST, content can be GZIP compressed.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") EPubnubPublishMethod PublishMethod = EPubnubPublishMethod::pubnubSendViaGET;
};

USTRUCT(BlueprintType)
struct FPubnubListUsersFromChannelSettings
{
	GENERATED_BODY()

	//Comma-delimited list of channel group names. If NULL, will not be used.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ChannelGroup = "";
	//If true will not give uuids associated with occupancy.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool DisableUUID = true;
	//If true (and if disable_uuds is false), will give associated state alongside uuid info.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool State = false;
};

USTRUCT(BlueprintType)
struct FPubnubSetStateSettings
{
	GENERATED_BODY()

	//The string with the channel name (or comma-delimited list of channel group names) to set state for.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ChannelGroup = "";
	//The user_id of the user for which to set state for. If NULL, the current user_id of the p context will be used.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString UserID = "";
	//If set to true, you can set the state and make a heartbeat call at the same time via the /heartbeat endpoint.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool HeartBeat = false;
};

USTRUCT(BlueprintType)
struct FPubnubHistorySettings
{
	GENERATED_BODY()

	//If false, the returned start and end Timetoken values will be returned as long ints. If true, the start and end Timetoken values will be returned as strings. Default is false.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool StringToken = false;
	//The maximum number of messages to return per response. Has to be between 1 and 100 messages. Default is 100.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int Count = 100;
	//Direction of time traversal. Default is false, which means timeline is traversed newest to oldest.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Reverse = false;
	//If provided (not NULL), lets you select a "start date," in Timetoken format. If not provided, it will default to current time.
	//Page through results by providing a start OR end timetoken. Retrieve a slice of the time line by providing both a start AND end timetoken.
	//Start is exclusive – that is, the first item returned will be the one immediately after the start Timetoken value. Default is NULL.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Start = "";
	//If provided (not NULL), lets you select an "end date," in Timetoken format. If not provided, it will provide up to the number of messages defined in the "count" parameter.
	//Page through results by providing a start OR end timetoken. Retrieve a slice of the time line by providing both a start AND end timetoken.
	//End is exclusive – that is, if a message is associated exactly with the end Timetoken, it will be included in the result. Default is NULL.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString End = "";
	//Pass true to receive a timetoken with each history message. Defaults to false.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeToken = false;
	//Pass true to receive a meta with each history message. Defaults to false.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeMeta = false;
};

USTRUCT(BlueprintType)
struct FPubnubFetchHistorySettings
{
	GENERATED_BODY()
	
	/** The maximum number of messages to return per channel if multiple channels provided. 
	 * Has to be between 1 and 25 messages. Default is 25.
	 * If single channel is provided, maximum 100 messages. Default is 100.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int MaxPerChannel = 0;
	/** Direction of time traversal. Default is false, which means
	* timeline is traversed newest to oldest. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Reverse = false;
	/** If provided (not NULL), lets you select a "start date", in
	 * Timetoken format. If not provided, it will default to current
	 * time. Page through results by providing a start OR end time
	 * token. Retrieve a slice of the time line by providing both a
	 * start AND end time token. Start is "exclusive" – that is, the
	 * first item returned will be the one immediately after the start
	 * Timetoken value. Default is NULL.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Start = "";
	/** If provided (not NULL), lets you select an "end date", in
	 * Timetoken format. If not provided, it will provide up to the
	 * number of messages defined in the "count" parameter. Page
	 * through results by providing a start OR end time
	 * token. Retrieve a slice of the time line by providing both a
	 * start AND end time token. End is "inclusive" – that is, if a
	 * message is associated exactly with the end Timetoken, it will
	 * be included in the result. Default is NULL.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString End = "";
	/** If true to recieve metadata with each history
	 * message if any. If false, no metadata per message. Defaults to
	 * false.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeMeta = false;
	/** If true to recieve message type with each history
	 * message. If false, no message type per message. Defaults to
	 * false.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeMessageType = false;
	/** If true to receive user_id with each history
	 * message. If false, no user_id per message. Defaults to
	 * false.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeUserID = false;
	/** If true to recieve message actions with each history
	 * message. If false, no message actions per message. Defaults to
	 * false.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeMessageActions = false;
};

USTRUCT(BlueprintType)
struct FPubnubChannelPermissions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Read = false;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Write = false;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Delete = false;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Get = false;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Update = false;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Manage = false;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Join = false;
};

USTRUCT(BlueprintType)
struct FPubnubChannelGroupPermissions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Read = false;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Manage = false;
};

USTRUCT(BlueprintType)
struct FPubnubUserPermissions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Delete = false;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Get = false;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Update = false;
};

USTRUCT(BlueprintType)
struct FPubnubGrantTokenStructure
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int TTLMinutes = 0;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString AuthorizedUUID = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> Channels;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubChannelPermissions> ChannelPermissions;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> ChannelGroups;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubChannelGroupPermissions> ChannelGroupPermissions;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> UUIDs;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubUserPermissions> UUIDPermissions;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> ChannelPatterns;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubChannelPermissions> ChannelPatternPermissions;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> ChannelGroupPatterns;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubChannelGroupPermissions> ChannelGroupPatternPermissions;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> UUIDPatterns;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubUserPermissions> UUIDPatternPermissions;
};

USTRUCT(BlueprintType)
struct FPubnubListUsersFromChannelWrapper
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int Occupancy = 0;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TMap<FString, FString> UuidsState;
};

USTRUCT(BlueprintType)
struct FPNMessageAction
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Type = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Value = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString UserID = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ActionTimetoken = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString MessageTimetoken = "";
	
};

USTRUCT(BlueprintType)
struct FPNMessage
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Message = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString UserID = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Timetoken = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Meta = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString MessageType = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPNMessageAction> MessageActions;
};

USTRUCT(BlueprintType)
struct FPubnubUserData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString UserID = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString UserName = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ExternalID = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ProfileUrl = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Email = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Custom = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Status = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Type = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Updated = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ETag = "";
};