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

