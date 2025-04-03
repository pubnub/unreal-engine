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
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") EPubnubPublishMethod PublishMethod = EPubnubPublishMethod::PPM_SendViaGET;
	//For how many hours message should be kept and available with history API. If 0, server default will be used.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int Ttl = 0;
	/** User-specified message type.
	Important: String limited by **3**-**50** case-sensitive alphanumeric characters with only `-` and `_` special characters allowed.
	*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString CustomMessageType = "";
};

USTRUCT(BlueprintType)
struct FPubnubSignalSettings
{
	GENERATED_BODY()

	/** User-specified message type.
	Important: String limited by **3**-**50** case-sensitive alphanumeric characters with only `-` and `_` special characters allowed.
	*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString CustomMessageType = "";
};

USTRUCT(BlueprintType)
struct FPubnubListUsersFromChannelSettings
{
	GENERATED_BODY()

	//Comma-delimited list of channel group names. If NULL, will not be used.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ChannelGroup = "";
	//If true will not give users associated with occupancy.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool DisableUserID = true;
	//If true (and if DisableUserID is false), will give associated state alongside user info.
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
	/** Include messages' custom type flag.
	Message / signal and file messages may contain user-provided type.
	*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeCustomMessageType = false;
};

USTRUCT(BlueprintType)
struct FPubnubChannelPermissions
{
	GENERATED_BODY()

	//Read permission. Applies to Subscribe, History, and Presence.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Read = false;
	//Write permission. Applies to Publish.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Write = false;
	//Delete permission. Applies to History and App Context.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Delete = false;
	//Get permission. Applies to App Context.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Get = false;
	//Update permission. Applies to App Context.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Update = false;
	//Manage permission. Applies to Channel Groups and App Context.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Manage = false;
	//Join permission. Applies to App Context.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Join = false;
};

USTRUCT(BlueprintType)
struct FPubnubChannelGroupPermissions
{
	GENERATED_BODY()

	//Read permission. Applies to presence and history access for the group.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Read = false;
	//Manage permission. Applies to modifying members of the group.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Manage = false;
};

USTRUCT(BlueprintType)
struct FPubnubUserPermissions
{
	GENERATED_BODY()

	//Delete permission. Allows deletion of user metadata.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Delete = false;
	//Get permission. Allows retrieval of user metadata.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Get = false;
	//Update permission. Allows updating of user metadata.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Update = false;
};

USTRUCT(BlueprintType)
struct FPubnubGrantTokenStructure
{
	GENERATED_BODY()

	//Time-To-Live (TTL) in minutes for the granted token.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int TTLMinutes = 0;
	//The User that is authorized by this grant.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString AuthorizedUser = "";
	//List of channel names included in this grant.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> Channels;
	//Permissions applied to the listed channels. Has to be either 1 or the same amount as Channels.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubChannelPermissions> ChannelPermissions;
	//List of channel group names included in this grant.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> ChannelGroups;
	//Permissions applied to the listed channel groups. Has to be either 1 or the same amount as ChannelGroups.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubChannelGroupPermissions> ChannelGroupPermissions;
	//List of Users included in this grant.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> Users;
	//Permissions applied to the listed Users. Has to be either 1 or the same amount as Users.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubUserPermissions> UserPermissions;
	//List of channel name patterns included in this grant.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> ChannelPatterns;
	//Permissions applied to the listed channel name patterns. Has to be either 1 or the same amount as ChannelPatterns.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubChannelPermissions> ChannelPatternPermissions;
	//List of channel group name patterns included in this grant.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> ChannelGroupPatterns;
	//Permissions applied to the listed channel group name patterns. Has to be either 1 or the same amount as ChannelGroupPatterns.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubChannelGroupPermissions> ChannelGroupPatternPermissions;
	//List of User name patterns included in this grant.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> UserPatterns;
	//Permissions applied to the listed User name patterns. Has to be either 1 or the same amount as UserPatterns.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubUserPermissions> UserPatternPermissions;
};

USTRUCT(BlueprintType)
struct FPubnubListUsersFromChannelWrapper
{
	GENERATED_BODY()

	//The number of users in a given channel.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int Occupancy = 0;
	//A map of user IDs and their respective state.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TMap<FString, FString> UsersState;
};

USTRUCT(BlueprintType)
struct FPubnubMessageActionData
{
	GENERATED_BODY()

	//Message action type.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Type = "";
	//Message action value.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Value = "";
	//User ID of the user who added the action.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString UserID = "";
	//Timetoken indicating when the message action was added.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ActionTimetoken = "";
	//Timetoken indicating when the message the action was added to had been sent.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString MessageTimetoken = "";
};

USTRUCT(BlueprintType)
struct FPubnubHistoryMessageData
{
	GENERATED_BODY()

	//The message text.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Message = "";
	//User ID of the user who sent the message.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString UserID = "";
	//Timetoken indicating when the message was sent.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Timetoken = "";
	//Additional information.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Meta = "";
	//Type of the message. Refer to Message types for more information.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString MessageType = "";
	//User-specified message type.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString CustomMessageType = "";
	//An array of FPubnubMessageActionData structs which are message actions that were added to the historical messages.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubMessageActionData> MessageActions;
};

USTRUCT(BlueprintType)
struct FPubnubUserData
{
	GENERATED_BODY()

	//Unique user identifier.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString UserID = "";
	//Display name for the user.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString UserName = "";
	//User's identifier in an external system.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ExternalID = "";
	//The URL of the user's profile picture.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ProfileUrl = "";
	//The user's email address.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Email = "";
	//JSON object providing custom user data. Only a single level of key-value pairs is allowed. Nested JSON objects or arrays are not supported.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Custom = "";
	//User status. Max. 50 characters.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Status = "";
	//User type. Max. 50 characters.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Type = "";
	//The date when the user's metadata was last updated.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Updated = "";
	//Information on the object's content fingerprint.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ETag = "";
};

USTRUCT(BlueprintType)
struct FPubnubChannelData
{
	GENERATED_BODY()

	//Unique channel identifier.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ChannelID = "";
	//Display name for the channel.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ChannelName = "";
	//Description of the channel.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Description = "";
	//JSON object providing custom channel data. Only a single level of key-value pairs is allowed. Nested JSON objects or arrays are not supported.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Custom = "";
	//Channel status. Max 50 characters.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Status = "";
	//Channel type. Max 50 characters.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Type = "";
	//The date when the channel's metadata was last updated.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Updated = "";
	//Version identifier of the channel's metadata.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ETag = "";
};

USTRUCT(BlueprintType)
struct FPubnubGetMembershipsWrapper
{
	GENERATED_BODY()

	//Contains channel metadata, depending on requested includes in the call
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubChannelData Channel;
	//JSON providing custom data about the membership. Only a single level of key-value pairs is allowed. Nested JSON objects or arrays are not supported.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Custom = "";
	//Status of the membership. Max 50 characters.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Status = "";
	//Type of the membership. Max 50 characters.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Type = "";
	//The date when the channel's membership was last updated.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Updated = "";
	//Version identifier of the membership metadata.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ETag = "";
};

USTRUCT(BlueprintType)
struct FPubnubGetChannelMembersWrapper
{
	GENERATED_BODY()

	//Contains user metadata, depending on requested includes in the call
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubUserData User;
	//JSON providing custom data about the member. Only a single level of key-value pairs is allowed. Nested JSON objects or arrays are not supported.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Custom = "";
	//Status of the membership. Max 50 characters.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Status = "";
	//Type of the membership. Max 50 characters.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Type = "";
	//The date when the membership was last updated.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Updated = "";
	//Version identifier of the membership metadata.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ETag = "";
};

USTRUCT(BlueprintType)
struct FPubnubMessageData
{
	GENERATED_BODY()
	
	/** The message itself */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Message = "";
	/** Channel that message was published to */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Channel = "";
	/** The message information about publisher */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString UserID = "";
	/** The time token of the message - when it was published. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Timetoken = "";
	/** The message metadata, as published */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Metadata = "";
	/** Indicates the message type: a signal, published, or something else */ 
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") EPubnubMessageType MessageType = EPubnubMessageType::PMT_Published;
	/** User-provided message type. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString CustomMessageType = "";
	/** Subscription match or the channel group */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString MatchOrGroup = "";
	/** Region of the message - not interesting in most cases */
	int region = 0;
	/** Message flags */
	int flags = 0;
};


USTRUCT(BlueprintType)
struct FPubnubMembershipInclude
{
	GENERATED_BODY()

	/* Include Membership Custom field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeCustom = false;
	/* Include Membership Status field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeStatus = false;
	/* Include Membership Type field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeType = false;
	/* Include Membership's Channel data field - in forn of FPubnubChannelData */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeChannel = false;
	/* Include Membership's Channel Custom field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeChannelCustom = false;
	/* Include Membership's Channel Status field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeChannelStatus = false;
	/* Include Membership's Channel Type field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeChannelType = false;
	/* Include Total Count */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeTotalCount = false;

	/* Set all Includes to the given bool Value */
	static FPubnubMembershipInclude SetAllToValue(bool Value)
	{
		return FPubnubMembershipInclude(Value, Value, Value, Value, Value, Value, Value, Value);
	}
};

USTRUCT(BlueprintType)
struct FPubnubMemberInclude
{
	GENERATED_BODY()

	/* Include Membership Custom field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeCustom = false;
	/* Include Membership Status field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeStatus = false;
	/* Include Membership Type field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeType = false;
	/* Include Membership's User data field - in form of FPubnubUserData */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeUser = false;
	/* Include Membership's User Custom field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeUserCustom = false;
	/* Include Membership's User Status field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeUserStatus = false;
	/* Include Membership's User Type field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeUserType = false;
	/* Include Total Count */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeTotalCount = false;

	/* Set all Includes to the given bool Value */
	static FPubnubMemberInclude SetAllToValue(bool Value)
	{
		return FPubnubMemberInclude(Value, Value, Value, Value, Value, Value, Value, Value);
	}
};

USTRUCT(BlueprintType)
struct FPubnubMembershipSingleSort
{
	GENERATED_BODY()

	/**Property which will be used to sort Membership objects*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") EPubnubMembershipSortType SortType = EPubnubMembershipSortType::PMST_Status;
	/**Sort order - ascending when false*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool SortOrder = false;
	
};

USTRUCT(BlueprintType)
struct FPubnubMemberSingleSort
{
	GENERATED_BODY()

	/**Property which will be used to sort Membership objects*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") EPubnubMemberSortType SortType = EPubnubMemberSortType::PMST_Status;
	/**Sort order - ascending when false*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool SortOrder = false;
	
};

USTRUCT(BlueprintType)
struct FPubnubMembershipSort
{
	GENERATED_BODY()

	/**Array of sorts for Membership related function. The order matters, sorts will be applied from the first index to the last*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubMembershipSingleSort> MembershipSort;
	
};

USTRUCT(BlueprintType)
struct FPubnubMemberSort
{
	GENERATED_BODY()

	/**Array of sorts for Member related functions. The order matters, sorts will be applied from the first index to the last*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubMemberSingleSort> MemberSort;
	
};