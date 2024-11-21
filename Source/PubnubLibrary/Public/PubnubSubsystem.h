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
class FPubnubLoopingThread;
class UPubnubChatSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMessageReceived, FString, MessageJson, FString, Channel);
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
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Init")
	void InitPubnub();
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Init")
	void DeinitPubnub();
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Users")
	void SetUserID(FString UserID);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pubnub|Users")
	FString GetUserID();

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Init")
	void SetSecretKey();

	/**
	 * Publish message in Json format to a specified channel
	 * @param ChannelName Channel to publish message to. Can't be empty.
	 * @param Message Message to publish. Has to be in Json format.
	 * @param PublishSettings Optional advanced publish settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Publish")
	void PublishMessage(FString ChannelName, FString Message, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Publish")
	void Signal(FString ChannelName, FString Message);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void SubscribeToChannel(FString ChannelName);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void SubscribeToGroup(FString GroupName);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void UnsubscribeFromChannel(FString ChannelName);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void UnsubscribeFromGroup(FString GroupName);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Subscribe")
	void UnsubscribeFromAll();

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channels")
	void AddChannelToGroup(FString ChannelName, FString ChannelGroup);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channels")
	void RemoveChannelFromGroup(FString ChannelName, FString ChannelGroup);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channels")
	void ListChannelsFromGroup(FString ChannelGroup, FOnListChannelsFromGroupResponse OnListChannelsResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channels")
	void ListChannelsFromGroup_JSON(FString ChannelGroup, FOnPubnubResponse OnListChannelsResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Channels")
	void RemoveChannelGroup(FString ChannelGroup);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void ListUsersFromChannel(FString ChannelName, FOnListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void ListUsersFromChannel_JSON(FString ChannelName, FOnPubnubResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void ListUserSubscribedChannels(FString UserID, FOnListUsersSubscribedChannelsResponse ListUserSubscribedChannelsResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void ListUserSubscribedChannels_JSON(FString UserID, FOnPubnubResponse ListUserSubscribedChannelsResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void SetState(FString ChannelName, FString StateJson, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void GetState(FString ChannelName, FString ChannelGroup, FString UserID, FOnPubnubResponse OnGetStateResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void Heartbeat(FString ChannelName, FString ChannelGroup);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AccessManager")
	void GrantToken(FString PermissionObject, FOnPubnubResponse OnGrantTokenResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AccessManager")
	void RevokeToken(FString Token);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AccessManager")
	void ParseToken(FString Token, FOnPubnubResponse OnParseTokenResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AccessManager")
	void SetAuthToken(FString Token);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|MessagePersistence")
	void FetchHistory(FString ChannelName, FOnFetchHistoryResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());

	UFUNCTION(BlueprintCallable, Category = "Pubnub|MessagePersistence")
	void FetchHistory_JSON(FString ChannelName, FOnPubnubResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|MessagePersistence")
	void MessageCounts(FString ChannelName, FString Timetoken, FOnPubnubIntResponse OnMessageCountsResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetAllUserMetadata(FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::pbccNotSet);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetAllUserMetadata_JSON(FOnPubnubResponse OnGetAllUserMetadataResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::pbccNotSet);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void SetUserMetadata(FString User, FString UserMetadataObj, FString Include = "");

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void GetUserMetadata(FString User, FOnGetUserMetadataResponse OnGetUserMetadataResponse, FString Include = "");

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void GetUserMetadata_JSON(FString User, FOnPubnubResponse OnGetUserMetadataResponse, FString Include = "");

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void RemoveUserMetadata(FString User);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetAllChannelMetadata(FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::pbccNotSet);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetAllChannelMetadata_JSON(FOnPubnubResponse OnGetAllChannelMetadataResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::pbccNotSet);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void SetChannelMetadata(FString ChannelMetadataID, FString ChannelMetadataObj, FString Include = "");

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void GetChannelMetadata(FString ChannelMetadataID, FOnGetChannelMetadataResponse OnGetChannelMetadataResponse, FString Include = "");

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void GetChannelMetadata_JSON(FString ChannelMetadataID, FOnPubnubResponse OnGetChannelMetadataResponse, FString Include = "");

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void RemoveChannelMetadata(FString ChannelMetadataID);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetMemberships(FString User, FOnGetMembershipsResponse OnGetMembershipResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::pbccNotSet);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetMemberships_JSON(FString User, FOnPubnubResponse OnGetMembershipResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::pbccNotSet);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void SetMemberships(FString User, FString SetObj, FString Include = "");

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void RemoveMemberships(FString User, FString RemoveObj, FString Include = "");

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetChannelMembers(FString ChannelMetadataID, FOnGetChannelMembersResponse OnGetMembersResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::pbccNotSet);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext", meta=(AdvancedDisplay="Filter,Sort,PageNext,PagePrev,Count"))
	void GetChannelMembers_JSON(FString ChannelMetadataID, FOnPubnubResponse OnGetMembersResponse, FString Include = "", int Limit = 100, FString Filter = "", FString Sort = "", FString PageNext = "", FString PagePrev = "", EPubnubTribool Count = EPubnubTribool::pbccNotSet);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void AddChannelMembers(FString ChannelMetadataID, FString AddObj, FString Include = "");

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void SetChannelMembers(FString ChannelMetadataID, FString SetObj, FString Include = "");

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void RemoveChannelMembers(FString ChannelMetadataID, FString RemoveObj, FString Include = "");

	UFUNCTION(BlueprintCallable, Category = "Pubnub|MessageActions")
	void AddMessageAction(FString ChannelName, FString MessageTimetoken, FString ActionType,  FString Value, FOnAddMessageActionsResponse AddActionResponse);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|MessageActions")
	void GetMessageActions(FString ChannelName, FString Start, FString End, int SizeLimit, FOnGetMessageActionsResponse OnGetMessageActionsResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|MessageActions")
	void GetMessageActions_JSON(FString ChannelName, FString Start, FString End, int SizeLimit, FOnPubnubResponse OnGetMessageActionsResponse);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|MessageActions")
	void RemoveMessageAction(FString ChannelName, FString MessageTimetoken, FString ActionTimetoken);
	
	//UFUNCTION(BlueprintCallable, Category = "Pubnub|MessageActions")
	//void GetMessageActionsContinue(FOnPubnubResponse OnGetMessageActionsContinueResponse);

#pragma endregion

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pubnub|AccessManager")
	FString GrantTokenStructureToJsonString(FPubnubGrantTokenStructure TokenStructure, bool &success);

	bool CheckIsFieldEmpty(FString Field, FString FieldName, FString FunctionName);
	
private:

	inline static const FString SystemPublishMessage = "\"Pubnub system unlock message\"";
	
	//New threads to call all C-Core functions asynchronously
	
	//Thread for quick operations, generally everything except subscribe
	TObjectPtr<FPubnubFunctionThread> QuickActionThread = nullptr;
	//Thread for long operations that need to be called in a loop, mostly for subcribe
	TObjectPtr<FPubnubLoopingThread> LongpollThread = nullptr;

	//Pubnub contexts for publishing data and subscribing to channels
	pubnub_t *ctx_pub = nullptr;
	pubnub_t *ctx_sub = nullptr;

	TArray<FString> SubscribedChannels;
	TArray<FString> SubscribedGroups;
	
	//Publish to the first subscribed channel to unlock subscribe context
	void SystemPublish(FString ChannelOpt = "");

	//Register to PubnubLoopingThread function to check in loop for messages from subscribed channels and groups
	void StartPubnubSubscribeLoop();

	//Useful for subscribing into multiple channels/groups. Returns Strings in format String1,String2,...
	FString StringArrayToCommaSeparated(TArray<FString> StringArray);

	//Returns FString from the pubnub_get response
	FString GetLastResponse(pubnub_t* context);
	
	//Returns FString from the pubnub_get_channel response
	FString GetLastChannelResponse(pubnub_t* context);

#pragma region ERROR FUNCTIONS
	
	/* ERROR FUNCTIONS */
	//Every Error function prints error to the Log and Broadcasts OnPubnubError delegate
	
	//Default error for most use cases
	void PubnubError(FString ErrorMessage, EPubnubErrorType ErrorType = EPubnubErrorType::PET_Error);
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
	void DeinitPubnub_priv();
	void SetUserID_priv(FString UserID);
	void SetSecretKey_priv();
	void PublishMessage_priv(FString ChannelName, FString Message, FPubnubPublishSettings PublishSettings = FPubnubPublishSettings());
	void Signal_priv(FString ChannelName, FString Message);
	void SubscribeToChannel_priv(FString ChannelName);
	void SubscribeToGroup_priv(FString GroupName);
	void UnsubscribeFromChannel_priv(FString ChannelName);
	void UnsubscribeFromGroup_priv(FString GroupName);
	void UnsubscribeFromAll_priv();
	void AddChannelToGroup_priv(FString ChannelName, FString ChannelGroup);
	void RemoveChannelFromGroup_priv(FString ChannelName, FString ChannelGroup);
	FString ListChannelsFromGroup_pn(FString ChannelGroup);
	void ListChannelsFromGroup_JSON_priv(FString ChannelGroup, FOnPubnubResponse OnListChannelsResponse);
	void ListChannelsFromGroup_DATA_priv(FString ChannelGroup, FOnListChannelsFromGroupResponse OnListChannelsResponse);
	void RemoveChannelGroup_priv(FString ChannelGroup);
	FString ListUsersFromChannel_pn(FString ChannelName, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());
	void ListUsersFromChannel_JSON_priv(FString ChannelName, FOnPubnubResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());
	void ListUsersFromChannel_DATA_priv(FString ChannelName, FOnListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings = FPubnubListUsersFromChannelSettings());
	FString ListUserSubscribedChannels_pn(FString UserID);
	void ListUserSubscribedChannels_JSON_priv(FString UserID, FOnPubnubResponse ListUserSubscribedChannelsResponse);
	void ListUserSubscribedChannels_DATA_priv(FString UserID, FOnListUsersSubscribedChannelsResponse ListUserSubscribedChannelsResponse);
	void SetState_priv(FString ChannelName, FString StateJson, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());
	void GetState_priv(FString ChannelName, FString ChannelGroup, FString UserID, FOnPubnubResponse OnGetStateResponse);
	void Heartbeat_priv(FString ChannelName, FString ChannelGroup);
	void GrantToken_priv(FString PermissionObject, FOnPubnubResponse OnGrantTokenResponse);
	void RevokeToken_priv(FString Token);
	void ParseToken_priv(FString Token, FOnPubnubResponse OnParseTokenResponse);
	void SetAuthToken_priv(FString Token);
	FString FetchHistory_pn(FString ChannelName, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());
	void FetchHistory_JSON_priv(FString ChannelName, FOnPubnubResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());
	void FetchHistory_DATA_priv(FString ChannelName, FOnFetchHistoryResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings = FPubnubFetchHistorySettings());
	void MessageCounts_priv(FString ChannelName, FString Timetoken, FOnPubnubIntResponse OnMessageCountsResponse);
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
	void SetChannelMetadata_priv(FString ChannelMetadataID, FString ChannelMetadataObj, FString Include);
	FString GetChannelMetadata_pn(FString ChannelMetadataID, FString Include);
	void GetChannelMetadata_JSON_priv(FString ChannelMetadataID, FOnPubnubResponse OnGetChannelMetadataResponse, FString Include);
	void GetChannelMetadata_DATA_priv(FString ChannelMetadataID, FOnGetChannelMetadataResponse OnGetChannelMetadataResponse, FString Include);
	void RemoveChannelMetadata_priv(FString ChannelMetadataID);
	FString GetMemberships_pn(FString User, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void GetMemberships_JSON_priv(FString User, FOnPubnubResponse OnGetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void GetMemberships_DATA_priv(FString User, FOnGetMembershipsResponse OnGetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void SetMemberships_priv(FString User, FString SetObj, FString Include);
	void RemoveMemberships_priv(FString User, FString RemoveObj, FString Include);
	FString GetChannelMembers_pn(FString ChannelMetadataID, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void GetChannelMembers_JSON_priv(FString ChannelMetadataID, FOnPubnubResponse OnGetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void GetChannelMembers_DATA_priv(FString ChannelMetadataID, FOnGetChannelMembersResponse OnGetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count);
	void AddChannelMembers_priv(FString ChannelMetadataID, FString AddObj, FString Include);
	void SetChannelMembers_priv(FString ChannelMetadataID, FString SetObj, FString Include);
	void RemoveChannelMembers_priv(FString ChannelMetadataID, FString Include, FString RemoveObj);
	void AddMessageAction_priv(FString ChannelName, FString MessageTimetoken, FString ActionType,  FString Value, FOnAddMessageActionsResponse AddActionResponse);
	void RemoveMessageAction_priv(FString ChannelName, FString MessageTimetoken, FString ActionTimetoken);
	FString GetMessageActions_pn(FString ChannelName, FString Start, FString End, int SizeLimit);
	void GetMessageActions_JSON_priv(FString ChannelName, FString Start, FString End, int SizeLimit, FOnPubnubResponse OnGetMessageActionsResponse);
	void GetMessageActions_DATA_priv(FString ChannelName, FString Start, FString End, int SizeLimit, FOnGetMessageActionsResponse OnGetMessageActionsResponse);
	void GetMessageActionsContinue_priv(FOnPubnubResponse OnGetMessageActionsContinueResponse);

#pragma endregion

	/* STRUCT CONVERTERS */
	
	void PublishUESettingsToPubnubPublishOptions(FPubnubPublishSettings &PublishSettings, pubnub_publish_options &PubnubPublishOptions);
	void HereNowUESettingsToPubnubHereNowOptions(FPubnubListUsersFromChannelSettings &HereNowSettings, pubnub_here_now_options &PubnubHereNowOptions);
	void SetStateUESettingsToPubnubSetStateOptions(FPubnubSetStateSettings &SetStateSettings, pubnub_set_state_options &PubnubSetStateOptions);
	void FetchHistoryUESettingsToPbFetchHistoryOptions(FPubnubFetchHistorySettings &FetchHistorySettings, pubnub_fetch_history_options &PubnubFetchHistoryOptions);

	/* GRANT TOKEN HELPERS */

	TSharedPtr<FJsonObject> AddChannelPermissionsToJson(TArray<FString> Channels, TArray<FPubnubChannelPermissions> ChannelPermissions);
	TSharedPtr<FJsonObject> AddChannelGroupPermissionsToJson(TArray<FString> ChannelGroups, TArray<FPubnubChannelGroupPermissions> ChannelGroupPermissions);
	TSharedPtr<FJsonObject> AddUserPermissionsToJson(TArray<FString> Users, TArray<FPubnubUserPermissions> UserPermissions);
};


