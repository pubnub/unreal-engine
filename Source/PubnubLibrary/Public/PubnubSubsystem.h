// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubNub.h"
#include "PubnubStructLibrary.h"
#include "PubnubEnumLibrary.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PubnubSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PubnubLog, Log, All);

class FJsonObject;

class UPubnubSettings;
class FPubnubFunctionThread;
class UPubnubChatSystem;


struct CCoreSubscriptionData
{
	pubnub_subscribe_message_callback_t Callback;
	pubnub_subscription_t* Subscription;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageReceived, FPubnubMessageData, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPubnubError, FString, ErrorMessage, EPubnubErrorType, ErrorType);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubIntResponse, int, IntValue);

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnListChannelsFromGroupResponse, bool, Error, int, Status, const TArray<FString>&, Channels);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnListUsersSubscribedChannelsResponse, int, Status, FString, Message, const TArray<FString>&, Channels);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnListUsersFromChannelResponse, int, Status, FString, Message, FPubnubListUsersFromChannelWrapper, Data);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnFetchHistoryResponse, bool, Error, int, Status, FString, ErrorMessage, const TArray<FPubnubHistoryMessageData>&, Messages);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnGetAllUserMetadataResponse, int, Status, const TArray<FPubnubUserData>&, UsersData, FString, PageNext, FString, PagePrev);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGetUserMetadataResponse, int, Status, FPubnubUserData, UserData);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnGetAllChannelMetadataResponse, int, Status, const TArray<FPubnubChannelData>&, ChannelsData, FString, PageNext, FString, PagePrev);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGetChannelMetadataResponse, int, Status, FPubnubChannelData, ChannelData);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGetMessageActionsResponse, int, Status, const TArray<FPubnubMessageActionData>&, MessageActions);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAddMessageActionsResponse, FString, MessageActionTimetoken);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnGetMembershipsResponse, int, Status, const TArray<FPubnubGetMembershipsWrapper>&, MembershipsData, FString, PageNext, FString, PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnGetChannelMembersResponse, int, Status, const TArray<FPubnubGetChannelMembersWrapper>&, MembersData, FString, PageNext, FString, PagePrev);


UCLASS()
class PUBNUBLIBRARY_API UPubnubSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FOnMessageReceived OnMessageReceived;

	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FOnPubnubError OnPubnubError;

#pragma region BLUEPRINT EXPOSED

	
	/* BLUEPRINT EXPOSED FUNCTIONS */
	//These functions don't have actual logic, they just call corresponding private functions on Pubnub threads

	/**
	 * Initializes PubNub systems. Needs to be called before starting using any other PubNub features.
	 * Don't call it manually if "InitializeAutomatically" in plugin settings is set to true.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Init")
	void InitPubnub();

	/**
	 * Deinitializes PubNub systems. Call it only if you want to  manually stop all PubNub systems.
	 * It's called automatically when closing the game.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Init")
	void DeinitPubnub();
	
	/**
	 * Sets the user ID for the current session.
	 * 
	 * @param UserID The user ID to set.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Users")
	void SetUserID(FString UserID);

	/**
	 * Gets the current user ID.
	 * 
	 * @return The current user ID.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pubnub|Users")
	FString GetUserID();

	/**
	 * Sets the secret key for the PubNub account. Uses SecretKey provided in plugin settings.
	 * Don't call it manually if "SetSecretKeyAutomatically" in plugin settings is set to true.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Init")
	void SetSecretKey();

	/**
	 * Publishes a message to a specified channel.
	 * 
	 * @param Channel The ID of the channel to publish the message to.
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Publish")
	void PublishMessage(FString Channel, FString Message, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

	/**
	 * Sends a signal to a specified channel.
	 * 
	 * @param Channel The ID of the channel to send the signal to.
	 * @param Message The message to send as the signal. This message can be any data type that can be serialized into JSON.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Publish")
	void Signal(FString Channel, FString Message, FPubnubSignalSettings SignalSettings = FPubnubSignalSettings());

	/**
	 * Subscribes to a specified channel - start listening for messages on that channel.
	 * Use OnMessageReceived Callback to get those messages.
	 * 
	 * @param Channel The ID of the channel to subscribe to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void SubscribeToChannel(FString Channel, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());
	
	/**
	 * Subscribes to a specified channels - start listening for messages on those channels.
	 * Use OnMessageReceived Callback to get those messages.
	 * 
	 * @param Channels The ID of the channels to subscribe to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void SubscribeToChannels(TArray<FString> Channels);

	/**
	 * Subscribes to a specified group - start listening for messages on that group.
	 * Use OnMessageReceived Callback to get those messages.
	 * 
	 * @param GroupName The name of the channel to subscribe to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void SubscribeToGroup(FString GroupName, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());

	/**
	 * Unsubscribes from a specified channel - stop listening for messages on that channel.
	 * 
	 * @param Channel The ID of the channel to unsubscribe from.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void UnsubscribeFromChannel(FString Channel);

	/**
	 * Unsubscribes from a specified group - stop listening for messages on that group.
	 * 
	 * @param GroupName The name of the group to unsubscribe from.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void UnsubscribeFromGroup(FString GroupName);

	/**
	 * Unsubscribes from all subscribed channels and groups - basically stop listening for any messages.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void UnsubscribeFromAll();

	/**
	 * Adds a channel to a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to add to the channel group.
	 * @param ChannelGroup The name of the channel group to add the channel to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups")
	void AddChannelToGroup(FString Channel, FString ChannelGroup);

	/**
	 * Removes a channel from a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to remove from the channel group.
	 * @param ChannelGroup The name of the channel group to remove the channel from.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups")
	void RemoveChannelFromGroup(FString Channel, FString ChannelGroup);

	/**
	 * Lists the channels that belong to a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param ChannelGroup The name of the channel group to list channels from.
	 * @param OnListChannelsResponse The callback function used to handle the result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups")
	void ListChannelsFromGroup(FString ChannelGroup, FOnListChannelsFromGroupResponse OnListChannelsResponse);

	/**
	 * Lists the channels that belong to a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param ChannelGroup The name of the channel group to list channels from.
	 * @param OnListChannelsResponse The callback function used to handle the result in JSON format.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups")
	void ListChannelsFromGroup_JSON(FString ChannelGroup, FOnPubnubResponse OnListChannelsResponse);

	/**
	 * Removes a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param ChannelGroup The name of the channel group to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups")
	void RemoveChannelGroup(FString ChannelGroup);

	/**
	 * Lists the users currently present on a specified channel.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to list users from.
	 * @param ListUsersFromChannelResponse The callback function used to handle the result.
	 * @param ListUsersFromChannelSettings Optional settings for the list users operation. See FPubnubListUsersFromChannelSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void ListUsersFromChannel(FString Channel, FOnListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());

	/**
	 * Lists the users currently present on a specified channel.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to list users from.
	 * @param ListUsersFromChannelResponse The callback function used to handle the result in Json format.
	 * @param ListUsersFromChannelSettings Optional settings for the list users operation. See FPubnubListUsersFromChannelSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void ListUsersFromChannel_JSON(FString Channel, FOnPubnubResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());

	/**
	 * Lists the channels that a specified user is currently subscribed to.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 	
	 * @param UserID The user ID to list subscribed channels for.
	 * @param ListUserSubscribedChannelsResponse The callback function used to handle the result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void ListUserSubscribedChannels(FString UserID, FOnListUsersSubscribedChannelsResponse ListUserSubscribedChannelsResponse);

	/**
	 * Lists the channels that a specified user is currently subscribed to.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 	
	 * @param UserID The user ID to list subscribed channels for.
	 * @param ListUserSubscribedChannelsResponse The callback function used to handle the result in JSON format.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void ListUserSubscribedChannels_JSON(FString UserID, FOnPubnubResponse ListUserSubscribedChannelsResponse);

	/**
	 * Sets the presence state for the current user on a specified channel.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to set the state on.
	 * @param StateJson The JSON string representing the state to set.
	 * @param SetStateSettings Optional settings for the set state operation. See FPubnubSetStateSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void SetState(FString Channel, FString StateJson, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());

	/**
	 * Gets the presence state for a specified user on a specified channel.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to get the state from.
	 * @param ChannelGroup The name of the channel group to get the state from.
	 * @param UserID The user ID to get the state for.
	 * @param OnGetStateResponse The callback function used to handle the result in JSON format.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void GetState(FString Channel, FString ChannelGroup, FString UserID, FOnPubnubResponse OnGetStateResponse);

	/**
	 * This method notifies channels and channel groups about a client's presence.
	 * You can send heartbeats to channels you are not subscribed to.
	 * 
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to send the heartbeat to.
	 * @param ChannelGroup The name of the channel group to send the heartbeat to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void Heartbeat(FString Channel, FString ChannelGroup);

	/**
	 * Requests an access token from the PubNub server with the specified permissions.
	 * Use GrantTokenStructureToJsonString function to easily create correct PermissionObject.
	 * 
	 * @Note Requires the *Access Manager* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param PermissionObject A JSON object representing the desired permissions for the token.
	 * @param OnGrantTokenResponse The callback function used to handle the result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Access Manager")
	void GrantToken(FString PermissionObject, FOnPubnubResponse OnGrantTokenResponse);

	/**
	 * Revokes a previously granted access token.
	 * 
	 * @Note Requires the *Revoke v3 Token* in *Access Manager* section to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Token The access token to revoke.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Access Manager")
	void RevokeToken(FString Token);

	/**
	 * Parses an access token and retrieves information about its permissions.
	 * 
	 * @Note Requires the *Access Manager* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * Permissions are written in bit mask int:
	 * READ = 1
	 * WRITE = 2
	 * MANAGE = 4
	 * DELETE = 8
	 * CREATE = 16
	 * GET = 32
	 * UPDATE = 64
	 * JOIN = 128
	 * 
	 * @param Token The access token to parse.
	 * @param OnParseTokenResponse The callback function used to handle the result in JSON format.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Access Manager")
	void ParseToken(FString Token, FOnPubnubResponse OnParseTokenResponse);

	/**
	 * This method is used by the client devices to update the authentication token granted by the server.
	 * 
	 * @param Token Existing token with embedded permissions.
	 * 
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Access Manager")
	void SetAuthToken(FString Token);
	
	/**
	 * Fetches historical messages from a specified channel using Message Persistence.
	 * 
	 * @Note Requires the *Message Persistence* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The ID of the channel to fetch messages from.
	 * @param OnFetchHistoryResponse The callback function used to handle the result.
	 * @param FetchHistorySettings Optional settings for the fetch history operation. See FPubnubFetchHistorySettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Message Persistence")
	void FetchHistory(FString Channel, FOnFetchHistoryResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());

	/**
	 * Fetches historical messages from a specified channel using Message Persistence.
	 * 
	 * @Note Requires the *Message Persistence* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The ID of the channel to fetch messages from.
	 * @param OnFetchHistoryResponse The callback function used to handle the result in JSON format.
	 * @param FetchHistorySettings Optional settings for the fetch history operation. See FPubnubFetchHistorySettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Message Persistence")
	void FetchHistory_JSON(FString Channel, FOnPubnubResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());
	
	/**
	 * Returns the number of messages published on one or more channels since a given time.
	 * The count returned is the number of messages in history with a Timetoken value greater
	 * than or equal to than the passed value in the Timetoken parameter.
	 * 
	 * @Note Requires the *Message Persistence* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The ID of the channel to count messages for.
	 * @param Timetoken The timetoken to start counting messages from.
	 * @param OnMessageCountsResponse The callback function used to handle the result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Message Persistence")
	void MessageCounts(FString Channel, FString Timetoken, FOnPubnubIntResponse OnMessageCountsResponse);

	/**
	 * Returns a paginated list of User Metadata objects, optionally including the custom data object for each.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param OnGetAllUserMetadataResponse The callback function used to handle the result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction. For example: {name: 'asc'}
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable).
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetAllUserMetadata(FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Returns a paginated list of User Metadata objects, optionally including the custom data object for each.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param OnGetAllUserMetadataResponse The callback function used to handle the result in JSON format.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction. For example: {name: 'asc'}
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable).
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetAllUserMetadata_JSON(FOnPubnubResponse OnGetAllUserMetadataResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Sets metadata for a specified User in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to set metadata.
	 * @param UserMetadataObj A JSON string representing the metadata to set.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void SetUserMetadata(FString User, FString UserMetadataObj, FString Include = "");

	/**
	 * Retrieves metadata for a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to retrieve metadata.
	 * @param OnGetUserMetadataResponse The callback function used to handle the result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void GetUserMetadata(FString User, FOnGetUserMetadataResponse OnGetUserMetadataResponse, FString Include = "");

	/**
	 * Retrieves metadata for a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to retrieve metadata.
	 * @param OnGetUserMetadataResponse The callback function used to handle the result in JSON format.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void GetUserMetadata_JSON(FString User, FOnPubnubResponse OnGetUserMetadataResponse, FString Include = "");

	/**
	 * Removes all metadata associated with a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to remove metadata.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void RemoveUserMetadata(FString User);

	/**
	 * Returns a paginated list of Channel Metadata objects, optionally including the custom data object for each.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param OnGetAllChannelMetadataResponse The callback function used to handle the result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction. For example: {name: 'asc'}
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable).
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetAllChannelMetadata(FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Returns a paginated list of Channel Metadata objects, optionally including the custom data object for each.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param OnGetAllChannelMetadataResponse The callback function used to handle the result in JSON format.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction. For example: {name: 'asc'}
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable).
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetAllChannelMetadata_JSON(FOnPubnubResponse OnGetAllChannelMetadataResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Sets metadata for a specified Channel in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The user ID for whom to set metadata.
	 * @param ChannelMetadataObj A JSON string representing the metadata to set.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void SetChannelMetadata(FString Channel, FString ChannelMetadataObj, FString Include = "");

	/**
	 * Retrieves metadata for a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The user ID for whom to retrieve metadata.
	 * @param OnGetChannelMetadataResponse The callback function used to handle the result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void GetChannelMetadata(FString Channel, FOnGetChannelMetadataResponse OnGetChannelMetadataResponse, FString Include = "");

	/**
	 * Retrieves metadata for a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The user ID for whom to retrieve metadata.
	 * @param OnGetChannelMetadataResponse The callback function used to handle the result in JSON format.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void GetChannelMetadata_JSON(FString Channel, FOnPubnubResponse OnGetChannelMetadataResponse, FString Include = "");

	/**
	 * Removes all metadata associated with a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The user ID for whom to remove metadata.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void RemoveChannelMetadata(FString Channel);

	/**
	 * Retrieves a list of memberships for a specified User in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to retrieve memberships.
	 * @param OnGetMembershipResponse The callback function used to handle the result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction. For example: {name: 'asc'}
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable).
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetMemberships(FString User, FOnGetMembershipsResponse OnGetMembershipResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Retrieves a list of memberships for a specified User in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to retrieve memberships.
	 * @param OnGetMembershipResponse The callback function used to handle the result in JSON format.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction. For example: {name: 'asc'}
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable).
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetMemberships_JSON(FString User, FOnPubnubResponse OnGetMembershipResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Sets memberships for a specified User in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to set memberships.
	 * @param SetObj A JSON string representing the memberships to set.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void SetMemberships(FString User, FString SetObj, FString Include = "");

	/**
	 * Removes memberships for a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to set memberships.
	 * @param RemoveObj A JSON string representing the memberships to remove.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void RemoveMemberships(FString User, FString RemoveObj, FString Include = "");

	/**
	 * Retrieves a list of members for a specified Channel in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The user ID for whom to retrieve memberships.
	 * @param OnGetMembersResponse The callback function used to handle the result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction. For example: {name: 'asc'}
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable).
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetChannelMembers(FString Channel, FOnGetChannelMembersResponse OnGetMembersResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Retrieves a list of members for a specified Channel in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The user ID for whom to retrieve memberships.
	 * @param OnGetMembersResponse The callback function used to handle the result in JSON format.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction. For example: {name: 'asc'}
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable).
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetChannelMembers_JSON(FString Channel, FOnPubnubResponse OnGetMembersResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Adds Users to a specified Channel in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The channel name to add members to.
	 * @param AddObj A JSON string representing the users to add.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void AddChannelMembers(FString Channel, FString AddObj, FString Include = "");

	/**
	 * Sets the members of a specified channel in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The channel name to add members to.
	 * @param SetObj A JSON string representing the users to set as members.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void SetChannelMembers(FString Channel, FString SetObj, FString Include = "");

	/**
	 * Removes users from a specified channel in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The channel name to add members to.
	 * @param RemoveObj A JSON string representing the users to remove.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void RemoveChannelMembers(FString Channel, FString RemoveObj, FString Include = "");

	/**
	 * Adds a message action to a specific message in a channel.
	 * 
	 * @param Channel The ID of the channel.
	 * @param MessageTimetoken The timetoken of the message to add the action to.
	 * @param ActionType The type of action to add.
	 * @param Value The value associated with the action.
	 * @param AddActionResponse The callback function used to handle the result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Message Actions")
	void AddMessageAction(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnAddMessageActionsResponse AddActionResponse);

	/**
	 * Retrieves message actions for a specified channel within a given time range.
	 * 
	 * @param Channel The ID of the channel.
	 * @param Start The starting timetoken for the range.
	 * @param End The ending timetoken for the range.
	 * @param SizeLimit The maximum number of actions to retrieve.
	 * @param OnGetMessageActionsResponse The callback function used to handle the result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Message Actions")
	void GetMessageActions(FString Channel, FString Start, FString End, int SizeLimit, FOnGetMessageActionsResponse OnGetMessageActionsResponse);

	/**
	 * Retrieves message actions for a specified channel within a given time range.
	 * 
	 * @param Channel The ID of the channel.
	 * @param Start The starting timetoken for the range.
	 * @param End The ending timetoken for the range.
	 * @param SizeLimit The maximum number of actions to retrieve.
	 * @param OnGetMessageActionsResponse The callback function used to handle the result in JSON format.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Message Actions")
	void GetMessageActions_JSON(FString Channel, FString Start, FString End, int SizeLimit, FOnPubnubResponse OnGetMessageActionsResponse);

	/**
	 * Removes a specific message action from a message in a channel.
	 * 
	 * @param Channel The ID of the channel.
	 * @param MessageTimetoken The timetoken of the message.
	 * @param ActionTimetoken The timetoken of the action to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Message Actions")
	void RemoveMessageAction(FString Channel, FString MessageTimetoken, FString ActionTimetoken);
	
	//UFUNCTION(BlueprintCallable, Category = "Pubnub|Message Actions")
	//void GetMessageActionsContinue(FOnPubnubResponse OnGetMessageActionsContinueResponse);

#pragma endregion

	/**
	 * Helper function to create correct PermissionObject for GrantToken function.
	 *
	 * @Note For an object type there has to be 1 permission or the same amount of permissions as there are objects.
	 * For example. If there are 3 Channels, there can be either 1 ChannelPermission (it will be given for all those
	 * 3 channels) or 3 ChannelPermission (each channel will receive permission from corresponding permissions index)
	 * 
	 * @param TokenStructure Structure containing all required data to GrantToken.
	 * @param success True if data was provided correctly.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pubnub|Access Manager")
	FString GrantTokenStructureToJsonString(FPubnubGrantTokenStructure TokenStructure, bool &success);

	bool CheckIsFieldEmpty(FString Field, FString FieldName, FString FunctionName);
	
private:
	
	//Thread for quick operations, generally everything except subscribe
	TObjectPtr<FPubnubFunctionThread> QuickActionThread = nullptr;

	//Pubnub context for the most of the pubnub operations
	pubnub_t *ctx_pub = nullptr;
	//Pubnub context for the event engine - subscribe operations
	pubnub_t *ctx_ee = nullptr;
	
	TMap<FString, CCoreSubscriptionData> ChannelSubscriptions;
	TMap<FString, CCoreSubscriptionData> ChannelGroupSubscriptions;
	
	//Returns FString from the pubnub_get response
	FString GetLastResponse(pubnub_t* context);
	
	//Returns FString from the pubnub_get_channel response
	FString GetLastChannelResponse(pubnub_t* context);

#pragma region ERROR FUNCTIONS

public:
	/* ERROR FUNCTIONS */
	//Every Error function prints error to the Log and Broadcasts OnPubnubError delegate
	
	//Default error for most use cases. Internal usage only.
	void PubnubError(FString ErrorMessage, EPubnubErrorType ErrorType = EPubnubErrorType::PET_Error);
private:
	//Error when the response was not OK
	void PubnubResponseError(pubnub_res PubnubResponse, FString ErrorMessage);
	//Error during publishing a message
	void PubnubPublishError();

#pragma endregion

#pragma region PLUGIN SETTINGS
	
	/* PLUGIN SETTINGS */
	
	TObjectPtr<UPubnubSettings> PubnubSettings = nullptr;

	//Containers for keys stored from settings
	static const int PublishKeySize = 42;
	static const int SecretKeySize = 54;
	char PublishKey[PublishKeySize + 1];
	char SubscribeKey[PublishKeySize + 1];
	char SecretKey[SecretKeySize + 1];
	
	void LoadPluginSettings();

#pragma endregion

	/* INITIALIZATION CHECKS */
	
	bool IsInitialized = false;
	bool IsUserIDSet = false;
	bool CheckIsPubnubInitialized();
	bool CheckIsUserIDSet();
	bool CheckQuickActionThreadValidity();

	FString GetUserIDInternal();

#pragma region PRIVATE FUNCTIONS

	/* PRIVATE FUNCTIONS */
	//These functions are called from "BLUEPRINT EXPOSED" functions on PubNub threads. They shouldn't be called directly on Game Thread.
	
	void InitPubnub_priv();
	void SetUserID_priv(FString UserID);
	void PublishMessage_priv(FString Channel, FString Message, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());
	void Signal_priv(FString Channel, FString Message, FPubnubSignalSettings SignalSettings = FPubnubSignalSettings());
	void SubscribeToChannel_priv(FString Channel, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());
	void SubscribeToGroup_priv(FString GroupName, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());
	void UnsubscribeFromChannel_priv(FString Channel);
	void UnsubscribeFromGroup_priv(FString GroupName);
	void UnsubscribeFromAll_priv();
	void AddChannelToGroup_priv(FString Channel, FString ChannelGroup);
	void RemoveChannelFromGroup_priv(FString Channel, FString ChannelGroup);
	FString ListChannelsFromGroup_pn(FString ChannelGroup);
	void ListChannelsFromGroup_JSON_priv(FString ChannelGroup, FOnPubnubResponse OnListChannelsResponse);
	void ListChannelsFromGroup_DATA_priv(FString ChannelGroup, FOnListChannelsFromGroupResponse OnListChannelsResponse);
	void RemoveChannelGroup_priv(FString ChannelGroup);
	FString ListUsersFromChannel_pn(FString Channel, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());
	void ListUsersFromChannel_JSON_priv(FString Channel, FOnPubnubResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());
	void ListUsersFromChannel_DATA_priv(FString Channel, FOnListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());
	FString ListUserSubscribedChannels_pn(FString UserID);
	void ListUserSubscribedChannels_JSON_priv(FString UserID, FOnPubnubResponse ListUserSubscribedChannelsResponse);
	void ListUserSubscribedChannels_DATA_priv(FString UserID, FOnListUsersSubscribedChannelsResponse ListUserSubscribedChannelsResponse);
	void SetState_priv(FString Channel, FString StateJson, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());
	void GetState_priv(FString Channel, FString ChannelGroup, FString UserID, FOnPubnubResponse OnGetStateResponse);
	void Heartbeat_priv(FString Channel, FString ChannelGroup);
	void GrantToken_priv(FString PermissionObject, FOnPubnubResponse OnGrantTokenResponse);
	void RevokeToken_priv(FString Token);
	void ParseToken_priv(FString Token, FOnPubnubResponse OnParseTokenResponse);
	FString FetchHistory_pn(FString Channel, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());
	void FetchHistory_JSON_priv(FString Channel, FOnPubnubResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());
	void FetchHistory_DATA_priv(FString Channel, FOnFetchHistoryResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());
	void MessageCounts_priv(FString Channel, FString Timetoken, FOnPubnubIntResponse OnMessageCountsResponse);
	FString GetAllUserMetadata_pn(FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void GetAllUserMetadata_JSON_priv(FOnPubnubResponse OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void GetAllUserMetadata_DATA_priv(FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetUserMetadata_priv(FString User, FString UserMetadataObj, FString Include);
	FString GetUserMetadata_pn(FString User, FString Include);
	void GetUserMetadata_JSON_priv(FString User, FOnPubnubResponse OnGetUserMetadataResponse, FString Include);
	void GetUserMetadata_DATA_priv(FString User, FOnGetUserMetadataResponse OnGetUserMetadataResponse, FString Include);
	void RemoveUserMetadata_priv(FString User);
	FString GetAllChannelMetadata_pn(FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void GetAllChannelMetadata_JSON_priv(FOnPubnubResponse OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void GetAllChannelMetadata_DATA_priv(FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetChannelMetadata_priv(FString Channel, FString ChannelMetadataObj, FString Include);
	FString GetChannelMetadata_pn(FString Channel, FString Include);
	void GetChannelMetadata_JSON_priv(FString Channel, FOnPubnubResponse OnGetChannelMetadataResponse, FString Include);
	void GetChannelMetadata_DATA_priv(FString Channel, FOnGetChannelMetadataResponse OnGetChannelMetadataResponse, FString Include);
	void RemoveChannelMetadata_priv(FString Channel);
	FString GetMemberships_pn(FString User, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void GetMemberships_JSON_priv(FString User, FOnPubnubResponse OnGetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void GetMemberships_DATA_priv(FString User, FOnGetMembershipsResponse OnGetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetMemberships_priv(FString User, FString SetObj, FString Include);
	void RemoveMemberships_priv(FString User, FString RemoveObj, FString Include);
	FString GetChannelMembers_pn(FString Channel, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void GetChannelMembers_JSON_priv(FString Channel, FOnPubnubResponse OnGetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void GetChannelMembers_DATA_priv(FString Channel, FOnGetChannelMembersResponse OnGetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void AddChannelMembers_priv(FString Channel, FString AddObj, FString Include);
	void SetChannelMembers_priv(FString Channel, FString SetObj, FString Include);
	void RemoveChannelMembers_priv(FString Channel, FString Include, FString RemoveObj);
	void AddMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnAddMessageActionsResponse AddActionResponse);
	void RemoveMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionTimetoken);
	FString GetMessageActions_pn(FString Channel, FString Start, FString End, int SizeLimit);
	void GetMessageActions_JSON_priv(FString Channel, FString Start, FString End, int SizeLimit, FOnPubnubResponse OnGetMessageActionsResponse);
	void GetMessageActions_DATA_priv(FString Channel, FString Start, FString End, int SizeLimit, FOnGetMessageActionsResponse OnGetMessageActionsResponse);
	void GetMessageActionsContinue_priv(FOnPubnubResponse OnGetMessageActionsContinueResponse);

#pragma endregion

	/* STRUCT CONVERTERS */
	
	void PublishUESettingsToPubnubPublishOptions(FPubnubPublishSettings &PublishSettings, pubnub_publish_options &PubnubPublishOptions);
	void HereNowUESettingsToPubnubHereNowOptions(FPubnubListUsersFromChannelSettings &HereNowSettings, pubnub_here_now_options &PubnubHereNowOptions);
	void SetStateUESettingsToPubnubSetStateOptions(FPubnubSetStateSettings &SetStateSettings, pubnub_set_state_options &PubnubSetStateOptions);
	void FetchHistoryUESettingsToPbFetchHistoryOptions(FPubnubFetchHistorySettings &FetchHistorySettings, pubnub_fetch_history_options &PubnubFetchHistoryOptions);
	static FPubnubMessageData UEMessageFromPubnub(pubnub_v2_message PubnubMessage);
	
	/* GRANT TOKEN HELPERS */

	TSharedPtr<FJsonObject> AddChannelPermissionsToJson(TArray<FString> Channels, TArray<FPubnubChannelPermissions> ChannelPermissions);
	TSharedPtr<FJsonObject> AddChannelGroupPermissionsToJson(TArray<FString> ChannelGroups, TArray<FPubnubChannelGroupPermissions> ChannelGroupPermissions);
	TSharedPtr<FJsonObject> AddUserPermissionsToJson(TArray<FString> Users, TArray<FPubnubUserPermissions> UserPermissions);

	//Function that is sent to Pubnub sdk (c-core) to pass sdk logs to Unreal
	static void PubnubSDKLogConverter(enum pubnub_log_level log_level, const char* message) {
		switch (log_level)
		{
		case pubnub_log_level::PUBNUB_LOG_LEVEL_WARNING:
			UE_LOG(PubnubLog, Warning, TEXT("%s"), UTF8_TO_TCHAR(message));
			break;
		case pubnub_log_level::PUBNUB_LOG_LEVEL_ERROR:
			UE_LOG(PubnubLog, Error, TEXT("%s"), UTF8_TO_TCHAR(message));
			break;
		default:
			UE_LOG(PubnubLog, Log, TEXT("%s"), UTF8_TO_TCHAR(message));
			break;
		};
	};
};
