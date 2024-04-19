// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubNub.h"
#include "PubnubStructLibrary.h"
#include "PubnubEnumLibrary.h"
#include "PubnubSubsystem.generated.h"


class UPubnubSettings;
class FPubnubFunctionThread;
class FPubnubLoopingThread;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMessageReceived, FString, MessageJson, FString, Channel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPubnubError, FString, ErrorMessage, EPubnubErrorType, ErrorType);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnListChannelsFromGroupResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHereNowResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnWhereNowResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetStateResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGrantTokenResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHistoryResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnMessageCountsResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetAllUUIDMetadataResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetUUIDMetadataResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetAllChannelMetadataResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetChannelMetadataResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetMembershipResponse, FString, JsonResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetMembersResponse, FString, JsonResponse);


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
	void RemoveChannelGroup(FString ChannelGroup);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void HereNow(FString ChannelName, FOnHereNowResponse HereNowResponse, FPubnubHereNowSettings HereNowSettings = FPubnubHereNowSettings());

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void WhereNow(FString UserID, FOnWhereNowResponse WhereNowResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void SetState(FString ChannelName, FString StateJson, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void GetState(FString ChannelName, FString ChannelGroup, FString UserID, FOnGetStateResponse OnGetStateResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Presence")
	void Heartbeat(FString ChannelName, FString ChannelGroup);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AccessManager")
	void GrantToken(int TTLMinutes, FString AuthorizedUUID, FOnGrantTokenResponse OnGrantTokenResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AccessManager")
	void RevokeToken(FString Token);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AccessManager")
	void ParseToken(FString Token);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AccessManager")
	void SetAuthToken(FString Token);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|MessagePersistence")
	void History(FString ChannelName, FOnHistoryResponse OnHistoryResponse, FPubnubHistorySettings HistorySettings = FPubnubHistorySettings());

	UFUNCTION(BlueprintCallable, Category = "Pubnub|MessagePersistence")
	void MessageCounts(FString ChannelName, FDateTime TimeStamp, FOnMessageCountsResponse OnMessageCountsResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void GetAllUUIDMetadata(FString Include, int Limit, FString Start, FString End, EPubnubTribool Count, FOnGetAllUUIDMetadataResponse OnGetAllUUIDMetadataResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void SetUUIDMetadata(FString UUIDMetadataID, FString Include, FString UUIDMetadataObj);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void GetUUIDMetadata(FString Include, FString UUIDMetadataID, FOnGetUUIDMetadataResponse OnGetUUIDMetadataResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void RemoveUUIDMetadata(FString UUIDMetadataID);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void GetAllChannelMetadata(FString Include, int Limit, FString Start, FString End, EPubnubTribool Count, FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void SetChannelMetadata(FString ChannelMetadataID, FString Include, FString ChannelMetadataObj);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void GetChannelMetadata(FString Include, FString ChannelMetadataID, FOnGetChannelMetadataResponse OnGetChannelMetadataResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void RemoveChannelMetadata(FString ChannelMetadataID);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void GetMemberships(FString UUIDMetadataID, FString Include, int Limit, FString Start, FString End, EPubnubTribool Count, FOnGetMembershipResponse OnGetMembershipResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void SetMemberships(FString UUIDMetadataID, FString Include, FString SetObj);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void RemoveMemberships(FString UUIDMetadataID, FString Include, FString RemoveObj);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void GetMembers(FString ChannelMetadataID, FString Include, int Limit, FString Start, FString End, EPubnubTribool Count, FOnGetMembersResponse OnGetMembersResponse);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void AddMembers(FString ChannelMetadataID, FString Include, FString AddObj);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void SetMembers(FString ChannelMetadataID, FString Include, FString SetObj);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|AppContext")
	void RemoveMembers(FString ChannelMetadataID, FString Include, FString RemoveObj);

#pragma endregion
	
private:

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
	void SystemPublish();

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
	bool CheckIsFieldEmpty(FString Field, FString FieldName, FString FunctionName);


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
	void ListChannelsFromGroup_priv(FString ChannelGroup, FOnListChannelsFromGroupResponse OnListChannelsResponse);
	void RemoveChannelGroup_priv(FString ChannelGroup);
	void HereNow_priv(FString ChannelName, FOnHereNowResponse HereNowResponse, FPubnubHereNowSettings HereNowSettings = FPubnubHereNowSettings());
	void WhereNow_priv(FString UserID, FOnWhereNowResponse WhereNowResponse);
	void SetState_priv(FString ChannelName, FString StateJson, FPubnubSetStateSettings SetStateSettings = FPubnubSetStateSettings());
	void GetState_priv(FString ChannelName, FString ChannelGroup, FString UserID, FOnGetStateResponse OnGetStateResponse);
	void Heartbeat_priv(FString ChannelName, FString ChannelGroup);
	void GrantToken_priv(int TTLMinutes, FString AuthorizedUUID, FOnGrantTokenResponse OnGrantTokenResponse);
	void RevokeToken_priv(FString Token);
	void ParseToken_priv(FString Token);
	void SetAuthToken_priv(FString Token);
	void History_priv(FString ChannelName, FOnHistoryResponse OnHistoryResponse, FPubnubHistorySettings HistorySettings = FPubnubHistorySettings());
	void MessageCounts_priv(FString ChannelName, FDateTime TimeStamp, FOnMessageCountsResponse OnMessageCountsResponse);
	void GetAllUUIDMetadata_priv(FString Include, int Limit, FString Start, FString End, EPubnubTribool Count, FOnGetAllUUIDMetadataResponse OnGetAllUUIDMetadataResponse);
	void SetUUIDMetadata_priv(FString UUIDMetadataID, FString Include, FString UUIDMetadataObj);
	void GetUUIDMetadata_priv(FString Include, FString UUIDMetadataID, FOnGetUUIDMetadataResponse OnGetUUIDMetadataResponse);
	void RemoveUUIDMetadata_priv(FString UUIDMetadataID);
	void GetAllChannelMetadata_priv(FString Include, int Limit, FString Start, FString End, EPubnubTribool Count, FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse);
	void SetChannelMetadata_priv(FString ChannelMetadataID, FString Include, FString ChannelMetadataObj);
	void GetChannelMetadata_priv(FString Include, FString ChannelMetadataID, FOnGetChannelMetadataResponse OnGetChannelMetadataResponse);
	void RemoveChannelMetadata_priv(FString ChannelMetadataID);
	void GetMemberships_priv(FString UUIDMetadataID, FString Include, int Limit, FString Start, FString End, EPubnubTribool Count, FOnGetMembershipResponse OnGetMembershipResponse);
	void SetMemberships_priv(FString UUIDMetadataID, FString Include, FString SetObj);
	void RemoveMemberships_priv(FString UUIDMetadataID, FString Include, FString RemoveObj);
	void GetMembers_priv(FString ChannelMetadataID, FString Include, int Limit, FString Start, FString End, EPubnubTribool Count, FOnGetMembersResponse OnGetMembersResponse);
	void AddMembers_priv(FString ChannelMetadataID, FString Include, FString AddObj);
	void SetMembers_priv(FString ChannelMetadataID, FString Include, FString SetObj);
	void RemoveMembers_priv(FString ChannelMetadataID, FString Include, FString RemoveObj);

#pragma endregion

	/* STRUCT CONVERTERS */
	
	void PublishUESettingsToPubnubPublishOptions(FPubnubPublishSettings &PublishSettings, pubnub_publish_options &PubnubPublishOptions);
	void HereNowUESettingsToPubnubHereNowOptions(FPubnubHereNowSettings &HereNowSettings, pubnub_here_now_options &PubnubHereNowOptions);
	void SetStateUESettingsToPubnubSetStateOptions(FPubnubSetStateSettings &SetStateSettings, pubnub_set_state_options &PubnubSetStateOptions);
	void HistoryUESettingsToPubnubHistoryOptions(FPubnubHistorySettings &HistorySettings, pubnub_history_options &PubnubHistoryOptions);
};

