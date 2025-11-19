// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubNub.h"
#include "PubnubStructLibrary.h"
#include "PubnubEnumLibrary.h"
#include "Crypto/PubnubCryptorInterface.h"
#include "Crypto/PubnubCryptoBridge.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PubnubSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PubnubLog, Log, All);

constexpr int PUBNUB_MAX_LIMIT = 100;

class FJsonObject;
class UPubnubSettings;
class FPubnubFunctionThread;
class UPubnubChatSystem;
class UPubnubAesCryptor;
class UPubnubBaseEntity;
class UPubnubChannelEntity;
class UPubnubChannelGroupEntity;
class UPubnubChannelMetadataEntity;
class UPubnubUserMetadataEntity;
class UPubnubSubscriptionSet;
class UPubnubClient;

struct CCoreSubscriptionData
{
	pubnub_subscribe_message_callback_t Callback;
	pubnub_subscription_t* Subscription;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPubnubSubsystemDeinitialized);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageReceived, FPubnubMessageData, Message);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMessageReceivedNative, const FPubnubMessageData& Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPubnubError, FString, ErrorMessage, EPubnubErrorType, ErrorType);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPubnubErrorNative, FString ErrorMessage, EPubnubErrorType ErrorType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSubscriptionStatusChanged, EPubnubSubscriptionStatus, Status, FPubnubSubscriptionStatusData, StatusData);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSubscriptionStatusChangedNative, EPubnubSubscriptionStatus Status, const FPubnubSubscriptionStatusData& StatusData);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPublishMessageResponse, FPubnubOperationResult, Result, FPubnubMessageData, PublishedMessage);
DECLARE_DELEGATE_TwoParams(FOnPublishMessageResponseNative, const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnSignalResponse, FPubnubOperationResult, Result, FPubnubMessageData, SignalMessage);
DECLARE_DELEGATE_TwoParams(FOnSignalResponseNative, const FPubnubOperationResult& Result, const FPubnubMessageData& SignalMessage);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSubscribeOperationResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnSubscribeOperationResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAddChannelToGroupResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnAddChannelToGroupResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnRemoveChannelFromGroupResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnRemoveChannelFromGroupResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnListChannelsFromGroupResponse, FPubnubOperationResult, Result, const TArray<FString>&, Channels);
DECLARE_DELEGATE_TwoParams(FOnListChannelsFromGroupResponseNative, const FPubnubOperationResult& Result, const TArray<FString>& Channels);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnRemoveChannelGroupResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnRemoveChannelGroupResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnListUsersSubscribedChannelsResponse, FPubnubOperationResult, Result, const TArray<FString>&, Channels);
DECLARE_DELEGATE_TwoParams(FOnListUsersSubscribedChannelsResponseNative, const FPubnubOperationResult& Result, const TArray<FString>& Channels);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnListUsersFromChannelResponse, FPubnubOperationResult, Result, FPubnubListUsersFromChannelWrapper, Data);
DECLARE_DELEGATE_TwoParams(FOnListUsersFromChannelResponseNative, const FPubnubOperationResult& Result, const FPubnubListUsersFromChannelWrapper& Data);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetStateResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnSetStateResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGetStateResponse, FPubnubOperationResult, Result, FString, StateResponse);
DECLARE_DELEGATE_TwoParams(FOnGetStateResponseNative, const FPubnubOperationResult& Result, FString StateResponse);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGrantTokenResponse, FPubnubOperationResult, Result, FString, Token);
DECLARE_DELEGATE_TwoParams(FOnGrantTokenResponseNative, const FPubnubOperationResult& Result, FString Token);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnRevokeTokenResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnRevokeTokenResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnFetchHistoryResponse, FPubnubOperationResult, Result, const TArray<FPubnubHistoryMessageData>&, Messages);
DECLARE_DELEGATE_TwoParams(FOnFetchHistoryResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubHistoryMessageData>& Messages);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnMessageCountsResponse, FPubnubOperationResult, Result, int, MessageCounts);
DECLARE_DELEGATE_TwoParams(FOnMessageCountsResponseNative, const FPubnubOperationResult& Result, int MessageCounts);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDeleteMessagesResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnDeleteMessagesResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnGetAllUserMetadataResponse, FPubnubOperationResult, Result, const TArray<FPubnubUserData>&, UsersData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnGetAllUserMetadataResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubUserData>& UsersData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGetUserMetadataResponse, FPubnubOperationResult, Result, FPubnubUserData, UserData);
DECLARE_DELEGATE_TwoParams(FOnGetUserMetadataResponseNative, const FPubnubOperationResult& Result, const FPubnubUserData& UserData);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnSetUserMetadataResponse, FPubnubOperationResult, Result, FPubnubUserData, UserData);
DECLARE_DELEGATE_TwoParams(FOnSetUserMetadataResponseNative, const FPubnubOperationResult& Result, const FPubnubUserData& UserData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnRemoveUserMetadataResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnRemoveUserMetadataResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnGetAllChannelMetadataResponse, FPubnubOperationResult, Result, const TArray<FPubnubChannelData>&, ChannelsData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnGetAllChannelMetadataResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubChannelData>& ChannelsData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGetChannelMetadataResponse, FPubnubOperationResult, Result, FPubnubChannelData, ChannelData);
DECLARE_DELEGATE_TwoParams(FOnGetChannelMetadataResponseNative, const FPubnubOperationResult& Result, const FPubnubChannelData& ChannelData);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnSetChannelMetadataResponse, FPubnubOperationResult, Result, FPubnubChannelData, ChannelData);
DECLARE_DELEGATE_TwoParams(FOnSetChannelMetadataResponseNative, const FPubnubOperationResult& Result, const FPubnubChannelData& ChannelData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnRemoveChannelMetadataResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnRemoveChannelMetadataResponseNative, const FPubnubOperationResult& Result);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnGetMembershipsResponse, FPubnubOperationResult, Result, const TArray<FPubnubMembershipData>&, MembershipsData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnGetMembershipsResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnSetMembershipsResponse, FPubnubOperationResult, Result, const TArray<FPubnubMembershipData>&, MembershipsData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnSetMembershipsResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnRemoveMembershipsResponse, FPubnubOperationResult, Result, const TArray<FPubnubMembershipData>&, MembershipsData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnRemoveMembershipsResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnGetChannelMembersResponse, FPubnubOperationResult, Result, const TArray<FPubnubChannelMemberData>&, MembersData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnGetChannelMembersResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnSetChannelMembersResponse, FPubnubOperationResult, Result, const TArray<FPubnubChannelMemberData>&, MembersData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnSetChannelMembersResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnRemoveChannelMembersResponse, FPubnubOperationResult, Result, const TArray<FPubnubChannelMemberData>&, MembersData, FString, PageNext, FString, PagePrev);
DECLARE_DELEGATE_FourParams(FOnRemoveChannelMembersResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGetMessageActionsResponse, FPubnubOperationResult, Result, const TArray<FPubnubMessageActionData>&, MessageActions);
DECLARE_DELEGATE_TwoParams(FOnGetMessageActionsResponseNative, const FPubnubOperationResult& Result, const TArray<FPubnubMessageActionData>& MessageActions);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnAddMessageActionResponse, FPubnubOperationResult, Result, FPubnubMessageActionData, MessageActionData);
DECLARE_DELEGATE_TwoParams(FOnAddMessageActionResponseNative, const FPubnubOperationResult& Result, const FPubnubMessageActionData& MessageActionData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnRemoveMessageActionResponse, FPubnubOperationResult, Result);
DECLARE_DELEGATE_OneParam(FOnRemoveMessageActionResponseNative, const FPubnubOperationResult& Result);


UCLASS()
class PUBNUBLIBRARY_API UPubnubSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	friend class UPubnubSubscription;
	friend class UPubnubSubscriptionSet;
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**Delegate that is called after PubnubSubsystem is deinitialized*/
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FOnPubnubSubsystemDeinitialized OnPubnubSubsystemDeinitialized;

	/**Global listener for all messages received on subscribed channels*/
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FOnMessageReceived OnMessageReceived;

	/**Global listener for all messages received on subscribed channels, equivalent that accepts lambdas*/
	FOnMessageReceivedNative OnMessageReceivedNative;

	/**Listener to react for all Errors in Pubnub functions */
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FOnPubnubError OnPubnubError;
	/**Listener to react for all Errors in Pubnub functions, equivalent that accepts lambdas*/
	FOnPubnubErrorNative OnPubnubErrorNative;

	/**Listener to react for subscription status changed */
	UPROPERTY(BlueprintAssignable, Category = "Pubnub|Delegates")
	FOnSubscriptionStatusChanged OnSubscriptionStatusChanged;

	/**Listener to react for subscription status changed , equivalent that accepts lambdas*/
	FOnSubscriptionStatusChangedNative OnSubscriptionStatusChangedNative;

#pragma region PUBNUB CLIENT

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Client")
	UPubnubClient* CreatePubnubClient(FPubnubConfig Config, FString DebugName = "");

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Client")
	UPubnubClient* GetPubnubClient(int ClientID);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Client")
	bool DestroyPubnubClient(UPubnubClient* ClientToDestroy);


#pragma endregion

#pragma region BLUEPRINT EXPOSED

	
	/* BLUEPRINT EXPOSED FUNCTIONS */
	//These functions don't have actual logic, they just call corresponding private functions on Pubnub threads

	/**
	 * Initializes PubNub systems with data provided in plug settings. Needs to be called before starting using any other PubNub features.
	 * Don't call it manually if "InitializeAutomatically" in plugin settings is set to true.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Init")
	void InitPubnub();

	/**
	 * Initializes PubNub systems with provided Config. Needs to be called before starting using any other PubNub features.
	 * Don't call it manually if "InitializeAutomatically" in plugin settings is set to true.
	 *
	 * @param Config Configuration settings for the PubNub Systems
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Init")
	void InitPubnubWithConfig(FPubnubConfig Config);

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
	 * @param OnPublishMessageResponse Optional delegate to listen for the publish result.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Publish", meta = (AutoCreateRefTerm = "OnPublishMessageResponse"))
	void PublishMessage(FString Channel, FString Message, FOnPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

	/**
	 * Publishes a message to a specified channel.
	 * 
	 * @param Channel The ID of the channel to publish the message to.
	 * @param Message The message to publish. This message can be any data type that can be serialized into JSON.
	 * @param NativeCallback Optional delegate to listen for the publish result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if publish result is not needed.
	 * @param PublishSettings Optional settings for the publish operation. See FPubnubPublishSettings for more details.
	 */
	void PublishMessage(FString Channel, FString Message, FOnPublishMessageResponseNative NativeCallback = nullptr, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

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
	void Signal(FString Channel, FString Message, FOnSignalResponse OnSignalResponse, FPubnubSignalSettings SignalSettings = FPubnubSignalSettings());

	/**
	 * Sends a signal to a specified channel.
	 * 
	 * @param Channel The ID of the channel to send the signal to.
	 * @param Message The message to send as the signal. This message can be any data type that can be serialized into JSON.
	 * @param NativeCallback Optional delegate to listen for the signal result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if signal result is not needed.
	 * @param SignalSettings Optional settings for the signal operation. See FPubnubSignalSettings for more details.
	 */
	void Signal(FString Channel, FString Message, FOnSignalResponseNative NativeCallback = nullptr, FPubnubSignalSettings SignalSettings = FPubnubSignalSettings());
	
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
	void SubscribeToChannel(FString Channel, FOnSubscribeOperationResponse OnSubscribeToChannelResponse, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());

	/**
	 * Subscribes to a specified channel - start listening for messages on that channel.
	 * Use OnMessageReceived Callback to get those messages.
	 * 
	 * @param Channel The ID of the channel to subscribe to.
	 * @param NativeCallback Optional delegate to listen for the subscribe result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if subscribe result is not needed.
	 * @param SubscribeSettings Optional settings for the subscribe operation. See FPubnubSubscribeSettings for more details.
	 */
	void SubscribeToChannel(FString Channel, FOnSubscribeOperationResponseNative NativeCallback = nullptr, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());

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
	void SubscribeToGroup(FString ChannelGroup, FOnSubscribeOperationResponse OnSubscribeToGroupResponse, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());

	/**
	 * Subscribes to a specified group - start listening for messages on that group.
	 * Use OnMessageReceived Callback to get those messages.
	 * 
	 * @param ChannelGroup The name of the channel to subscribe to.
	 * @param NativeCallback Optional delegate to listen for the subscribe result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if subscribe result is not needed.
	 * @param SubscribeSettings Optional settings for the subscribe operation. See FPubnubSubscribeSettings for more details.
	 */
	void SubscribeToGroup(FString ChannelGroup, FOnSubscribeOperationResponseNative NativeCallback = nullptr, FPubnubSubscribeSettings SubscribeSettings = FPubnubSubscribeSettings());

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
	void UnsubscribeFromChannel(FString Channel, FOnSubscribeOperationResponse OnUnsubscribeFromChannelResponse);

	/**
	 * Unsubscribes from a specified channel - stop listening for messages on that channel.
	 * 
	 * @param Channel The ID of the channel to unsubscribe from.
	 * @param NativeCallback Optional delegate to listen for the unsubscribe result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if unsubscribe result is not needed.
	 */
	void UnsubscribeFromChannel(FString Channel, FOnSubscribeOperationResponseNative NativeCallback = nullptr);

	/**
	 * Unsubscribes from a specified group - stop listening for messages on that group.
	 * 
	 * @param ChannelGroup The name of the group to unsubscribe from.
	 * @param OnUnsubscribeFromGroupResponse Optional delegate to listen for the unsubscribe result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe", meta = (AutoCreateRefTerm = "OnUnsubscribeFromGroupResponse"))
	void UnsubscribeFromGroup(FString ChannelGroup, FOnSubscribeOperationResponse OnUnsubscribeFromGroupResponse);

	/**
	 * Unsubscribes from a specified group - stop listening for messages on that group.
	 * 
	 * @param ChannelGroup The name of the group to unsubscribe from.
 	 * @param NativeCallback Optional delegate to listen for the unsubscribe result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if unsubscribe result is not needed.
	 */
	void UnsubscribeFromGroup(FString ChannelGroup, FOnSubscribeOperationResponseNative NativeCallback = nullptr);

	/**
	 * Unsubscribes from all subscribed channels and groups - basically stop listening for any messages.
	 * NOTE:: This also unsubscribes all subscribed Subscription and SubscriptionSet Objects.
	 * 
	 * @param OnUnsubscribeFromAllResponse Optional delegate to listen for the unsubscribe result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe", meta = (AutoCreateRefTerm = "OnUnsubscribeFromAllResponse"))
	void UnsubscribeFromAll(FOnSubscribeOperationResponse OnUnsubscribeFromAllResponse);

	/**
	 * Unsubscribes from all subscribed channels and groups - basically stop listening for any messages.
	 * 
	 * @param NativeCallback Optional delegate to listen for the unsubscribe result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if unsubscribe result is not needed.
	 */
	void UnsubscribeFromAll(FOnSubscribeOperationResponseNative NativeCallback = nullptr);

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
	void AddChannelToGroup(FString Channel, FString ChannelGroup, FOnAddChannelToGroupResponse OnAddChannelToGroupResponse);

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
	void AddChannelToGroup(FString Channel, FString ChannelGroup, FOnAddChannelToGroupResponseNative NativeCallback = nullptr);

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
	void RemoveChannelFromGroup(FString Channel, FString ChannelGroup, FOnRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse);

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
	void RemoveChannelFromGroup(FString Channel, FString ChannelGroup, FOnRemoveChannelFromGroupResponseNative NativeCallback = nullptr);

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
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 */
	void ListChannelsFromGroup(FString ChannelGroup, FOnListChannelsFromGroupResponseNative NativeCallback);



	/**
	 * Removes a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param ChannelGroup The name of the channel group to remove.
	 * @param OnRemoveChannelGroupResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channel Groups", meta = (AutoCreateRefTerm = "OnRemoveChannelGroupResponse"))
	void RemoveChannelGroup(FString ChannelGroup, FOnRemoveChannelGroupResponse OnRemoveChannelGroupResponse);

	/**
	 * Removes a specified channel group.
	 * 
	 * @Note Requires the *Stream Controller* add-on to be enabled for your key in the PubNub Admin Portal.
	 * 
	 * @param ChannelGroup The name of the channel group to remove.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 *						 Can be skipped if operation result is not needed.
	 */
	void RemoveChannelGroup(FString ChannelGroup, FOnRemoveChannelGroupResponseNative NativeCallback = nullptr);

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
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param ListUsersFromChannelSettings Optional settings for the list users operation. See FPubnubListUsersFromChannelSettings for more details. 
	 */
	void ListUsersFromChannel(FString Channel, FOnListUsersFromChannelResponseNative NativeCallback, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());

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
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 */
	void ListUserSubscribedChannels(FString UserID, FOnListUsersSubscribedChannelsResponseNative NativeCallback);



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
	void SetState(FString Channel, FString StateJson, FOnSetStateResponse OnSetStateResponse, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());

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
	void SetState(FString Channel, FString StateJson, FOnSetStateResponseNative NativeCallback = nullptr, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());

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
	void GetState(FString Channel, FString ChannelGroup, FString UserID, FOnGetStateResponse OnGetStateResponse);

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
	void GetState(FString Channel, FString ChannelGroup, FString UserID, FOnGetStateResponseNative NativeCallback);

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
	void GrantToken(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FOnGrantTokenResponse OnGrantTokenResponse, FString Meta = "");

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
	void GrantToken(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FOnGrantTokenResponseNative NativeCallback, FString Meta = "");
	
	/**
	 * Revokes a previously granted access token.
	 * 
	 * @Note Requires the *Revoke v3 Token* in *Access Manager* section to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Token The access token to revoke.
	 * @param OnRevokeTokenResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Access Manager", meta = (AutoCreateRefTerm = "OnRevokeTokenResponse"))
	void RevokeToken(FString Token, FOnRevokeTokenResponse OnRevokeTokenResponse);

	/**
	 * Revokes a previously granted access token.
	 * 
	 * @Note Requires the *Revoke v3 Token* in *Access Manager* section to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Token The access token to revoke.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 */
	void RevokeToken(FString Token, FOnRevokeTokenResponseNative NativeCallback = nullptr);

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
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param FetchHistorySettings Optional settings for the fetch history operation. See FPubnubFetchHistorySettings for more details.
	 */
	void FetchHistory(FString Channel, FOnFetchHistoryResponseNative NativeCallback, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());


	
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
	void DeleteMessages(FString Channel, FOnDeleteMessagesResponse OnDeleteMessagesResponse, FPubnubDeleteMessagesSettings DeleteMessagesSettings = FPubnubDeleteMessagesSettings());

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
	void DeleteMessages(FString Channel, FOnDeleteMessagesResponseNative NativeCallback = nullptr, FPubnubDeleteMessagesSettings DeleteMessagesSettings = FPubnubDeleteMessagesSettings());

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
	void MessageCounts(FString Channel, FString Timetoken, FOnMessageCountsResponse OnMessageCountsResponse);

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
	void MessageCounts(FString Channel, FString Timetoken, FOnMessageCountsResponseNative NativeCallback);

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
	void GetAllUserMetadataRaw(FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void GetAllUserMetadataRaw(FOnGetAllUserMetadataResponseNative NativeCallback, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void GetAllUserMetadata(FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FPubnubGetAllInclude Include = FPubnubGetAllInclude(), int Limit = 100, FString Filter = "", FPubnubGetAllSort Sort = FPubnubGetAllSort(), FString PageNext = "", FString PagePrev = "");

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
	void GetAllUserMetadata(FOnGetAllUserMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include = FPubnubGetAllInclude(), int Limit = 100, FString Filter = "", FPubnubGetAllSort Sort = FPubnubGetAllSort(), FString PageNext = "", FString PagePrev = "");
	


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
	void SetUserMetadataRaw(FString User, FString UserMetadataObj, FOnSetUserMetadataResponse OnSetUserMetadataResponse, FString Include = "");

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
	void SetUserMetadataRaw(FString User, FString UserMetadataObj, FOnSetUserMetadataResponseNative NativeCallback = nullptr, FString Include = "");
	
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
	void SetUserMetadata(FString User, FPubnubUserData UserMetadata, FOnSetUserMetadataResponse OnSetUserMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

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
	void SetUserMetadata(FString User, FPubnubUserData UserMetadata, FOnSetUserMetadataResponseNative NativeCallback = nullptr, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	
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
	void GetUserMetadataRaw(FString User, FOnGetUserMetadataResponse OnGetUserMetadataResponse, FString Include = "");

	/**
	 * Retrieves metadata for a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to retrieve metadata.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	void GetUserMetadataRaw(FString User, FOnGetUserMetadataResponseNative NativeCallback, FString Include = "");

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
	void GetUserMetadata(FString User, FOnGetUserMetadataResponse OnGetUserMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Retrieves metadata for a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to retrieve metadata.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	void GetUserMetadata(FString User, FOnGetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());
	

	/**
	 * Removes all metadata associated with a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to remove metadata.
	 * @param OnRemoveUserMetadataResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnRemoveUserMetadataResponse"))
	void RemoveUserMetadata(FString User, FOnRemoveUserMetadataResponse OnRemoveUserMetadataResponse);

	/**
	 * Removes all metadata associated with a specified User from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param User The user ID for whom to remove metadata.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 */
	void RemoveUserMetadata(FString User, FOnRemoveUserMetadataResponseNative NativeCallback = nullptr);

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
	void GetAllChannelMetadataRaw(FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void GetAllChannelMetadataRaw(FOnGetAllChannelMetadataResponseNative NativeCallback, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void GetAllChannelMetadata(FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FPubnubGetAllInclude Include = FPubnubGetAllInclude(), int Limit = 100, FString Filter = "", FPubnubGetAllSort Sort = FPubnubGetAllSort(), FString PageNext = "", FString PagePrev = "");

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
	void GetAllChannelMetadata(FOnGetAllChannelMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include = FPubnubGetAllInclude(), int Limit = 100, FString Filter = "", FPubnubGetAllSort Sort = FPubnubGetAllSort(), FString PageNext = "", FString PagePrev = "");



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
	void SetChannelMetadataRaw(FString Channel, FString ChannelMetadataObj, FOnSetChannelMetadataResponse OnSetChannelMetadataResponse, FString Include = "");

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
	void SetChannelMetadataRaw(FString Channel, FString ChannelMetadataObj, FOnSetChannelMetadataResponseNative NativeCallback = nullptr, FString Include = "");

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
	void SetChannelMetadata(FString Channel, FPubnubChannelData ChannelMetadata, FOnSetChannelMetadataResponse OnSetChannelMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

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
	void SetChannelMetadata(FString Channel, FPubnubChannelData ChannelMetadata, FOnSetChannelMetadataResponseNative NativeCallback = nullptr, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

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
	void GetChannelMetadataRaw(FString Channel, FOnGetChannelMetadataResponse OnGetChannelMetadataResponse, FString Include = "");

	/**
	 * Retrieves metadata for a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to retrieve metadata.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) A comma-separated list of property names to include in the response.
	 */
	void GetChannelMetadataRaw(FString Channel, FOnGetChannelMetadataResponseNative NativeCallback, FString Include = "");

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
	void GetChannelMetadata(FString Channel, FOnGetChannelMetadataResponse OnGetChannelMetadataResponse, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());

	/**
	 * Retrieves metadata for a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to retrieve metadata.
	 * @param NativeCallback The callback function used to handle the result. Delegate in native form that can accept lambdas.
	 * @param Include (Optional) List of property names to include in the response.
	 */
	void GetChannelMetadata(FString Channel, FOnGetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include = FPubnubGetMetadataInclude());
	
	/**
	 * Removes all metadata associated with a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to remove metadata.
	 * @param OnRemoveChannelMetadataResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|App Context", meta = (AutoCreateRefTerm = "OnRemoveChannelMetadataResponse"))
	void RemoveChannelMetadata(FString Channel, FOnRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse);

	/**
	 * Removes all metadata associated with a specified Channel from the PubNub App Context.
	 * 
	 * @Note Requires the *App Context* add-on to be enabled for your key in the PubNub Admin Portal
	 * 
	 * @param Channel The channel ID for which to remove metadata.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * 						 Can be skipped if operation result is not needed.
	 */
	void RemoveChannelMetadata(FString Channel, FOnRemoveChannelMetadataResponseNative NativeCallback = nullptr);

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
	void GetMembershipsRaw(FString User, FOnGetMembershipsResponse OnGetMembershipsResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void GetMembershipsRaw(FString User, FOnGetMembershipsResponseNative NativeCallback, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void GetMemberships(FString User, FOnGetMembershipsResponse OnGetMembershipsResponse, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");

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
	void GetMemberships(FString User, FOnGetMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");
	
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
	void SetMembershipsRaw(FString User, FString SetObj, FOnSetMembershipsResponse OnSetMembershipsResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void SetMembershipsRaw(FString User, FString SetObj, FOnSetMembershipsResponseNative NativeCallback = nullptr, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void SetMemberships(FString User, TArray<FPubnubMembershipInputData> Channels, FOnSetMembershipsResponse OnSetMembershipsResponse, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");
	
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
	void SetMemberships(FString User, TArray<FPubnubMembershipInputData> Channels, FOnSetMembershipsResponseNative NativeCallback = nullptr, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");

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
	void RemoveMembershipsRaw(FString User, FString RemoveObj, FOnRemoveMembershipsResponse OnRemoveMembershipsResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void RemoveMembershipsRaw(FString User, FString RemoveObj, FOnRemoveMembershipsResponseNative NativeCallback = nullptr, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void RemoveMemberships(FString User, TArray<FString> Channels, FOnRemoveMembershipsResponse OnRemoveMembershipsResponse, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");

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
	void RemoveMemberships(FString User, TArray<FString> Channels, FOnRemoveMembershipsResponseNative NativeCallback = nullptr, FPubnubMembershipInclude Include = FPubnubMembershipInclude(), int Limit = 100, FString Filter = "", FPubnubMembershipSort Sort = FPubnubMembershipSort(), FString PageNext = "", FString PagePrev = "");

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
	void GetChannelMembersRaw(FString Channel, FOnGetChannelMembersResponse OnGetMembersResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);
	
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
	void GetChannelMembersRaw(FString Channel, FOnGetChannelMembersResponseNative NativeCallback, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void GetChannelMembers(FString Channel, FOnGetChannelMembersResponse OnGetMembersResponse, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");
	
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
	void GetChannelMembers(FString Channel, FOnGetChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");
	
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
	void SetChannelMembersRaw(FString Channel, FString SetObj, FOnSetChannelMembersResponse OnSetChannelMembersResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void SetChannelMembersRaw(FString Channel, FString SetObj, FOnSetChannelMembersResponseNative NativeCallback = nullptr, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void SetChannelMembers(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FOnSetChannelMembersResponse OnSetChannelMembersResponse, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");
	
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
	void SetChannelMembers(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FOnSetChannelMembersResponseNative NativeCallback = nullptr, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");

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
	void RemoveChannelMembersRaw(FString Channel, FString RemoveObj, FOnRemoveChannelMembersResponse OnRemoveChannelMembersResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void RemoveChannelMembersRaw(FString Channel, FString RemoveObj, FOnRemoveChannelMembersResponseNative NativeCallback = nullptr, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::PT_NotSet);

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
	void RemoveChannelMembers(FString Channel, TArray<FString> Users, FOnRemoveChannelMembersResponse OnRemoveChannelMembersResponse, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");
	
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
	void RemoveChannelMembers(FString Channel, TArray<FString> Users, FOnRemoveChannelMembersResponseNative NativeCallback = nullptr, FPubnubMemberInclude Include = FPubnubMemberInclude(), int Limit = 100, FString Filter = "", FPubnubMemberSort Sort = FPubnubMemberSort(), FString PageNext = "", FString PagePrev = "");

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
	void AddMessageAction(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnAddMessageActionResponse OnAddMessageActionResponse);

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
	void AddMessageAction(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnAddMessageActionResponseNative NativeCallback = nullptr);

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
	void GetMessageActions(FString Channel, FOnGetMessageActionsResponse OnGetMessageActionsResponse, FString Start = "", FString End = "", int Limit = 0);

	/**
	 * Retrieves message actions for a specified channel within a given time range.
	 * 
	 * @param Channel The ID of the channel.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 * @param Start The starting timetoken for the range. Has to be greater (newer) than the End timetoken. 
	 * @param End The ending timetoken for the range.
	 * @param Limit The maximum number of actions to retrieve.
	 */
	void GetMessageActions(FString Channel, FOnGetMessageActionsResponseNative NativeCallback, FString Start = "", FString End = "", int Limit = 0);
	
	/**
	 * Removes a specific message action from a message in a channel.
	 * 
	 * @param Channel The ID of the channel.
	 * @param MessageTimetoken The timetoken of the message.
	 * @param ActionTimetoken The timetoken of the action to remove.
	 * @param OnRemoveMessageActionResponse (Optional) Delegate to listen for the operation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Message Actions", meta = (AutoCreateRefTerm = "OnRemoveMessageActionResponse"))
	void RemoveMessageAction(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnRemoveMessageActionResponse OnRemoveMessageActionResponse);
	
	/**
	 * Removes a specific message action from a message in a channel.
	 * 
	 * @param Channel The ID of the channel.
	 * @param MessageTimetoken The timetoken of the message.
	 * @param ActionTimetoken The timetoken of the action to remove.
	 * @param NativeCallback (Optional) Delegate to listen for the operation result. Delegate in native form that can accept lambdas.
	 */
	void RemoveMessageAction(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnRemoveMessageActionResponseNative NativeCallback = nullptr);
		
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

	UPROPERTY()
	TMap<int, TObjectPtr<UPubnubClient>> PubnubClients;
	int NextClientID = 0;

	UPROPERTY()
	UPubnubClient* DefaultClient = nullptr;
	
	//Thread for all PubNub operations, this thread will queue all PubNub calls and trigger them one by one
	FPubnubFunctionThread* QuickActionThread = nullptr;

	//Pubnub context for the most of the pubnub operations
	pubnub_t *ctx_pub = nullptr;
	//Pubnub context for the event engine - subscribe operations
	pubnub_t *ctx_ee = nullptr;

	//Auth token has to be kept alive for the lifetime of the sdk, so this is the container for it
	char* AuthTokenBuffer = nullptr;
	size_t AuthTokenLength = 0;

	//Storage for global subscriptions (not from Entities)
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

#pragma endregion

#pragma region PUBNUB CONFIG
	
	/* PUBNUB CONFIG */

	//Plugin settings from ProjectSettings
	TObjectPtr<UPubnubSettings> PubnubPluginSettings = nullptr;

	//Container for all configuration settings
	FPubnubConfig PubnubConfig;
	
	//Containers for keys stored from settings
	static const int PublishKeySize = 42;
	static const int SecretKeySize = 54;
	char PublishKey[PublishKeySize + 1] = {};
	char SubscribeKey[PublishKeySize + 1] = {};
	char SecretKey[SecretKeySize + 1] = {};
	
	void LoadPluginSettings();
	void SavePubnubConfig(const FPubnubConfig &Config);

#pragma endregion

	/* CRYPTO */

	//CryptoBridge class that holds provided CryptoModule and inserts it into C-Core system - it keeps all required references alive
	UPROPERTY()
	TObjectPtr<UPubnubCryptoBridge> CryptoBridge;

	/* INITIALIZATION CHECKS */
	
	bool IsInitialized = false;
	bool IsUserIDSet = false;
	bool CheckIsPubnubInitialized();

	//TO DELETE
	FString GetUserIDInternal();

#pragma region PRIVATE FUNCTIONS

	/* PRIVATE FUNCTIONS */
	//These functions are called from "BLUEPRINT EXPOSED" functions on PubNub threads. They shouldn't be called directly on Game Thread.
	
	void AddChannelToGroup_priv(FString Channel, FString ChannelGroup, FOnAddChannelToGroupResponseNative OnAddChannelToGroupResponse);
	void RemoveChannelFromGroup_priv(FString Channel, FString ChannelGroup, FOnRemoveChannelFromGroupResponseNative OnRemoveChannelFromGroupResponse);
	void ListChannelsFromGroup_priv(FString ChannelGroup, FOnListChannelsFromGroupResponseNative OnListChannelsResponse);
	void RemoveChannelGroup_priv(FString ChannelGroup, FOnRemoveChannelGroupResponseNative OnRemoveChannelGroupResponse);
	void ListUsersFromChannel_priv(FString Channel, FOnListUsersFromChannelResponseNative ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());
	void ListUserSubscribedChannels_priv(FString UserID, FOnListUsersSubscribedChannelsResponseNative ListUserSubscribedChannelsResponse);
	void SetState_priv(FString Channel, FString StateJson, FOnSetStateResponseNative OnSetStateResponse, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());
	void GetState_priv(FString Channel, FString ChannelGroup, FString UserID, FOnGetStateResponseNative OnGetStateResponse);
	void Heartbeat_priv(FString Channel, FString ChannelGroup);
	void GrantToken_priv(FString PermissionObject, FOnGrantTokenResponseNative OnGrantTokenResponse);
	void RevokeToken_priv(FString Token, FOnRevokeTokenResponseNative OnRevokeTokenResponse);
	FString ParseToken_priv(FString Token);
	void FetchHistory_priv(FString Channel, FOnFetchHistoryResponseNative OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());
	void DeleteMessages_priv(FString Channel, FOnDeleteMessagesResponseNative OnDeleteMessagesResponse, FPubnubDeleteMessagesSettings DeleteMessagesSettings);
	void MessageCounts_priv(FString Channel, FString Timetoken, FOnMessageCountsResponseNative OnMessageCountsResponse);
	void GetAllUserMetadata_priv(FOnGetAllUserMetadataResponseNative OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetUserMetadata_priv(FString User, FString UserMetadataObj, FOnSetUserMetadataResponseNative OnSetUserMetadataResponse, FString Include);
	void GetUserMetadata_priv(FString User, FOnGetUserMetadataResponseNative OnGetUserMetadataResponse, FString Include);
	void RemoveUserMetadata_priv(FString User, FOnRemoveUserMetadataResponseNative OnRemoveUserMetadataResponse);
	void GetAllChannelMetadata_priv(FOnGetAllChannelMetadataResponseNative OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetChannelMetadata_priv(FString Channel, FString ChannelMetadataObj, FOnSetChannelMetadataResponseNative OnSetChannelMetadataResponse, FString Include);
	void GetChannelMetadata_priv(FString Channel, FOnGetChannelMetadataResponseNative OnGetChannelMetadataResponse, FString Include);
	void RemoveChannelMetadata_priv(FString Channel, FOnRemoveChannelMetadataResponseNative OnRemoveChannelMetadataResponse);
	void GetMemberships_priv(FString User, FOnGetMembershipsResponseNative OnGetMembershipsResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetMemberships_priv(FString User, FString SetObj, FOnSetMembershipsResponseNative OnSetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void RemoveMemberships_priv(FString User, FString RemoveObj, FOnRemoveMembershipsResponseNative OnRemoveMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void GetChannelMembers_priv(FString Channel, FOnGetChannelMembersResponseNative OnGetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetChannelMembers_priv(FString Channel, FString SetObj, FOnSetChannelMembersResponseNative OnSetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void RemoveChannelMembers_priv(FString Channel, FString RemoveObj, FOnRemoveChannelMembersResponseNative OnRemoveMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void AddMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnAddMessageActionResponseNative AddMessageActionResponse);
	void RemoveMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnRemoveMessageActionResponseNative OnRemoveMessageActionResponse);
	void GetMessageActions_priv(FString Channel, FOnGetMessageActionsResponseNative OnGetMessageActionsResponse, FString Start, FString End, int Limit);


	void SubscribeWithSubscription(UPubnubSubscription* Subscription, FPubnubSubscriptionCursor Cursor, FOnSubscribeOperationResponseNative OnSubscribeResponse);
	void SubscribeWithSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet, FPubnubSubscriptionCursor Cursor, FOnSubscribeOperationResponseNative OnSubscribeResponse);
	void UnsubscribeWithSubscription(UPubnubSubscription* Subscription, FOnSubscribeOperationResponseNative OnUnsubscribeResponse);
	void UnsubscribeWithSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet, FOnSubscribeOperationResponseNative OnUnsubscribeResponse);
#pragma endregion
	

	/* STRUCT CONVERTERS */
	//ALL 4 TO DELETE
	void PublishUESettingsToPubnubPublishOptions(FPubnubPublishSettings &PublishSettings, pubnub_publish_options &PubnubPublishOptions);
	void HereNowUESettingsToPubnubHereNowOptions(FPubnubListUsersFromChannelSettings &HereNowSettings, pubnub_here_now_options &PubnubHereNowOptions);
	void SetStateUESettingsToPubnubSetStateOptions(FPubnubSetStateSettings &SetStateSettings, pubnub_set_state_options &PubnubSetStateOptions);
	void FetchHistoryUESettingsToPbFetchHistoryOptions(FPubnubFetchHistorySettings &FetchHistorySettings, pubnub_fetch_history_options &PubnubFetchHistoryOptions);

	void DecryptHistoryMessages(TArray<FPubnubHistoryMessageData>& Messages);
	
	/* GRANT TOKEN HELPERS */

	TSharedPtr<FJsonObject> AddChannelPermissionsToJson(TArray<FString> Channels, TArray<FPubnubChannelPermissions> ChannelPermissions);
	TSharedPtr<FJsonObject> AddChannelGroupPermissionsToJson(TArray<FString> ChannelGroups, TArray<FPubnubChannelGroupPermissions> ChannelGroupPermissions);
	TSharedPtr<FJsonObject> AddUserPermissionsToJson(TArray<FString> Users, TArray<FPubnubUserPermissions> UserPermissions);

	//C-Core logging
	static TArray<FString> FalseCCoreLogPhrases;
	static bool ShouldCCoreLogBeSkipped(FString Message);
	
	//Function that is sent to Pubnub sdk (c-core) to pass sdk logs to Unreal
	static void PubnubSDKLogConverter(enum pubnub_log_level log_level, const char* message);

	//Array storing delegates for all queued subscription operations
	TArray<FOnSubscribeOperationResponseNative> SubscriptionResultDelegates;

	void OnCCoreSubscriptionStatusReceived(const pubnub_subscription_status status, const pubnub_subscription_status_data_t status_data);
};
