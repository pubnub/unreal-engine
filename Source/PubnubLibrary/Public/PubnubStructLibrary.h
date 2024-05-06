#pragma once

#include "CoreMinimal.h"
#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.generated.h"


USTRUCT(BlueprintType)
struct FPubnubPublishSettings
{
	GENERATED_BODY()

	//If true, the message is stored in history. If false, the message is not stored in history.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool StoreInHistory = true;
	//TODO: CipherKey is deprecated, decide what to do (hide variable or mark is as deprecated)
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString CipherKey = "";
	//If true, the message is replicated, thus will be received by all subscribers. If false, the message is not replicated
	//and will be delivered only to Function event handlers. Setting false here and false on store is referred to as a Fire (instead of a publish).
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool Replicate = true;
	//An optional JSON object, used to send additional (meta) data about the message, which can be used for stream filtering.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString MetaData = "";
	//Defines the method by which publish transaction will be performed. Can be HTTP GET or POST. If using POST, content can be GZIP compressed.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) EPubnubPublishMethod PublishMethod = EPubnubPublishMethod::pubnubSendViaGET;
};

USTRUCT(BlueprintType)
struct FPubnubListUsersFromChannelSettings
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

USTRUCT(BlueprintType)
struct FPubnubHistorySettings
{
	GENERATED_BODY()

	//If false, the returned start and end Timetoken values will be returned as long ints. If true, the start and end Timetoken values will be returned as strings. Default is false.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool StringToken = false;
	//The maximum number of messages to return per response. Has to be between 1 and 100 messages. Default is 100.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) int Count = 100;
	//Direction of time traversal. Default is false, which means timeline is traversed newest to oldest.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool Reverse = false;
	//If provided (not NULL), lets you select a "start date," in Timetoken format. If not provided, it will default to current time.
	//Page through results by providing a start OR end timetoken. Retrieve a slice of the time line by providing both a start AND end timetoken.
	//Start is exclusive – that is, the first item returned will be the one immediately after the start Timetoken value. Default is NULL.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString Start = "";
	//If provided (not NULL), lets you select an "end date," in Timetoken format. If not provided, it will provide up to the number of messages defined in the "count" parameter.
	//Page through results by providing a start OR end timetoken. Retrieve a slice of the time line by providing both a start AND end timetoken.
	//End is exclusive – that is, if a message is associated exactly with the end Timetoken, it will be included in the result. Default is NULL.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString End = "";
	//Pass true to receive a timetoken with each history message. Defaults to false.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool IncludeToken = false;
	//Pass true to receive a meta with each history message. Defaults to false.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool IncludeMeta = false;
};

USTRUCT(BlueprintType)
struct FPubnubFetchHistorySettings
{
	GENERATED_BODY()
	
	/** The maximum number of messages to return per channel if multiple channels provided. 
	 * Has to be between 1 and 25 messages. Default is 25.
	 * If single channel is provided, maximum 100 messages. Default is 100.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) int MaxPerChannel = 0;
	/** Direction of time traversal. Default is false, which means
	* timeline is traversed newest to oldest. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool Reverse = false;
	/** If provided (not NULL), lets you select a "start date", in
	 * Timetoken format. If not provided, it will default to current
	 * time. Page through results by providing a start OR end time
	 * token. Retrieve a slice of the time line by providing both a
	 * start AND end time token. Start is "exclusive" – that is, the
	 * first item returned will be the one immediately after the start
	 * Timetoken value. Default is NULL.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString Start = "";
	/** If provided (not NULL), lets you select an "end date", in
	 * Timetoken format. If not provided, it will provide up to the
	 * number of messages defined in the "count" parameter. Page
	 * through results by providing a start OR end time
	 * token. Retrieve a slice of the time line by providing both a
	 * start AND end time token. End is "inclusive" – that is, if a
	 * message is associated exactly with the end Timetoken, it will
	 * be included in the result. Default is NULL.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString End = "";
	/** If true to recieve metadata with each history
	 * message if any. If false, no metadata per message. Defaults to
	 * false.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool IncludeMeta = false;
	/** If true to recieve message type with each history
	 * message. If false, no message type per message. Defaults to
	 * false.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool IncludeMessageType = false;
	/** If true to receive user_id with each history
	 * message. If false, no user_id per message. Defaults to
	 * false.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool IncludeUserID = false;
	/** If true to recieve message actions with each history
	 * message. If false, no message actions per message. Defaults to
	 * false.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) bool IncludeMessageActions = false;
};