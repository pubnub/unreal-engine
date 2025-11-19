// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PubnubStructLibrary.h"
#include "PubnubEnumLibrary.h"
#include "UObject/Object.h"
#include "PubnubClient.generated.h"

class UPubnubSubsystem;
class UPubnubCryptoBridge;
class FPubnubFunctionThread;
struct CCoreSubscriptionCallback;

struct pubnub_;
typedef struct pubnub_ pubnub_t;
enum pubnub_res;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPubnubOnClientDeinitialized, int, ClientID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPubnubOnMessageReceived, FPubnubMessageData, Message);
DECLARE_MULTICAST_DELEGATE_OneParam(FPubnubOnMessageReceivedNative, const FPubnubMessageData& Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPubnubOnError, FString, ErrorMessage, EPubnubErrorType, ErrorType);
DECLARE_MULTICAST_DELEGATE_TwoParams(FPubnubOnErrorNative, FString ErrorMessage, EPubnubErrorType ErrorType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPubnubOnSubscriptionStatusChanged, EPubnubSubscriptionStatus, Status, FPubnubSubscriptionStatusData, StatusData);
DECLARE_MULTICAST_DELEGATE_TwoParams(FPubnubOnSubscriptionStatusChangedNative, EPubnubSubscriptionStatus Status, const FPubnubSubscriptionStatusData& StatusData);


DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnPublishMessageResponse, FPubnubOperationResult, Result, FPubnubMessageData, PublishedMessage);
DECLARE_DELEGATE_TwoParams(FPubnubOnPublishMessageResponseNative, const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnSignalResponse, FPubnubOperationResult, Result, FPubnubMessageData, SignalMessage);
DECLARE_DELEGATE_TwoParams(FPubnubOnSignalResponseNative, const FPubnubOperationResult& Result, const FPubnubMessageData& SignalMessage);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPubnubOnSubscribeOperationResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FPubnubOnSubscribeOperationResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPubnubOnAddChannelToGroupResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FPubnubOnAddChannelToGroupResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPubnubOnRemoveChannelFromGroupResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FPubnubOnRemoveChannelFromGroupResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnListChannelsFromGroupResponse, FPubnubOperationResult, Result, const TArray<FString>&, Channels);
DECLARE_DELEGATE_TwoParams(FPubnubOnListChannelsFromGroupResponseNative, const FPubnubOperationResult& Result, const TArray<FString>& Channels);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPubnubOnRemoveChannelGroupResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FPubnubOnRemoveChannelGroupResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnListUsersSubscribedChannelsResponse, FPubnubOperationResult, Result, const TArray<FString>&, Channels);
DECLARE_DELEGATE_TwoParams(FPubnubOnListUsersSubscribedChannelsResponseNative, const FPubnubOperationResult& Result, const TArray<FString>& Channels);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnListUsersFromChannelResponse, FPubnubOperationResult, Result, FPubnubListUsersFromChannelWrapper, Data);
DECLARE_DELEGATE_TwoParams(FPubnubOnListUsersFromChannelResponseNative, const FPubnubOperationResult& Result, const FPubnubListUsersFromChannelWrapper& Data);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPubnubOnSetStateResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FPubnubOnSetStateResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnGetStateResponse, FPubnubOperationResult, Result, FString, StateResponse);
DECLARE_DELEGATE_TwoParams(FPubnubOnGetStateResponseNative, const FPubnubOperationResult& Result, FString StateResponse);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnGrantTokenResponse, FPubnubOperationResult, Result, FString, Token);
DECLARE_DELEGATE_TwoParams(FPubnubOnGrantTokenResponseNative, const FPubnubOperationResult& Result, FString Token);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPubnubOnRevokeTokenResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FPubnubOnRevokeTokenResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnFetchHistoryResponse, FPubnubOperationResult, Result, const TArray<FPubnubHistoryMessageData>&, Messages);
DECLARE_DELEGATE_TwoParams(FPubnubOnFetchHistoryResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubHistoryMessageData>& Messages);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnMessageCountsResponse, FPubnubOperationResult, Result, int, MessageCounts);
DECLARE_DELEGATE_TwoParams(FPubnubOnMessageCountsResponseNative, const FPubnubOperationResult& Result, int MessageCounts);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPubnubOnDeleteMessagesResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FPubnubOnDeleteMessagesResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_FourParams(FPubnubOnGetAllUserMetadataResponse, FPubnubOperationResult, Result, const TArray<FPubnubUserData>&, UsersData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FPubnubOnGetAllUserMetadataResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubUserData>& UsersData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnGetUserMetadataResponse, FPubnubOperationResult, Result, FPubnubUserData, UserData);
DECLARE_DELEGATE_TwoParams(FPubnubOnGetUserMetadataResponseNative, const FPubnubOperationResult& Result, const FPubnubUserData& UserData);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnSetUserMetadataResponse, FPubnubOperationResult, Result, FPubnubUserData, UserData);
DECLARE_DELEGATE_TwoParams(FPubnubOnSetUserMetadataResponseNative, const FPubnubOperationResult& Result, const FPubnubUserData& UserData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPubnubOnRemoveUserMetadataResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FPubnubOnRemoveUserMetadataResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_FourParams(FPubnubOnGetAllChannelMetadataResponse, FPubnubOperationResult, Result, const TArray<FPubnubChannelData>&, ChannelsData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FPubnubOnGetAllChannelMetadataResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubChannelData>& ChannelsData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnGetChannelMetadataResponse, FPubnubOperationResult, Result, FPubnubChannelData, ChannelData);
DECLARE_DELEGATE_TwoParams(FPubnubOnGetChannelMetadataResponseNative, const FPubnubOperationResult& Result, const FPubnubChannelData& ChannelData);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnSetChannelMetadataResponse, FPubnubOperationResult, Result, FPubnubChannelData, ChannelData);
DECLARE_DELEGATE_TwoParams(FPubnubOnSetChannelMetadataResponseNative, const FPubnubOperationResult& Result, const FPubnubChannelData& ChannelData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPubnubOnRemoveChannelMetadataResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FPubnubOnRemoveChannelMetadataResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_FourParams(FPubnubOnGetMembershipsResponse, FPubnubOperationResult, Result, const TArray<FPubnubMembershipData>&, MembershipsData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FPubnubOnGetMembershipsResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FPubnubOnSetMembershipsResponse, FPubnubOperationResult, Result, const TArray<FPubnubMembershipData>&, MembershipsData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FPubnubOnSetMembershipsResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FPubnubOnRemoveMembershipsResponse, FPubnubOperationResult, Result, const TArray<FPubnubMembershipData>&, MembershipsData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FPubnubOnRemoveMembershipsResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FPubnubOnGetChannelMembersResponse, FPubnubOperationResult, Result, const TArray<FPubnubChannelMemberData>&, MembersData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FPubnubOnGetChannelMembersResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FPubnubOnSetChannelMembersResponse, FPubnubOperationResult, Result, const TArray<FPubnubChannelMemberData>&, MembersData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FPubnubOnSetChannelMembersResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FPubnubOnRemoveChannelMembersResponse, FPubnubOperationResult, Result, const TArray<FPubnubChannelMemberData>&, MembersData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FPubnubOnRemoveChannelMembersResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnGetMessageActionsResponse, FPubnubOperationResult, Result, const TArray<FPubnubMessageActionData>&, MessageActions);
DECLARE_DELEGATE_TwoParams(FPubnubOnGetMessageActionsResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubMessageActionData>& MessageActions);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPubnubOnAddMessageActionResponse, FPubnubOperationResult, Result, FPubnubMessageActionData, MessageActionData);
DECLARE_DELEGATE_TwoParams(FPubnubOnAddMessageActionResponseNative, const FPubnubOperationResult& Result, const FPubnubMessageActionData& MessageActionData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPubnubOnRemoveMessageActionResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FPubnubOnRemoveMessageActionResponseNative, const FPubnubOperationResult& Result);

/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubClient : public UObject
{
	GENERATED_BODY()

	friend class UPubnubSubsystem;

public:

	/* PUBLIC DELEGATES */

	/**Delegate that is called when PubnubClient is deinitialized*/
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FPubnubOnClientDeinitialized OnClientDeinitialized;
	
	/**Listener to react for subscription status changed*/
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FPubnubOnSubscriptionStatusChanged OnSubscriptionStatusChanged;

	/**Listener to react for subscription status changed , equivalent that accepts lambdas*/
	FPubnubOnSubscriptionStatusChangedNative OnSubscriptionStatusChangedNative;

	/**Global listener for all messages received on subscribed channels*/
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FPubnubOnMessageReceived OnMessageReceived;

	/**Global listener for all messages received on subscribed channels, equivalent that accepts lambdas*/
	FPubnubOnMessageReceivedNative OnMessageReceivedNative;

	/**Listener to react for all Errors in Pubnub functions */
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FPubnubOnError OnError;
	
	/**Listener to react for all Errors in Pubnub functions, equivalent that accepts lambdas*/
	FPubnubOnErrorNative OnErrorNative;

	
	/* GENERAL FUNCTIONS */

	UFUNCTION(BlueprintPure, BlueprintCallable, Category="PubnubClient")
	int GetClientID() const {return ClientID;};

	UFUNCTION(BlueprintCallable, Category="PubnubClient")
	void DestroyClient();

	/**
	 * Sets the user ID for the current session.
	 * 
	 * @param UserID The user ID to set.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|General")
	void SetUserID(FString UserID);

	/**
	 * Gets the current user ID.
	 * 
	 * @return The current user ID.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pubnub|General")
	FString GetUserID();

	/**
	 * Sets the secret key for the PubNub account. Uses SecretKey provided in plugin settings.
	 * Don't call it manually if "SetSecretKeyAutomatically" in plugin settings is set to true.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|AccessManager")
	void SetSecretKey();

	

	/* PUBSUB API */

	/**
	 * Publishes a message to a specified channel.
	 * 
	 * @param Channel The ID of the channel to publish the message to.
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param OnPublishMessageResponse Optional delegate to listen for the publish result.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Publish", meta = (AutoCreateRefTerm = "OnPublishMessageResponse"))
	void PublishMessage(FString Channel, FString Message, FPubnubOnPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

	/**
	 * Publishes a message to a specified channel.
	 * 
	 * @param Channel The ID of the channel to publish the message to.
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param NativeCallback Optional delegate to listen for the publish result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if publish result is not needed.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	void PublishMessage(FString Channel, FString Message, FPubnubOnPublishMessageResponseNative NativeCallback = nullptr, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

	/**
	 * Publishes a message to a specified channel. Overload without delegate to get publish result.
	 * 
	 * @param Channel The ID of the channel to publish the message to.
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	void PublishMessage(FString Channel, FString Message, FPubnubPublishSettings PublishSettings);

	
	/**
	 * Sends a signal to a specified channel.
	 * 
	 * @param Channel The ID of the channel to send the signal to.
	 * @param Message The message to send as the signal. This message can be any data type that can be serialized into JSON.
	 * @param OnSignalResponse Optional delegate to listen for the signal result.
	 * @param SignalSettings Optional settings for the signal operation. See FPubnubSignalSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Publish", meta = (AutoCreateRefTerm = "OnSignalResponse"))
	void Signal(FString Channel, FString Message, FPubnubOnSignalResponse OnSignalResponse, FPubnubSignalSettings SignalSettings = FPubnubSignalSettings());

	/**
	 * Sends a signal to a specified channel.
	 * 
	 * @param Channel The ID of the channel to send the signal to.
	 * @param Message The message to send as the signal. This message can be any data type that can be serialized into JSON.
	 * @param NativeCallback Optional delegate to listen for the signal result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if signal result is not needed.
	 * @param SignalSettings Optional settings for the signal operation. See FPubnubSignalSettings for more details.
	 */
	void Signal(FString Channel, FString Message, FPubnubOnSignalResponseNative NativeCallback = nullptr, FPubnubSignalSettings SignalSettings = FPubnubSignalSettings());
	
	/**
	 * Sends a signal to a specified channel. Overload without delegate to get signal result.
	 * 
	 * @param Channel The ID of the channel to send the signal to.
	 * @param Message The message to send as the signal. This message can be any data type that can be serialized into JSON.
	 * @param SignalSettings Optional settings for the signal operation. See FPubnubSignalSettings for more details.
	 */
	void Signal(FString Channel, FString Message, FPubnubSignalSettings SignalSettings);

	
	/**
	 * Subscribes to a specified channel - start listening for messages on that channel.
	 * Use OnMessageReceived Callback to get those messages.
	 * 
	 * @param Channel The ID of the channel to subscribe to.
	 * @param OnSubscribeToChannelResponse Optional delegate to listen for the subscribe result.
	 * @param SubscribeSettings Optional settings for the subscribe operation. See FPubnubSubscribeSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe", meta = (AutoCreateRefTerm = "OnSubscribeToChannelResponse"))
	void SubscribeToChannel(FString Channel, FPubnubOnSubscribeOperationResponse OnSubscribeToChannelResponse, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());

	/**
	 * Subscribes to a specified channel - start listening for messages on that channel.
	 * Use OnMessageReceived Callback to get those messages.
	 * 
	 * @param Channel The ID of the channel to subscribe to.
	 * @param NativeCallback Optional delegate to listen for the subscribe result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if subscribe result is not needed.
	 * @param SubscribeSettings Optional settings for the subscribe operation. See FPubnubSubscribeSettings for more details.
	 */
	void SubscribeToChannel(FString Channel, FPubnubOnSubscribeOperationResponseNative NativeCallback = nullptr, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());

	/**
	 * Subscribes to a specified channel - start listening for messages on that channel. Overload without delegate to get subscribe result.
	 * Use OnMessageReceived Callback to get those messages.
	 * 
	 * @param Channel The ID of the channel to subscribe to.
	 * @param SubscribeSettings Optional settings for the subscribe operation. See FPubnubSubscribeSettings for more details.
	 */
	void SubscribeToChannel(FString Channel, FPubnubSubscribeSettings SubscribeSettings);

	
	/**
	 * Subscribes to a specified group - start listening for messages on that group.
	 * Use OnMessageReceived Callback to get those messages.
	 * 
	 * @param ChannelGroup The name of the channel to subscribe to.
	 * @param OnSubscribeToGroupResponse Optional delegate to listen for the subscribe result.
	 * @param SubscribeSettings Optional settings for the subscribe operation. See FPubnubSubscribeSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe", meta = (AutoCreateRefTerm = "OnSubscribeToGroupResponse"))
	void SubscribeToGroup(FString ChannelGroup, FPubnubOnSubscribeOperationResponse OnSubscribeToGroupResponse, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());
	
	/**
	 * Subscribes to a specified group - start listening for messages on that group.
	 * Use OnMessageReceived Callback to get those messages.
	 * 
	 * @param ChannelGroup The name of the channel to subscribe to.
	 * @param NativeCallback Optional delegate to listen for the subscribe result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if subscribe result is not needed.
	 * @param SubscribeSettings Optional settings for the subscribe operation. See FPubnubSubscribeSettings for more details.
	 */
	void SubscribeToGroup(FString ChannelGroup, FPubnubOnSubscribeOperationResponseNative NativeCallback = nullptr, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());

	/**
	 * Subscribes to a specified group - start listening for messages on that group.
	 * Use OnMessageReceived Callback to get those messages.
	 * 
	 * @param ChannelGroup The name of the channel to subscribe to.
	 * @param SubscribeSettings Optional settings for the subscribe operation. See FPubnubSubscribeSettings for more details.
	 */
	void SubscribeToGroup(FString ChannelGroup, FPubnubSubscribeSettings SubscribeSettings);

	
	/**
	 * Unsubscribes from a specified channel - stop listening for messages on that channel.
	 * 
	 * @param Channel The ID of the channel to unsubscribe from.
     * @param OnUnsubscribeFromChannelResponse Optional delegate to listen for the unsubscribe result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe", meta = (AutoCreateRefTerm = "OnUnsubscribeFromChannelResponse"))
	void UnsubscribeFromChannel(FString Channel, FPubnubOnSubscribeOperationResponse OnUnsubscribeFromChannelResponse);

	/**
	 * Unsubscribes from a specified channel - stop listening for messages on that channel.
	 * 
	 * @param Channel The ID of the channel to unsubscribe from.
	 * @param NativeCallback Optional delegate to listen for the unsubscribe result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if unsubscribe result is not needed.
	 */
	void UnsubscribeFromChannel(FString Channel, FPubnubOnSubscribeOperationResponseNative NativeCallback = nullptr);

	
	/**
	 * Unsubscribes from a specified group - stop listening for messages on that group.
	 * 
	 * @param ChannelGroup The name of the group to unsubscribe from.
	 * @param OnUnsubscribeFromGroupResponse Optional delegate to listen for the unsubscribe result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe", meta = (AutoCreateRefTerm = "OnUnsubscribeFromGroupResponse"))
	void UnsubscribeFromGroup(FString ChannelGroup, FPubnubOnSubscribeOperationResponse OnUnsubscribeFromGroupResponse);

	/**
	 * Unsubscribes from a specified group - stop listening for messages on that group.
	 * 
	 * @param ChannelGroup The name of the group to unsubscribe from.
 	 * @param NativeCallback Optional delegate to listen for the unsubscribe result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if unsubscribe result is not needed.
	 */
	void UnsubscribeFromGroup(FString ChannelGroup, FPubnubOnSubscribeOperationResponseNative NativeCallback = nullptr);

	
	/**
	 * Unsubscribes from all subscribed channels and groups - basically stop listening for any messages.
	 * NOTE:: This also unsubscribes all subscribed Subscription and SubscriptionSet Objects.
	 * 
	 * @param OnUnsubscribeFromAllResponse Optional delegate to listen for the unsubscribe result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe", meta = (AutoCreateRefTerm = "OnUnsubscribeFromAllResponse"))
	void UnsubscribeFromAll(FPubnubOnSubscribeOperationResponse OnUnsubscribeFromAllResponse);

	/**
	 * Unsubscribes from all subscribed channels and groups - basically stop listening for any messages.
	 * 
	 * @param NativeCallback Optional delegate to listen for the unsubscribe result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if unsubscribe result is not needed.
	 */
	void UnsubscribeFromAll(FPubnubOnSubscribeOperationResponseNative NativeCallback = nullptr);


	
	/* CHANNEL GROUPS API */
	
	/**
	 * Adds a channel to a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to add to the channel group.
	 * @param ChannelGroup The name of the channel group to add the channel to.
	 * @param OnAddChannelToGroupResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups", meta = (AutoCreateRefTerm = "OnAddChannelToGroupResponse"))
	void AddChannelToGroup(FString Channel, FString ChannelGroup, FPubnubOnAddChannelToGroupResponse OnAddChannelToGroupResponse);

	/**
	 * Adds a channel to a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to add to the channel group.
	 * @param ChannelGroup The name of the channel group to add the channel to.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if operation result is not needed.
	 */
	void AddChannelToGroup(FString Channel, FString ChannelGroup, FPubnubOnAddChannelToGroupResponseNative NativeCallback = nullptr);

	
	/**
	 * Removes a channel from a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to remove from the channel group.
	 * @param ChannelGroup The name of the channel group to remove the channel from.
	 * @param OnRemoveChannelFromGroupResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups", meta = (AutoCreateRefTerm = "OnRemoveChannelFromGroupResponse"))
	void RemoveChannelFromGroup(FString Channel, FString ChannelGroup, FPubnubOnRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse);

	/**
	 * Removes a channel from a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to remove from the channel group.
	 * @param ChannelGroup The name of the channel group to remove the channel from.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if operation result is not needed.
	 */
	void RemoveChannelFromGroup(FString Channel, FString ChannelGroup, FPubnubOnRemoveChannelFromGroupResponseNative NativeCallback = nullptr);

	
	/**
	 * Lists the channels that belong to a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param ChannelGroup The name of the channel group to list channels from.
	 * @param OnListChannelsResponse The callback function used to handle the result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups")
	void ListChannelsFromGroup(FString ChannelGroup, FPubnubOnListChannelsFromGroupResponse OnListChannelsResponse);

	/**
	 * Lists the channels that belong to a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param ChannelGroup The name of the channel group to list channels from.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 */
	void ListChannelsFromGroup(FString ChannelGroup, FPubnubOnListChannelsFromGroupResponseNative NativeCallback);

	
	/**
	 * Removes a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param ChannelGroup The name of the channel group to remove.
	 * @param OnRemoveChannelGroupResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups", meta = (AutoCreateRefTerm = "OnRemoveChannelGroupResponse"))
	void RemoveChannelGroup(FString ChannelGroup, FPubnubOnRemoveChannelGroupResponse OnRemoveChannelGroupResponse);

	/**
	 * Removes a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param ChannelGroup The name of the channel group to remove.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if operation result is not needed.
	 */
	void RemoveChannelGroup(FString ChannelGroup, FPubnubOnRemoveChannelGroupResponseNative NativeCallback = nullptr);

	
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
	void ListUsersFromChannel(FString Channel, FPubnubOnListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());

	/**
	 * Lists the users currently present on a specified channel.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to list users from.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param ListUsersFromChannelSettings Optional settings for the list users operation. See FPubnubListUsersFromChannelSettings for more details. 
	 */
	void ListUsersFromChannel(FString Channel, FPubnubOnListUsersFromChannelResponseNative NativeCallback, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());

	
	/**
	 * Lists the channels that a specified user is currently subscribed to.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 	
	 * @param UserID The user ID to list subscribed channels for.
	 * @param ListUserSubscribedChannelsResponse The callback function used to handle the result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void ListUserSubscribedChannels(FString UserID, FPubnubOnListUsersSubscribedChannelsResponse ListUserSubscribedChannelsResponse);
	
	/**
	 * Lists the channels that a specified user is currently subscribed to.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 	
	 * @param UserID The user ID to list subscribed channels for.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 */
	void ListUserSubscribedChannels(FString UserID, FPubnubOnListUsersSubscribedChannelsResponseNative NativeCallback);

	
	/**
	 * Sets the presence state for the current user on a specified channel.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to set the state on.
	 * @param StateJson The JSON string representing the state to set.
	 * @param OnSetStateResponse (Optional) Delegate to listen for the operation result.
	 * @param SetStateSettings Optional settings for the set state operation. See FPubnubSetStateSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence", meta = (AutoCreateRefTerm = "OnSetStateResponse"))
	void SetState(FString Channel, FString StateJson, FPubnubOnSetStateResponse OnSetStateResponse, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());

	/**
	 * Sets the presence state for the current user on a specified channel.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to set the state on.
	 * @param StateJson The JSON string representing the state to set.
	 * @param NativeCallback Optional delegate to listen for the set state result. Delegate in native form that can accept lambdas.
	 * @param SetStateSettings Optional settings for the set state operation. See FPubnubSetStateSettings for more details.
	 */
	void SetState(FString Channel, FString StateJson, FPubnubOnSetStateResponseNative NativeCallback = nullptr, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());

	/**
	 * Sets the presence state for the current user on a specified channel. Overload without delegate to get result.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to set the state on.
	 * @param StateJson The JSON string representing the state to set.
	 * @param SetStateSettings Optional settings for the set state operation. See FPubnubSetStateSettings for more details.
	 */
	void SetState(FString Channel, FString StateJson, FPubnubSetStateSettings SetStateSettings);

	
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
	void GetState(FString Channel, FString ChannelGroup, FString UserID, FPubnubOnGetStateResponse OnGetStateResponse);

	/**
	 * Gets the presence state for a specified user on a specified channel.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to get the state from.
	 * @param ChannelGroup The name of the channel group to get the state from.
	 * @param UserID The user ID to get the state for.
	 * @param NativeCallback The callback function used to handle the result in JSON format. Delegate in native form that can accept lambdas.
	 */
	void GetState(FString Channel, FString ChannelGroup, FString UserID, FPubnubOnGetStateResponseNative NativeCallback);

	
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



	/* ACCESS MANAGER API */

		
	/**
	 * Requests an access token from the PubNub server with the specified permissions.
	 * Requires SecretKey to be set.
	 * 
	 * @Note Requires the *Access Manager* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Ttl Time-To-Live (TTL) in minutes for the granted token.
	 * @param AuthorizedUser The User that is authorized by this grant.
	 * @param Permissions A struct containing all permissions that will be granted with this token.
	 * @param OnGrantTokenResponse The callback function used to handle the result.
	 * @param Meta (Optional) metadata that will be embedded into the token.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Access Manager")
	void GrantToken(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FPubnubOnGrantTokenResponse OnGrantTokenResponse, FString Meta = "");

	/**
	 * Requests an access token from the PubNub server with the specified permissions.
	 * Requires SecretKey to be set.
	 * 
	 * @Note Requires the *Access Manager* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Ttl Time-To-Live (TTL) in minutes for the granted token.
	 * @param AuthorizedUser The User that is authorized by this grant.
	 * @param Permissions A struct containing all permissions that will be granted with this token.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Meta (Optional) metadata that will be embedded into the token.
	 */
	void GrantToken(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FPubnubOnGrantTokenResponseNative NativeCallback, FString Meta = "");

	
	/**
	 * Revokes a previously granted access token.
	 * 
	 * @Note Requires the *Revoke v3 Token* in *Access Manager* section to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Token The access token to revoke.
	 * @param OnRevokeTokenResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Access Manager", meta = (AutoCreateRefTerm = "OnRevokeTokenResponse"))
	void RevokeToken(FString Token, FPubnubOnRevokeTokenResponse OnRevokeTokenResponse);

	/**
	 * Revokes a previously granted access token.
	 * 
	 * @Note Requires the *Revoke v3 Token* in *Access Manager* section to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Token The access token to revoke.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 */
	void RevokeToken(FString Token, FPubnubOnRevokeTokenResponseNative NativeCallback = nullptr);

	
	/**
	 * Parses an access token and retrieves information about its permissions.
	 * 
	 * @param Token The access token to parse.
	 * @return Parsed token
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Access Manager")
	FString ParseToken(FString Token);

	
	/**
	 * This method is used by the client devices to update the authentication token granted by the server.
	 * 
	 * @param Token Existing token with embedded permissions.
	 * 
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Access Manager")
	void SetAuthToken(FString Token);



	/* MESSAGE PERSISTENCE API */
		
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
	void FetchHistory(FString Channel, FPubnubOnFetchHistoryResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());

	/**
	 * Fetches historical messages from a specified channel using Message Persistence.
	 * 
	 * @Note Requires the *Message Persistence* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The ID of the channel to fetch messages from.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param FetchHistorySettings Optional settings for the fetch history operation. See FPubnubFetchHistorySettings for more details.
	 */
	void FetchHistory(FString Channel, FPubnubOnFetchHistoryResponseNative NativeCallback, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());

	
	/**
	 * Deletes historical messages from a specified channel using Message Persistence.
	 * 
	 * @Note Requires the *Message Persistence* add-on to be enabled for your key in the PubNub Admin Portal
	 * @Note Requires Enable Delete-From-History in Message Persistence tab for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The ID of the channel to delete messages from.
	 * @param OnDeleteMessagesResponse The callback function used to handle the result.
	 * @param DeleteMessagesSettings Optional settings for the delete messages operation - Start and End parameters to specify delete messages time range.
	 *								 See FPubnubDeleteMessagesSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Message Persistence", meta = (AutoCreateRefTerm = "OnDeleteMessagesResponse"))
	void DeleteMessages(FString Channel, FPubnubOnDeleteMessagesResponse OnDeleteMessagesResponse, FPubnubDeleteMessagesSettings DeleteMessagesSettings = FPubnubDeleteMessagesSettings());

	/**
	 * Deletes historical messages from a specified channel using Message Persistence.
	 * 
	 * @Note Requires the *Message Persistence* add-on to be enabled for your key in the PubNub Admin Portal
	 * @Note Requires Enable Delete-From-History in Message Persistence tab for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The ID of the channel to delete messages from.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 * @param DeleteMessagesSettings Optional settings for the delete messages operation - Start and End parameters to specify delete messages time range.
	 *						 See FPubnubDeleteMessagesSettings for more details.
	 */
	void DeleteMessages(FString Channel, FPubnubOnDeleteMessagesResponseNative NativeCallback = nullptr, FPubnubDeleteMessagesSettings DeleteMessagesSettings = FPubnubDeleteMessagesSettings());

	/**
	 * Deletes historical messages from a specified channel using Message Persistence.
	 * 
	 * @Note Requires the *Message Persistence* add-on to be enabled for your key in the PubNub Admin Portal
	 * @Note Requires Enable Delete-From-History in Message Persistence tab for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The ID of the channel to delete messages from.
	 * @param DeleteMessagesSettings Optional settings for the delete messages operation - Start and End parameters to specify delete messages time range.
	 *						 See FPubnubDeleteMessagesSettings for more details.
	 */
	void DeleteMessages(FString Channel, FPubnubDeleteMessagesSettings DeleteMessagesSettings);

	
	/**
	 * Returns the number of messages published on one or more channels since a given time.
	 * The count returned is the number of messages in history with a Timetoken value greater
	 * than or equal to than the passed value in the Timetoken parameter.
	 * 
	 * @Note Requires the *Message Persistence* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The ID of the channel to count messages for.
	 * @param Timetoken The timetoken to start counting messages from. (Exclusive, messages with the same timetoken, won't be counted).
	 * @param OnMessageCountsResponse The callback function used to handle the result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Message Persistence")
	void MessageCounts(FString Channel, FString Timetoken, FPubnubOnMessageCountsResponse OnMessageCountsResponse);

	/**
	 * Returns the number of messages published on one or more channels since a given time.
	 * The count returned is the number of messages in history with a Timetoken value greater
	 * than or equal to than the passed value in the Timetoken parameter.
	 * 
	 * @Note Requires the *Message Persistence* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The ID of the channel to count messages for.
	 * @param Timetoken The timetoken to start counting messages from. (Exclusive, messages with the same timetoken, won't be counted).
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 */
	void MessageCounts(FString Channel, FString Timetoken, FPubnubOnMessageCountsResponseNative NativeCallback);


	/* APP CONTEXT API */
	
	/**
	 * Returns a paginated list of User Metadata objects, optionally including the custom data object for each.
	 * (Generally the same as GetAllUserMetadata just using raw strings as Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param OnGetAllUserMetadataResponse The callback function used to handle the result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.  For example: "id" or "name:desc" or "id:desc,status".
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetAllUserMetadataRaw(FPubnubOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Returns a paginated list of User Metadata objects, optionally including the custom data object for each.
	 * (Generally the same as GetAllUserMetadata just using raw strings as Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.  For example: "id" or "name:desc" or "id:desc,status".
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	void GetAllUserMetadataRaw(FPubnubOnGetAllUserMetadataResponseNative NativeCallback, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Returns a paginated list of User Metadata objects, optionally including the custom data object for each.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param OnGetAllUserMetadataResponse The callback function used to handle the result.
	 * @param Include (Optional) List of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev"))
	void GetAllUserMetadata(FPubnubOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FPubnubGetAllInclude Include = FPubnubGetAllInclude(), int Limit = 100, FString Filter = "", FPubnubGetAllSort Sort = FPubnubGetAllSort(), FString PageNext = "", FString PagePrev = "");

	/**
	 * Returns a paginated list of User Metadata objects, optionally including the custom data object for each.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	void GetAllUserMetadata(FPubnubOnGetAllUserMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include = FPubnubGetAllInclude(), int Limit = 100, FString Filter = "", FPubnubGetAllSort Sort = FPubnubGetAllSort(), FString PageNext = "", FString PagePrev = "");
	

	/**
	 * Sets metadata for a specified User in the PubNub App Context.
	 * (Generally the same as SetUserMetadata just using raw string as UserMetadata and Include inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to set metadata.
	 * @param UserMetadataObj A JSON string representing the metadata to set.
	 * @param OnSetUserMetadataResponse (Optional) Delegate to listen for the operation result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnSetUserMetadataResponse"))
	void SetUserMetadataRaw(FString User, FString UserMetadataObj, FPubnubOnSetUserMetadataResponse OnSetUserMetadataResponse, FString Include = "");

	/**
	 * Sets metadata for a specified User in the PubNub App Context.
	 * (Generally the same as SetUserMetadata just using raw string as UserMetadata and Include inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to set metadata.
	 * @param UserMetadataObj A JSON string representing the metadata to set.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	void SetUserMetadataRaw(FString User, FString UserMetadataObj, FPubnubOnSetUserMetadataResponseNative NativeCallback = nullptr, FString Include = "");
	
	/**
	 * Sets metadata for a specified User in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to set metadata.
	 * @param UserMetadata User Metadata object to set.
	 * @param OnSetUserMetadataResponse (Optional) Delegate to listen for the operation result.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnSetUserMetadataResponse"))
	void SetUserMetadata(FString User, FPubnubUserData UserMetadata, FPubnubOnSetUserMetadataResponse OnSetUserMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Sets metadata for a specified User in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to set metadata.
	 * @param UserMetadata User Metadata object to set.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	void SetUserMetadata(FString User, FPubnubUserData UserMetadata, FPubnubOnSetUserMetadataResponseNative NativeCallback = nullptr, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	
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
	void GetUserMetadataRaw(FString User, FPubnubOnGetUserMetadataResponse OnGetUserMetadataResponse, FString Include = "");

	/**
	 * Retrieves metadata for a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to retrieve metadata.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	void GetUserMetadataRaw(FString User, FPubnubOnGetUserMetadataResponseNative NativeCallback, FString Include = "");

	/**
	 * Retrieves metadata for a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to retrieve metadata.
	 * @param OnGetUserMetadataResponse The callback function used to handle the result.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void GetUserMetadata(FString User, FPubnubOnGetUserMetadataResponse OnGetUserMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Retrieves metadata for a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to retrieve metadata.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	void GetUserMetadata(FString User, FPubnubOnGetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());
	

	/**
	 * Removes all metadata associated with a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to remove metadata.
	 * @param OnRemoveUserMetadataResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnRemoveUserMetadataResponse"))
	void RemoveUserMetadata(FString User, FPubnubOnRemoveUserMetadataResponse OnRemoveUserMetadataResponse);

	/**
	 * Removes all metadata associated with a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to remove metadata.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 */
	void RemoveUserMetadata(FString User, FPubnubOnRemoveUserMetadataResponseNative NativeCallback = nullptr);

	
	/**
	 * Returns a paginated list of Channel Metadata objects, optionally including the custom data object for each.
	 * (Generally the same as GetAllChannelMetadata just using raw strings as Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param OnGetAllChannelMetadataResponse The callback function used to handle the result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.  For example: "id" or "name:desc" or "id:desc,status".
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetAllChannelMetadataRaw(FPubnubOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Returns a paginated list of Channel Metadata objects, optionally including the custom data object for each.
	 * (Generally the same as GetAllChannelMetadata just using raw strings as Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.  For example: "id" or "name:desc" or "id:desc,status".
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	void GetAllChannelMetadataRaw(FPubnubOnGetAllChannelMetadataResponseNative NativeCallback, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Returns a paginated list of Channel Metadata objects, optionally including the custom data object for each.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param OnGetAllChannelMetadataResponse The callback function used to handle the result.
	 * @param Include (Optional) List of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev"))
	void GetAllChannelMetadata(FPubnubOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FPubnubGetAllInclude Include = FPubnubGetAllInclude(), int Limit = 100, FString Filter = "", FPubnubGetAllSort Sort = FPubnubGetAllSort(), FString PageNext = "", FString PagePrev = "");

	/**
	 * Returns a paginated list of Channel Metadata objects, optionally including the custom data object for each.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	void GetAllChannelMetadata(FPubnubOnGetAllChannelMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include = FPubnubGetAllInclude(), int Limit = 100, FString Filter = "", FPubnubGetAllSort Sort = FPubnubGetAllSort(), FString PageNext = "", FString PagePrev = "");



	/**
	 * Sets metadata for a specified Channel in the PubNub App Context.
	 * (Generally the same as SetChannelMetadata just using raw string as ChannelMetadata and Include inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to set metadata.
	 * @param ChannelMetadataObj A JSON string representing the metadata to set.
	 * @param OnSetChannelMetadataResponse (Optional) Delegate to listen for the operation result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnSetChannelMetadataResponse"))
	void SetChannelMetadataRaw(FString Channel, FString ChannelMetadataObj, FPubnubOnSetChannelMetadataResponse OnSetChannelMetadataResponse, FString Include = "");

	/**
	 * Sets metadata for a specified Channel in the PubNub App Context.
	 * (Generally the same as SetChannelMetadata just using raw string as ChannelMetadata and Include inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to set metadata.
	 * @param ChannelMetadataObj A JSON string representing the metadata to set.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	void SetChannelMetadataRaw(FString Channel, FString ChannelMetadataObj, FPubnubOnSetChannelMetadataResponseNative NativeCallback = nullptr, FString Include = "");

	/**
	 * Sets metadata for a specified Channel in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to set metadata.
	 * @param ChannelMetadata Channel Metadata object to set.
	 * @param OnSetChannelMetadataResponse (Optional) Delegate to listen for the operation result.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnSetChannelMetadataResponse"))
	void SetChannelMetadata(FString Channel, FPubnubChannelData ChannelMetadata, FPubnubOnSetChannelMetadataResponse OnSetChannelMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Sets metadata for a specified Channel in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to set metadata.
	 * @param ChannelMetadata Channel Metadata object to set.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	void SetChannelMetadata(FString Channel, FPubnubChannelData ChannelMetadata, FPubnubOnSetChannelMetadataResponseNative NativeCallback = nullptr, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	
	/**
	 * Retrieves metadata for a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to retrieve metadata.
	 * @param OnGetChannelMetadataResponse The callback function used to handle the result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void GetChannelMetadataRaw(FString Channel, FPubnubOnGetChannelMetadataResponse OnGetChannelMetadataResponse, FString Include = "");

	/**
	 * Retrieves metadata for a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to retrieve metadata.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	void GetChannelMetadataRaw(FString Channel, FPubnubOnGetChannelMetadataResponseNative NativeCallback, FString Include = "");

	/**
	 * Retrieves metadata for a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to retrieve metadata.
	 * @param OnGetChannelMetadataResponse The callback function used to handle the result.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context")
	void GetChannelMetadata(FString Channel, FPubnubOnGetChannelMetadataResponse OnGetChannelMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Retrieves metadata for a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to retrieve metadata.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	void GetChannelMetadata(FString Channel, FPubnubOnGetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	
	/**
	 * Removes all metadata associated with a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to remove metadata.
	 * @param OnRemoveChannelMetadataResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnRemoveChannelMetadataResponse"))
	void RemoveChannelMetadata(FString Channel, FPubnubOnRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse);

	/**
	 * Removes all metadata associated with a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to remove metadata.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 */
	void RemoveChannelMetadata(FString Channel, FPubnubOnRemoveChannelMetadataResponseNative NativeCallback = nullptr);

	
	/**
	 * Retrieves a list of memberships for a specified User in the PubNub App Context.
	 * (Generally the same as GetMemberships just using raw strings as Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to retrieve memberships.
	 * @param OnGetMembershipsResponse The callback function used to handle the result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Comma-separated Key-value pair of a property to sort by, and a sort direction. For example: "channel.id" or "channel.name:desc" or "channel.id:desc,status".
	 *			   If direction is not specified, ascending will be used.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetMembershipsRaw(FString User, FPubnubOnGetMembershipsResponse OnGetMembershipsResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Retrieves a list of memberships for a specified User in the PubNub App Context.
	 * (Generally the same as GetMemberships just using raw strings as Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to retrieve memberships.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Comma-separated Key-value pair of a property to sort by, and a sort direction. For example: "channel.id" or "channel.name:desc" or "channel.id:desc,status".
	 *			   If direction is not specified, ascending will be used.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	void GetMembershipsRaw(FString User, FPubnubOnGetMembershipsResponseNative NativeCallback, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Retrieves a list of memberships for a specified User in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to retrieve memberships.
	 * @param OnGetMembershipsResponse The callback function used to handle the result.
	 * @param Include (Optional) List of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev"))
	void GetMemberships(FString User, FPubnubOnGetMembershipsResponse OnGetMembershipsResponse, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");

	/**
	 * Retrieves a list of memberships for a specified User in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to retrieve memberships.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	void GetMemberships(FString User, FPubnubOnGetMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");

	
	/**
	 * Sets or updates a User's Memberships to one or more Channels.
	 * A Membership is the relationship between a User and a Channel, and can contain its own metadata (e.g., `Custom` and `Status`).
	 * This is the user-centric counterpart to the `SetChannelMembers` function. Both functions modify the same underlying data.
	 * (Generally the same as SetMemberships just using raw strings as SetObj, Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to set memberships.
	 * @param SetObj A JSON string representing the memberships to set.
	 * @param OnSetMembershipsResponse (Optional) Delegate to listen for the operation result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Comma-separated Key-value pair of a property to sort by, and a sort direction. For example: "channel.id" or "channel.name:desc" or "channel.id:desc,status".
	 *			   If direction is not specified, ascending will be used.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count", AutoCreateRefTerm = "OnSetMembershipsResponse"))
	void SetMembershipsRaw(FString User, FString SetObj, FPubnubOnSetMembershipsResponse OnSetMembershipsResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Sets or updates a User's Memberships to one or more Channels.
	 * A Membership is the relationship between a User and a Channel, and can contain its own metadata (e.g., `Custom` and `Status`).
	 * This is the user-centric counterpart to the `SetChannelMembers` function. Both functions modify the same underlying data.
	 * (Generally the same as SetMemberships just using raw strings as SetObj, Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to set memberships.
	 * @param SetObj A JSON string representing the memberships to set.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Comma-separated Key-value pair of a property to sort by, and a sort direction. For example: "channel.id" or "channel.name:desc" or "channel.id:desc,status".
	 *			   If direction is not specified, ascending will be used.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	void SetMembershipsRaw(FString User, FString SetObj, FPubnubOnSetMembershipsResponseNative NativeCallback = nullptr, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Sets or updates a User's Memberships to one or more Channels.
	 * A Membership is the relationship between a User and a Channel, and can contain its own metadata (e.g., `Custom` and `Status`).
	 * This is the user-centric counterpart to the `SetChannelMembers` function. Both functions modify the same underlying data.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to set memberships.
	 * @param Channels Channels with their associated MembershipData to set.
	 * @param OnSetMembershipsResponse (Optional) Delegate to listen for the operation result.
	 * @param Include (Optional) List of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev", AutoCreateRefTerm = "OnSetMembershipsResponse"))
	void SetMemberships(FString User, TArray<FPubnubMembershipInputData> Channels, FPubnubOnSetMembershipsResponse OnSetMembershipsResponse, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");
	
	/**
	 * Sets or updates a User's Memberships to one or more Channels.
	 * A Membership is the relationship between a User and a Channel, and can contain its own metadata (e.g., `Custom` and `Status`).
	 * This is the user-centric counterpart to the `SetChannelMembers` function. Both functions modify the same underlying data.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to set memberships.
	 * @param Channels Channels with their associated MembershipData to set.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	void SetMemberships(FString User, TArray<FPubnubMembershipInputData> Channels, FPubnubOnSetMembershipsResponseNative NativeCallback = nullptr, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");

	
	/**
	 * Removes memberships for a specified User from the PubNub App Context.
	 * (Generally the same as RemoveMemberships just using raw strings as RemoveObj, Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to remove memberships.
	 * @param RemoveObj A JSON string representing the memberships to remove.
	 * @param OnRemoveMembershipsResponse (Optional) Delegate to listen for the operation result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Comma-separated Key-value pair of a property to sort by, and a sort direction. For example: "channel.id" or "channel.name:desc" or "channel.id:desc,status".
	 *			   If direction is not specified, ascending will be used.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count", AutoCreateRefTerm = "OnRemoveMembershipsResponse"))
	void RemoveMembershipsRaw(FString User, FString RemoveObj, FPubnubOnRemoveMembershipsResponse OnRemoveMembershipsResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Removes memberships for a specified User from the PubNub App Context.
	 * (Generally the same as RemoveMemberships just using raw strings as RemoveObj, Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to remove memberships.
	 * @param RemoveObj A JSON string representing the memberships to remove.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Comma-separated Key-value pair of a property to sort by, and a sort direction. For example: "channel.id" or "channel.name:desc" or "channel.id:desc,status".
	 *			   If direction is not specified, ascending will be used.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	void RemoveMembershipsRaw(FString User, FString RemoveObj, FPubnubOnRemoveMembershipsResponseNative NativeCallback = nullptr, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Removes memberships for a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to remove memberships.
	 * @param Channels Array of Memberships (Channels) to remove.
	 * @param OnRemoveMembershipsResponse (Optional) Delegate to listen for the operation result.
	 * @param Include (Optional) List of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev", AutoCreateRefTerm = "OnRemoveMembershipsResponse"))
	void RemoveMemberships(FString User, TArray<FString> Channels, FPubnubOnRemoveMembershipsResponse OnRemoveMembershipsResponse, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");

	/**
	 * Removes memberships for a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param User The user ID for whom to remove memberships.
	 * @param Channels Array of Memberships (Channels) to remove.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	void RemoveMemberships(FString User, TArray<FString> Channels, FPubnubOnRemoveMembershipsResponseNative NativeCallback = nullptr, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");

	
	/**
	 * Retrieves a list of members for a specified Channel in the PubNub App Context.
	 *
	 * (Generally the same as GetChannelMembers just using raw strings as Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The Channel ID for which to retrieve member.
	 * @param OnGetMembersResponse The callback function used to handle the result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Comma-separated Key-value pair of a property to sort by, and a sort direction. For example: "user.id" or "user.name:desc" or "user.id:desc,status".
	 * 			   If direction is not specified, ascending will be used.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetChannelMembersRaw(FString Channel, FPubnubOnGetChannelMembersResponse OnGetMembersResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);
	
	/**
	 * Retrieves a list of members for a specified Channel in the PubNub App Context.
	 *
	 * (Generally the same as GetChannelMembers just using raw strings as Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The Channel ID for which to retrieve member.
	 * @param NativeCallback Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Comma-separated Key-value pair of a property to sort by, and a sort direction. For example: "user.id" or "user.name:desc" or "user.id:desc,status".
	 * 			   If direction is not specified, ascending will be used.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	void GetChannelMembersRaw(FString Channel, FPubnubOnGetChannelMembersResponseNative NativeCallback, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Retrieves a list of members for a specified Channel in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The Channel ID for which to retrieve member.
	 * @param OnGetMembersResponse The callback function used to handle the result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev"))
	void GetChannelMembers(FString Channel, FPubnubOnGetChannelMembersResponse OnGetMembersResponse, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");
	
	/**
	 * Retrieves a list of members for a specified Channel in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The Channel ID for which to retrieve member.
	 * @param NativeCallback Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	void GetChannelMembers(FString Channel, FPubnubOnGetChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");

	
	/**
	 * Sets or updates the members for a specified Channel.
	 * A member (Membership) represents a User's relationship to the Channel and can contain its own metadata (e.g., `Custom` and `Status`).
	 * This is the channel-centric counterpart to the `SetMemberships` function. Both functions modify the same underlying data.
	 * (Generally the same as SetChannelMembers just using raw strings as SetObj, Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The channel name to set members for.
	 * @param SetObj A JSON string representing the users to set as members.
	 * @param OnSetChannelMembersResponse (Optional) Delegate to listen for the operation result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Comma-separated Key-value pair of a property to sort by, and a sort direction. For example: "user.id" or "user.name:desc" or "user.id:desc,status".
	 *			   If direction is not specified, ascending will be used.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count", AutoCreateRefTerm = "OnSetChannelMembersResponse"))
	void SetChannelMembersRaw(FString Channel, FString SetObj, FPubnubOnSetChannelMembersResponse OnSetChannelMembersResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Sets or updates the members for a specified Channel.
	 * A member (Membership) represents a User's relationship to the Channel and can contain its own metadata (e.g., `Custom` and `Status`).
	 * This is the channel-centric counterpart to the `SetMemberships` function. Both functions modify the same underlying data.
	 * (Generally the same as SetChannelMembers just using raw strings as SetObj, Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The channel name to set members for.
	 * @param SetObj A JSON string representing the users to set as members.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Comma-separated Key-value pair of a property to sort by, and a sort direction. For example: "user.id" or "user.name:desc" or "user.id:desc,status".
	 *			   If direction is not specified, ascending will be used.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	void SetChannelMembersRaw(FString Channel, FString SetObj, FPubnubOnSetChannelMembersResponseNative NativeCallback = nullptr, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Sets or updates the members for a specified Channel.
	 * A member (Membership) represents a User's relationship to the Channel and can contain its own metadata (e.g., `Custom` and `Status`).
	 * This is the channel-centric counterpart to the `SetMemberships` function. Both functions modify the same underlying data.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The channel name to add members to.
	 * @param Users Users with their associated MembershipData to set.
	 * @param OnSetChannelMembersResponse (Optional) Delegate to listen for the operation result.
	 * @param Include (Optional) List of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev", AutoCreateRefTerm = "OnSetChannelMembersResponse"))
	void SetChannelMembers(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FPubnubOnSetChannelMembersResponse OnSetChannelMembersResponse, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");
	
	/**
	 * Sets or updates the members for a specified Channel.
	 * A member (Membership) represents a User's relationship to the Channel and can contain its own metadata (e.g., `Custom` and `Status`).
	 * This is the channel-centric counterpart to the `SetMemberships` function. Both functions modify the same underlying data.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The channel name to add members to.
	 * @param Users Users with their associated MembershipData to set.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	void SetChannelMembers(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FPubnubOnSetChannelMembersResponseNative NativeCallback = nullptr, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");

	
	/**
	 * Removes users from a specified channel in the PubNub App Context.
	 * (Generally the same as RemoveChannelMembers just using raw strings as RemoveObj, Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The channel name to remove members from.
	 * @param RemoveObj A JSON string representing the users to remove.
	 * @param OnRemoveChannelMembersResponse (Optional) Delegate to listen for the operation result.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Comma-separated Key-value pair of a property to sort by, and a sort direction. For example: "user.id" or "user.name:desc" or "user.id:desc,status".
	 *			   If direction is not specified, ascending will be used.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count", AutoCreateRefTerm = "OnRemoveChannelMembersResponse"))
	void RemoveChannelMembersRaw(FString Channel, FString RemoveObj, FPubnubOnRemoveChannelMembersResponse OnRemoveChannelMembersResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Removes users from a specified channel in the PubNub App Context.
	 * (Generally the same as RemoveChannelMembers just using raw strings as RemoveObj, Include and Sort inputs)
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The channel name to remove members from.
	 * @param RemoveObj A JSON string representing the users to remove.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Comma-separated Key-value pair of a property to sort by, and a sort direction. For example: "user.id" or "user.name:desc" or "user.id:desc,status".
	 *			   If direction is not specified, ascending will be used.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 * @param Count (Optional) Whether to include a total count of users in the response (default: not set).
	 */
	void RemoveChannelMembersRaw(FString Channel, FString RemoveObj, FPubnubOnRemoveChannelMembersResponseNative NativeCallback = nullptr, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

	/**
	 * Removes users from a specified channel in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The channel name to add members to.
	 * @param Users Array of ChannelMembers (Users) to remove.
	 * @param OnRemoveChannelMembersResponse (Optional) Delegate to listen for the operation result.
	 * @param Include (Optional) List of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev", AutoCreateRefTerm = "OnRemoveChannelMembersResponse"))
	void RemoveChannelMembers(FString Channel, TArray<FString> Users, FPubnubOnRemoveChannelMembersResponse OnRemoveChannelMembersResponse, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");
	
	/**
	 * Removes users from a specified channel in the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 *
	 * @param Channel The channel name to add members to.
	 * @param Users Array of ChannelMembers (Users) to remove.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 * @param Limit (Optional) The maximum number of results to return (default: 100).
	 * @param Filter (Optional) Expression used to filter the results. Check online documentation to see exact filter formulas;
	 * @param Sort (Optional) Key-value pair of a property to sort by, and a sort direction.
	 * @param PageNext (Optional) A string to retrieve the next page of results (if applicable).
	 * @param PagePrev (Optional) A string to retrieve the previous page of results (if applicable). Ignored if PageNext is provided.
	 */
	void RemoveChannelMembers(FString Channel, TArray<FString> Users, FPubnubOnRemoveChannelMembersResponseNative NativeCallback = nullptr, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");

	

private:

	//Thread for all PubNub operations, this thread will queue all PubNub calls and trigger them one by one
	FPubnubFunctionThread* PubnubCallsThread = nullptr;

	//Pubnub context for the most of the pubnub operations
	pubnub_t *ctx_pub = nullptr;
	//Pubnub context for the event engine - subscribe operations
	pubnub_t *ctx_ee = nullptr;

#pragma region PUBNUB INIT

	void InitWithConfig(UPubnubSubsystem* InPubnubSubsystem, FPubnubConfig InConfig, int InClientID, FString InDebugName = "");
	void DeinitializeClient();
	
	UPROPERTY()
	TObjectPtr<UPubnubSubsystem> PubnubSubsystem = nullptr;
	int ClientID = -1;
	FString DebugName = "";
	bool IsInitialized = false;
	bool IsUserIDSet = false;

#pragma endregion

#pragma region PUBNUB CRYPTO
	
	//CryptoBridge class that holds provided CryptoModule and inserts it into C-Core system - it keeps all required references alive
	UPROPERTY()
	TObjectPtr<UPubnubCryptoBridge> CryptoBridge;

	void DecryptHistoryMessages(TArray<FPubnubHistoryMessageData>& Messages);

#pragma endregion

#pragma region PUBNUB AUTH

	//Auth token has to be kept alive for the lifetime of the sdk, so this is the container for it
	char* AuthTokenBuffer = nullptr;
	size_t AuthTokenLength = 0;

#pragma endregion

#pragma region PUBNUB CONFIG

	//Container for all configuration settings
	//TODO:: DO we even need to save config??
	UPROPERTY()
	FPubnubConfig PubnubConfig;

	void SavePubnubConfig(const FPubnubConfig &InConfig);
	
	//Containers for keys stored from settings
	static const int PublishKeySize = 42;
	static const int SecretKeySize = 54;
	char PublishKey[PublishKeySize + 1] = {};
	char SubscribeKey[PublishKeySize + 1] = {};
	char SecretKey[SecretKeySize + 1] = {};

#pragma endregion 

#pragma region PUBNUB SUBSCRIPTION

	//Storage for global subscriptions (not from Entities)
	TMap<FString, CCoreSubscriptionCallback*> ChannelSubscriptions;
	TMap<FString, CCoreSubscriptionCallback*> ChannelGroupSubscriptions;

	//Array storing delegates for all queued subscription operations
	TArray<FPubnubOnSubscribeOperationResponseNative> SubscriptionResultDelegates;

	void OnCCoreSubscriptionStatusReceived(int StatusEnum, const void* StatusData);

#pragma endregion
	
	//Returns FString from the pubnub_get response
	FString GetLastResponse(pubnub_t* context);
	
	//Returns FString from the pubnub_get_channel response
	FString GetLastChannelResponse(pubnub_t* context);

	//TODO:: Move these functions to the logger
	void PubnubError(FString ErrorMessage, EPubnubErrorType ErrorType = EPubnubErrorType::PET_Error);
	void PubnubResponseError(pubnub_res PubnubResponse, FString ErrorMessage);

	
	void InitPubnub_priv(const FPubnubConfig& Config);
	void SetUserID_priv(FString UserID);
	FString GetUserID_priv();
	void SetSecretKey_priv();
	void PublishMessage_priv(FString Channel, FString Message, FPubnubOnPublishMessageResponseNative OnPublishMessageResponse, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());
	void Signal_priv(FString Channel, FString Message, FPubnubOnSignalResponseNative OnSignalResponse, FPubnubSignalSettings SignalSettings = FPubnubSignalSettings());
	void SubscribeToChannel_priv(FString Channel, FPubnubOnSubscribeOperationResponseNative OnSubscribeToChannelResponse, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());
	void SubscribeToGroup_priv(FString ChannelGroup, FPubnubOnSubscribeOperationResponseNative OnSubscribeToGroupResponse, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());
	void UnsubscribeFromChannel_priv(FString Channel, FPubnubOnSubscribeOperationResponseNative OnUnsubscribeFromChannelResponse);
	void UnsubscribeFromGroup_priv(FString ChannelGroup, FPubnubOnSubscribeOperationResponseNative OnUnsubscribeFromGroupResponse);
	void UnsubscribeFromAll_priv(FPubnubOnSubscribeOperationResponseNative OnUnsubscribeFromAllResponse = nullptr);
	void AddChannelToGroup_priv(FString Channel, FString ChannelGroup, FPubnubOnAddChannelToGroupResponseNative OnAddChannelToGroupResponse);
	void RemoveChannelFromGroup_priv(FString Channel, FString ChannelGroup, FPubnubOnRemoveChannelFromGroupResponseNative OnRemoveChannelFromGroupResponse);
	void ListChannelsFromGroup_priv(FString ChannelGroup, FPubnubOnListChannelsFromGroupResponseNative OnListChannelsResponse);
	void RemoveChannelGroup_priv(FString ChannelGroup, FPubnubOnRemoveChannelGroupResponseNative OnRemoveChannelGroupResponse);
	void ListUsersFromChannel_priv(FString Channel, FPubnubOnListUsersFromChannelResponseNative ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());
	void ListUserSubscribedChannels_priv(FString UserID, FPubnubOnListUsersSubscribedChannelsResponseNative ListUserSubscribedChannelsResponse);
	void SetState_priv(FString Channel, FString StateJson, FPubnubOnSetStateResponseNative OnSetStateResponse, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());
	void GetState_priv(FString Channel, FString ChannelGroup, FString UserID, FPubnubOnGetStateResponseNative OnGetStateResponse);
	void Heartbeat_priv(FString Channel, FString ChannelGroup);
	void GrantToken_priv(FString PermissionObject, FPubnubOnGrantTokenResponseNative OnGrantTokenResponse);
	void RevokeToken_priv(FString Token, FPubnubOnRevokeTokenResponseNative OnRevokeTokenResponse);
	FString ParseToken_priv(FString Token);
	void SetAuthToken_priv(FString Token);
	void FetchHistory_priv(FString Channel, FPubnubOnFetchHistoryResponseNative OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());
	void DeleteMessages_priv(FString Channel, FPubnubOnDeleteMessagesResponseNative OnDeleteMessagesResponse, FPubnubDeleteMessagesSettings DeleteMessagesSettings);
	void MessageCounts_priv(FString Channel, FString Timetoken, FPubnubOnMessageCountsResponseNative OnMessageCountsResponse);
	void GetAllUserMetadata_priv(FPubnubOnGetAllUserMetadataResponseNative OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetUserMetadata_priv(FString User, FString UserMetadataObj, FPubnubOnSetUserMetadataResponseNative OnSetUserMetadataResponse, FString Include);
	void GetUserMetadata_priv(FString User, FPubnubOnGetUserMetadataResponseNative OnGetUserMetadataResponse, FString Include);
	void RemoveUserMetadata_priv(FString User, FPubnubOnRemoveUserMetadataResponseNative OnRemoveUserMetadataResponse);
	void GetAllChannelMetadata_priv(FPubnubOnGetAllChannelMetadataResponseNative OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetChannelMetadata_priv(FString Channel, FString ChannelMetadataObj, FPubnubOnSetChannelMetadataResponseNative OnSetChannelMetadataResponse, FString Include);
	void GetChannelMetadata_priv(FString Channel, FPubnubOnGetChannelMetadataResponseNative OnGetChannelMetadataResponse, FString Include);
	void RemoveChannelMetadata_priv(FString Channel, FPubnubOnRemoveChannelMetadataResponseNative OnRemoveChannelMetadataResponse);
	void GetMemberships_priv(FString User, FPubnubOnGetMembershipsResponseNative OnGetMembershipsResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetMemberships_priv(FString User, FString SetObj, FPubnubOnSetMembershipsResponseNative OnSetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void RemoveMemberships_priv(FString User, FString RemoveObj, FPubnubOnRemoveMembershipsResponseNative OnRemoveMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void GetChannelMembers_priv(FString Channel, FPubnubOnGetChannelMembersResponseNative OnGetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetChannelMembers_priv(FString Channel, FString SetObj, FPubnubOnSetChannelMembersResponseNative OnSetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void RemoveChannelMembers_priv(FString Channel, FString RemoveObj, FPubnubOnRemoveChannelMembersResponseNative OnRemoveMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void AddMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FPubnubOnAddMessageActionResponseNative AddMessageActionResponse);
	void RemoveMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FPubnubOnRemoveMessageActionResponseNative OnRemoveMessageActionResponse);
	void GetMessageActions_priv(FString Channel, FPubnubOnGetMessageActionsResponseNative OnGetMessageActionsResponse, FString Start, FString End, int Limit);

};

