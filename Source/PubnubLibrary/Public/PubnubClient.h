// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PubnubStructLibrary.h"
#include "PubnubEnumLibrary.h"
#include "Crypto/PubnubCryptorInterface.h"
#include "PubnubClient.generated.h"

class UPubnubSubsystem;
class UPubnubCryptoBridge;
class FPubnubFunctionThread;
class UPubnubSubscription;
class UPubnubSubscriptionSet;
class UPubnubBaseEntity;
class UPubnubChannelEntity;
class UPubnubChannelGroupEntity;
class UPubnubChannelMetadataEntity;
class UPubnubUserMetadataEntity;
struct CCoreSubscriptionCallback;

struct pubnub_;
typedef struct pubnub_ pubnub_t;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPubnubClientDeinitialized);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPubnubMessageReceived, FPubnubMessageData, Message);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPubnubMessageReceivedNative, const FPubnubMessageData& Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPubnubError, FString, ErrorMessage, EPubnubErrorType, ErrorType);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPubnubErrorNative, FString ErrorMessage, EPubnubErrorType ErrorType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPubnubSubscriptionStatusChanged, EPubnubSubscriptionStatus, Status, FPubnubSubscriptionStatusData, StatusData);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPubnubSubscriptionStatusChangedNative, EPubnubSubscriptionStatus Status, const FPubnubSubscriptionStatusData& StatusData);


DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPubnubPublishMessageResponse, FPubnubOperationResult, Result, FPubnubMessageData, PublishedMessage);
DECLARE_DELEGATE_TwoParams(FOnPubnubPublishMessageResponseNative, const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPubnubSignalResponse, FPubnubOperationResult, Result, FPubnubMessageData, SignalMessage);
DECLARE_DELEGATE_TwoParams(FOnPubnubSignalResponseNative, const FPubnubOperationResult& Result, const FPubnubMessageData& SignalMessage);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubSubscribeOperationResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnPubnubSubscribeOperationResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubAddChannelToGroupResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnPubnubAddChannelToGroupResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubRemoveChannelFromGroupResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnPubnubRemoveChannelFromGroupResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPubnubListChannelsFromGroupResponse, FPubnubOperationResult, Result, const TArray<FString>&, Channels);
DECLARE_DELEGATE_TwoParams(FOnPubnubListChannelsFromGroupResponseNative, const FPubnubOperationResult& Result, const TArray<FString>& Channels);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubRemoveChannelGroupResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnPubnubRemoveChannelGroupResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPubnubListUsersSubscribedChannelsResponse, FPubnubOperationResult, Result, const TArray<FString>&, Channels);
DECLARE_DELEGATE_TwoParams(FOnPubnubListUsersSubscribedChannelsResponseNative, const FPubnubOperationResult& Result, const TArray<FString>& Channels);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPubnubListUsersFromChannelResponse, FPubnubOperationResult, Result, FPubnubListUsersFromChannelWrapper, Data);
DECLARE_DELEGATE_TwoParams(FOnPubnubListUsersFromChannelResponseNative, const FPubnubOperationResult& Result, const FPubnubListUsersFromChannelWrapper& Data);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubSetStateResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnPubnubSetStateResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPubnubGetStateResponse, FPubnubOperationResult, Result, FString, StateResponse);
DECLARE_DELEGATE_TwoParams(FOnPubnubGetStateResponseNative, const FPubnubOperationResult& Result, FString StateResponse);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPubnubGrantTokenResponse, FPubnubOperationResult, Result, FString, Token);
DECLARE_DELEGATE_TwoParams(FOnPubnubGrantTokenResponseNative, const FPubnubOperationResult& Result, FString Token);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubRevokeTokenResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnPubnubRevokeTokenResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPubnubFetchHistoryResponse, FPubnubOperationResult, Result, const TArray<FPubnubHistoryMessageData>&, Messages);
DECLARE_DELEGATE_TwoParams(FOnPubnubFetchHistoryResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubHistoryMessageData>& Messages);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPubnubMessageCountsResponse, FPubnubOperationResult, Result, int, MessageCounts);
DECLARE_DELEGATE_TwoParams(FOnPubnubMessageCountsResponseNative, const FPubnubOperationResult& Result, int MessageCounts);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubDeleteMessagesResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnPubnubDeleteMessagesResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnPubnubGetAllUserMetadataResponse, FPubnubOperationResult, Result, const TArray<FPubnubUserData>&, UsersData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnPubnubGetAllUserMetadataResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubUserData>& UsersData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPubnubGetUserMetadataResponse, FPubnubOperationResult, Result, FPubnubUserData, UserData);
DECLARE_DELEGATE_TwoParams(FOnPubnubGetUserMetadataResponseNative, const FPubnubOperationResult& Result, const FPubnubUserData& UserData);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPubnubSetUserMetadataResponse, FPubnubOperationResult, Result, FPubnubUserData, UserData);
DECLARE_DELEGATE_TwoParams(FOnPubnubSetUserMetadataResponseNative, const FPubnubOperationResult& Result, const FPubnubUserData& UserData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubRemoveUserMetadataResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnPubnubRemoveUserMetadataResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnPubnubGetAllChannelMetadataResponse, FPubnubOperationResult, Result, const TArray<FPubnubChannelData>&, ChannelsData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnPubnubGetAllChannelMetadataResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubChannelData>& ChannelsData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPubnubGetChannelMetadataResponse, FPubnubOperationResult, Result, FPubnubChannelData, ChannelData);
DECLARE_DELEGATE_TwoParams(FOnPubnubGetChannelMetadataResponseNative, const FPubnubOperationResult& Result, const FPubnubChannelData& ChannelData);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPubnubSetChannelMetadataResponse, FPubnubOperationResult, Result, FPubnubChannelData, ChannelData);
DECLARE_DELEGATE_TwoParams(FOnPubnubSetChannelMetadataResponseNative, const FPubnubOperationResult& Result, const FPubnubChannelData& ChannelData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubRemoveChannelMetadataResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnPubnubRemoveChannelMetadataResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnPubnubGetMembershipsResponse, FPubnubOperationResult, Result, const TArray<FPubnubMembershipData>&, MembershipsData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnPubnubGetMembershipsResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnPubnubSetMembershipsResponse, FPubnubOperationResult, Result, const TArray<FPubnubMembershipData>&, MembershipsData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnPubnubSetMembershipsResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnPubnubRemoveMembershipsResponse, FPubnubOperationResult, Result, const TArray<FPubnubMembershipData>&, MembershipsData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnPubnubRemoveMembershipsResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnPubnubGetChannelMembersResponse, FPubnubOperationResult, Result, const TArray<FPubnubChannelMemberData>&, MembersData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnPubnubGetChannelMembersResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnPubnubSetChannelMembersResponse, FPubnubOperationResult, Result, const TArray<FPubnubChannelMemberData>&, MembersData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnPubnubSetChannelMembersResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnPubnubRemoveChannelMembersResponse, FPubnubOperationResult, Result, const TArray<FPubnubChannelMemberData>&, MembersData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnPubnubRemoveChannelMembersResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPubnubGetMessageActionsResponse, FPubnubOperationResult, Result, const TArray<FPubnubMessageActionData>&, MessageActions);
DECLARE_DELEGATE_TwoParams(FOnPubnubGetMessageActionsResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubMessageActionData>& MessageActions);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPubnubAddMessageActionResponse, FPubnubOperationResult, Result, FPubnubMessageActionData, MessageActionData);
DECLARE_DELEGATE_TwoParams(FOnPubnubAddMessageActionResponseNative, const FPubnubOperationResult& Result, const FPubnubMessageActionData& MessageActionData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubRemoveMessageActionResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnPubnubRemoveMessageActionResponseNative, const FPubnubOperationResult& Result);

/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubClient : public UObject
{
	GENERATED_BODY()

	friend class UPubnubSubsystem;
	friend class UPubnubSubscription;
	friend class UPubnubSubscriptionSet;

public:
	
	/* PUBLIC DELEGATES */

	/**Delegate that is called when PubnubClient is deinitialized*/
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FOnPubnubClientDeinitialized OnPubnubClientDeinitialized;
	
	/**Listener to react for subscription status changed*/
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FOnPubnubSubscriptionStatusChanged OnPubnubSubscriptionStatusChanged;

	/**Listener to react for subscription status changed , equivalent that accepts lambdas*/
	FOnPubnubSubscriptionStatusChangedNative OnPubnubSubscriptionStatusChangedNative;

	/**Global listener for all messages received on subscribed channels*/
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FOnPubnubMessageReceived OnPubnubMessageReceived;

	/**Global listener for all messages received on subscribed channels, equivalent that accepts lambdas*/
	FOnPubnubMessageReceivedNative OnPubnubMessageReceivedNative;

	/**Listener to react for all Errors in Pubnub functions */
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FOnPubnubError OnPubnubError;
	
	/**Listener to react for all Errors in Pubnub functions, equivalent that accepts lambdas*/
	FOnPubnubErrorNative OnPubnubErrorNative;

	
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
	void PublishMessage(FString Channel, FString Message, FOnPubnubPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

	/**
	 * Publishes a message to a specified channel.
	 * 
	 * @param Channel The ID of the channel to publish the message to.
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param NativeCallback Optional delegate to listen for the publish result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if publish result is not needed.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	void PublishMessage(FString Channel, FString Message, FOnPubnubPublishMessageResponseNative NativeCallback = nullptr, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

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
	void Signal(FString Channel, FString Message, FOnPubnubSignalResponse OnSignalResponse, FPubnubSignalSettings SignalSettings = FPubnubSignalSettings());

	/**
	 * Sends a signal to a specified channel.
	 * 
	 * @param Channel The ID of the channel to send the signal to.
	 * @param Message The message to send as the signal. This message can be any data type that can be serialized into JSON.
	 * @param NativeCallback Optional delegate to listen for the signal result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if signal result is not needed.
	 * @param SignalSettings Optional settings for the signal operation. See FPubnubSignalSettings for more details.
	 */
	void Signal(FString Channel, FString Message, FOnPubnubSignalResponseNative NativeCallback = nullptr, FPubnubSignalSettings SignalSettings = FPubnubSignalSettings());
	
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
	void SubscribeToChannel(FString Channel, FOnPubnubSubscribeOperationResponse OnSubscribeToChannelResponse, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());

	/**
	 * Subscribes to a specified channel - start listening for messages on that channel.
	 * Use OnMessageReceived Callback to get those messages.
	 * 
	 * @param Channel The ID of the channel to subscribe to.
	 * @param NativeCallback Optional delegate to listen for the subscribe result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if subscribe result is not needed.
	 * @param SubscribeSettings Optional settings for the subscribe operation. See FPubnubSubscribeSettings for more details.
	 */
	void SubscribeToChannel(FString Channel, FOnPubnubSubscribeOperationResponseNative NativeCallback = nullptr, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());

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
	void SubscribeToGroup(FString ChannelGroup, FOnPubnubSubscribeOperationResponse OnSubscribeToGroupResponse, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());
	
	/**
	 * Subscribes to a specified group - start listening for messages on that group.
	 * Use OnMessageReceived Callback to get those messages.
	 * 
	 * @param ChannelGroup The name of the channel to subscribe to.
	 * @param NativeCallback Optional delegate to listen for the subscribe result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if subscribe result is not needed.
	 * @param SubscribeSettings Optional settings for the subscribe operation. See FPubnubSubscribeSettings for more details.
	 */
	void SubscribeToGroup(FString ChannelGroup, FOnPubnubSubscribeOperationResponseNative NativeCallback = nullptr, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());

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
	void UnsubscribeFromChannel(FString Channel, FOnPubnubSubscribeOperationResponse OnUnsubscribeFromChannelResponse);

	/**
	 * Unsubscribes from a specified channel - stop listening for messages on that channel.
	 * 
	 * @param Channel The ID of the channel to unsubscribe from.
	 * @param NativeCallback Optional delegate to listen for the unsubscribe result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if unsubscribe result is not needed.
	 */
	void UnsubscribeFromChannel(FString Channel, FOnPubnubSubscribeOperationResponseNative NativeCallback = nullptr);

	
	/**
	 * Unsubscribes from a specified group - stop listening for messages on that group.
	 * 
	 * @param ChannelGroup The name of the group to unsubscribe from.
	 * @param OnUnsubscribeFromGroupResponse Optional delegate to listen for the unsubscribe result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe", meta = (AutoCreateRefTerm = "OnUnsubscribeFromGroupResponse"))
	void UnsubscribeFromGroup(FString ChannelGroup, FOnPubnubSubscribeOperationResponse OnUnsubscribeFromGroupResponse);

	/**
	 * Unsubscribes from a specified group - stop listening for messages on that group.
	 * 
	 * @param ChannelGroup The name of the group to unsubscribe from.
 	 * @param NativeCallback Optional delegate to listen for the unsubscribe result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if unsubscribe result is not needed.
	 */
	void UnsubscribeFromGroup(FString ChannelGroup, FOnPubnubSubscribeOperationResponseNative NativeCallback = nullptr);

	
	/**
	 * Unsubscribes from all subscribed channels and groups - basically stop listening for any messages.
	 * NOTE:: This also unsubscribes all subscribed Subscription and SubscriptionSet Objects.
	 * 
	 * @param OnUnsubscribeFromAllResponse Optional delegate to listen for the unsubscribe result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe", meta = (AutoCreateRefTerm = "OnUnsubscribeFromAllResponse"))
	void UnsubscribeFromAll(FOnPubnubSubscribeOperationResponse OnUnsubscribeFromAllResponse);

	/**
	 * Unsubscribes from all subscribed channels and groups - basically stop listening for any messages.
	 * 
	 * @param NativeCallback Optional delegate to listen for the unsubscribe result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if unsubscribe result is not needed.
	 */
	void UnsubscribeFromAll(FOnPubnubSubscribeOperationResponseNative NativeCallback = nullptr);


	
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
	void AddChannelToGroup(FString Channel, FString ChannelGroup, FOnPubnubAddChannelToGroupResponse OnAddChannelToGroupResponse);

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
	void AddChannelToGroup(FString Channel, FString ChannelGroup, FOnPubnubAddChannelToGroupResponseNative NativeCallback = nullptr);

	
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
	void RemoveChannelFromGroup(FString Channel, FString ChannelGroup, FOnPubnubRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse);

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
	void RemoveChannelFromGroup(FString Channel, FString ChannelGroup, FOnPubnubRemoveChannelFromGroupResponseNative NativeCallback = nullptr);

	
	/**
	 * Lists the channels that belong to a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param ChannelGroup The name of the channel group to list channels from.
	 * @param OnListChannelsResponse The callback function used to handle the result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups")
	void ListChannelsFromGroup(FString ChannelGroup, FOnPubnubListChannelsFromGroupResponse OnListChannelsResponse);

	/**
	 * Lists the channels that belong to a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param ChannelGroup The name of the channel group to list channels from.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 */
	void ListChannelsFromGroup(FString ChannelGroup, FOnPubnubListChannelsFromGroupResponseNative NativeCallback);

	
	/**
	 * Removes a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param ChannelGroup The name of the channel group to remove.
	 * @param OnRemoveChannelGroupResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups", meta = (AutoCreateRefTerm = "OnRemoveChannelGroupResponse"))
	void RemoveChannelGroup(FString ChannelGroup, FOnPubnubRemoveChannelGroupResponse OnRemoveChannelGroupResponse);

	/**
	 * Removes a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param ChannelGroup The name of the channel group to remove.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if operation result is not needed.
	 */
	void RemoveChannelGroup(FString ChannelGroup, FOnPubnubRemoveChannelGroupResponseNative NativeCallback = nullptr);

	
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
	void ListUsersFromChannel(FString Channel, FOnPubnubListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());

	/**
	 * Lists the users currently present on a specified channel.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The ID of the channel to list users from.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param ListUsersFromChannelSettings Optional settings for the list users operation. See FPubnubListUsersFromChannelSettings for more details. 
	 */
	void ListUsersFromChannel(FString Channel, FOnPubnubListUsersFromChannelResponseNative NativeCallback, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());

	
	/**
	 * Lists the channels that a specified user is currently subscribed to.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 	
	 * @param UserID The user ID to list subscribed channels for.
	 * @param ListUserSubscribedChannelsResponse The callback function used to handle the result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void ListUserSubscribedChannels(FString UserID, FOnPubnubListUsersSubscribedChannelsResponse ListUserSubscribedChannelsResponse);
	
	/**
	 * Lists the channels that a specified user is currently subscribed to.
	 *
	 * @Note Requires the *Presence* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 	
	 * @param UserID The user ID to list subscribed channels for.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 */
	void ListUserSubscribedChannels(FString UserID, FOnPubnubListUsersSubscribedChannelsResponseNative NativeCallback);

	
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
	void SetState(FString Channel, FString StateJson, FOnPubnubSetStateResponse OnSetStateResponse, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());

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
	void SetState(FString Channel, FString StateJson, FOnPubnubSetStateResponseNative NativeCallback = nullptr, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());

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
	void GetState(FString Channel, FString ChannelGroup, FString UserID, FOnPubnubGetStateResponse OnGetStateResponse);

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
	void GetState(FString Channel, FString ChannelGroup, FString UserID, FOnPubnubGetStateResponseNative NativeCallback);

	
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
	void GrantToken(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FOnPubnubGrantTokenResponse OnGrantTokenResponse, FString Meta = "");

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
	void GrantToken(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FOnPubnubGrantTokenResponseNative NativeCallback, FString Meta = "");

	
	/**
	 * Revokes a previously granted access token.
	 * 
	 * @Note Requires the *Revoke v3 Token* in *Access Manager* section to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Token The access token to revoke.
	 * @param OnRevokeTokenResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Access Manager", meta = (AutoCreateRefTerm = "OnRevokeTokenResponse"))
	void RevokeToken(FString Token, FOnPubnubRevokeTokenResponse OnRevokeTokenResponse);

	/**
	 * Revokes a previously granted access token.
	 * 
	 * @Note Requires the *Revoke v3 Token* in *Access Manager* section to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Token The access token to revoke.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 */
	void RevokeToken(FString Token, FOnPubnubRevokeTokenResponseNative NativeCallback = nullptr);

	
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
	void FetchHistory(FString Channel, FOnPubnubFetchHistoryResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());

	/**
	 * Fetches historical messages from a specified channel using Message Persistence.
	 * 
	 * @Note Requires the *Message Persistence* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The ID of the channel to fetch messages from.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param FetchHistorySettings Optional settings for the fetch history operation. See FPubnubFetchHistorySettings for more details.
	 */
	void FetchHistory(FString Channel, FOnPubnubFetchHistoryResponseNative NativeCallback, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());

	
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
	void DeleteMessages(FString Channel, FOnPubnubDeleteMessagesResponse OnDeleteMessagesResponse, FPubnubDeleteMessagesSettings DeleteMessagesSettings = FPubnubDeleteMessagesSettings());

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
	void DeleteMessages(FString Channel, FOnPubnubDeleteMessagesResponseNative NativeCallback = nullptr, FPubnubDeleteMessagesSettings DeleteMessagesSettings = FPubnubDeleteMessagesSettings());

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
	void MessageCounts(FString Channel, FString Timetoken, FOnPubnubMessageCountsResponse OnMessageCountsResponse);

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
	void MessageCounts(FString Channel, FString Timetoken, FOnPubnubMessageCountsResponseNative NativeCallback);


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
	void GetAllUserMetadataRaw(FOnPubnubGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void GetAllUserMetadataRaw(FOnPubnubGetAllUserMetadataResponseNative NativeCallback, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void GetAllUserMetadata(FOnPubnubGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FPubnubGetAllInclude Include = FPubnubGetAllInclude(), int Limit = 100, FString Filter = "", FPubnubGetAllSort Sort = FPubnubGetAllSort(), FString PageNext = "", FString PagePrev = "");

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
	void GetAllUserMetadata(FOnPubnubGetAllUserMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include = FPubnubGetAllInclude(), int Limit = 100, FString Filter = "", FPubnubGetAllSort Sort = FPubnubGetAllSort(), FString PageNext = "", FString PagePrev = "");
	

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
	void SetUserMetadataRaw(FString User, FString UserMetadataObj, FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse, FString Include = "");

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
	void SetUserMetadataRaw(FString User, FString UserMetadataObj, FOnPubnubSetUserMetadataResponseNative NativeCallback = nullptr, FString Include = "");
	
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
	void SetUserMetadata(FString User, FPubnubUserData UserMetadata, FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

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
	void SetUserMetadata(FString User, FPubnubUserData UserMetadata, FOnPubnubSetUserMetadataResponseNative NativeCallback = nullptr, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	
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
	void GetUserMetadataRaw(FString User, FOnPubnubGetUserMetadataResponse OnGetUserMetadataResponse, FString Include = "");

	/**
	 * Retrieves metadata for a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to retrieve metadata.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	void GetUserMetadataRaw(FString User, FOnPubnubGetUserMetadataResponseNative NativeCallback, FString Include = "");

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
	void GetUserMetadata(FString User, FOnPubnubGetUserMetadataResponse OnGetUserMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Retrieves metadata for a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to retrieve metadata.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	void GetUserMetadata(FString User, FOnPubnubGetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());
	

	/**
	 * Removes all metadata associated with a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to remove metadata.
	 * @param OnRemoveUserMetadataResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnRemoveUserMetadataResponse"))
	void RemoveUserMetadata(FString User, FOnPubnubRemoveUserMetadataResponse OnRemoveUserMetadataResponse);

	/**
	 * Removes all metadata associated with a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to remove metadata.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 */
	void RemoveUserMetadata(FString User, FOnPubnubRemoveUserMetadataResponseNative NativeCallback = nullptr);

	
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
	void GetAllChannelMetadataRaw(FOnPubnubGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void GetAllChannelMetadataRaw(FOnPubnubGetAllChannelMetadataResponseNative NativeCallback, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void GetAllChannelMetadata(FOnPubnubGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FPubnubGetAllInclude Include = FPubnubGetAllInclude(), int Limit = 100, FString Filter = "", FPubnubGetAllSort Sort = FPubnubGetAllSort(), FString PageNext = "", FString PagePrev = "");

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
	void GetAllChannelMetadata(FOnPubnubGetAllChannelMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include = FPubnubGetAllInclude(), int Limit = 100, FString Filter = "", FPubnubGetAllSort Sort = FPubnubGetAllSort(), FString PageNext = "", FString PagePrev = "");



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
	void SetChannelMetadataRaw(FString Channel, FString ChannelMetadataObj, FOnPubnubSetChannelMetadataResponse OnSetChannelMetadataResponse, FString Include = "");

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
	void SetChannelMetadataRaw(FString Channel, FString ChannelMetadataObj, FOnPubnubSetChannelMetadataResponseNative NativeCallback = nullptr, FString Include = "");

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
	void SetChannelMetadata(FString Channel, FPubnubChannelData ChannelMetadata, FOnPubnubSetChannelMetadataResponse OnSetChannelMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

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
	void SetChannelMetadata(FString Channel, FPubnubChannelData ChannelMetadata, FOnPubnubSetChannelMetadataResponseNative NativeCallback = nullptr, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	
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
	void GetChannelMetadataRaw(FString Channel, FOnPubnubGetChannelMetadataResponse OnGetChannelMetadataResponse, FString Include = "");

	/**
	 * Retrieves metadata for a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to retrieve metadata.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	void GetChannelMetadataRaw(FString Channel, FOnPubnubGetChannelMetadataResponseNative NativeCallback, FString Include = "");

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
	void GetChannelMetadata(FString Channel, FOnPubnubGetChannelMetadataResponse OnGetChannelMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Retrieves metadata for a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to retrieve metadata.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	void GetChannelMetadata(FString Channel, FOnPubnubGetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	
	/**
	 * Removes all metadata associated with a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to remove metadata.
	 * @param OnRemoveChannelMetadataResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnRemoveChannelMetadataResponse"))
	void RemoveChannelMetadata(FString Channel, FOnPubnubRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse);

	/**
	 * Removes all metadata associated with a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to remove metadata.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 */
	void RemoveChannelMetadata(FString Channel, FOnPubnubRemoveChannelMetadataResponseNative NativeCallback = nullptr);

	
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
	void GetMembershipsRaw(FString User, FOnPubnubGetMembershipsResponse OnGetMembershipsResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void GetMembershipsRaw(FString User, FOnPubnubGetMembershipsResponseNative NativeCallback, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void GetMemberships(FString User, FOnPubnubGetMembershipsResponse OnGetMembershipsResponse, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");

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
	void GetMemberships(FString User, FOnPubnubGetMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");

	
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
	void SetMembershipsRaw(FString User, FString SetObj, FOnPubnubSetMembershipsResponse OnSetMembershipsResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void SetMembershipsRaw(FString User, FString SetObj, FOnPubnubSetMembershipsResponseNative NativeCallback = nullptr, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void SetMemberships(FString User, TArray<FPubnubMembershipInputData> Channels, FOnPubnubSetMembershipsResponse OnSetMembershipsResponse, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");
	
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
	void SetMemberships(FString User, TArray<FPubnubMembershipInputData> Channels, FOnPubnubSetMembershipsResponseNative NativeCallback = nullptr, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");

	
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
	void RemoveMembershipsRaw(FString User, FString RemoveObj, FOnPubnubRemoveMembershipsResponse OnRemoveMembershipsResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void RemoveMembershipsRaw(FString User, FString RemoveObj, FOnPubnubRemoveMembershipsResponseNative NativeCallback = nullptr, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void RemoveMemberships(FString User, TArray<FString> Channels, FOnPubnubRemoveMembershipsResponse OnRemoveMembershipsResponse, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");

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
	void RemoveMemberships(FString User, TArray<FString> Channels, FOnPubnubRemoveMembershipsResponseNative NativeCallback = nullptr, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");

	
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
	void GetChannelMembersRaw(FString Channel, FOnPubnubGetChannelMembersResponse OnGetMembersResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);
	
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
	void GetChannelMembersRaw(FString Channel, FOnPubnubGetChannelMembersResponseNative NativeCallback, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void GetChannelMembers(FString Channel, FOnPubnubGetChannelMembersResponse OnGetMembersResponse, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");
	
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
	void GetChannelMembers(FString Channel, FOnPubnubGetChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");

	
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
	void SetChannelMembersRaw(FString Channel, FString SetObj, FOnPubnubSetChannelMembersResponse OnSetChannelMembersResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void SetChannelMembersRaw(FString Channel, FString SetObj, FOnPubnubSetChannelMembersResponseNative NativeCallback = nullptr, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void SetChannelMembers(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FOnPubnubSetChannelMembersResponse OnSetChannelMembersResponse, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");
	
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
	void SetChannelMembers(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FOnPubnubSetChannelMembersResponseNative NativeCallback = nullptr, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");

	
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
	void RemoveChannelMembersRaw(FString Channel, FString RemoveObj, FOnPubnubRemoveChannelMembersResponse OnRemoveChannelMembersResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void RemoveChannelMembersRaw(FString Channel, FString RemoveObj, FOnPubnubRemoveChannelMembersResponseNative NativeCallback = nullptr, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void RemoveChannelMembers(FString Channel, TArray<FString> Users, FOnPubnubRemoveChannelMembersResponse OnRemoveChannelMembersResponse, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");
	
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
	void RemoveChannelMembers(FString Channel, TArray<FString> Users, FOnPubnubRemoveChannelMembersResponseNative NativeCallback = nullptr, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");



	/* MESSAGE ACTIONS API */
	
	/**
	 * Adds a message action to a specific message in a channel.
	 * 
	 * @param Channel The ID of the channel.
	 * @param MessageTimetoken The timetoken of the message to add the action to.
	 * @param ActionType The type of action to add.
	 * @param Value The value associated with the action.
	 * @param OnAddMessageActionResponse (Optional) The callback function used to handle the result and added MessageAction timetoken.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Message Actions", meta = (AutoCreateRefTerm = "OnAddMessageActionResponse"))
	void AddMessageAction(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnPubnubAddMessageActionResponse OnAddMessageActionResponse);

	/**
	 * Adds a message action to a specific message in a channel.
	 * 
	 * @param Channel The ID of the channel.
	 * @param MessageTimetoken The timetoken of the message to add the action to.
	 * @param ActionType The type of action to add.
	 * @param Value The value associated with the action.
	 * @param NativeCallback (Optional) The callback function used to handle the result and added MessageAction timetoken.
	 *						 Delegate in native form that can accept lambdas.
	 */
	void AddMessageAction(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnPubnubAddMessageActionResponseNative NativeCallback = nullptr);

	
	/**
	 * Retrieves message actions for a specified channel within a given time range.
	 * 
	 * @param Channel The ID of the channel.
	 * @param OnGetMessageActionsResponse The callback function used to handle the result.
	 * @param Start The starting timetoken for the range. Has to be greater (newer) than the End timetoken. 
	 * @param End The ending timetoken for the range.
	 * @param Limit The maximum number of actions to retrieve.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Message Actions")
	void GetMessageActions(FString Channel, FOnPubnubGetMessageActionsResponse OnGetMessageActionsResponse, FString Start = "", FString End = "", int Limit = 0);

	/**
	 * Retrieves message actions for a specified channel within a given time range.
	 * 
	 * @param Channel The ID of the channel.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * @param Start The starting timetoken for the range. Has to be greater (newer) than the End timetoken. 
	 * @param End The ending timetoken for the range.
	 * @param Limit The maximum number of actions to retrieve.
	 */
	void GetMessageActions(FString Channel, FOnPubnubGetMessageActionsResponseNative NativeCallback, FString Start = "", FString End = "", int Limit = 0);

	
	/**
	 * Removes a specific message action from a message in a channel.
	 * 
	 * @param Channel The ID of the channel.
	 * @param MessageTimetoken The timetoken of the message.
	 * @param ActionTimetoken The timetoken of the action to remove.
	 * @param OnRemoveMessageActionResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Message Actions", meta = (AutoCreateRefTerm = "OnRemoveMessageActionResponse"))
	void RemoveMessageAction(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnPubnubRemoveMessageActionResponse OnRemoveMessageActionResponse);
	
	/**
	 * Removes a specific message action from a message in a channel.
	 * 
	 * @param Channel The ID of the channel.
	 * @param MessageTimetoken The timetoken of the message.
	 * @param ActionTimetoken The timetoken of the action to remove.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 */
	void RemoveMessageAction(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnPubnubRemoveMessageActionResponseNative NativeCallback = nullptr);

		/**
	 * Tries to reconnect all active subscriptions. Could be used in case of receiving PSS_DisconnectedUnexpectedly or PSS_ConnectionError
	 * from OnSubscriptionStatusChanged listener. Or if DisconnectSubscriptions was called previously.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void ReconnectSubscriptions();

	/**
	 * Pauses all active subscriptions.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void DisconnectSubscriptions();

	/** Sets the provider-level crypto module to use for PubNub.
	 *
	 * Expects an object implementing IPubnubCryptoProviderInterface.
	 * Use UPubnubCryptoModule for default PubNub encryption implementation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Crypto")
	void SetCryptoModule(TScriptInterface<IPubnubCryptoProviderInterface> CryptoModule);

	/** Gets the currently configured provider-level crypto module.
	 *
	 * Returns the module previously set via SetCryptoModule.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Crypto")
	TScriptInterface<IPubnubCryptoProviderInterface> GetCryptoModule();

#pragma endregion

#pragma region ENTITIES

	/**
	 * Creates a PubNub Channel entity for the specified channel name.
	 * 
	 * The returned channel entity provides access to channel-specific operations
	 * such as publishing messages, sending signals, and managing presence information.
	 * It also allows creating subscriptions to receive real-time updates from the channel.
	 * 
	 * @param Channel The name of the channel to create an entity for.
	 * @return A new channel entity configured for the specified channel.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Entities")
	UPubnubChannelEntity* CreateChannelEntity(FString Channel);

	/**
	 * Creates a PubNub Channel Group entity for the specified channel group name.
	 * 
	 * The returned channel group entity provides access to channel group-specific operations
	 * such as adding/removing channels, listing channels in the group, and managing the
	 * group lifecycle. It also allows creating subscriptions to receive real-time updates
	 * from all channels in the group.
	 * 
	 * @param ChannelGroup The name of the channel group to create an entity for.
	 * @return A new channel group entity configured for the specified channel group.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Entities")
	UPubnubChannelGroupEntity* CreateChannelGroupEntity(FString ChannelGroup);
	
	/**
	 * Creates a PubNub Channel Metadata entity for the specified channel.
	 * 
	 * The returned channel metadata entity provides access to App Context operations
	 * for managing metadata associated with the specified channel. This includes
	 * setting, retrieving, and removing channel metadata information.
	 * 
	 * @note Requires the App Context add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param Channel The name of the channel to create a metadata entity for.
	 * @return A new channel metadata entity configured for the specified channel.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Entities")
	UPubnubChannelMetadataEntity* CreateChannelMetadataEntity(FString Channel);
	
	/**
	 * Creates a PubNub User Metadata entity for the specified user.
	 * 
	 * The returned user metadata entity provides access to App Context operations
	 * for managing metadata associated with the specified user. This includes
	 * setting, retrieving, and removing user metadata information.
	 * 
	 * @note Requires the App Context add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param User The user identifier to create a metadata entity for.
	 * @return A new user metadata entity configured for the specified user.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Entities")
	UPubnubUserMetadataEntity* CreateUserMetadataEntity(FString User);

	/**
	 * Creates a subscription set for multiple channels and channel groups.
	 * 
	 * The returned subscription set allows you to manage subscriptions to multiple
	 * entities as a single unit, enabling efficient subscribe/unsubscribe operations
	 * across all specified channels and channel groups simultaneously.
	 * 
	 * @note At least one Channel or ChannelGroup is needed to create SubscriptionSet.
	 * 
	 * @param Channels Array of channel names to include in the subscription set.
	 * @param ChannelGroups Array of channel group names to include in the subscription set.
	 * @param SubscriptionSettings Optional settings to configure the subscription behavior.
	 * @return A new subscription set configured for the specified channels and channel groups.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscriptions")
	UPubnubSubscriptionSet* CreateSubscriptionSet(TArray<FString> Channels, TArray<FString> ChannelGroups, FPubnubSubscribeSettings SubscriptionSettings = FPubnubSubscribeSettings());

	/**
	 * Creates a subscription set from an array of existing PubNub entities.
	 * 
	 * The returned subscription set allows you to manage subscriptions to multiple
	 * entities as a single unit. This is useful when you already have entity objects
	 * and want to group their subscriptions together.
	 * 
	 * @param Entities Array of PubNub entity objects to include in the subscription set.
	 * @param SubscriptionSettings Optional settings to configure the subscription behavior.
	 * @return A new subscription set configured for the specified entities.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscriptions")
	UPubnubSubscriptionSet* CreateSubscriptionSetFromEntities(TArray<UPubnubBaseEntity*> Entities, FPubnubSubscribeSettings SubscriptionSettings = FPubnubSubscribeSettings());

	/**
	 * Gets all currently active individual subscriptions.
	 * 
	 * Returns an array of all subscription objects that are currently active
	 * and receiving real-time updates from PubNub. This includes subscriptions
	 * created directly from entities, but excludes those managed by subscription sets.
	 * 
	 * @return Array of all active individual subscriptions.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscriptions")
	TArray<UPubnubSubscription*> GetActiveSubscriptions();

	/**
	 * Gets all currently active subscription sets.
	 * 
	 * Returns an array of all subscription set objects that are currently active
	 * and managing multiple subscriptions as a single unit. Each subscription set
	 * may contain multiple individual subscriptions.
	 * 
	 * @return Array of all active subscription sets.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscriptions")
	TArray<UPubnubSubscriptionSet*> GetActiveSubscriptionSets();

#pragma endregion 
	

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
	TArray<FOnPubnubSubscribeOperationResponseNative> SubscriptionResultDelegates;

	void OnCCoreSubscriptionStatusReceived(int StatusEnum, const void* StatusData);

#pragma endregion
	
	//Returns FString from the pubnub_get response
	FString GetLastResponse(pubnub_t* context);
	
	//Returns FString from the pubnub_get_channel response
	FString GetLastChannelResponse(pubnub_t* context);

	//TODO:: Move these functions to the logger
	void PubnubError(FString ErrorMessage, EPubnubErrorType ErrorType = EPubnubErrorType::PET_Error);
	void PubnubResponseError(int PubnubResponse, FString ErrorMessage);

	
	void InitPubnub_priv(const FPubnubConfig& Config);
	void SetUserID_priv(FString UserID);
	FString GetUserID_priv();
	void SetSecretKey_priv();
	void PublishMessage_priv(FString Channel, FString Message, FOnPubnubPublishMessageResponseNative OnPublishMessageResponse, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());
	void Signal_priv(FString Channel, FString Message, FOnPubnubSignalResponseNative OnSignalResponse, FPubnubSignalSettings SignalSettings = FPubnubSignalSettings());
	void SubscribeToChannel_priv(FString Channel, FOnPubnubSubscribeOperationResponseNative OnSubscribeToChannelResponse, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());
	void SubscribeToGroup_priv(FString ChannelGroup, FOnPubnubSubscribeOperationResponseNative OnSubscribeToGroupResponse, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());
	void UnsubscribeFromChannel_priv(FString Channel, FOnPubnubSubscribeOperationResponseNative OnUnsubscribeFromChannelResponse);
	void UnsubscribeFromGroup_priv(FString ChannelGroup, FOnPubnubSubscribeOperationResponseNative OnUnsubscribeFromGroupResponse);
	void UnsubscribeFromAll_priv(FOnPubnubSubscribeOperationResponseNative OnUnsubscribeFromAllResponse = nullptr);
	void AddChannelToGroup_priv(FString Channel, FString ChannelGroup, FOnPubnubAddChannelToGroupResponseNative OnAddChannelToGroupResponse);
	void RemoveChannelFromGroup_priv(FString Channel, FString ChannelGroup, FOnPubnubRemoveChannelFromGroupResponseNative OnRemoveChannelFromGroupResponse);
	void ListChannelsFromGroup_priv(FString ChannelGroup, FOnPubnubListChannelsFromGroupResponseNative OnListChannelsResponse);
	void RemoveChannelGroup_priv(FString ChannelGroup, FOnPubnubRemoveChannelGroupResponseNative OnRemoveChannelGroupResponse);
	void ListUsersFromChannel_priv(FString Channel, FOnPubnubListUsersFromChannelResponseNative ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());
	void ListUserSubscribedChannels_priv(FString UserID, FOnPubnubListUsersSubscribedChannelsResponseNative ListUserSubscribedChannelsResponse);
	void SetState_priv(FString Channel, FString StateJson, FOnPubnubSetStateResponseNative OnSetStateResponse, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());
	void GetState_priv(FString Channel, FString ChannelGroup, FString UserID, FOnPubnubGetStateResponseNative OnGetStateResponse);
	void Heartbeat_priv(FString Channel, FString ChannelGroup);
	void GrantToken_priv(FString PermissionObject, FOnPubnubGrantTokenResponseNative OnGrantTokenResponse);
	void RevokeToken_priv(FString Token, FOnPubnubRevokeTokenResponseNative OnRevokeTokenResponse);
	FString ParseToken_priv(FString Token);
	void SetAuthToken_priv(FString Token);
	void FetchHistory_priv(FString Channel, FOnPubnubFetchHistoryResponseNative OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());
	void DeleteMessages_priv(FString Channel, FOnPubnubDeleteMessagesResponseNative OnDeleteMessagesResponse, FPubnubDeleteMessagesSettings DeleteMessagesSettings);
	void MessageCounts_priv(FString Channel, FString Timetoken, FOnPubnubMessageCountsResponseNative OnMessageCountsResponse);
	void GetAllUserMetadata_priv(FOnPubnubGetAllUserMetadataResponseNative OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetUserMetadata_priv(FString User, FString UserMetadataObj, FOnPubnubSetUserMetadataResponseNative OnSetUserMetadataResponse, FString Include);
	void GetUserMetadata_priv(FString User, FOnPubnubGetUserMetadataResponseNative OnGetUserMetadataResponse, FString Include);
	void RemoveUserMetadata_priv(FString User, FOnPubnubRemoveUserMetadataResponseNative OnRemoveUserMetadataResponse);
	void GetAllChannelMetadata_priv(FOnPubnubGetAllChannelMetadataResponseNative OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetChannelMetadata_priv(FString Channel, FString ChannelMetadataObj, FOnPubnubSetChannelMetadataResponseNative OnSetChannelMetadataResponse, FString Include);
	void GetChannelMetadata_priv(FString Channel, FOnPubnubGetChannelMetadataResponseNative OnGetChannelMetadataResponse, FString Include);
	void RemoveChannelMetadata_priv(FString Channel, FOnPubnubRemoveChannelMetadataResponseNative OnRemoveChannelMetadataResponse);
	void GetMemberships_priv(FString User, FOnPubnubGetMembershipsResponseNative OnGetMembershipsResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetMemberships_priv(FString User, FString SetObj, FOnPubnubSetMembershipsResponseNative OnSetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void RemoveMemberships_priv(FString User, FString RemoveObj, FOnPubnubRemoveMembershipsResponseNative OnRemoveMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void GetChannelMembers_priv(FString Channel, FOnPubnubGetChannelMembersResponseNative OnGetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetChannelMembers_priv(FString Channel, FString SetObj, FOnPubnubSetChannelMembersResponseNative OnSetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void RemoveChannelMembers_priv(FString Channel, FString RemoveObj, FOnPubnubRemoveChannelMembersResponseNative OnRemoveMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void AddMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnPubnubAddMessageActionResponseNative AddMessageActionResponse);
	void RemoveMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnPubnubRemoveMessageActionResponseNative OnRemoveMessageActionResponse);
	void GetMessageActions_priv(FString Channel, FOnPubnubGetMessageActionsResponseNative OnGetMessageActionsResponse, FString Start, FString End, int Limit);

	void SubscribeWithSubscription(UPubnubSubscription* Subscription, FPubnubSubscriptionCursor Cursor, FOnPubnubSubscribeOperationResponseNative OnSubscribeResponse);
	void SubscribeWithSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet, FPubnubSubscriptionCursor Cursor, FOnPubnubSubscribeOperationResponseNative OnSubscribeResponse);
	void UnsubscribeWithSubscription(UPubnubSubscription* Subscription, FOnPubnubSubscribeOperationResponseNative OnUnsubscribeResponse);
	void UnsubscribeWithSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet, FOnPubnubSubscribeOperationResponseNative OnUnsubscribeResponse);
};

