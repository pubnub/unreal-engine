// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.generated.h"


USTRUCT(BlueprintType)
struct FPubnubConfig
{
	GENERATED_BODY()

	/** Specifies the Publish Key to be used for publishing messages to a channel. You can get one from the PubNub Admin Portal. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString PublishKey = "demo";
	/** Specifies the Subscribe Key to be used for subscribing to a channel. You can get one from the PubNub Admin Portal. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString SubscribeKey = "demo";
	/** Secret key from Admin Portal. When set, it gives user root permissions for Access Manager.
	 * To use it set SetSecretKeyAutomatically to true or call SetSecretKey function.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString SecretKey = "";
	/** Identify the user or the device that connects to PubNub. Necessary for all PubNub operations. If provided, this UsedID will be set automatically.
	 * If you keep this field empty, use SetUserID before the first PubNub operation.
	 * It's a UTF-8 encoded string of up to 92 alphanumeric characters.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString UserID = "";
	/** If true SecretKey will be set during Initialization phase. Secret key gives user root permissions for Access Manager */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool SetSecretKeyAutomatically = false;
	
};

/**
 * Pages used for pagination result. Use Next to get further chunk of data or Prev to get previous chunk.
 */
USTRUCT(BlueprintType)
struct FPubnubPage
{
	GENERATED_BODY()
	
	/** Use to get next chunk of data in paginated result */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Next = "";
	/** Use to get previous chunk of data in paginated result */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Prev = "";
};

USTRUCT(BlueprintType)
struct FPubnubPublishSettings
{
	GENERATED_BODY()

	//If true, the message is stored in history. If false, the message is not stored in history.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool StoreInHistory = true;
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
struct FPubnubSubscribeSettings
{
	GENERATED_BODY()

	/** Whether presence events should be received or not. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool ReceivePresenceEvents = false;
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
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int Limit = 1000;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int Offset = 0;
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
	 * 17-digit Unix timetoken format. If not provided, it will default to current
	 * time. Page through results by providing a start OR end time
	 * token. Retrieve a slice of the time line by providing both a
	 * start AND end time token. Start is "exclusive" – that is, the
	 * first item returned will be the one immediately after the start
	 * Timetoken value. Default is empty String.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Start = "";
	/** If provided (not NULL), lets you select an "end date", in
	 * 17-digit Unix timetoken format. If not provided, it will provide up to the
	 * number of messages defined in the "count" parameter. Page
	 * through results by providing a start OR end time
	 * token. Retrieve a slice of the time line by providing both a
	 * start AND end time token. End is "inclusive" – that is, if a
	 * message is associated exactly with the end Timetoken, it will
	 * be included in the result. Default is empty String.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString End = "";
	/** If true to recieve metadata with each history
	 * message if any. If false, no metadata per message. Defaults to false.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeMeta = false;
	/** If true to recieve message type with each history
	 * message. If false, no message type per message. Defaults to false.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeMessageType = false;
	/** If true to receive user_id with each history
	 * message. If false, no user_id per message. Defaults to false.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeUserID = false;
	/** If true to recieve message actions with each history
	 * message. If false, no message actions per message. Defaults to false.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeMessageActions = false;
	/** Include messages' custom type flag.
	 * Message / signal and file messages may contain user-provided type.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeCustomMessageType = false;
};

USTRUCT(BlueprintType)
struct FPubnubDeleteMessagesSettings
{
	GENERATED_BODY()
	/**
	 * 17-digit Unix timetoken delimiting the start of time slice (exclusive) to delete messages from.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Start = "";
	/**
	 * 17-digit Unix timetoken delimiting the end of time slice (inclusive) to delete messages to.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString End = "";
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
struct FChannelGrant
{
	GENERATED_BODY()

	/**
	 * The ID of a single Channel if used in the "Channels" field of FPubnubGrantTokenPermissions.
	 * Or a regular expression pattern if used in the "ChannelPatterns" field.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Pubnub")
	FString Channel;

	/**
	 * Permissions to grant for the specified Channel name or pattern.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Pubnub")
	FPubnubChannelPermissions Permissions;
};

USTRUCT(BlueprintType)
struct FChannelGroupGrant
{
	GENERATED_BODY()

	/**
	 * The name of a single Channel Group if used in the "ChannelGroups" field of FPubnubGrantTokenPermissions.
	 * Or a regular expression pattern if used in the "ChannelGroupPatterns" field.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Pubnub")
	FString ChannelGroup;

	/**
	 * Permissions to grant for the specified Channel Group name or pattern.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Pubnub")
	FPubnubChannelGroupPermissions Permissions;
};

USTRUCT(BlueprintType)
struct FUserGrant
{
	GENERATED_BODY()

	/**
	 * The ID of a single User if used in the "Users" field of FPubnubGrantTokenPermissions.
	 * Or a regular expression pattern if used in the "UserPatterns" field.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Pubnub")
	FString User;

	/**
	 * Permissions to grant for the specified User ID or pattern.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Pubnub")
	FPubnubUserPermissions Permissions;
};

USTRUCT(BlueprintType)
struct FPubnubGrantTokenPermissions
{
	GENERATED_BODY()

	/**
	 * A list of exact Channel names and their associated permissions.
	 * These are applied to the "resources.channels" section of the grant token.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Pubnub")
	TArray<FChannelGrant> Channels;

	/**
	 * A list of exact Channel Group names and their associated permissions.
	 * These are applied to the "resources.groups" section of the grant token.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Pubnub")
	TArray<FChannelGroupGrant> ChannelGroups;

	/**
	 * A list of exact User IDs and their associated permissions.
	 * These are applied to the "resources.uuids" section of the grant token.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Pubnub")
	TArray<FUserGrant> Users;

	/**
	 * A list of Channel name patterns (regular expressions) and their associated permissions.
	 * These are applied to the "patterns.channels" section of the grant token.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Pubnub")
	TArray<FChannelGrant> ChannelPatterns;

	/**
	 * A list of Channel Group name patterns (regular expressions) and their associated permissions.
	 * These are applied to the "patterns.groups" section of the grant token.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Pubnub")
	TArray<FChannelGroupGrant> ChannelGroupPatterns;

	/**
	 * A list of User ID patterns (regular expressions) and their associated permissions.
	 * These are applied to the "patterns.uuids" section of the grant token.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Pubnub")
	TArray<FUserGrant> UserPatterns;

	//Helper to check if Permissions struct contains at least one permission
	bool ArePermissionsEmpty() const
	{
		return Channels.IsEmpty() && ChannelGroups.IsEmpty() && Users.IsEmpty() &&
		ChannelPatterns.IsEmpty() && ChannelGroupPatterns.IsEmpty() && UserPatterns.IsEmpty();
	}
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

	//Unique user identifier. Ignored during set operations.
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
	/** The date when the user's metadata was last updated. Filled when receiving UserMetadata from server.
	 * Setting this manually has no effect.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Updated = "";
	/** Information on the object's content fingerprint. Filled when receiving UserMetadata from server.
	 * Setting this manually has no effect.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ETag = "";
};

USTRUCT(BlueprintType)
struct FPubnubChannelData
{
	GENERATED_BODY()

	//Unique channel identifier. Ignored during set operations.
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
	/** The date when the channel's metadata was last updated. Filled when receiving UserMetadata from server.
	 * Setting this manually has no effect.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Updated = "";
	/** Version identifier of the channel's metadata. Filled when receiving UserMetadata from server.
	 * Setting this manually has no effect.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ETag = "";
};

USTRUCT(BlueprintType)
struct FPubnubMembershipData
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
	/** The date when the membership was last updated. Filled when receiving UserMetadata from server.
	 * Setting this manually has no effect.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Updated = "";
	/** Version identifier of the membership metadata. Filled when receiving UserMetadata from server.
	 * Setting this manually has no effect.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ETag = "";
};

USTRUCT(BlueprintType)
struct FPubnubChannelMemberData
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
	/** The date when the membership was last updated. Filled when receiving UserMetadata from server.
	 * Setting this manually has no effect.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Updated = "";
	/** Version identifier of the membership metadata. Filled when receiving UserMetadata from server.
	 * Setting this manually has no effect.
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ETag = "";
};

USTRUCT(BlueprintType)
struct FPubnubUserInputData
{
	GENERATED_BODY()
	
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
	
	/** If true, the UserName field will be included in JSON even if empty (as null). If false, empty UserName field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddUserName = false;
	/** If true, the ExternalID field will be included in JSON even if empty (as null). If false, empty ExternalID field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddExternalID = false;
	/** If true, the ProfileUrl field will be included in JSON even if empty (as null). If false, empty ProfileUrl field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddProfileUrl = false;
	/** If true, the Email field will be included in JSON even if empty (as null). If false, empty Email field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddEmail = false;
	/** If true, the Custom field will be included in JSON even if empty (as null). If false, empty Custom field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddCustom = false;
	/** If true, the Status field will be included in JSON even if empty (as null). If false, empty Status field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddStatus = false;
	/** If true, the Type field will be included in JSON even if empty (as null). If false, empty Type field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddType = false;

	/**
	 * Sets all ForceAdd flags to true. Useful for full replacement scenarios where you want to explicitly
	 * include all fields in the JSON (empty fields will be set to null).
	 */
	void ForceAddAllFields()
	{
		ForceAddUserName = true;
		ForceAddExternalID = true;
		ForceAddProfileUrl = true;
		ForceAddEmail = true;
		ForceAddCustom = true;
		ForceAddStatus = true;
		ForceAddType = true;
	}

	/**
	 * Utility converter from FPubnubUserData. Copies all related fields from UserData to FPubnubUserInputData.
	 */
	static FPubnubUserInputData FromPubnubUserData(const FPubnubUserData& UserData)
	{
		return FPubnubUserInputData({.UserName = UserData.UserName, .ExternalID = UserData.ExternalID, .ProfileUrl = UserData.ProfileUrl, .Email = UserData.Email, .Custom = UserData.Custom, .Status = UserData.Status, .Type = UserData.Type});
	}
};

USTRUCT(BlueprintType)
struct FPubnubChannelInputData
{
	GENERATED_BODY()
	
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
	
	/** If true, the ChannelName field will be included in JSON even if empty (as null). If false, empty ChannelName field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddChannelName = false;
	/** If true, the Description field will be included in JSON even if empty (as null). If false, empty Description field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddDescription = false;
	/** If true, the Custom field will be included in JSON even if empty (as null). If false, empty Custom field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddCustom = false;
	/** If true, the Status field will be included in JSON even if empty (as null). If false, empty Status field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddStatus = false;
	/** If true, the Type field will be included in JSON even if empty (as null). If false, empty Type field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddType = false;

	/**
	 * Sets all ForceAdd flags to true. Useful for full replacement scenarios where you want to explicitly
	 * include all fields in the JSON (empty fields will be set to null).
	 */
	void ForceAddAllFields()
	{
		ForceAddChannelName = true;
		ForceAddDescription = true;
		ForceAddCustom = true;
		ForceAddStatus = true;
		ForceAddType = true;
	}

	/**
	 * Utility converter from FPubnubChannelData. Copies all related fields from ChannelData to FPubnubChannelInputData.
	 */
	static FPubnubChannelInputData FromPubnubChannelData(const FPubnubChannelData& ChannelData)
	{
		return FPubnubChannelInputData({.ChannelName = ChannelData.ChannelName, .Description = ChannelData.Description, .Custom = ChannelData.Custom, .Status = ChannelData.Status, .Type = ChannelData.Type});
	}
};

USTRUCT(BlueprintType)
struct FPubnubMembershipInputData
{
	GENERATED_BODY()

	//ID of the Channel associated with this membership 
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Channel;
	//JSON providing custom data about the membership. Only a single level of key-value pairs is allowed. Nested JSON objects or arrays are not supported.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Custom = "";
	//Status of the membership. Max 50 characters.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Status = "";
	//Type of the membership. Max 50 characters.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Type = "";
	
	/** If true, the Custom field will be included in JSON even if empty (as null). If false, empty Custom field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddCustom = false;
	/** If true, the Status field will be included in JSON even if empty (as null). If false, empty Status field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddStatus = false;
	/** If true, the Type field will be included in JSON even if empty (as null). If false, empty Type field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddType = false;

	/**
	 * Sets all ForceAdd flags to true. Useful for full replacement scenarios where you want to explicitly
	 * include all fields in the JSON (empty fields will be set to null).
	 */
	void ForceAddAllFields()
	{
		ForceAddCustom = true;
		ForceAddStatus = true;
		ForceAddType = true;
	}

	/**
	 * Utility converter from FPubnubMembershipData. Copies all related fields from MembershipData to FPubnubMembershipInputData.
	 */
	static FPubnubMembershipInputData FromPubnubMembershipData(const FPubnubMembershipData& MembershipData)
	{
		return FPubnubMembershipInputData({.Channel = MembershipData.Channel.ChannelID, .Custom = MembershipData.Custom, .Status = MembershipData.Status, .Type = MembershipData.Type});
	}
};

USTRUCT(BlueprintType)
struct FPubnubChannelMemberInputData
{
	GENERATED_BODY()

	//ID of the User associated with this membership 
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString User;
	//JSON providing custom data about the member. Only a single level of key-value pairs is allowed. Nested JSON objects or arrays are not supported.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Custom = "";
	//Status of the membership. Max 50 characters.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Status = "";
	//Type of the membership. Max 50 characters.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Type = "";
	
	/** If true, the Custom field will be included in JSON even if empty (as null). If false, empty Custom field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddCustom = false;
	/** If true, the Status field will be included in JSON even if empty (as null). If false, empty Status field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddStatus = false;
	/** If true, the Type field will be included in JSON even if empty (as null). If false, empty Type field is omitted in the request. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, AdvancedDisplay, Category = "Pubnub") bool ForceAddType = false;

	/**
	 * Sets all ForceAdd flags to true. Useful for full replacement scenarios where you want to explicitly
	 * include all fields in the JSON (empty fields will be set to null).
	 */
	void ForceAddAllFields()
	{
		ForceAddCustom = true;
		ForceAddStatus = true;
		ForceAddType = true;
	}
	
	/**
	 * Utility converter from FPubnubChannelMemberData. Copies all related fields from ChannelMemberData to FPubnubChannelMemberInputData.
	 */
	static FPubnubChannelMemberInputData FromPubnubMembershipData(const FPubnubChannelMemberData& ChannelMemberData)
	{
		return FPubnubChannelMemberInputData({.User = ChannelMemberData.User.UserID, .Custom = ChannelMemberData.Custom, .Status = ChannelMemberData.Status, .Type = ChannelMemberData.Type});
	}
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
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int region = 0;
	/** Message flags */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int flags = 0;
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
	static FPubnubMembershipInclude FromValue(bool Value)
	{
		return FPubnubMembershipInclude{Value, Value, Value, Value, Value, Value, Value, Value};
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
	static FPubnubMemberInclude FromValue(bool Value)
	{
		return FPubnubMemberInclude{Value, Value, Value, Value, Value, Value, Value, Value};
	}
};

USTRUCT(BlueprintType)
struct FPubnubGetAllInclude
{
	GENERATED_BODY()

	/* Include Object Custom field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeCustom = false;
	/* Include Object Status field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeStatus = false;
	/* Include Object Type field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeType = false;
	/* Include Total Count */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeTotalCount = false;
	
	/* Set all Includes to the given bool Value */
	static FPubnubGetAllInclude FromValue(bool Value)
	{
		return FPubnubGetAllInclude{Value, Value, Value, Value};
	}
};

USTRUCT(BlueprintType)
struct FPubnubGetMetadataInclude
{
	GENERATED_BODY()

	/* Include Object Custom field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeCustom = false;
	/* Include Object Status field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeStatus = false;
	/* Include Object Type field */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool IncludeType = false;

	/* Set all Includes to the given bool Value */
	static FPubnubGetMetadataInclude FromValue(bool Value)
	{
		return FPubnubGetMetadataInclude{Value, Value, Value};
	}
};

USTRUCT(BlueprintType)
struct FPubnubMembershipSingleSort
{
	GENERATED_BODY()

	/**Property which will be used to sort Membership objects*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") EPubnubMembershipSortType SortType = EPubnubMembershipSortType::PMST_ChannelID;
	/**Sort order - ascending when false*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool SortOrder = false;
	
};

USTRUCT(BlueprintType)
struct FPubnubMemberSingleSort
{
	GENERATED_BODY()

	/**Property which will be used to sort Membership objects*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") EPubnubMemberSortType SortType = EPubnubMemberSortType::PMeST_UserID;
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

USTRUCT(BlueprintType)
struct FPubnubGetAllSingleSort
{
	GENERATED_BODY()

	/**Property which will be used to sort User or Channel objects from Get All function*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") EPubnubGetAllSortType SortType = EPubnubGetAllSortType::PGAST_ID;
	/**Sort order - ascending when false*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool SortOrder = false;
	
};

USTRUCT(BlueprintType)
struct FPubnubGetAllSort
{
	GENERATED_BODY()

	/**Array of sorts for Membership related function. The order matters, sorts will be applied from the first index to the last*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubGetAllSingleSort> GetAllSort;
	
};

USTRUCT(BlueprintType)
struct FPubnubSubscriptionStatusData
{
	GENERATED_BODY()

	/**Error details in case of `PSS_ConnectionError` and `PSS_DisconnectedUnexpectedly` may contain additional information about reasons of failure.*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Reason = "";
	/** All currently subscribed channels */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> Channels;
	/** All currently subscribed channel groups */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> ChannelGroups;
	
};

USTRUCT(BlueprintType)
struct FPubnubOperationResult
{
	GENERATED_BODY()

	/**Status of the operation. 200 if operation succeeded */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int Status = 0;
	/**General check whether operation succeeded */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") bool Error = false;
	/**In case of error should contain useful information about the error */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString ErrorMessage = "";
};

USTRUCT(BlueprintType)
struct FPubnubEncryptedData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString EncryptedData = "";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Metadata = "";
};

USTRUCT(BlueprintType)
struct FPubnubSubscriptionCursor
{
	GENERATED_BODY()

	/**Time from which messages should be retrieved */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Timetoken = "";
	/**Region of the messages */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int Region = 0;
};

USTRUCT(BlueprintType)
struct FPubnubUserMetadataResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** The user metadata (returned by Get/Set operations) */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubUserData UserData;
};

USTRUCT(BlueprintType)
struct FPubnubPublishMessageResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** The published message data including timetoken and channel information */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubMessageData PublishedMessage;
};

USTRUCT(BlueprintType)
struct FPubnubSignalResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** The signal message data including timetoken and channel information */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubMessageData SignalMessage;
};

USTRUCT(BlueprintType)
struct FPubnubListChannelsFromGroupResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** Array of channel names that belong to the channel group */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> Channels;
};

USTRUCT(BlueprintType)
struct FPubnubListUsersFromChannelResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** Occupancy and user state information for the channel */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubListUsersFromChannelWrapper Data;
};

USTRUCT(BlueprintType)
struct FPubnubListUsersSubscribedChannelsResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** Array of channel names that the user is currently subscribed to */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FString> Channels;
};

USTRUCT(BlueprintType)
struct FPubnubGetStateResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** The state data as JSON string */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString StateResponse;
};

USTRUCT(BlueprintType)
struct FPubnubGrantTokenResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** The generated access token string */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FString Token;
};

USTRUCT(BlueprintType)
struct FPubnubFetchHistoryResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** Array of historical messages retrieved from the channel */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubHistoryMessageData> Messages;
};

USTRUCT(BlueprintType)
struct FPubnubMessageCountsResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** The number of messages in the specified time range */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int MessageCounts;
};

USTRUCT(BlueprintType)
struct FPubnubGetAllUserMetadataResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** Array of user metadata objects retrieved */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubUserData> UsersData;
	/** Pagination information for navigating through results */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubPage Page;
	/** Total count of users matching the query (if requested) */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int TotalCount = 0;
};

USTRUCT(BlueprintType)
struct FPubnubGetAllChannelMetadataResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** Array of channel metadata objects retrieved */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubChannelData> ChannelsData;
	/** Pagination information for navigating through results */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubPage Page;
	/** Total count of channels matching the query (if requested) */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int TotalCount = 0;
};

USTRUCT(BlueprintType)
struct FPubnubChannelMetadataResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** The channel metadata (returned by Get/Set operations) */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubChannelData ChannelData;
};

USTRUCT(BlueprintType)
struct FPubnubMembershipsResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** Array of membership data (returned by Get/Set/Remove operations) */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubMembershipData> MembershipsData;
	/** Pagination information for navigating through results */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubPage Page;
	/** Total count of memberships matching the query (if requested) */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int TotalCount = 0;
};

USTRUCT(BlueprintType)
struct FPubnubChannelMembersResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** Array of member data (returned by Get/Set/Remove operations) */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubChannelMemberData> MembersData;
	/** Pagination information for navigating through results */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubPage Page;
	/** Total count of channel members matching the query (if requested) */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") int TotalCount = 0;
};

USTRUCT(BlueprintType)
struct FPubnubGetMessageActionsResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** Array of message actions retrieved */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") TArray<FPubnubMessageActionData> MessageActions;
};

USTRUCT(BlueprintType)
struct FPubnubAddMessageActionResult
{
	GENERATED_BODY()
	
	/** Status and error information for this operation */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubOperationResult Result;
	/** The added message action data including timetokens */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Pubnub") FPubnubMessageActionData MessageActionData;
};