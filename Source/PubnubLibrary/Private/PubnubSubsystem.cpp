// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"

#include "Async/Async.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Config/PubnubSettings.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "FunctionLibraries/PubnubUtilities.h"
#include "Threads/PubnubFunctionThread.h"

DEFINE_LOG_CATEGORY(PubnubLog)

void UPubnubSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//Load all settings from plugin config
	LoadPluginSettings();
	if(PubnubPluginSettings->InitializeAutomatically)
	{
		InitPubnub();
	}
}

void UPubnubSubsystem::Deinitialize()
{
	//First clean up all Pubnub data
	DeinitPubnub();
	Super::Deinitialize();
}

void UPubnubSubsystem::InitPubnub()
{
	InitPubnubWithConfig(UPubnubUtilities::PubnubConfigFromPluginSettings(PubnubPluginSettings));
}

void UPubnubSubsystem::InitPubnubWithConfig(FPubnubConfig Config)
{
	if(IsInitialized)
	{
		PubnubError("Pubnub is already initialized. Disable InitializeAutomatically in Pubnub SDK Project Settings to be able to Init Pubnub manually", EPubnubErrorType::PET_Warning);
		return;
	}

	SavePubnubConfig(Config);

	InitPubnub_priv(Config);

	//If initialized correctly, create required thread.
	if(IsInitialized)
	{
		//Create new thread to queue all pubnub sync operations
		QuickActionThread = new FPubnubFunctionThread;
	}
}

void UPubnubSubsystem::DeinitPubnub()
{
	if(!IsInitialized)
	{return;}

	if(QuickActionThread)
	{
		QuickActionThread->Stop();
	}

	//Unsubscribe from all channels and groups so this user will not be visible for others anymore
	UnsubscribeFromAll_priv();
	
	IsInitialized = false;
	
	if(ctx_pub)
	{
		pubnub_free(ctx_pub);
		ctx_pub = nullptr;
	}
	if(ctx_ee)
	{
		pubnub_free(ctx_ee);
		ctx_ee = nullptr;
	}
	
	ChannelSubscriptions.Empty();
	ChannelGroupSubscriptions.Empty();
	IsUserIDSet = false;
}

void UPubnubSubsystem::SetUserID(FString UserID)
{
	if(!CheckIsPubnubInitialized())
	{return;}

	SetUserID_priv(UserID);
}

FString UPubnubSubsystem::GetUserID()
{
	return GetUserIDInternal();
}

void UPubnubSubsystem::SetSecretKey()
{
	if(!CheckIsPubnubInitialized())
	{return;}
	
	if(std::strlen(SecretKey) == 0)
	{
		PubnubError("Can't set Secret Key. Secret Key is empty.");
		return;
	}

	//This function only changes data locally, doesn't do any networking operations, so no need to call it on separate thread
	pubnub_set_secret_key(ctx_pub, SecretKey);
	pubnub_set_secret_key(ctx_ee, SecretKey);
}

void UPubnubSubsystem::PublishMessage(FString Channel, FString Message, FOnPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings)
{
	FOnPublishMessageResponseNative NativeCallback;
	NativeCallback.BindLambda([OnPublishMessageResponse](FPubnubOperationResult Result, FPubnubMessageData PublishedMessage)
	{
		OnPublishMessageResponse.ExecuteIfBound(Result, PublishedMessage);
	});

	PublishMessage(Channel, Message, NativeCallback, PublishSettings);
}

void UPubnubSubsystem::PublishMessage(FString Channel, FString Message, FOnPublishMessageResponseNative NativeCallback, FPubnubPublishSettings PublishSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, Message, NativeCallback, PublishSettings]
	{
		PublishMessage_priv(Channel, Message, NativeCallback, PublishSettings);
	});
}

void UPubnubSubsystem::PublishMessage(FString Channel, FString Message, FPubnubPublishSettings PublishSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, Message, PublishSettings]
	{
		PublishMessage_priv(Channel, Message, nullptr, PublishSettings);
	});
}

void UPubnubSubsystem::Signal(FString Channel, FString Message, FOnSignalResponse OnSignalResponse, FPubnubSignalSettings SignalSettings)
{
	FOnSignalResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSignalResponse](const FPubnubOperationResult& Result, const FPubnubMessageData& SignalMessage)
	{
		OnSignalResponse.ExecuteIfBound(Result, SignalMessage);
	});

	Signal(Channel, Message, NativeCallback, SignalSettings);
}

void UPubnubSubsystem::Signal(FString Channel, FString Message, FOnSignalResponseNative NativeCallback, FPubnubSignalSettings SignalSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, Message, NativeCallback, SignalSettings]
	{
		Signal_priv(Channel, Message, NativeCallback, SignalSettings);
	});
}

void UPubnubSubsystem::Signal(FString Channel, FString Message, FPubnubSignalSettings SignalSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, Message, SignalSettings]
	{
		Signal_priv(Channel, Message, nullptr, SignalSettings);
	});
}

void UPubnubSubsystem::SubscribeToChannel(FString Channel, FPubnubSubscribeSettings SubscribeSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, SubscribeSettings]
	{
		SubscribeToChannel_priv(Channel, SubscribeSettings);
	});
}

void UPubnubSubsystem::SubscribeToGroup(FString GroupName, FPubnubSubscribeSettings SubscribeSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, GroupName, SubscribeSettings]
	{
		SubscribeToGroup_priv(GroupName, SubscribeSettings);
	});
}

void UPubnubSubsystem::UnsubscribeFromChannel(FString Channel)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel]
	{
		UnsubscribeFromChannel_priv(Channel);
	});
}

void UPubnubSubsystem::UnsubscribeFromGroup(FString GroupName)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, GroupName]
	{
		UnsubscribeFromGroup_priv(GroupName);
	});
}

void UPubnubSubsystem::UnsubscribeFromAll()
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this]
	{
		UnsubscribeFromAll_priv();
	});
}

void UPubnubSubsystem::AddChannelToGroup(FString Channel, FString ChannelGroup, FOnAddChannelToGroupResponse OnAddChannelToGroupResponse)
{
	FOnAddChannelToGroupResponseNative NativeCallback;
	NativeCallback.BindLambda([OnAddChannelToGroupResponse](const FPubnubOperationResult& Result)
	{
		OnAddChannelToGroupResponse.ExecuteIfBound(Result);
	});
	AddChannelToGroup(Channel, ChannelGroup, NativeCallback);
}

void UPubnubSubsystem::AddChannelToGroup(FString Channel, FString ChannelGroup, FOnAddChannelToGroupResponseNative NativeCallback)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, ChannelGroup, NativeCallback]
	{
		AddChannelToGroup_priv(Channel, ChannelGroup, NativeCallback);
	});
}

void UPubnubSubsystem::RemoveChannelFromGroup(FString Channel, FString ChannelGroup, FOnRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse)
{
	FOnRemoveChannelFromGroupResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveChannelFromGroupResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveChannelFromGroupResponse.ExecuteIfBound(Result);
	});
	RemoveChannelFromGroup(Channel, ChannelGroup, NativeCallback);
}

void UPubnubSubsystem::RemoveChannelFromGroup(FString Channel, FString ChannelGroup, FOnRemoveChannelFromGroupResponseNative NativeCallback)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, ChannelGroup, NativeCallback]
	{
		RemoveChannelFromGroup_priv(Channel, ChannelGroup, NativeCallback);
	});
}

void UPubnubSubsystem::ListChannelsFromGroup(FString ChannelGroup, FOnListChannelsFromGroupResponse OnListChannelsResponse)
{
	FOnListChannelsFromGroupResponseNative NativeCallback;
	NativeCallback.BindLambda([OnListChannelsResponse](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		OnListChannelsResponse.ExecuteIfBound(Result, Channels);
	});

	ListChannelsFromGroup(ChannelGroup, NativeCallback);
}

void UPubnubSubsystem::ListChannelsFromGroup(FString ChannelGroup, FOnListChannelsFromGroupResponseNative NativeCallback)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelGroup, NativeCallback]
	{
		ListChannelsFromGroup_DATA_priv(ChannelGroup, NativeCallback);
	});
}

void UPubnubSubsystem::ListChannelsFromGroup_JSON(FString ChannelGroup, FOnPubnubResponse OnListChannelsResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelGroup, OnListChannelsResponse]
	{
		ListChannelsFromGroup_JSON_priv(ChannelGroup, OnListChannelsResponse);
	});
}

void UPubnubSubsystem::RemoveChannelGroup(FString ChannelGroup, FOnRemoveChannelGroupResponse OnRemoveChannelGroupResponse)
{
	FOnRemoveChannelGroupResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveChannelGroupResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveChannelGroupResponse.ExecuteIfBound(Result);
	});
	RemoveChannelGroup(ChannelGroup, NativeCallback);
}

void UPubnubSubsystem::RemoveChannelGroup(FString ChannelGroup, FOnRemoveChannelGroupResponseNative NativeCallback)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelGroup, NativeCallback]
	{
		RemoveChannelGroup_priv(ChannelGroup, NativeCallback);
	});
}

void UPubnubSubsystem::ListUsersFromChannel(FString Channel, FOnListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	FOnListUsersFromChannelResponseNative NativeCallback;
	NativeCallback.BindLambda([ListUsersFromChannelResponse](const FPubnubOperationResult& Result, FPubnubListUsersFromChannelWrapper Data)
	{
		ListUsersFromChannelResponse.ExecuteIfBound(Result, Data);
	});

	ListUsersFromChannel(Channel, NativeCallback, ListUsersFromChannelSettings);
}

void UPubnubSubsystem::ListUsersFromChannel(FString Channel, FOnListUsersFromChannelResponseNative NativeCallback, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, NativeCallback, ListUsersFromChannelSettings]
	{
		ListUsersFromChannel_DATA_priv(Channel, NativeCallback, ListUsersFromChannelSettings);
	});
}

void UPubnubSubsystem::ListUsersFromChannel_JSON(FString Channel, FOnPubnubResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, ListUsersFromChannelResponse, ListUsersFromChannelSettings]
	{
		ListUsersFromChannel_JSON_priv(Channel, ListUsersFromChannelResponse, ListUsersFromChannelSettings);
	});
}

void UPubnubSubsystem::ListUserSubscribedChannels(FString UserID, FOnListUsersSubscribedChannelsResponse ListUserSubscribedChannelsResponse)
{
	FOnListUsersSubscribedChannelsResponseNative NativeCallback;
	NativeCallback.BindLambda([ListUserSubscribedChannelsResponse](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		ListUserSubscribedChannelsResponse.ExecuteIfBound(Result, Channels);
	});

	ListUserSubscribedChannels(UserID, NativeCallback);
}

void UPubnubSubsystem::ListUserSubscribedChannels(FString UserID, FOnListUsersSubscribedChannelsResponseNative NativeCallback)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, UserID, NativeCallback]
	{
		ListUserSubscribedChannels_DATA_priv(UserID, NativeCallback);
	});
}

void UPubnubSubsystem::ListUserSubscribedChannels_JSON(FString UserID, FOnPubnubResponse ListUserSubscribedChannelsResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, UserID, ListUserSubscribedChannelsResponse]
	{
		ListUserSubscribedChannels_JSON_priv(UserID, ListUserSubscribedChannelsResponse);
	});
}

void UPubnubSubsystem::SetState(FString Channel, FString StateJson, FOnSetStateResponse OnSetStateResponse, FPubnubSetStateSettings SetStateSettings)
{
	FOnSetStateResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSetStateResponse](const FPubnubOperationResult& Result)
	{
		OnSetStateResponse.ExecuteIfBound(Result);
	});
	SetState(Channel, StateJson, NativeCallback, SetStateSettings);
}

void UPubnubSubsystem::SetState(FString Channel, FString StateJson, FOnSetStateResponseNative NativeCallback, FPubnubSetStateSettings SetStateSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, StateJson, NativeCallback, SetStateSettings]
	{
		SetState_priv(Channel, StateJson, NativeCallback, SetStateSettings);
	});
}

void UPubnubSubsystem::SetState(FString Channel, FString StateJson, FPubnubSetStateSettings SetStateSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, StateJson, SetStateSettings]
	{
		SetState_priv(Channel, StateJson, nullptr, SetStateSettings);
	});
}

void UPubnubSubsystem::GetState(FString Channel, FString ChannelGroup, FString UserID, FOnGetStateResponse OnGetStateResponse)
{
	FOnGetStateResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetStateResponse](const FPubnubOperationResult& Result, FString JsonResponse)
	{
		OnGetStateResponse.ExecuteIfBound(Result, JsonResponse);
	});

	GetState(Channel, ChannelGroup, UserID, NativeCallback);
}

void UPubnubSubsystem::GetState(FString Channel, FString ChannelGroup, FString UserID, FOnGetStateResponseNative NativeCallback)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, ChannelGroup, UserID, NativeCallback]
	{
		GetState_priv(Channel, ChannelGroup, UserID, NativeCallback);
	});
}

void UPubnubSubsystem::Heartbeat(FString Channel, FString ChannelGroup)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, ChannelGroup]
	{
		Heartbeat_priv(Channel, ChannelGroup);
	});
}

void UPubnubSubsystem::GrantToken(FString PermissionObject, FOnGrantTokenResponse OnGrantTokenResponse)
{
	FOnGrantTokenResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGrantTokenResponse](const FPubnubOperationResult& Result, FString Token)
	{
		OnGrantTokenResponse.ExecuteIfBound(Result, Token);
	});

	GrantToken(PermissionObject, NativeCallback);
}

void UPubnubSubsystem::GrantToken(FString PermissionObject, FOnGrantTokenResponseNative NativeCallback)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, PermissionObject, NativeCallback]
	{
		GrantToken_priv(PermissionObject, NativeCallback);
	});
}

void UPubnubSubsystem::RevokeToken(FString Token, FOnRevokeTokenResponse OnRevokeTokenResponse)
{
	FOnRevokeTokenResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRevokeTokenResponse](const FPubnubOperationResult& Result)
	{
		OnRevokeTokenResponse.ExecuteIfBound(Result);
	});
	RevokeToken(Token, NativeCallback);
}

void UPubnubSubsystem::RevokeToken(FString Token, FOnRevokeTokenResponseNative NativeCallback)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Token, NativeCallback]
	{
		RevokeToken_priv(Token, NativeCallback);
	});
}

FString UPubnubSubsystem::ParseToken(FString Token)
{
	if(!CheckIsPubnubInitialized())
	{return "";}

	return ParseToken_priv(Token);
}

void UPubnubSubsystem::SetAuthToken(FString Token)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}

	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Token, "Token", "SetAuthToken"))
	{return;}

	//This is just a setter, so no need to call it on a separate thread
	pubnub_set_auth_token(ctx_pub, TCHAR_TO_ANSI(*Token));
}

void UPubnubSubsystem::FetchHistory(FString Channel, FOnFetchHistoryResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings)
{
	FOnFetchHistoryResponseNative NativeCallback;
	NativeCallback.BindLambda([OnFetchHistoryResponse](const FPubnubOperationResult& Result, const TArray<FPubnubHistoryMessageData>& Messages)
	{
		OnFetchHistoryResponse.ExecuteIfBound(Result, Messages);
	});

	FetchHistory(Channel, NativeCallback, FetchHistorySettings);
}

void UPubnubSubsystem::FetchHistory(FString Channel, FOnFetchHistoryResponseNative NativeCallback, FPubnubFetchHistorySettings FetchHistorySettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, NativeCallback, FetchHistorySettings]
	{
		FetchHistory_DATA_priv(Channel, NativeCallback, FetchHistorySettings);
	});
}

void UPubnubSubsystem::FetchHistory_JSON(FString Channel, FOnPubnubResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, OnFetchHistoryResponse, FetchHistorySettings]
	{
		FetchHistory_JSON_priv(Channel, OnFetchHistoryResponse, FetchHistorySettings);
	});
}

void UPubnubSubsystem::DeleteMessages(FString Channel, FOnDeleteMessagesResponse OnDeleteMessagesResponse, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	FOnDeleteMessagesResponseNative NativeCallback;
	NativeCallback.BindLambda([OnDeleteMessagesResponse](FPubnubOperationResult Result)
	{
		OnDeleteMessagesResponse.ExecuteIfBound(Result);
	});

	DeleteMessages(Channel, NativeCallback, DeleteMessagesSettings);
}

void UPubnubSubsystem::DeleteMessages(FString Channel, FOnDeleteMessagesResponseNative NativeCallback, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, NativeCallback, DeleteMessagesSettings]
	{
		DeleteMessages_priv(Channel, NativeCallback, DeleteMessagesSettings);
	});
}

void UPubnubSubsystem::DeleteMessages(FString Channel, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, DeleteMessagesSettings]
	{
		DeleteMessages_priv(Channel, nullptr, DeleteMessagesSettings);
	});
}

void UPubnubSubsystem::MessageCounts(FString Channel, FString Timetoken, FOnMessageCountsResponse OnMessageCountsResponse)
{
	FOnMessageCountsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnMessageCountsResponse](const FPubnubOperationResult& Result, int MessageCounts)
	{
		OnMessageCountsResponse.ExecuteIfBound(Result, MessageCounts);
	});

	MessageCounts(Channel, Timetoken, NativeCallback);
}

void UPubnubSubsystem::MessageCounts(FString Channel, FString Timetoken, FOnMessageCountsResponseNative NativeCallback)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, Timetoken, NativeCallback]
	{
		MessageCounts_priv(Channel, Timetoken, NativeCallback);
	});
}

void UPubnubSubsystem::GetAllUserMetadataRaw(FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	FOnGetAllUserMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetAllUserMetadataResponse](const FPubnubOperationResult& Result, const TArray<FPubnubUserData>& UsersData, FString PageNext, FString PagePrev)
	{
		OnGetAllUserMetadataResponse.ExecuteIfBound(Result, UsersData, PageNext, PagePrev);
	});
	GetAllUserMetadataRaw(NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::GetAllUserMetadataRaw(FOnGetAllUserMetadataResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetAllUserMetadata_DATA_priv(NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::GetAllUserMetadata(FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FString PageNext, FString PagePrev)
{
	GetAllUserMetadataRaw(OnGetAllUserMetadataResponse, UPubnubUtilities::GetAllIncludeToString(Include), Limit, Filter, UPubnubUtilities::GetAllSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::GetAllUserMetadata(FOnGetAllUserMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FString PageNext, FString PagePrev)
{
	GetAllUserMetadataRaw(NativeCallback, UPubnubUtilities::GetAllIncludeToString(Include), Limit, Filter, UPubnubUtilities::GetAllSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::GetAllUserMetadata_JSON(FOnPubnubResponse OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, OnGetAllUserMetadataResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetAllUserMetadata_JSON_priv(OnGetAllUserMetadataResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::SetUserMetadataRaw(FString User, FString UserMetadataObj, FOnSetUserMetadataResponse OnSetUserMetadataResponse, FString Include)
{
	FOnSetUserMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSetUserMetadataResponse](const FPubnubOperationResult& Result, FPubnubUserData UserData)
	{
		OnSetUserMetadataResponse.ExecuteIfBound(Result, UserData);
	});
	SetUserMetadataRaw(User, UserMetadataObj, NativeCallback, Include);
}

void UPubnubSubsystem::SetUserMetadataRaw(FString User, FString UserMetadataObj, FOnSetUserMetadataResponseNative NativeCallback, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User, UserMetadataObj, NativeCallback, Include]
	{
		SetUserMetadata_priv(User, UserMetadataObj, NativeCallback, Include);
	});
}

void UPubnubSubsystem::SetUserMetadata(FString User, FPubnubUserData UserMetadata, FOnSetUserMetadataResponse OnSetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	SetUserMetadataRaw(User, UPubnubJsonUtilities::GetJsonFromUserData(UserMetadata), OnSetUserMetadataResponse, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubSubsystem::SetUserMetadata(FString User, FPubnubUserData UserMetadata, FOnSetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	SetUserMetadataRaw(User, UPubnubJsonUtilities::GetJsonFromUserData(UserMetadata), NativeCallback, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubSubsystem::GetUserMetadataRaw(FString User, FOnGetUserMetadataResponse OnGetUserMetadataResponse, FString Include)
{
	FOnGetUserMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetUserMetadataResponse](const FPubnubOperationResult& Result, FPubnubUserData UserData)
	{
		OnGetUserMetadataResponse.ExecuteIfBound(Result, UserData);
	});
	GetUserMetadataRaw(User, NativeCallback, Include);
}

void UPubnubSubsystem::GetUserMetadataRaw(FString User, FOnGetUserMetadataResponseNative NativeCallback, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User, NativeCallback, Include]
	{
		GetUserMetadata_DATA_priv(User, NativeCallback, Include);
	});
}

void UPubnubSubsystem::GetUserMetadata(FString User, FOnGetUserMetadataResponse OnGetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	GetUserMetadataRaw(User, OnGetUserMetadataResponse, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubSubsystem::GetUserMetadata(FString User, FOnGetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	GetUserMetadataRaw(User, NativeCallback, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubSubsystem::GetUserMetadata_JSON(FString User, FOnPubnubResponse OnGetUserMetadataResponse, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User, OnGetUserMetadataResponse, Include]
	{
		GetUserMetadata_JSON_priv(User, OnGetUserMetadataResponse, Include);
	});
}

void UPubnubSubsystem::RemoveUserMetadata(FString User, FOnRemoveUserMetadataResponse OnRemoveUserMetadataResponse)
{
	FOnRemoveUserMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveUserMetadataResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveUserMetadataResponse.ExecuteIfBound(Result);
	});
	RemoveUserMetadata(User, NativeCallback);
}

void UPubnubSubsystem::RemoveUserMetadata(FString User, FOnRemoveUserMetadataResponseNative NativeCallback)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User, NativeCallback]
	{
		RemoveUserMetadata_priv(User, NativeCallback);
	});
}

void UPubnubSubsystem::GetAllChannelMetadataRaw(FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	FOnGetAllChannelMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetAllChannelMetadataResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelData>& ChannelsData, FString PageNext, FString PagePrev)
	{
		OnGetAllChannelMetadataResponse.ExecuteIfBound(Result, ChannelsData, PageNext, PagePrev);
	});
	GetAllChannelMetadataRaw(NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::GetAllChannelMetadataRaw(FOnGetAllChannelMetadataResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetAllChannelMetadata_DATA_priv(NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::GetAllChannelMetadata(FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FString PageNext, FString PagePrev)
{
	GetAllChannelMetadataRaw(OnGetAllChannelMetadataResponse, UPubnubUtilities::GetAllIncludeToString(Include), Limit, Filter, UPubnubUtilities::GetAllSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::GetAllChannelMetadata(FOnGetAllChannelMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FString PageNext, FString PagePrev)
{
	GetAllChannelMetadataRaw(NativeCallback, UPubnubUtilities::GetAllIncludeToString(Include), Limit, Filter, UPubnubUtilities::GetAllSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::GetAllChannelMetadata_JSON(FOnPubnubResponse OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, OnGetAllChannelMetadataResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetAllChannelMetadata_JSON_priv(OnGetAllChannelMetadataResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::SetChannelMetadataRaw(FString Channel, FString ChannelMetadataObj, FOnSetChannelMetadataResponse OnSetChannelMetadataResponse, FString Include)
{
	FOnSetChannelMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSetChannelMetadataResponse](const FPubnubOperationResult& Result, FPubnubChannelData ChannelData)
	{
		OnSetChannelMetadataResponse.ExecuteIfBound(Result, ChannelData);
	});
	SetChannelMetadataRaw(Channel, ChannelMetadataObj, NativeCallback, Include);
}

void UPubnubSubsystem::SetChannelMetadataRaw(FString Channel, FString ChannelMetadataObj, FOnSetChannelMetadataResponseNative NativeCallback, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, ChannelMetadataObj, NativeCallback, Include]
	{
		SetChannelMetadata_priv(Channel, ChannelMetadataObj, NativeCallback, Include);
	});
}

void UPubnubSubsystem::SetChannelMetadata(FString Channel, FPubnubChannelData ChannelMetadata, FOnSetChannelMetadataResponse OnSetChannelMetadataResponse, FPubnubGetMetadataInclude Include)
{
	SetChannelMetadataRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelData(ChannelMetadata), OnSetChannelMetadataResponse, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubSubsystem::SetChannelMetadata(FString Channel, FPubnubChannelData ChannelMetadata, FOnSetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	SetChannelMetadataRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelData(ChannelMetadata), NativeCallback, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubSubsystem::GetChannelMetadataRaw(FString Channel, FOnGetChannelMetadataResponse OnGetChannelMetadataResponse, FString Include)
{
    FOnGetChannelMetadataResponseNative NativeCallback;
    NativeCallback.BindLambda([OnGetChannelMetadataResponse](const FPubnubOperationResult& Result, FPubnubChannelData ChannelData)
    {
        OnGetChannelMetadataResponse.ExecuteIfBound(Result, ChannelData);
    });
    GetChannelMetadataRaw(Channel, NativeCallback, Include);
}

void UPubnubSubsystem::GetChannelMetadataRaw(FString Channel, FOnGetChannelMetadataResponseNative NativeCallback, FString Include)
{
    if (!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
    {
        return;
    }

    QuickActionThread->AddFunctionToQueue([this, Channel, NativeCallback, Include]
    {
        GetChannelMetadata_DATA_priv(Channel, NativeCallback, Include);
    });
}

void UPubnubSubsystem::GetChannelMetadata(FString Channel, FOnGetChannelMetadataResponse OnGetChannelMetadataResponse, FPubnubGetMetadataInclude Include)
{
	GetChannelMetadataRaw(Channel, OnGetChannelMetadataResponse, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubSubsystem::GetChannelMetadata(FString Channel, FOnGetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	GetChannelMetadataRaw(Channel, NativeCallback, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubSubsystem::GetChannelMetadata_JSON(FString Channel, FOnPubnubResponse OnGetChannelMetadataResponse, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, OnGetChannelMetadataResponse, Include]
	{
		GetChannelMetadata_JSON_priv(Channel, OnGetChannelMetadataResponse, Include);
	});
}

void UPubnubSubsystem::RemoveChannelMetadata(FString Channel, FOnRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse)
{
	FOnRemoveChannelMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveChannelMetadataResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveChannelMetadataResponse.ExecuteIfBound(Result);
	});
	RemoveChannelMetadata(Channel, NativeCallback);
}

void UPubnubSubsystem::RemoveChannelMetadata(FString Channel, FOnRemoveChannelMetadataResponseNative NativeCallback)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, NativeCallback]
	{
		RemoveChannelMetadata_priv(Channel, NativeCallback);
	});
}

void UPubnubSubsystem::GetMembershipsRaw(FString User, FOnGetMembershipsResponse OnGetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	FOnGetMembershipsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetMembershipResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		OnGetMembershipResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});

	GetMembershipsRaw(User, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
}

void UPubnubSubsystem::GetMembershipsRaw(FString User, FOnGetMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetMemberships_DATA_priv(User, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::GetMemberships(FString User, FOnGetMembershipsResponse OnGetMembershipResponse, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	GetMembershipsRaw(User, OnGetMembershipResponse, UPubnubUtilities::MembershipIncludeToString(Include), Limit, Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::GetMemberships(FString User, FOnGetMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	GetMembershipsRaw(User, NativeCallback, UPubnubUtilities::MembershipIncludeToString(Include), Limit, Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::GetMemberships_JSON(FString User, FOnPubnubResponse OnGetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User, OnGetMembershipResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetMemberships_JSON_priv(User, OnGetMembershipResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::SetMembershipsRaw(FString User, FString SetObj, FOnSetMembershipsResponse OnSetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	FOnGetMembershipsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSetMembershipResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		OnSetMembershipResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});

	SetMembershipsRaw(User, SetObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
}

void UPubnubSubsystem::SetMembershipsRaw(FString User, FString SetObj, FOnSetMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User, SetObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		SetMemberships_priv(User, SetObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::SetMemberships(FString User, TArray<FPubnubMembershipData> MembershipsData, FOnSetMembershipsResponse OnSetMembershipResponse, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	SetMembershipsRaw(User, UPubnubJsonUtilities::GetJsonFromMembershipsDataArray(MembershipsData), OnSetMembershipResponse, UPubnubUtilities::MembershipIncludeToString(Include), Limit, Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::SetMemberships(FString User, TArray<FPubnubMembershipData> MembershipsData, FOnSetMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	SetMembershipsRaw(User, UPubnubJsonUtilities::GetJsonFromMembershipsDataArray(MembershipsData), NativeCallback, UPubnubUtilities::MembershipIncludeToString(Include), Limit, Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::RemoveMembershipsRaw(FString User, FString RemoveObj, FOnRemoveMembershipsResponse OnRemoveMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	FOnGetMembershipsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveMembershipResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		OnRemoveMembershipResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});

	RemoveMembershipsRaw(User, RemoveObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
}

void UPubnubSubsystem::RemoveMembershipsRaw(FString User, FString RemoveObj, FOnRemoveMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User, RemoveObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		RemoveMemberships_priv(User, RemoveObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::RemoveMemberships(FString User, TArray<FString> Channels, FOnRemoveMembershipsResponse OnRemoveMembershipResponse, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	RemoveMembershipsRaw(User, UPubnubJsonUtilities::GetJsonFromMembershipsToRemove(Channels), OnRemoveMembershipResponse, UPubnubUtilities::MembershipIncludeToString(Include), Limit, Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::RemoveMemberships(FString User, TArray<FString> Channels, FOnRemoveMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	RemoveMembershipsRaw(User, UPubnubJsonUtilities::GetJsonFromMembershipsToRemove(Channels), NativeCallback, UPubnubUtilities::MembershipIncludeToString(Include), Limit, Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::GetChannelMembersRaw(FString Channel, FOnGetChannelMembersResponse OnGetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    FOnGetChannelMembersResponseNative NativeCallback;
    NativeCallback.BindLambda([OnGetMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
    {
        OnGetMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
    });
    GetChannelMembersRaw(Channel, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::GetChannelMembersRaw(FString Channel, FOnGetChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    if (!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
    {
        return;
    }

    QuickActionThread->AddFunctionToQueue([this, Channel, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count]
    {
        GetChannelMembers_DATA_priv(Channel, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);
    });
}

void UPubnubSubsystem::GetChannelMembers(FString Channel, FOnGetChannelMembersResponse OnGetMembersResponse, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    GetChannelMembersRaw(Channel, OnGetMembersResponse, UPubnubUtilities::MemberIncludeToString(Include), Limit, Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::GetChannelMembers(FString Channel, FOnGetChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    GetChannelMembersRaw(Channel, NativeCallback, UPubnubUtilities::MemberIncludeToString(Include), Limit, Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::GetChannelMembers_JSON(FString Channel, FOnPubnubResponse OnGetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
    {return;}
    
    QuickActionThread->AddFunctionToQueue( [this, Channel, OnGetMembersResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
    {
        GetChannelMembers_JSON_priv(Channel, OnGetMembersResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
    });
}

void UPubnubSubsystem::AddChannelMembers(FString Channel, FString AddObj, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, AddObj, Include]
	{
		AddChannelMembers_priv(Channel, AddObj, Include);
	});
}

void UPubnubSubsystem::SetChannelMembersRaw(FString Channel, FString SetObj, FOnSetChannelMembersResponse OnSetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    FOnSetChannelMembersResponseNative NativeCallback;
    NativeCallback.BindLambda([OnSetMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
    {
        OnSetMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
    });
    SetChannelMembersRaw(Channel, SetObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::SetChannelMembersRaw(FString Channel, FString SetObj, FOnSetChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    if (!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
    {
        return;
    }

    QuickActionThread->AddFunctionToQueue([this, Channel, SetObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count]
    {
        SetChannelMembers_priv(Channel, SetObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);
    });
}

void UPubnubSubsystem::SetChannelMembers(FString Channel, TArray<FPubnubChannelMemberData> ChannelMembersData, FOnSetChannelMembersResponse OnSetMembersResponse, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    SetChannelMembersRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersDataArray(ChannelMembersData), OnSetMembersResponse, UPubnubUtilities::MemberIncludeToString(Include), Limit, Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::SetChannelMembers(FString Channel, TArray<FPubnubChannelMemberData> ChannelMembersData, FOnSetChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    SetChannelMembersRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersDataArray(ChannelMembersData), NativeCallback, UPubnubUtilities::MemberIncludeToString(Include), Limit, Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::RemoveChannelMembersRaw(FString Channel, FString RemoveObj, FOnRemoveChannelMembersResponse OnRemoveMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    FOnRemoveChannelMembersResponseNative NativeCallback;
    NativeCallback.BindLambda([OnRemoveMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
    {
        OnRemoveMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
    });
    RemoveChannelMembersRaw(Channel, RemoveObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::RemoveChannelMembersRaw(FString Channel, FString RemoveObj, FOnRemoveChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    if (!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
    {
        return;
    }

    QuickActionThread->AddFunctionToQueue([this, Channel, RemoveObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count]
    {
        RemoveChannelMembers_priv(Channel, RemoveObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);
    });
}

void UPubnubSubsystem::RemoveChannelMembers(FString Channel, TArray<FString> Users, FOnRemoveChannelMembersResponse OnRemoveMembersResponse, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    RemoveChannelMembersRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersToRemove(Users), OnRemoveMembersResponse, UPubnubUtilities::MemberIncludeToString(Include), Limit, Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::RemoveChannelMembers(FString Channel, TArray<FString> Users, FOnRemoveChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    RemoveChannelMembersRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersToRemove(Users), NativeCallback, UPubnubUtilities::MemberIncludeToString(Include), Limit, Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::AddMessageAction(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnAddMessageActionResponse OnAddMessageActionResponse)
{
	FOnAddMessageActionResponseNative NativeCallback;
	NativeCallback.BindLambda([OnAddMessageActionResponse](const FPubnubOperationResult& Result, FPubnubMessageActionData MessageActionData)
	{
		OnAddMessageActionResponse.ExecuteIfBound(Result, MessageActionData);
	});
	AddMessageAction(Channel, MessageTimetoken, ActionType, Value, NativeCallback);
}

void UPubnubSubsystem::AddMessageAction(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnAddMessageActionResponseNative NativeCallback)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, MessageTimetoken, ActionType, Value, NativeCallback]
	{
		AddMessageAction_priv(Channel, MessageTimetoken, ActionType, Value, NativeCallback);
	});
}

void UPubnubSubsystem::GetMessageActions(FString Channel, FString Start, FString End, int Limit, FOnGetMessageActionsResponse OnGetMessageActionsResponse)
{
	FOnGetMessageActionsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetMessageActionsResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMessageActionData>& MessageActions)
	{
		OnGetMessageActionsResponse.ExecuteIfBound(Result, MessageActions);
	});
	GetMessageActions(Channel, Start, End, Limit, NativeCallback);
}

void UPubnubSubsystem::GetMessageActions(FString Channel, FString Start, FString End, int Limit, FOnGetMessageActionsResponseNative NativeCallback)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, Start, End, Limit, NativeCallback]
	{
		GetMessageActions_DATA_priv(Channel, Start, End, Limit, NativeCallback);
	});
}

void UPubnubSubsystem::GetMessageActions_JSON(FString Channel, FString Start, FString End, int SizeLimit, FOnPubnubResponse OnGetMessageActionsResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, Start, End, SizeLimit, OnGetMessageActionsResponse]
	{
		GetMessageActions_JSON_priv(Channel, Start, End, SizeLimit, OnGetMessageActionsResponse);
	});
}

void UPubnubSubsystem::RemoveMessageAction(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnRemoveMessageActionResponse OnRemoveMessageActionResponse)
{
	FOnRemoveMessageActionResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveMessageActionResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveMessageActionResponse.ExecuteIfBound(Result);
	});
	RemoveMessageAction(Channel, MessageTimetoken, ActionTimetoken, NativeCallback);
}

void UPubnubSubsystem::RemoveMessageAction(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnRemoveMessageActionResponseNative NativeCallback)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, MessageTimetoken, ActionTimetoken, NativeCallback]
	{
		RemoveMessageAction_priv(Channel, MessageTimetoken, ActionTimetoken, NativeCallback);
	});
}

/* DISABLED 
void UPubnubSubsystem::GetMessageActionsContinue(FOnPubnubResponse OnGetMessageActionsContinueResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, OnGetMessageActionsContinueResponse]
	{
		GetMessageActionsContinue_priv(OnGetMessageActionsContinueResponse);
	});
}
*/

void UPubnubSubsystem::ReconnectSubscriptions()
{
	pubnub_reconnect(ctx_ee, nullptr);
}


void UPubnubSubsystem::DisconnectSubscriptions()
{
	pubnub_disconnect(ctx_ee);
}

FString UPubnubSubsystem::GrantTokenStructureToJsonString(FPubnubGrantTokenStructure TokenStructure, bool &success)
{
	FString TokenJsonString;

	//Make sure token data is provided in correct form. There must be the same amount of object and permissions or just one permission,
	//then one permission is used for every object
	
	if(TokenStructure.Channels.Num() != TokenStructure.ChannelPermissions.Num() && TokenStructure.ChannelPermissions.Num() != 1)
	{
		PubnubError("Grant Token Structure To JsonString - Provide the same amount of ChannelPermissions and Channels (or only 1 ChannelPermission).", EPubnubErrorType::PET_Warning);
		success = false;
		return TokenJsonString;
	}

	if(TokenStructure.ChannelGroups.Num() != TokenStructure.ChannelGroupPermissions.Num() && TokenStructure.ChannelGroupPermissions.Num() != 1)
	{
		PubnubError("Grant Token Structure To JsonString - Provide the same amount of ChannelGroupPermissions and ChannelGroups (or only 1 ChannelGroupPermissions).", EPubnubErrorType::PET_Warning);
		success = false;
		return TokenJsonString;
	}

	if(TokenStructure.Users.Num() != TokenStructure.UserPermissions.Num() && TokenStructure.UserPermissions.Num() != 1)
	{
		PubnubError("Grant Token Structure To JsonString - Provide the same amount of UserPermissions and Users (or only 1 UserPermissions).", EPubnubErrorType::PET_Warning);
		success = false;
		return TokenJsonString;
	}

	if(TokenStructure.ChannelPatterns.Num() != TokenStructure.ChannelPatternPermissions.Num() && TokenStructure.ChannelPatternPermissions.Num() != 1)
	{
		PubnubError("Grant Token Structure To JsonString - Provide the same amount of ChannelPatternPermissions and ChannelsPatterns (or only 1 ChannelPatternPermissions).", EPubnubErrorType::PET_Warning);
		success = false;
		return TokenJsonString;
	}

	if(TokenStructure.ChannelGroupPatterns.Num() != TokenStructure.ChannelGroupPatternPermissions.Num() && TokenStructure.ChannelGroupPatternPermissions.Num() != 1)
	{
		PubnubError("Grant Token Structure To JsonString - Provide the same amount of ChannelGroupPatternPermissions and ChannelGroupsPatterns (or only 1 ChannelGroupPatternPermissions).", EPubnubErrorType::PET_Warning);
		success = false;
		return TokenJsonString;
	}

	if(TokenStructure.UserPatterns.Num() != TokenStructure.UserPatternPermissions.Num() && TokenStructure.UserPatternPermissions.Num() != 1)
	{
		PubnubError("Grant Token Structure To JsonString - Provide the same amount of UserPatternPermissions and UsersPatterns (or only 1 UserPatternPermissions).", EPubnubErrorType::PET_Warning);
		success = false;
		return TokenJsonString;
	}
	

	//Create Json objects with channels, groups, users permissions and their patterns
	TSharedPtr<FJsonObject> ChannelsJsonObject = AddChannelPermissionsToJson(TokenStructure.Channels, TokenStructure.ChannelPermissions);
	TSharedPtr<FJsonObject> ChannelGroupsJsonObject = AddChannelGroupPermissionsToJson(TokenStructure.ChannelGroups, TokenStructure.ChannelGroupPermissions);
	TSharedPtr<FJsonObject> UsersJsonObject = AddUserPermissionsToJson(TokenStructure.Users, TokenStructure.UserPermissions);
	TSharedPtr<FJsonObject> ChannelPatternsJsonObject = AddChannelPermissionsToJson(TokenStructure.ChannelPatterns, TokenStructure.ChannelPatternPermissions);
	TSharedPtr<FJsonObject> ChannelGroupPatternsJsonObject = AddChannelGroupPermissionsToJson(TokenStructure.ChannelGroupPatterns, TokenStructure.ChannelGroupPatternPermissions);
	TSharedPtr<FJsonObject> UserPatternsJsonObject = AddUserPermissionsToJson(TokenStructure.UserPatterns, TokenStructure.UserPatternPermissions);

	//Add resources fields
	TSharedPtr<FJsonObject> ResourcesJsonObject = MakeShareable(new FJsonObject);
	if(TokenStructure.Channels.Num() > 0)
	{
		ResourcesJsonObject->SetObjectField("channels", ChannelsJsonObject);
	}
	if(TokenStructure.ChannelGroups.Num() > 0)
	{
		ResourcesJsonObject->SetObjectField("groups", ChannelGroupsJsonObject);
	}
	if(TokenStructure.Users.Num() > 0)
	{
		ResourcesJsonObject->SetObjectField("uuids", UsersJsonObject);
	}

	//Add patterns fields
	TSharedPtr<FJsonObject> PatternsJsonObject = MakeShareable(new FJsonObject);
	if(TokenStructure.ChannelPatterns.Num() > 0)
	{
		PatternsJsonObject->SetObjectField("channels", ChannelPatternsJsonObject);
	}
	if(TokenStructure.ChannelGroupPatterns.Num() > 0)
	{
		PatternsJsonObject->SetObjectField("groups", ChannelGroupPatternsJsonObject);
	}
	if(TokenStructure.UserPatterns.Num() > 0)
	{
		PatternsJsonObject->SetObjectField("uuids", UserPatternsJsonObject);
	}

	TSharedPtr<FJsonObject> TokenStructureJsonObject = MakeShareable(new FJsonObject);
	TokenStructureJsonObject->SetObjectField("resources", ResourcesJsonObject);
	TokenStructureJsonObject->SetObjectField("patterns", PatternsJsonObject);

	TSharedPtr<FJsonObject> PermissionsJsonObject = MakeShareable(new FJsonObject);
	PermissionsJsonObject->SetNumberField("ttl", TokenStructure.TTLMinutes);
	PermissionsJsonObject->SetStringField("authorized_uuid", TokenStructure.AuthorizedUser);
	PermissionsJsonObject->SetObjectField("permissions", TokenStructureJsonObject);

	success = true;
	//Convert created Json object to string
	return UPubnubJsonUtilities::JsonObjectToString(PermissionsJsonObject);
}

FString UPubnubSubsystem::GetLastResponse(pubnub_t* context)
{
	FString Response;
	
	if(!context)
	{return Response;}
	
	pubnub_res PubnubResponse = pubnub_await(context);
	if (PNR_OK == PubnubResponse) {
		
		Response = pubnub_get(context);
	}
	else
	{
		PubnubResponseError(PubnubResponse, "Failed to get last response.");
	}
	return Response;
}

FString UPubnubSubsystem::GetLastChannelResponse(pubnub_t* context)
{
	FString Response;
	
	if(!context)
	{return Response;}
	
	pubnub_res PubnubResponse = pubnub_await(context);
	if (PNR_OK == PubnubResponse) {
		
		Response = pubnub_get_channel(context);
	}
	else
	{
		PubnubResponseError(PubnubResponse, "Failed to get last channel response.");
	}
	return Response;
}

void UPubnubSubsystem::PubnubError(FString ErrorMessage, EPubnubErrorType ErrorType)
{
	//Log and broadcast error message
	if(ErrorType == EPubnubErrorType::PET_Error)
	{
		UE_LOG(PubnubLog, Error, TEXT("%s"), *ErrorMessage);
	}
	else
	{
		UE_LOG(PubnubLog, Warning, TEXT("%s"), *ErrorMessage);
	}

	//Errors has to be broadcasted on GameThread, otherwise engine will crash if someone uses them for example with widgets
	AsyncTask(ENamedThreads::GameThread, [this, ErrorMessage, ErrorType]()
	{
		//Broadcast bound delegate with JsonResponse
		OnPubnubError.Broadcast(ErrorMessage, ErrorType);
		OnPubnubErrorNative.Broadcast(ErrorMessage, ErrorType);
	});
}

void UPubnubSubsystem::PubnubResponseError(pubnub_res PubnubResponse, FString ErrorMessage)
{
	//Convert all error data into single string
	FString ResponseString(pubnub_res_2_string(PubnubResponse));
	FString FinalErrorMessage = FString::Printf(TEXT("%s Error: %s."), *ErrorMessage, *ResponseString);

	//Log and broadcast error
	UE_LOG(PubnubLog, Error, TEXT("%s"), *FinalErrorMessage);

	//Errors has to be broadcasted on GameThread, otherwise engine will crash if someone uses them for example with widgets
	AsyncTask(ENamedThreads::GameThread, [this, FinalErrorMessage]()
	{
		//Broadcast bound delegate with JsonResponse
		OnPubnubError.Broadcast(FinalErrorMessage, EPubnubErrorType::PET_Error);
		OnPubnubErrorNative.Broadcast(FinalErrorMessage, EPubnubErrorType::PET_Error);
	});
}

void UPubnubSubsystem::PubnubPublishError()
{
	FString FinalErrorMessage;
	if(ctx_pub == nullptr)
	{
		FinalErrorMessage = "Can't publish message. Publish context is invalid";
	}
	
	FString PublishError(pubnub_last_publish_result(ctx_pub));
	FinalErrorMessage = FString::Printf(TEXT("Publish message failed. Error: %s."), *PublishError);

	//Log and broadcast error
	UE_LOG(PubnubLog, Error, TEXT("%s"), *FinalErrorMessage);

	//Errors has to be broadcasted on GameThread, otherwise engine will crash if someone uses them for example with widgets
	AsyncTask(ENamedThreads::GameThread, [this, FinalErrorMessage]()
	{
		//Broadcast bound delegate with JsonResponse
		OnPubnubError.Broadcast(FinalErrorMessage, EPubnubErrorType::PET_Error);
		OnPubnubErrorNative.Broadcast(FinalErrorMessage, EPubnubErrorType::PET_Error);
	});
}

void UPubnubSubsystem::LoadPluginSettings()
{
	//Save all settings
	PubnubPluginSettings = GetMutableDefault<UPubnubSettings>();
}

void UPubnubSubsystem::SavePubnubConfig(const FPubnubConfig& Config)
{
	PubnubConfig = Config;
	
	//Copy memory for chars containing keys
	FMemory::Memcpy(PublishKey, TCHAR_TO_ANSI(*Config.PublishKey), PublishKeySize);
	FMemory::Memcpy(SubscribeKey, TCHAR_TO_ANSI(*Config.SubscribeKey), PublishKeySize);
	FMemory::Memcpy(SecretKey, TCHAR_TO_ANSI(*Config.SecretKey), SecretKeySize);
	PublishKey[PublishKeySize] = '\0';
	SubscribeKey[PublishKeySize] = '\0';
	SecretKey[SecretKeySize] = '\0';
}

//This functions is a wrapper to IsInitialized bool, so it can print error if user is trying to do anything before initializing Pubnub correctly
bool UPubnubSubsystem::CheckIsPubnubInitialized()
{
	if(!IsInitialized)
	{
		PubnubError("Pubnub is not initialized correctly. Aborting operation. Make sure to call InitPubnub or "
			"set InitializeAutomatically to true in Pubnub SDK settings. Also make sure that publish and subscribe keys are correct.");
	}
	
	return IsInitialized;
}

//This functions is a wrapper to IsUserIDSet bool, so it can print error if user is trying to do anything before setting user ID correctly
bool UPubnubSubsystem::CheckIsUserIDSet()
{
	if(!IsUserIDSet)
	{
		PubnubError("Pubnub user ID is not set. Aborting operation");
	}
	
	return IsUserIDSet;
}

bool UPubnubSubsystem::CheckQuickActionThreadValidity()
{
	if(!QuickActionThread)
	{
		PubnubError("PublishThread is invalid. Aborting operation");
		return false;
	}
	
	return true;
}

FString UPubnubSubsystem::GetUserIDInternal()
{
	const char* UserIDChar = pubnub_user_id_get(ctx_pub);
	if(UserIDChar)
	{
		FString UserIDString(UserIDChar);
		return UserIDString;
	}

	return "";
}

bool UPubnubSubsystem::CheckIsFieldEmpty(FString Field, FString FieldName, FString FunctionName)
{
	if(Field.IsEmpty())
	{
		FString ErrorMessage = FString::Printf(TEXT("Warning: Can't use %s function. %s field shouldn't be empty."), *FunctionName, *FieldName);
		PubnubError(ErrorMessage, EPubnubErrorType::PET_Warning);
		return true;
	}

	return false;
}


/* PRIV FUNCTIONS */

void UPubnubSubsystem::InitPubnub_priv(const FPubnubConfig& Config)
{
	if(IsInitialized)
	{return;}
	
	//Make sure that keys are filled
	if(std::strlen(PublishKey) == 0 )
	{
		PubnubError("Publish key is empty, can't initialize Pubnub");
		return;
	}

	if(std::strlen(SubscribeKey) == 0 )
	{
		PubnubError("Subscribe key is empty, can't initialize Pubnub");
		return;
	}
	
	ctx_pub = pubnub_alloc();
	ctx_ee = pubnub_alloc();

	//Send logging callback to Pubnub sdk, so we can pass all logs to UE
	pubnub_set_log_callback(PubnubSDKLogConverter);
	
	pubnub_enforce_api(ctx_pub, PNA_SYNC);
	pubnub_enforce_api(ctx_ee, PNA_CALLBACK);

	pubnub_init(ctx_pub, PublishKey, SubscribeKey);
	pubnub_init(ctx_ee, PublishKey, SubscribeKey);

	pubnub_subscribe_status_callback_t Callback = +[](const pubnub_t *pb, const pubnub_subscription_status status, const pubnub_subscription_status_data_t status_data, void* _data)
	{
		UPubnubSubsystem* ThisSubsystem = static_cast<UPubnubSubsystem*>(_data);
		if(!ThisSubsystem)
		{return;}

		ThisSubsystem->OnCCoreSubscriptionStatusReceived(status, status_data);
	};
	//Register subscription status listener with callback created above
	pubnub_subscribe_add_status_listener(ctx_ee, Callback, this);
	
	IsInitialized = true;

	if(!Config.UserID.IsEmpty())
	{
		this->SetUserID(Config.UserID);
	}
	
	if(PubnubConfig.SetSecretKeyAutomatically)
	{
		SetSecretKey();
	}
}

void UPubnubSubsystem::SetUserID_priv(FString UserID)
{
	if(UserID.IsEmpty())
	{
		PubnubError("Can't Set User ID. User ID can't be empty");
		return;
	}

	pubnub_set_user_id(ctx_pub, TCHAR_TO_ANSI(*UserID));
	pubnub_set_user_id(ctx_ee, TCHAR_TO_ANSI(*UserID));

	IsUserIDSet = true;
}

void UPubnubSubsystem::PublishMessage_priv(FString Channel, FString Message, FOnPublishMessageResponseNative OnPublishMessageResponse, FPubnubPublishSettings PublishSettings)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Channel, "Channel", "PublishMessage") || CheckIsFieldEmpty(Message, "Message", "PublishMessage"))
	{return;}

	FString FinalMessage = Message;

	//If provided string is not a valid Json object or array, we treat it as literal string and serialize it
	if(!UPubnubJsonUtilities::IsCorrectJsonString(Message, false))
	{
		FinalMessage = UPubnubJsonUtilities::SerializeString(FinalMessage);
	}
	
	//Convert all UE PublishSettings to Pubnub PublishOptions
	
	//Converted char needs to live in function scope, so we need to create it here
	pubnub_publish_options PubnubOptions;
	auto CharConverter = StringCast<ANSICHAR>(*PublishSettings.MetaData);
	PubnubOptions.meta = CharConverter.Get();
		
	auto TypeCharConverter = StringCast<ANSICHAR>(*PublishSettings.CustomMessageType);
	PubnubOptions.custom_message_type = TypeCharConverter.Get();
	
	PublishUESettingsToPubnubPublishOptions(PublishSettings, PubnubOptions);
	pubnub_publish_ex(ctx_pub, TCHAR_TO_ANSI(*Channel), TCHAR_TO_ANSI(*FinalMessage), PubnubOptions);

	pubnub_res PublishResultStatus = pubnub_await(ctx_pub);

	
	FPubnubMessageData PublishedMessage;
	FPubnubOperationResult PublishResult;

	//Fill data about Publish Result
	PublishResult.Status = pubnub_last_http_code(ctx_pub);
	PublishResult.ErrorMessage = pubnub_last_publish_result(ctx_pub);
	PublishResult.Error = PublishResultStatus != PNR_OK;
	
	if(PublishResultStatus == PNR_OK)
	{
		//If result is ok, fill all data about published message
		PublishedMessage.Message = Message;
		PublishedMessage.Channel = Channel;
		PublishedMessage.UserID = GetUserIDInternal();
		PublishedMessage.Timetoken = pubnub_last_publish_timetoken(ctx_pub);
		PublishedMessage.Metadata = PublishSettings.MetaData;
		PublishedMessage.MessageType = EPubnubMessageType::PMT_Published;
		PublishedMessage.CustomMessageType = PublishSettings.CustomMessageType;
	}
	else
	{
		PubnubPublishError();
	}
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnPublishMessageResponse, PublishResult, PublishedMessage]()
	{
		OnPublishMessageResponse.ExecuteIfBound(PublishResult, PublishedMessage);
	});
}

void UPubnubSubsystem::Signal_priv(FString Channel, FString Message, FOnSignalResponseNative OnSignalResponse, FPubnubSignalSettings SignalSettings)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Channel, "Channel", "Signal") || CheckIsFieldEmpty(Message, "Message", "Signal"))
	{return;}

	FString FinalMessage = Message;
	//If provided string is not a valid Json object or array, we treat it as literal string and serialize it
	if(!UPubnubJsonUtilities::IsCorrectJsonString(Message, false))
	{
		FinalMessage = UPubnubJsonUtilities::SerializeString(FinalMessage);
	}
	
	pubnub_signal_options PubnubOptions = pubnub_signal_defopts();
	auto CharConverter = StringCast<ANSICHAR>(*SignalSettings.CustomMessageType);
	PubnubOptions.custom_message_type = SignalSettings.CustomMessageType.IsEmpty() ? NULL : CharConverter.Get();
	pubnub_signal_ex(ctx_pub, TCHAR_TO_ANSI(*Channel), TCHAR_TO_ANSI(*FinalMessage), PubnubOptions);
	
	pubnub_res PublishResultStatus = pubnub_await(ctx_pub);

	FPubnubMessageData SignalMessage;
	FPubnubOperationResult PublishResult;

	PublishResult.Status = pubnub_last_http_code(ctx_pub);
	PublishResult.ErrorMessage = pubnub_last_publish_result(ctx_pub);
	PublishResult.Error = PublishResultStatus != PNR_OK;

	if(PublishResultStatus == PNR_OK)
	{
		SignalMessage.Message = Message;
		SignalMessage.Channel = Channel;
		SignalMessage.UserID = GetUserIDInternal();
		SignalMessage.Timetoken = pubnub_last_publish_timetoken(ctx_pub);
		SignalMessage.Metadata = ""; // Signals don't have metadata
		SignalMessage.MessageType = EPubnubMessageType::PMT_Signal;
		SignalMessage.CustomMessageType = SignalSettings.CustomMessageType;
	}
	else
	{
		PubnubPublishError();
	}

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnSignalResponse, PublishResult, SignalMessage]()
	{
		OnSignalResponse.ExecuteIfBound(PublishResult, SignalMessage);
	});
}

void UPubnubSubsystem::SubscribeToChannel_priv(FString Channel, FPubnubSubscribeSettings SubscribeSettings)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Channel, "Channel", "SubscribeToChannel"))
	{return;}

	if(ChannelSubscriptions.Contains(Channel))
	{return;}

	//Create subscription for channel entity
	pubnub_subscription_t* Subscription = UPubnubUtilities::EEGetSubscriptionForChannel(ctx_ee, Channel, SubscribeSettings);

	if(nullptr == Subscription)
	{
		PubnubError("Failed to subscribe to channel. Pubnub_subscription_alloc didn't create subscription");
		return;
	}

	//Create callback that will be triggered by the c-core event engine
	pubnub_subscribe_message_callback_t Callback = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		UPubnubSubsystem* ThisSubsystem = static_cast<UPubnubSubsystem*>(user_data);
		FPubnubMessageData MessageData = UEMessageFromPubnub(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, ThisSubsystem]()
		{
			if(ThisSubsystem)
			{
				ThisSubsystem->OnMessageReceived.Broadcast(MessageData);
				ThisSubsystem->OnMessageReceivedNative.Broadcast(MessageData);
			}
		});
	};

	//Add subscription listener and subscribe with subscription
	if(!UPubnubUtilities::EEAddListenerAndSubscribe(Subscription, Callback, this))
	{
		PubnubError("Failed to subscribe to channel.");
		return;
	}

	//Save Callback and Subscription, so later we can use it to unsubscribe
	CCoreSubscriptionData SubscriptionData{Callback, Subscription};
	ChannelSubscriptions.Add(Channel, SubscriptionData);
}

void UPubnubSubsystem::SubscribeToGroup_priv(FString GroupName, FPubnubSubscribeSettings SubscribeSettings)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(GroupName, "GroupName", "SubscribeToGroup"))
	{return;}

	if(ChannelGroupSubscriptions.Contains(GroupName))
	{return;}
	
	//Create subscription for channel group entity
	pubnub_subscription_t* Subscription = UPubnubUtilities::EEGetSubscriptionForChannelGroup(ctx_ee, GroupName, SubscribeSettings);

	if(nullptr == Subscription)
	{
		PubnubError("Failed to subscribe to group. Pubnub_subscription_alloc didn't create subscription");
		return;
	}

	//Create callback that will be triggered by the c-core event engine
	pubnub_subscribe_message_callback_t Callback = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		UPubnubSubsystem* ThisSubsystem = static_cast<UPubnubSubsystem*>(user_data);
		if(!ThisSubsystem)
		{return;}
		FPubnubMessageData MessageData = UEMessageFromPubnub(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, ThisSubsystem]()
		{
			if(ThisSubsystem)
			{
				ThisSubsystem->OnMessageReceived.Broadcast(MessageData);
				ThisSubsystem->OnMessageReceivedNative.Broadcast(MessageData);
			}
		});
	};

	//Add subscription listener and subscribe with subscription
	if(!UPubnubUtilities::EEAddListenerAndSubscribe(Subscription, Callback, this))
	{
		PubnubError("Failed to subscribe to group.");
		return;
	}

	//Save Callback and Subscription, so later we can use it to unsubscribe
	CCoreSubscriptionData SubscriptionData{Callback, Subscription};
	ChannelGroupSubscriptions.Add(GroupName, SubscriptionData);
}

void UPubnubSubsystem::UnsubscribeFromChannel_priv(FString Channel)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	CCoreSubscriptionData* SubscriptionData =  ChannelSubscriptions.Find(Channel);
	if(!SubscriptionData)
	{
		PubnubError("Failed to unsubscribe from channel. There is no such subscription");
		return;
	}

	//Remove subscription listener and unsubscribe with subscription
	if(!UPubnubUtilities::EERemoveListenerAndUnsubscribe(&SubscriptionData->Subscription, SubscriptionData->Callback, this))
	{
		PubnubError("Failed to unsubscribe.");
		return;
	}

	//Free subscription memory
	pubnub_subscription_free(&SubscriptionData->Subscription);

	ChannelSubscriptions.Remove(Channel);
}

void UPubnubSubsystem::UnsubscribeFromGroup_priv(FString GroupName)
{
	if(!CheckIsUserIDSet())
	{return;}

	CCoreSubscriptionData* SubscriptionData =  ChannelGroupSubscriptions.Find(GroupName);
	if(!SubscriptionData)
	{
		PubnubError("Failed to unsubscribe from channel. There is no such subscription");
		return;
	}

	//Remove subscription listener and unsubscribe with subscription
	if(!UPubnubUtilities::EERemoveListenerAndUnsubscribe(&SubscriptionData->Subscription, SubscriptionData->Callback, this))
	{
		PubnubError("Failed to unsubscribe.");
		return;
	}

	//Free subscription memory
	pubnub_subscription_free(&SubscriptionData->Subscription);

	ChannelGroupSubscriptions.Remove(GroupName);
}

void UPubnubSubsystem::UnsubscribeFromAll_priv()
{
	if(ChannelSubscriptions.IsEmpty() && ChannelGroupSubscriptions.IsEmpty())
	{return;}
	
	if(!CheckIsUserIDSet())
	{return;}

	pubnub_unsubscribe_all(ctx_ee);
	
	ChannelSubscriptions.Empty();
	ChannelGroupSubscriptions.Empty();
}

void UPubnubSubsystem::AddChannelToGroup_priv(FString Channel, FString ChannelGroup, FOnAddChannelToGroupResponseNative OnAddChannelToGroupResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Channel, "Channel", "AddChannelToGroup") || CheckIsFieldEmpty(ChannelGroup, "ChannelGroup", "AddChannelToGroup"))
	{return;}
	
	pubnub_add_channel_to_group(ctx_pub, TCHAR_TO_ANSI(*Channel), TCHAR_TO_ANSI(*ChannelGroup));

	//This is just to clear the C-Core response buffer, but it doesn't return the server response
	GetLastResponse(ctx_pub);
	//So we need to get the response separately
	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnAddChannelToGroupResponse, JsonResponse]()
	{
		OnAddChannelToGroupResponse.ExecuteIfBound(UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse));
	});
}

void UPubnubSubsystem::RemoveChannelFromGroup_priv(FString Channel, FString ChannelGroup, FOnRemoveChannelFromGroupResponseNative OnRemoveChannelFromGroupResponse)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(Channel, "Channel", "RemoveChannelFromGroup") || CheckIsFieldEmpty(ChannelGroup, "ChannelGroup", "RemoveChannelFromGroup"))
	{return;}

	pubnub_remove_channel_from_group(ctx_pub, TCHAR_TO_ANSI(*Channel), TCHAR_TO_ANSI(*ChannelGroup));

	//This is just to clear the C-Core response buffer, but it doesn't return the server response
	GetLastResponse(ctx_pub);
	//So we need to get the response separately
	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnRemoveChannelFromGroupResponse, JsonResponse]()
	{
		OnRemoveChannelFromGroupResponse.ExecuteIfBound(UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse));
	});
}

FString UPubnubSubsystem::ListChannelsFromGroup_pn(FString ChannelGroup)
{
	pubnub_list_channel_group(ctx_pub, TCHAR_TO_ANSI(*ChannelGroup));
	return GetLastChannelResponse(ctx_pub);
}

void UPubnubSubsystem::ListChannelsFromGroup_JSON_priv(FString ChannelGroup, FOnPubnubResponse OnListChannelsResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelGroup, "ChannelGroup", "ListChannelsFromGroup"))
	{return;}
	
	FString JsonResponse = ListChannelsFromGroup_pn(ChannelGroup);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnListChannelsResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnListChannelsResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::ListChannelsFromGroup_DATA_priv(FString ChannelGroup, FOnListChannelsFromGroupResponseNative OnListChannelsResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelGroup, "ChannelGroup", "ListChannelsFromGroup"))
	{return;}
	
	FString JsonResponse = ListChannelsFromGroup_pn(ChannelGroup);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnListChannelsResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		TArray<FString> Channels;
		UPubnubJsonUtilities::ListChannelsFromGroupJsonToData(JsonResponse, Result, Channels);
		
		//Broadcast bound delegate with parsed response
		OnListChannelsResponse.ExecuteIfBound(Result, Channels);
	});
}

void UPubnubSubsystem::RemoveChannelGroup_priv(FString ChannelGroup, FOnRemoveChannelGroupResponseNative OnRemoveChannelGroupResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelGroup, "ChannelGroup", "RemoveChannelGroup"))
	{return;}

	pubnub_remove_channel_group(ctx_pub, TCHAR_TO_ANSI(*ChannelGroup));
	
	//This is just to clear the C-Core response buffer, but it doesn't return the server response
	GetLastResponse(ctx_pub);
	//So we need to get the response separately
	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnRemoveChannelGroupResponse, JsonResponse]()
	{
		OnRemoveChannelGroupResponse.ExecuteIfBound(UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse));
	});
}

FString UPubnubSubsystem::ListUsersFromChannel_pn(FString Channel, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	//Set all options from ListUsersFromChannelSettings
	
	//Converted char needs to live in function scope, so we need to create it here
	pubnub_here_now_options HereNowOptions;
	auto CharConverter = StringCast<ANSICHAR>(*ListUsersFromChannelSettings.ChannelGroup);
	HereNowOptions.channel_group = CharConverter.Get();
	
	HereNowUESettingsToPubnubHereNowOptions(ListUsersFromChannelSettings, HereNowOptions);
	
	pubnub_here_now_ex(ctx_pub, TCHAR_TO_ANSI(*Channel), HereNowOptions);
	return GetLastResponse(ctx_pub);
}

void UPubnubSubsystem::ListUsersFromChannel_JSON_priv(FString Channel, FOnPubnubResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(Channel, "Channel", "ListUsersFromChannel"))
	{return;}
	
	FString JsonResponse = ListUsersFromChannel_pn(Channel, ListUsersFromChannelSettings);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, ListUsersFromChannelResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		ListUsersFromChannelResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::ListUsersFromChannel_DATA_priv(FString Channel, FOnListUsersFromChannelResponseNative ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(Channel, "Channel", "ListUsersFromChannel"))
	{return;}
	
	FString JsonResponse = ListUsersFromChannel_pn(Channel, ListUsersFromChannelSettings);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, ListUsersFromChannelResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		FPubnubListUsersFromChannelWrapper Data;
		UPubnubJsonUtilities::ListUsersFromChannelJsonToData(JsonResponse, Result, Data);
		
		//Broadcast bound delegate with parsed response
		ListUsersFromChannelResponse.ExecuteIfBound(Result, Data);
	});
}

FString UPubnubSubsystem::ListUserSubscribedChannels_pn(FString UserID)
{
	pubnub_where_now(ctx_pub, TCHAR_TO_ANSI(*UserID));
	return GetLastResponse(ctx_pub);
}

void UPubnubSubsystem::ListUserSubscribedChannels_JSON_priv(FString UserID, FOnPubnubResponse ListUserSubscribedChannelsResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(UserID, "UserID", "ListUserSubscribedChannels"))
	{return;}

	FString JsonResponse = ListUserSubscribedChannels_pn(UserID);
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, ListUserSubscribedChannelsResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		ListUserSubscribedChannelsResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::ListUserSubscribedChannels_DATA_priv(FString UserID, FOnListUsersSubscribedChannelsResponseNative ListUserSubscribedChannelsResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(UserID, "UserID", "ListUserSubscribedChannels"))
	{return;}

	FString JsonResponse = ListUserSubscribedChannels_pn(UserID);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, ListUserSubscribedChannelsResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		TArray<FString> Channels;
		UPubnubJsonUtilities::ListUserSubscribedChannelsJsonToData(JsonResponse, Result, Channels);
		
		//Broadcast bound delegate with parsed response
		ListUserSubscribedChannelsResponse.ExecuteIfBound(Result, Channels);
	});
}

void UPubnubSubsystem::SetState_priv(FString Channel, FString StateJson, FOnSetStateResponseNative OnSetStateResponse, FPubnubSetStateSettings SetStateSettings)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Channel, "Channel", "SetState") || CheckIsFieldEmpty(StateJson, "StateJson", "SetState"))
	{return;}

	if(!UPubnubJsonUtilities::IsCorrectJsonString(StateJson, false))
	{
		PubnubError("Can't Set State, StateJson has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}
	
	//Set all options from SetStateSettings

	//Converted char needs to live in function scope, so we need to create it here
	pubnub_set_state_options SetStateOptions;
	auto CharConverter = StringCast<ANSICHAR>(*SetStateSettings.ChannelGroup);
	SetStateOptions.channel_group = CharConverter.Get();
	auto UserIDCharConverter = StringCast<ANSICHAR>(*SetStateSettings.UserID);
	SetStateOptions.user_id = UserIDCharConverter.Get();

	SetStateUESettingsToPubnubSetStateOptions(SetStateSettings, SetStateOptions);
	
	pubnub_set_state_ex(ctx_pub, TCHAR_TO_ANSI(*Channel), TCHAR_TO_ANSI(*StateJson), SetStateOptions);
	
	//This is just to clear the C-Core response buffer, but it doesn't return the server response
	GetLastResponse(ctx_pub);
	//So we need to get the response separately
	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnSetStateResponse, JsonResponse]()
	{
		OnSetStateResponse.ExecuteIfBound(UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse));
	});
}

void UPubnubSubsystem::GetState_priv(FString Channel, FString ChannelGroup, FString UserID, FOnGetStateResponseNative OnGetStateResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(Channel.IsEmpty() && ChannelGroup.IsEmpty())
	{
		PubnubError("Warning: Can't use GetState function. At least one of: ChannelGroup, Channel can't be empty", EPubnubErrorType::PET_Warning);
		return;
	}

	pubnub_state_get(ctx_pub, TCHAR_TO_ANSI(*Channel), TCHAR_TO_ANSI(*ChannelGroup), TCHAR_TO_ANSI(*UserID));
	FString JsonResponse = GetLastResponse(ctx_pub);
	
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetStateResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetStateResponse.ExecuteIfBound(UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse), JsonResponse);
	});
}

void UPubnubSubsystem::Heartbeat_priv(FString Channel, FString ChannelGroup)
{
	if(!CheckIsUserIDSet())
	{return;}

	pubnub_heartbeat(ctx_pub, TCHAR_TO_ANSI(*Channel), TCHAR_TO_ANSI(*ChannelGroup));

	GetLastResponse(ctx_pub);
}

void UPubnubSubsystem::GrantToken_priv(FString PermissionObject, FOnGrantTokenResponseNative OnGrantTokenResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(PermissionObject, "PermissionObject", "GrantToken"))
	{return;}
	
	pubnub_grant_token(ctx_pub, TCHAR_TO_ANSI(*PermissionObject));

	pubnub_await(ctx_pub);
	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	//Access Manager has similar result structure to AppContext, so we use the same getter
	FPubnubOperationResult Result = UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(JsonResponse);
	FString Token = "";
	if(Result.Status == 200)
	{
		pubnub_chamebl_t grant_token_resp = pubnub_get_grant_token(ctx_pub);
		Token = FString(grant_token_resp.ptr);
	}
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGrantTokenResponse, Result, Token]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGrantTokenResponse.ExecuteIfBound(Result, Token);
	});
	
}

void UPubnubSubsystem::RevokeToken_priv(FString Token, FOnRevokeTokenResponseNative OnRevokeTokenResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Token, "Token", "RevokeToken"))
	{return;}
	
	pubnub_revoke_token(ctx_pub, TCHAR_TO_ANSI(*Token));

	FString JsonResponse = GetLastResponse(ctx_pub);

	//If response is empty, there was server error. 
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnRevokeTokenResponse, JsonResponse]()
	{
		//Access Manager has similar result structure to AppContext, so we use the same getter
		OnRevokeTokenResponse.ExecuteIfBound(UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(JsonResponse));
	});
}

FString UPubnubSubsystem::ParseToken_priv(FString Token)
{
	if(!CheckIsUserIDSet())
	{return "";}

	if(CheckIsFieldEmpty(Token, "Token", "ParseToken"))
	{return "";}
	
	char* TokenResponse = pubnub_parse_token(ctx_pub, TCHAR_TO_ANSI(*Token));
	FString ParsedToken(TokenResponse);
	//Free this char, as it's allocated with malloc inside of pubnub_parse_token
	free(TokenResponse);
	
	return ParsedToken;
}

FString UPubnubSubsystem::FetchHistory_pn(FString Channel, FPubnubFetchHistorySettings FetchHistorySettings)
{
	//Set all options from HistorySettings

	//Converted char needs to live in function scope, so we need to create it here
	pubnub_fetch_history_options FetchHistoryOptions;
	auto StartCharConverter = StringCast<ANSICHAR>(*FetchHistorySettings.Start);
	FetchHistoryOptions.start = StartCharConverter.Get();
	auto EndCharConverter = StringCast<ANSICHAR>(*FetchHistorySettings.End);
	FetchHistoryOptions.end = EndCharConverter.Get();

	FetchHistoryUESettingsToPbFetchHistoryOptions(FetchHistorySettings, FetchHistoryOptions);
	
	pubnub_fetch_history(ctx_pub, TCHAR_TO_ANSI(*Channel), FetchHistoryOptions);

	return GetLastResponse(ctx_pub);
}

void UPubnubSubsystem::FetchHistory_JSON_priv(FString Channel, FOnPubnubResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Channel, "Channel", "FetchHistory"))
	{return;}
	
	FString JsonResponse = FetchHistory_pn(Channel, FetchHistorySettings);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnFetchHistoryResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnFetchHistoryResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::FetchHistory_DATA_priv(FString Channel, FOnFetchHistoryResponseNative OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Channel, "Channel", "FetchHistory"))
	{return;}
	
	FString JsonResponse = FetchHistory_pn(Channel, FetchHistorySettings);

	//If response is empty, there was server error. 
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnFetchHistoryResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		TArray<FPubnubHistoryMessageData> Messages;
		UPubnubJsonUtilities::FetchHistoryJsonToData(JsonResponse, Result, Messages);
				
		//Broadcast bound delegate with parsed response
		OnFetchHistoryResponse.ExecuteIfBound(Result, Messages);
	});
}

void UPubnubSubsystem::DeleteMessages_priv(FString Channel, FOnDeleteMessagesResponseNative OnDeleteMessagesResponse, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Channel, "Channel", "DeleteMessages"))
	{return;}

	pubnub_delete_messages_options DeleteMessagesOptions = pubnub_delete_messages_defopts();
	auto StartCharConverter = StringCast<ANSICHAR>(*DeleteMessagesSettings.Start);
	DeleteMessagesOptions.start = StartCharConverter.Get();
	auto EndCharConverter = StringCast<ANSICHAR>(*DeleteMessagesSettings.End);
	DeleteMessagesOptions.end = EndCharConverter.Get();

	pubnub_delete_messages(ctx_pub, TCHAR_TO_ANSI(*Channel), DeleteMessagesOptions);
	
	FString JsonResponse = GetLastResponse(ctx_pub);

	//If response is empty, there was server error. 
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnDeleteMessagesResponse, JsonResponse]()
	{
		OnDeleteMessagesResponse.ExecuteIfBound(UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse));
	});
}

void UPubnubSubsystem::MessageCounts_priv(FString Channel, FString Timetoken, FOnMessageCountsResponseNative OnMessageCountsResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Channel, "Channel", "MessageCounts"))
	{return;}
	
	pubnub_message_counts(ctx_pub, TCHAR_TO_ANSI(*Channel), TCHAR_TO_ANSI(*Timetoken));

	pubnub_await(ctx_pub);

	int MessageCountsNumber = 0;
	pubnub_get_message_counts(ctx_pub, TCHAR_TO_ANSI(*Channel), &MessageCountsNumber);

	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnMessageCountsResponse, MessageCountsNumber, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnMessageCountsResponse.ExecuteIfBound(UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse), MessageCountsNumber);
	});
}

FString UPubnubSubsystem::GetAllUserMetadata_pn(FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	pubnub_getall_metadata_opts PubnubOptions = pubnub_getall_metadata_defopts();
	auto CharConverterInclude = StringCast<ANSICHAR>(*Include);
	PubnubOptions.include = Include.IsEmpty() ? NULL : CharConverterInclude.Get();
	auto CharConverterFilter = StringCast<ANSICHAR>(*Filter);
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  CharConverterFilter.Get();
	auto CharConverterSort = StringCast<ANSICHAR>(*Sort);
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  CharConverterSort.Get();
	auto CharConverterPageNext = StringCast<ANSICHAR>(*PageNext);
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  CharConverterPageNext.Get();
	auto CharConverterPagePrev = StringCast<ANSICHAR>(*PagePrev);
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  CharConverterPagePrev.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;
	
	pubnub_getall_uuidmetadata_ex(ctx_pub, PubnubOptions);

	FString Response = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(Response.IsEmpty())
	{
		Response = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	return Response;
}

void UPubnubSubsystem::GetAllUserMetadata_JSON_priv(FOnPubnubResponse OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	FString JsonResponse = GetAllUserMetadata_pn(Include, Limit, Filter, Sort, PageNext, PagePrev, Count);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetAllUserMetadataResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetAllUserMetadataResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::GetAllUserMetadata_DATA_priv(FOnGetAllUserMetadataResponseNative OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}

	FString JsonResponse = GetAllUserMetadata_pn(Include, Limit, Filter, Sort, PageNext, PagePrev, Count);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetAllUserMetadataResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		TArray<FPubnubUserData> UsersData;
		FString PageNext;
		FString PagePrev;
		UPubnubJsonUtilities::GetAllUserMetadataJsonToData(JsonResponse, Result, UsersData, PageNext, PagePrev);
						
		//Broadcast bound delegate with parsed response
		OnGetAllUserMetadataResponse.ExecuteIfBound(Result, UsersData, PageNext, PagePrev);
	});
}

void UPubnubSubsystem::SetUserMetadata_priv(FString User, FString UserMetadataObj, FOnSetUserMetadataResponseNative OnSetUserMetadataResponse, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "SetUserMetadata") || CheckIsFieldEmpty(UserMetadataObj, "UserMetadataObj", "SetUserMetadata"))
	{return;}

	if(!UPubnubJsonUtilities::IsCorrectJsonString(UserMetadataObj, false))
	{
		PubnubError("Can't Set User Metadata, UserMetadataObj has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}
	
	pubnub_set_uuidmetadata(ctx_pub, TCHAR_TO_ANSI(*User), TCHAR_TO_ANSI(*Include), TCHAR_TO_ANSI(*UserMetadataObj));

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnSetUserMetadataResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		FPubnubUserData UserData;
		UPubnubJsonUtilities::GetUserMetadataJsonToData(JsonResponse, Result, UserData);
								
		//Broadcast bound delegate with parsed response
		OnSetUserMetadataResponse.ExecuteIfBound(Result, UserData);
	});
}

FString UPubnubSubsystem::GetUserMetadata_pn(FString User, FString Include)
{
	pubnub_get_uuidmetadata(ctx_pub, TCHAR_TO_ANSI(*Include), TCHAR_TO_ANSI(*User));

	FString Response = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(Response.IsEmpty())
	{
		Response = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	return Response;
}

void UPubnubSubsystem::GetUserMetadata_JSON_priv(FString User, FOnPubnubResponse OnGetUserMetadataResponse, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "GetUUIDMetadata"))
	{return;}
	
	FString JsonResponse = GetUserMetadata_pn(User, Include);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetUserMetadataResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetUserMetadataResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::GetUserMetadata_DATA_priv(FString User, FOnGetUserMetadataResponseNative OnGetUserMetadataResponse, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "GetUserMetadata"))
	{return;}

	FString JsonResponse = GetUserMetadata_pn(User, Include);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetUserMetadataResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		FPubnubUserData UserData;
		UPubnubJsonUtilities::GetUserMetadataJsonToData(JsonResponse, Result, UserData);
								
		//Broadcast bound delegate with parsed response
		OnGetUserMetadataResponse.ExecuteIfBound(Result, UserData);
	});
}

void UPubnubSubsystem::RemoveUserMetadata_priv(FString User, FOnRemoveUserMetadataResponseNative OnRemoveUserMetadataResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "RemoveUserMetadata"))
	{return;}
	
	pubnub_remove_uuidmetadata(ctx_pub, TCHAR_TO_ANSI(*User));

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnRemoveUserMetadataResponse, JsonResponse]()
	{
		//Broadcast bound delegate with parsed response
		OnRemoveUserMetadataResponse.ExecuteIfBound(UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(JsonResponse));
	});
}

FString UPubnubSubsystem::GetAllChannelMetadata_pn(FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	pubnub_getall_metadata_opts PubnubOptions = pubnub_getall_metadata_defopts();
	auto CharConverterInclude = StringCast<ANSICHAR>(*Include);
	PubnubOptions.include = Include.IsEmpty() ? NULL : CharConverterInclude.Get();
	auto CharConverterFilter = StringCast<ANSICHAR>(*Filter);
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  CharConverterFilter.Get();
	auto CharConverterSort = StringCast<ANSICHAR>(*Sort);
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  CharConverterSort.Get();
	auto CharConverterPageNext = StringCast<ANSICHAR>(*PageNext);
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  CharConverterPageNext.Get();
	auto CharConverterPagePrev = StringCast<ANSICHAR>(*PagePrev);
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  CharConverterPagePrev.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;
	
	pubnub_getall_channelmetadata_ex(ctx_pub, PubnubOptions);

	FString Response = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(Response.IsEmpty())
	{
		Response = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	return Response;
}

void UPubnubSubsystem::GetAllChannelMetadata_JSON_priv(FOnPubnubResponse OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	FString JsonResponse = GetAllChannelMetadata_pn(Include, Limit, Filter, Sort, PageNext, PagePrev, Count);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetAllChannelMetadataResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetAllChannelMetadataResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::GetAllChannelMetadata_DATA_priv(FOnGetAllChannelMetadataResponseNative OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	FString JsonResponse = GetAllChannelMetadata_pn(Include, Limit, Filter, Sort, PageNext, PagePrev, Count);
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetAllChannelMetadataResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		TArray<FPubnubChannelData> ChannelsData;
		FString PageNext;
		FString PagePrev;
		UPubnubJsonUtilities::GetAllChannelMetadataJsonToData(JsonResponse, Result, ChannelsData, PageNext, PagePrev);
								
		//Broadcast bound delegate with parsed response
		OnGetAllChannelMetadataResponse.ExecuteIfBound(Result, ChannelsData, PageNext, PagePrev);
	});
}

void UPubnubSubsystem::SetChannelMetadata_priv(FString Channel, FString ChannelMetadataObj, FOnSetChannelMetadataResponseNative OnSetChannelMetadataResponse, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(Channel, "Channel", "SetChannelMetadata") || CheckIsFieldEmpty(ChannelMetadataObj, "ChannelMetadataObj", "SetChannelMetadata"))
	{return;}
	
	if(!UPubnubJsonUtilities::IsCorrectJsonString(ChannelMetadataObj, false))
	{
		PubnubError("Can't Set Channel Metadata, ChannelMetadataObj has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}
	
	pubnub_set_channelmetadata(ctx_pub, TCHAR_TO_ANSI(*Channel), TCHAR_TO_ANSI(*Include), TCHAR_TO_ANSI(*ChannelMetadataObj));

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnSetChannelMetadataResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		FPubnubChannelData ChannelData;
		UPubnubJsonUtilities::GetChannelMetadataJsonToData(JsonResponse, Result, ChannelData);
								
		//Broadcast bound delegate with parsed response
		OnSetChannelMetadataResponse.ExecuteIfBound(Result, ChannelData);
	});
}

FString UPubnubSubsystem::GetChannelMetadata_pn(FString Channel, FString Include)
{
	pubnub_get_channelmetadata(ctx_pub, TCHAR_TO_ANSI(*Include), TCHAR_TO_ANSI(*Channel));

	FString Response = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(Response.IsEmpty())
	{
		Response = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	return Response;
}

void UPubnubSubsystem::GetChannelMetadata_JSON_priv(FString Channel, FOnPubnubResponse OnGetChannelMetadataResponse, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(Channel, "Channel", "GetChannelMetadata"))
	{return;}
	
	FString JsonResponse = GetChannelMetadata_pn(Channel, Include);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetChannelMetadataResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetChannelMetadataResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::GetChannelMetadata_DATA_priv(FString Channel, FOnGetChannelMetadataResponseNative OnGetChannelMetadataResponse, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(Channel, "Channel", "GetChannelMetadata"))
	{return;}

	FString JsonResponse = GetChannelMetadata_pn(Channel, Include);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetChannelMetadataResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		FPubnubChannelData ChannelData;
		UPubnubJsonUtilities::GetChannelMetadataJsonToData(JsonResponse, Result, ChannelData);
								
		//Broadcast bound delegate with parsed response
		OnGetChannelMetadataResponse.ExecuteIfBound(Result, ChannelData);
	});
}

void UPubnubSubsystem::RemoveChannelMetadata_priv(FString Channel, FOnRemoveChannelMetadataResponseNative OnRemoveChannelMetadataResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Channel, "Channel", "RemoveChannelMetadata"))
	{return;}

	pubnub_remove_channelmetadata(ctx_pub, TCHAR_TO_ANSI(*Channel));

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnRemoveChannelMetadataResponse, JsonResponse]()
	{
		//Broadcast bound delegate with parsed response
		OnRemoveChannelMetadataResponse.ExecuteIfBound(UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(JsonResponse));
	});
}

FString UPubnubSubsystem::GetMemberships_pn(FString User, FString Include, int Limit,
	FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	pubnub_membership_opts PubnubOptions = pubnub_membership_opts();
	auto CharConverterUuid = StringCast<ANSICHAR>(*User);
	PubnubOptions.uuid = User.IsEmpty() ? NULL : CharConverterUuid.Get();
	auto CharConverterInclude = StringCast<ANSICHAR>(*Include);
	PubnubOptions.include = Include.IsEmpty() ? NULL : CharConverterInclude.Get();
	auto CharConverterFilter = StringCast<ANSICHAR>(*Filter);
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  CharConverterFilter.Get();
	auto CharConverterSort = StringCast<ANSICHAR>(*Sort);
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  CharConverterSort.Get();
	auto CharConverterPageNext = StringCast<ANSICHAR>(*PageNext);
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  CharConverterPageNext.Get();
	auto CharConverterPagePrev = StringCast<ANSICHAR>(*PagePrev);
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  CharConverterPagePrev.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;
	
	pubnub_get_memberships_ex(ctx_pub, PubnubOptions);

	FString Response = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(Response.IsEmpty())
	{
		Response = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	return Response;
}

void UPubnubSubsystem::GetMemberships_JSON_priv(FString User, FOnPubnubResponse OnGetMembershipResponse, FString Include, int Limit,
	FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "GetMemberships"))
	{return;}

	FString JsonResponse = GetMemberships_pn(User, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetMembershipResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetMembershipResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::GetMemberships_DATA_priv(FString User, FOnGetMembershipsResponseNative OnGetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "GetMemberships"))
	{return;}
	
	FString JsonResponse = GetMemberships_pn(User, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetMembershipResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		TArray<FPubnubMembershipData> MembershipsData;
		FString PageNext;
		FString PagePrev;
		UPubnubJsonUtilities::GetMembershipsJsonToData(JsonResponse, Result, MembershipsData, PageNext, PagePrev);

		//Broadcast bound delegate with parsed response
		OnGetMembershipResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});
}

void UPubnubSubsystem::SetMemberships_priv(FString User, FString SetObj, FOnSetMembershipsResponseNative OnSetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "SetMemberships") || CheckIsFieldEmpty(SetObj, "SetObj", "SetMemberships"))
	{return;}

	if(!UPubnubJsonUtilities::IsCorrectJsonString(SetObj, false))
	{
		PubnubError("Can't Set Memberships, SetObj has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}

	pubnub_membership_opts PubnubOptions = pubnub_membership_opts();
	auto CharConverterUuid = StringCast<ANSICHAR>(*User);
	PubnubOptions.uuid = User.IsEmpty() ? NULL : CharConverterUuid.Get();
	auto CharConverterInclude = StringCast<ANSICHAR>(*Include);
	PubnubOptions.include = Include.IsEmpty() ? NULL : CharConverterInclude.Get();
	auto CharConverterFilter = StringCast<ANSICHAR>(*Filter);
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  CharConverterFilter.Get();
	auto CharConverterSort = StringCast<ANSICHAR>(*Sort);
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  CharConverterSort.Get();
	auto CharConverterPageNext = StringCast<ANSICHAR>(*PageNext);
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  CharConverterPageNext.Get();
	auto CharConverterPagePrev = StringCast<ANSICHAR>(*PagePrev);
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  CharConverterPagePrev.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;

	pubnub_set_memberships_ex(ctx_pub, TCHAR_TO_ANSI(*SetObj), PubnubOptions);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnSetMembershipResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		TArray<FPubnubMembershipData> MembershipsData;
		FString PageNext;
		FString PagePrev;
		UPubnubJsonUtilities::GetMembershipsJsonToData(JsonResponse, Result, MembershipsData, PageNext, PagePrev);

		//Broadcast bound delegate with parsed response
		OnSetMembershipResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});
}

void UPubnubSubsystem::RemoveMemberships_priv(FString User, FString RemoveObj, FOnRemoveMembershipsResponseNative OnRemoveMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "RemoveMemberships") || CheckIsFieldEmpty(RemoveObj, "RemoveObj", "RemoveMemberships"))
	{return;}

	if(!UPubnubJsonUtilities::IsCorrectJsonString(RemoveObj, false))
	{
		PubnubError("Can't Remove Memberships, RemoveObj has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}

	pubnub_membership_opts PubnubOptions = pubnub_membership_opts();
	auto CharConverterUuid = StringCast<ANSICHAR>(*User);
	PubnubOptions.uuid = User.IsEmpty() ? NULL : CharConverterUuid.Get();
	auto CharConverterInclude = StringCast<ANSICHAR>(*Include);
	PubnubOptions.include = Include.IsEmpty() ? NULL : CharConverterInclude.Get();
	auto CharConverterFilter = StringCast<ANSICHAR>(*Filter);
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  CharConverterFilter.Get();
	auto CharConverterSort = StringCast<ANSICHAR>(*Sort);
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  CharConverterSort.Get();
	auto CharConverterPageNext = StringCast<ANSICHAR>(*PageNext);
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  CharConverterPageNext.Get();
	auto CharConverterPagePrev = StringCast<ANSICHAR>(*PagePrev);
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  CharConverterPagePrev.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;

	pubnub_remove_memberships_ex(ctx_pub, TCHAR_TO_ANSI(*RemoveObj), PubnubOptions);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnRemoveMembershipResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		TArray<FPubnubMembershipData> MembershipsData;
		FString PageNext;
		FString PagePrev;
		UPubnubJsonUtilities::GetMembershipsJsonToData(JsonResponse, Result, MembershipsData, PageNext, PagePrev);

		//Broadcast bound delegate with parsed response
		OnRemoveMembershipResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});
	
}

FString UPubnubSubsystem::GetChannelMembers_pn(FString Channel, FString Include, int Limit,
	FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	pubnub_members_opts PubnubOptions = pubnub_members_opts();
	auto CharConverterInclude = StringCast<ANSICHAR>(*Include);
	PubnubOptions.include = Include.IsEmpty() ? NULL : CharConverterInclude.Get();
	auto CharConverterFilter = StringCast<ANSICHAR>(*Filter);
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  CharConverterFilter.Get();
	auto CharConverterSort = StringCast<ANSICHAR>(*Sort);
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  CharConverterSort.Get();
	auto CharConverterPageNext = StringCast<ANSICHAR>(*PageNext);
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  CharConverterPageNext.Get();
	auto CharConverterPagePrev = StringCast<ANSICHAR>(*PagePrev);
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  CharConverterPagePrev.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;
	
	pubnub_get_members_ex(ctx_pub,TCHAR_TO_ANSI(*Channel), PubnubOptions);

	FString Response = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(Response.IsEmpty())
	{
		Response = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	return Response;
}

void UPubnubSubsystem::GetChannelMembers_JSON_priv(FString Channel, FOnPubnubResponse OnGetMembersResponse, FString Include, int Limit,
	FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Channel, "User", "GetChannelMembers"))
	{return;}

	FString JsonResponse = GetChannelMembers_pn(Channel, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetMembersResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetMembersResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::GetChannelMembers_DATA_priv(FString Channel, FOnGetChannelMembersResponseNative OnGetMembersResponse, FString Include, int Limit,
	FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Channel, "User", "GetChannelMembers"))
	{return;}
	
	FString JsonResponse = GetChannelMembers_pn(Channel, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetMembersResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		TArray<FPubnubChannelMemberData> MembersData;
		FString PageNext;
		FString PagePrev;
		UPubnubJsonUtilities::GetChannelMembersJsonToData(JsonResponse, Result, MembersData, PageNext, PagePrev);

		//Broadcast bound delegate with parsed response
		OnGetMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
	});
}


void UPubnubSubsystem::AddChannelMembers_priv(FString Channel, FString AddObj, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Channel, "Channel", "AddChannelMembers") || CheckIsFieldEmpty(AddObj, "AddObj", "AddChannelMembers"))
	{return;}

	if(!UPubnubJsonUtilities::IsCorrectJsonString(AddObj, false))
	{
		PubnubError("Can't Add Channel Members, AddObj has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}

	pubnub_add_members(ctx_pub, TCHAR_TO_ANSI(*Channel), TCHAR_TO_ANSI(*Include), TCHAR_TO_ANSI(*AddObj));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Add Channel Members.");
	}
}

void UPubnubSubsystem::SetChannelMembers_priv(FString Channel, FString SetObj, FOnSetChannelMembersResponseNative OnSetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Channel, "Channel", "SetChannelMembers") || CheckIsFieldEmpty(SetObj, "SetObj", "SetChannelMembers"))
	{return;}

	if(!UPubnubJsonUtilities::IsCorrectJsonString(SetObj, false))
	{
		PubnubError("Can't Set Channel Members, SetObj has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}

	pubnub_members_opts PubnubOptions = pubnub_members_opts();
	auto CharConverterInclude = StringCast<ANSICHAR>(*Include);
	PubnubOptions.include = Include.IsEmpty() ? NULL : CharConverterInclude.Get();
	auto CharConverterFilter = StringCast<ANSICHAR>(*Filter);
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  CharConverterFilter.Get();
	auto CharConverterSort = StringCast<ANSICHAR>(*Sort);
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  CharConverterSort.Get();
	auto CharConverterPageNext = StringCast<ANSICHAR>(*PageNext);
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  CharConverterPageNext.Get();
	auto CharConverterPagePrev = StringCast<ANSICHAR>(*PagePrev);
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  CharConverterPagePrev.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;

	pubnub_set_members_ex(ctx_pub, TCHAR_TO_ANSI(*Channel),TCHAR_TO_ANSI(*SetObj), PubnubOptions);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnSetMembersResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		TArray<FPubnubChannelMemberData> MembersData;
		FString PageNext;
		FString PagePrev;
		UPubnubJsonUtilities::GetChannelMembersJsonToData(JsonResponse, Result, MembersData, PageNext, PagePrev);

		//Broadcast bound delegate with parsed response
		OnSetMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
	});
}

void UPubnubSubsystem::RemoveChannelMembers_priv(FString Channel, FString RemoveObj, FOnRemoveChannelMembersResponseNative OnRemoveMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Channel, "Channel", "RemoveChannelMembers") || CheckIsFieldEmpty(RemoveObj, "RemoveObj", "RemoveChannelMembers"))
	{return;}

	if(!UPubnubJsonUtilities::IsCorrectJsonString(RemoveObj, false))
	{
		PubnubError("Can't Remove Channel Members, RemoveObj has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}

	pubnub_members_opts PubnubOptions = pubnub_members_opts();
	auto CharConverterInclude = StringCast<ANSICHAR>(*Include);
	PubnubOptions.include = Include.IsEmpty() ? NULL : CharConverterInclude.Get();
	auto CharConverterFilter = StringCast<ANSICHAR>(*Filter);
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  CharConverterFilter.Get();
	auto CharConverterSort = StringCast<ANSICHAR>(*Sort);
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  CharConverterSort.Get();
	auto CharConverterPageNext = StringCast<ANSICHAR>(*PageNext);
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  CharConverterPageNext.Get();
	auto CharConverterPagePrev = StringCast<ANSICHAR>(*PagePrev);
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  CharConverterPagePrev.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;

	pubnub_remove_members_ex(ctx_pub, TCHAR_TO_ANSI(*Channel), TCHAR_TO_ANSI(*RemoveObj), PubnubOptions);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnRemoveMembersResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		TArray<FPubnubChannelMemberData> MembersData;
		FString PageNext;
		FString PagePrev;
		UPubnubJsonUtilities::GetChannelMembersJsonToData(JsonResponse, Result, MembersData, PageNext, PagePrev);

		//Broadcast bound delegate with parsed response
		OnRemoveMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
	});
}

void UPubnubSubsystem::AddMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnAddMessageActionResponseNative AddMessageActionResponse)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(Channel, "Channel", "AddMessageAction") || CheckIsFieldEmpty(MessageTimetoken, "MessageTimetoken", "AddMessageAction"))
	{return;}

	//Add quotes to these fields as they are required by C-Core
	FString FinalActionType = UPubnubUtilities::AddQuotesToString(ActionType);
	FString FinalValue = UPubnubUtilities::AddQuotesToString(Value);
	
	pubnub_add_message_action_str(ctx_pub, TCHAR_TO_ANSI(*Channel), TCHAR_TO_ANSI(*MessageTimetoken), TCHAR_TO_ANSI(*FinalActionType),  TCHAR_TO_ANSI(*FinalValue));
	
	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, AddMessageActionResponse, JsonResponse]()
	{
		FPubnubOperationResult Result;
		FPubnubMessageActionData MessageActionData;
		
		//Broadcast bound delegate with JsonResponse
		UPubnubJsonUtilities::AddMessageActionJsonToData(JsonResponse, Result, MessageActionData);
		AddMessageActionResponse.ExecuteIfBound(Result, MessageActionData);
	});
}

void UPubnubSubsystem::RemoveMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnRemoveMessageActionResponseNative OnRemoveMessageActionResponse)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(Channel, "Channel", "RemoveMessageAction") || CheckIsFieldEmpty(MessageTimetoken, "MessageTimetoken", "RemoveMessageAction")
		|| CheckIsFieldEmpty(ActionTimetoken, "ActionTimetoken", "RemoveMessageAction"))
	{return;}
	
	//Add quotes to these fields as they are required by C-Core
	FString FinalMessageTimetoken = UPubnubUtilities::AddQuotesToString(MessageTimetoken);
	FString FinalActionTimetoken = UPubnubUtilities::AddQuotesToString(ActionTimetoken);

	FTCHARToUTF8 MessageConverter(*FinalMessageTimetoken);
	FTCHARToUTF8 ActionConverter(*FinalActionTimetoken);

	TArray<ANSICHAR> MessageTimetokenArray;
	MessageTimetokenArray.Append(MessageConverter.Get(), MessageConverter.Length() + 1);

	TArray<ANSICHAR> ActionTimetokenArray;
	ActionTimetokenArray.Append(ActionConverter.Get(), ActionConverter.Length() + 1);
	
	pubnub_char_mem_block message_timetoken_chamebl;
	message_timetoken_chamebl.ptr = MessageTimetokenArray.GetData();
	message_timetoken_chamebl.size = FinalMessageTimetoken.Len();
	
	pubnub_char_mem_block action_timetoken_chamebl;
	action_timetoken_chamebl.ptr = ActionTimetokenArray.GetData();
	action_timetoken_chamebl.size = FinalActionTimetoken.Len();
	
	pubnub_remove_message_action(ctx_pub, TCHAR_TO_ANSI(*Channel), message_timetoken_chamebl, action_timetoken_chamebl);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnRemoveMessageActionResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnRemoveMessageActionResponse.ExecuteIfBound(UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(JsonResponse));
	});
}

FString UPubnubSubsystem::GetMessageActions_pn(FString Channel, FString Start, FString End, int Limit)
{
	pubnub_get_message_actions(ctx_pub, TCHAR_TO_ANSI(*Channel), TCHAR_TO_ANSI(*Start), TCHAR_TO_ANSI(*End), Limit);
	
	FString Response = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(Response.IsEmpty())
	{
		Response = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	return Response;
}

void UPubnubSubsystem::GetMessageActions_JSON_priv(FString Channel, FString Start, FString End, int SizeLimit, FOnPubnubResponse OnGetMessageActionsResponse)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(Channel, "Channel", "HistoryWithMessageActions"))
	{return;}

	FString JsonResponse = GetMessageActions_pn(Channel, Start, End, SizeLimit);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetMessageActionsResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetMessageActionsResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::GetMessageActions_DATA_priv(FString Channel, FString Start, FString End, int SizeLimit, FOnGetMessageActionsResponseNative OnGetMessageActionsResponse)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(Channel, "Channel", "HistoryWithMessageActions"))
	{return;}

	FString JsonResponse = GetMessageActions_pn(Channel, Start, End, SizeLimit);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetMessageActionsResponse, JsonResponse]()
	{
		//Parse Json response into data
		FPubnubOperationResult Result;
		TArray<FPubnubMessageActionData> MessageActions;
		UPubnubJsonUtilities::GetMessageActionsJsonToData(JsonResponse, Result, MessageActions);
										
		//Broadcast bound delegate with parsed response
		OnGetMessageActionsResponse.ExecuteIfBound(Result, MessageActions);
	});
}

void UPubnubSubsystem::GetMessageActionsContinue_priv(FOnPubnubResponse OnGetMessageActionsContinueResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	pubnub_get_message_actions_more(ctx_pub);

	FString JsonResponse = GetLastResponse(ctx_pub);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetMessageActionsContinueResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetMessageActionsContinueResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::PublishUESettingsToPubnubPublishOptions(FPubnubPublishSettings &PublishSettings, pubnub_publish_options& PubnubPublishOptions)
{
	PubnubPublishOptions.store = PublishSettings.StoreInHistory;
	PubnubPublishOptions.replicate = PublishSettings.Replicate;
	PubnubPublishOptions.cipher_key = NULL;
	PubnubPublishOptions.ttl = PublishSettings.Ttl;
	PublishSettings.MetaData.IsEmpty() ? PubnubPublishOptions.meta = NULL : nullptr;
	PublishSettings.CustomMessageType.IsEmpty() ? PubnubPublishOptions.custom_message_type = NULL : nullptr;
	PubnubPublishOptions.method = (pubnub_method)(uint8)PublishSettings.PublishMethod;
}

void UPubnubSubsystem::HereNowUESettingsToPubnubHereNowOptions(FPubnubListUsersFromChannelSettings& HereNowSettings, pubnub_here_now_options& PubnubHereNowOptions)
{
	PubnubHereNowOptions.disable_uuids = HereNowSettings.DisableUserID;
	PubnubHereNowOptions.state = HereNowSettings.State;
	HereNowSettings.ChannelGroup.IsEmpty() ? PubnubHereNowOptions.channel_group = NULL : nullptr;
}

void UPubnubSubsystem::SetStateUESettingsToPubnubSetStateOptions(FPubnubSetStateSettings& SetStateSettings, pubnub_set_state_options& PubnubSetStateOptions)
{
	SetStateSettings.ChannelGroup.IsEmpty() ? PubnubSetStateOptions.channel_group = NULL : nullptr;
	SetStateSettings.UserID.IsEmpty() ? PubnubSetStateOptions.user_id = NULL : nullptr;
	PubnubSetStateOptions.heartbeat = SetStateSettings.HeartBeat;
}

void UPubnubSubsystem::FetchHistoryUESettingsToPbFetchHistoryOptions(FPubnubFetchHistorySettings& FetchHistorySettings, pubnub_fetch_history_options& PubnubFetchHistoryOptions)
{
	PubnubFetchHistoryOptions.max_per_channel = FetchHistorySettings.MaxPerChannel;
	PubnubFetchHistoryOptions.reverse = FetchHistorySettings.Reverse;
	PubnubFetchHistoryOptions.include_meta = FetchHistorySettings.IncludeMeta;
	PubnubFetchHistoryOptions.include_message_type = FetchHistorySettings.IncludeMessageType;
	PubnubFetchHistoryOptions.include_user_id = FetchHistorySettings.IncludeUserID;
	PubnubFetchHistoryOptions.include_message_actions = FetchHistorySettings.IncludeMessageActions;
	PubnubFetchHistoryOptions.include_custom_message_type = FetchHistorySettings.IncludeCustomMessageType;
	FetchHistorySettings.Start.IsEmpty() ? PubnubFetchHistoryOptions.start = NULL : nullptr;
	FetchHistorySettings.End.IsEmpty() ? PubnubFetchHistoryOptions.end = NULL : nullptr;
}

FPubnubMessageData UPubnubSubsystem::UEMessageFromPubnub(pubnub_v2_message PubnubMessage)
{
	FPubnubMessageData MessageData;
	MessageData.Message = UPubnubUtilities::PubnubCharMemBlockToString(PubnubMessage.payload);

	//If message was just a string, we need to deserialize it
	if(!UPubnubJsonUtilities::IsCorrectJsonString(MessageData.Message, false))
	{
		MessageData.Message = UPubnubJsonUtilities::DeserializeString(MessageData.Message);
	}
	
	MessageData.Channel = UPubnubUtilities::PubnubCharMemBlockToString(PubnubMessage.channel);
	MessageData.UserID = UPubnubUtilities::PubnubCharMemBlockToString(PubnubMessage.publisher);
	MessageData.Timetoken = UPubnubUtilities::PubnubCharMemBlockToString(PubnubMessage.tt);
	MessageData.Metadata = UPubnubUtilities::PubnubCharMemBlockToString(PubnubMessage.metadata);
	MessageData.MessageType = (EPubnubMessageType)(PubnubMessage.message_type);
	MessageData.CustomMessageType = UPubnubUtilities::PubnubCharMemBlockToString(PubnubMessage.custom_message_type);
	MessageData.MatchOrGroup = UPubnubUtilities::PubnubCharMemBlockToString(PubnubMessage.match_or_group);
	return MessageData;
}

//This functions assumes that Channels and Permissions are already checked. It means that there is the same amount of permissions as channels or there is exactly one permission
TSharedPtr<FJsonObject> UPubnubSubsystem::AddChannelPermissionsToJson(TArray<FString> Channels, TArray<FPubnubChannelPermissions> ChannelPermissions)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	bool UseOnePermission = ChannelPermissions.Num() == 1;

	TArray<TSharedPtr<FJsonValue>> ObjectValues;
	
	for(int i = 0; i < Channels.Num(); i++)
	{
		if(Channels[i].IsEmpty())
		{
			continue;
		}
		
		//For permissions use the first index if this is the only valid index or corresponding channel index
		FPubnubChannelPermissions CurrentPermissions;
		UseOnePermission ? CurrentPermissions = ChannelPermissions[0] : CurrentPermissions = ChannelPermissions[i];

		//Create bit mask value from all permissions
		struct pam_permission ChPerm;
		ChPerm.read = CurrentPermissions.Read;
		ChPerm.write = CurrentPermissions.Write;
		ChPerm.del = CurrentPermissions.Delete;
		ChPerm.get = CurrentPermissions.Get;
		ChPerm.update = CurrentPermissions.Update;
		ChPerm.manage = CurrentPermissions.Manage;
		ChPerm.join = CurrentPermissions.Join;
		ChPerm.create = false;
		int PermBitMask = pubnub_get_grant_bit_mask_value(ChPerm);

		JsonObject->SetNumberField(Channels[i], PermBitMask);
	}
	
	return JsonObject;
}

TSharedPtr<FJsonObject> UPubnubSubsystem::AddChannelGroupPermissionsToJson(TArray<FString> ChannelGroups, TArray<FPubnubChannelGroupPermissions> ChannelGroupPermissions)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	bool UseOnePermission = ChannelGroupPermissions.Num() == 1;

	TArray<TSharedPtr<FJsonValue>> ObjectValues;
	
	for(int i = 0; i < ChannelGroups.Num(); i++)
	{
		if(ChannelGroups[i].IsEmpty())
		{
			continue;
		}
		
		//For permissions use the first index if this is the only valid index or corresponding channel index
		FPubnubChannelGroupPermissions CurrentPermissions;
		UseOnePermission ? CurrentPermissions = ChannelGroupPermissions[0] : CurrentPermissions = ChannelGroupPermissions[i];

		//Create bit mask value from all permissions
		struct pam_permission ChPerm;
		ChPerm.read = CurrentPermissions.Read;
		ChPerm.manage = CurrentPermissions.Manage;
		ChPerm.write = false;
		ChPerm.del = false;
		ChPerm.get = false;
		ChPerm.update = false;
		ChPerm.join = false;
		ChPerm.create = false;
		int PermBitMask = pubnub_get_grant_bit_mask_value(ChPerm);

		JsonObject->SetNumberField(ChannelGroups[i], PermBitMask);
	}
	
	return JsonObject;
}

TSharedPtr<FJsonObject> UPubnubSubsystem::AddUserPermissionsToJson(TArray<FString> Users, TArray<FPubnubUserPermissions> UserPermissions)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	bool UseOnePermission = UserPermissions.Num() == 1;

	TArray<TSharedPtr<FJsonValue>> ObjectValues;
	
	for(int i = 0; i < Users.Num(); i++)
	{
		if(Users[i].IsEmpty())
		{
			continue;
		}
		
		//For permissions use the first index if this is the only valid index or corresponding channel index
		FPubnubUserPermissions CurrentPermissions;
		UseOnePermission ? CurrentPermissions = UserPermissions[0] : CurrentPermissions = UserPermissions[i];

		//Create bit mask value from all permissions
		struct pam_permission ChPerm;
		ChPerm.del = CurrentPermissions.Delete;
		ChPerm.get = CurrentPermissions.Get;
		ChPerm.update = CurrentPermissions.Update;
		ChPerm.read = false;
		ChPerm.write = false;
		ChPerm.manage = false;
		ChPerm.join = false;
		ChPerm.create = false;
		int PermBitMask = pubnub_get_grant_bit_mask_value(ChPerm);

		JsonObject->SetNumberField(Users[i], PermBitMask);
	}

	return JsonObject;
}

void UPubnubSubsystem::OnCCoreSubscriptionStatusReceived(const pubnub_subscription_status status, const pubnub_subscription_status_data_t status_data)
{
	//Don't waste resources to translate data if there is no delegate bound to it
	if(!OnSubscriptionStatusChanged.IsBound() && !OnSubscriptionStatusChangedNative.IsBound())
	{return;}

	FPubnubSubscriptionStatusData SubscriptionStatusData;
	SubscriptionStatusData.Reason = pubnub_res_2_string(status_data.reason);
	if (NULL != status_data.channels)
	{
		FString Channels(status_data.channels);
		Channels.ParseIntoArray(SubscriptionStatusData.Channels, TEXT(","));
	}
	if (NULL != status_data.channel_groups)
	{
		FString ChannelGroups(status_data.channel_groups);
		ChannelGroups.ParseIntoArray(SubscriptionStatusData.ChannelGroups, TEXT(","));
	}
	
	OnSubscriptionStatusChanged.Broadcast((EPubnubSubscriptionStatus)status, SubscriptionStatusData);
	OnSubscriptionStatusChangedNative.Broadcast((EPubnubSubscriptionStatus)status, SubscriptionStatusData);
}
