// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"

#include "Async/Async.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Config/PubnubSettings.h"
#include "PubnubInternalMacros.h"
#include "Crypto/PubnubAesCryptor.h"
#include "FunctionLibraries/PubnubTokenUtilities.h"
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

	if(ctx_pub && ctx_ee)
	{
		//We set this to prevent crash from C-Core when it's trying to clean up provider made in UE
		pubnub_set_crypto_module(ctx_pub, nullptr);
		pubnub_set_crypto_module(ctx_ee, nullptr);

		//Clean up Crypto bridge if it was created
		if(CryptoBridge)
		{
			CryptoBridge->CleanUpCryptoBridge();
		}

		pubnub_free(ctx_pub);
		pubnub_free(ctx_ee);
		ctx_pub = nullptr;
		ctx_ee = nullptr;
	}
	
	ChannelSubscriptions.Empty();
	ChannelGroupSubscriptions.Empty();
	IsUserIDSet = false;
	delete[] AuthTokenBuffer;
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubMessageData());
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, Message, NativeCallback, PublishSettings]
	{
		PublishMessage_priv(Channel, Message, NativeCallback, PublishSettings);
	});
}

void UPubnubSubsystem::PublishMessage(FString Channel, FString Message, FPubnubPublishSettings PublishSettings)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubMessageData());
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, Message, NativeCallback, SignalSettings]
	{
		Signal_priv(Channel, Message, NativeCallback, SignalSettings);
	});
}

void UPubnubSubsystem::Signal(FString Channel, FString Message, FPubnubSignalSettings SignalSettings)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, Message, SignalSettings]
	{
		Signal_priv(Channel, Message, nullptr, SignalSettings);
	});
}

void UPubnubSubsystem::SubscribeToChannel(FString Channel, FOnSubscribeOperationResponse OnSubscribeToChannelResponse, FPubnubSubscribeSettings SubscribeSettings)
{
	FOnSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSubscribeToChannelResponse](FPubnubOperationResult Result)
	{
		OnSubscribeToChannelResponse.ExecuteIfBound(Result);
	});

	SubscribeToChannel(Channel, NativeCallback, SubscribeSettings);
}

void UPubnubSubsystem::SubscribeToChannel(FString Channel, FOnSubscribeOperationResponseNative NativeCallback, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(NativeCallback);
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, NativeCallback, SubscribeSettings]
	{
		SubscribeToChannel_priv(Channel, NativeCallback, SubscribeSettings);
	});
}

void UPubnubSubsystem::SubscribeToChannel(FString Channel, FPubnubSubscribeSettings SubscribeSettings)
{
	SubscribeToChannel(Channel, nullptr, SubscribeSettings);
}

void UPubnubSubsystem::SubscribeToGroup(FString ChannelGroup, FOnSubscribeOperationResponse OnSubscribeToGroupResponse, FPubnubSubscribeSettings SubscribeSettings)
{
	FOnSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSubscribeToGroupResponse](FPubnubOperationResult Result)
	{
		OnSubscribeToGroupResponse.ExecuteIfBound(Result);
	});

	SubscribeToGroup(ChannelGroup, NativeCallback, SubscribeSettings);
}

void UPubnubSubsystem::SubscribeToGroup(FString ChannelGroup, FOnSubscribeOperationResponseNative NativeCallback, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(NativeCallback);
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelGroup, NativeCallback, SubscribeSettings]
	{
		SubscribeToGroup_priv(ChannelGroup, NativeCallback, SubscribeSettings);
	});
}

void UPubnubSubsystem::SubscribeToGroup(FString ChannelGroup, FPubnubSubscribeSettings SubscribeSettings)
{
	SubscribeToGroup(ChannelGroup, nullptr, SubscribeSettings);
}

void UPubnubSubsystem::UnsubscribeFromChannel(FString Channel, FOnSubscribeOperationResponse OnUnsubscribeFromChannelResponse)
{
	FOnSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeFromChannelResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromChannelResponse.ExecuteIfBound(Result);
	});

	UnsubscribeFromChannel(Channel, NativeCallback);
}

void UPubnubSubsystem::UnsubscribeFromChannel(FString Channel, FOnSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(NativeCallback);
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, NativeCallback]
	{
		UnsubscribeFromChannel_priv(Channel, NativeCallback);
	});
}

void UPubnubSubsystem::UnsubscribeFromGroup(FString ChannelGroup, FOnSubscribeOperationResponse OnUnsubscribeFromGroupResponse)
{
	FOnSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeFromGroupResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromGroupResponse.ExecuteIfBound(Result);
	});

	UnsubscribeFromGroup(ChannelGroup, NativeCallback);
}

void UPubnubSubsystem::UnsubscribeFromGroup(FString ChannelGroup, FOnSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(NativeCallback);
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelGroup, NativeCallback]
	{
		UnsubscribeFromGroup_priv(ChannelGroup, NativeCallback);
	});
}

void UPubnubSubsystem::UnsubscribeFromAll(FOnSubscribeOperationResponse OnUnsubscribeFromAllResponse)
{
	FOnSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeFromAllResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromAllResponse.ExecuteIfBound(Result);
	});

	UnsubscribeFromAll(NativeCallback);
}

void UPubnubSubsystem::UnsubscribeFromAll(FOnSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(NativeCallback);
	
	QuickActionThread->AddFunctionToQueue( [this, NativeCallback]
	{
		UnsubscribeFromAll_priv(NativeCallback);
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FString>{});
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelGroup, NativeCallback]
	{
		ListChannelsFromGroup_priv(ChannelGroup, NativeCallback);
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubListUsersFromChannelWrapper());
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, NativeCallback, ListUsersFromChannelSettings]
	{
		ListUsersFromChannel_priv(Channel, NativeCallback, ListUsersFromChannelSettings);
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FString>{});
	
	QuickActionThread->AddFunctionToQueue( [this, UserID, NativeCallback]
	{
		ListUserSubscribedChannels_priv(UserID, NativeCallback);
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, StateJson, NativeCallback, SetStateSettings]
	{
		SetState_priv(Channel, StateJson, NativeCallback, SetStateSettings);
	});
}

void UPubnubSubsystem::SetState(FString Channel, FString StateJson, FPubnubSetStateSettings SetStateSettings)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FString());
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, ChannelGroup, UserID, NativeCallback]
	{
		GetState_priv(Channel, ChannelGroup, UserID, NativeCallback);
	});
}

void UPubnubSubsystem::Heartbeat(FString Channel, FString ChannelGroup)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, ChannelGroup]
	{
		Heartbeat_priv(Channel, ChannelGroup);
	});
}

void UPubnubSubsystem::GrantToken(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FOnGrantTokenResponse OnGrantTokenResponse, FString Meta)
{

	FOnGrantTokenResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGrantTokenResponse](const FPubnubOperationResult& Result, FString Token)
	{
		OnGrantTokenResponse.ExecuteIfBound(Result, Token);
	});

	GrantToken(Ttl, AuthorizedUser, Permissions, NativeCallback, Meta);
}

void UPubnubSubsystem::GrantToken(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FOnGrantTokenResponseNative NativeCallback, FString Meta)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FString());
	
	QuickActionThread->AddFunctionToQueue( [this, Ttl, AuthorizedUser, Permissions, NativeCallback, Meta]
	{
		GrantToken_priv(UPubnubTokenUtilities::CreateGrantTokenPermissionObjectString(Ttl, AuthorizedUser, Permissions, Meta), NativeCallback);
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
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
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	PUBNUB_RETURN_IF_USER_ID_NOT_SET();

	//Auth token has to be kept alive for the lifetime of the sdk, so we copy it into AuthTokenBuffer
	FTCHARToUTF8 Converter(*Token);
	AuthTokenLength = Converter.Length();
	delete[] AuthTokenBuffer;
	AuthTokenBuffer = new char[AuthTokenLength + 1];
	FMemory::Memcpy(AuthTokenBuffer, Converter.Get(), AuthTokenLength);
	AuthTokenBuffer[AuthTokenLength] = '\0';
	
	//This is just a setter, so no need to call it on a separate thread
	pubnub_set_auth_token(ctx_pub, AuthTokenBuffer);
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubHistoryMessageData>());
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, NativeCallback, FetchHistorySettings]
	{
		FetchHistory_priv(Channel, NativeCallback, FetchHistorySettings);
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, NativeCallback, DeleteMessagesSettings]
	{
		DeleteMessages_priv(Channel, NativeCallback, DeleteMessagesSettings);
	});
}

void UPubnubSubsystem::DeleteMessages(FString Channel, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, 0);
	
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
	GetAllUserMetadataRaw(NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::GetAllUserMetadataRaw(FOnGetAllUserMetadataResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubUserData>(), FString(), FString());
	
	QuickActionThread->AddFunctionToQueue( [this, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetAllUserMetadata_priv(NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::GetAllUserMetadata(FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FString PageNext, FString PagePrev)
{
	GetAllUserMetadataRaw(OnGetAllUserMetadataResponse, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::GetAllUserMetadata(FOnGetAllUserMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FString PageNext, FString PagePrev)
{
	GetAllUserMetadataRaw(NativeCallback, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubUserData());
	
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubUserData());
	
	QuickActionThread->AddFunctionToQueue( [this, User, NativeCallback, Include]
	{
		GetUserMetadata_priv(User, NativeCallback, Include);
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
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
	GetAllChannelMetadataRaw(NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::GetAllChannelMetadataRaw(FOnGetAllChannelMetadataResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubChannelData>(), FString(), FString());
	
	QuickActionThread->AddFunctionToQueue( [this, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetAllChannelMetadata_priv(NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::GetAllChannelMetadata(FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FString PageNext, FString PagePrev)
{
	GetAllChannelMetadataRaw(OnGetAllChannelMetadataResponse, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::GetAllChannelMetadata(FOnGetAllChannelMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FString PageNext, FString PagePrev)
{
	GetAllChannelMetadataRaw(NativeCallback, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubChannelData());
	
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
    PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubChannelData());

    QuickActionThread->AddFunctionToQueue([this, Channel, NativeCallback, Include]
    {
        GetChannelMetadata_priv(Channel, NativeCallback, Include);
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, NativeCallback]
	{
		RemoveChannelMetadata_priv(Channel, NativeCallback);
	});
}

void UPubnubSubsystem::GetMembershipsRaw(FString User, FOnGetMembershipsResponse OnGetMembershipsResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	FOnGetMembershipsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetMembershipsResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		OnGetMembershipsResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});

	GetMembershipsRaw(User, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
}

void UPubnubSubsystem::GetMembershipsRaw(FString User, FOnGetMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FString(), FString());
	
	QuickActionThread->AddFunctionToQueue( [this, User, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetMemberships_priv(User, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::GetMemberships(FString User, FOnGetMembershipsResponse OnGetMembershipsResponse, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	GetMembershipsRaw(User, OnGetMembershipsResponse, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::GetMemberships(FString User, FOnGetMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	GetMembershipsRaw(User, NativeCallback, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::SetMembershipsRaw(FString User, FString SetObj, FOnSetMembershipsResponse OnSetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	FOnGetMembershipsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSetMembershipResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		OnSetMembershipResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});

	SetMembershipsRaw(User, SetObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
}

void UPubnubSubsystem::SetMembershipsRaw(FString User, FString SetObj, FOnSetMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FString(), FString());
	
	QuickActionThread->AddFunctionToQueue( [this, User, SetObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		SetMemberships_priv(User, SetObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::SetMemberships(FString User, TArray<FPubnubMembershipInputData> Channels, FOnSetMembershipsResponse OnSetMembershipResponse, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	SetMembershipsRaw(User, UPubnubJsonUtilities::GetJsonFromMembershipsDataArray(Channels), OnSetMembershipResponse, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::SetMemberships(FString User, TArray<FPubnubMembershipInputData> Channels, FOnSetMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	SetMembershipsRaw(User, UPubnubJsonUtilities::GetJsonFromMembershipsDataArray(Channels), NativeCallback, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::RemoveMembershipsRaw(FString User, FString RemoveObj, FOnRemoveMembershipsResponse OnRemoveMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	FOnGetMembershipsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveMembershipResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		OnRemoveMembershipResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});

	RemoveMembershipsRaw(User, RemoveObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
}

void UPubnubSubsystem::RemoveMembershipsRaw(FString User, FString RemoveObj, FOnRemoveMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FString(), FString());
	
	QuickActionThread->AddFunctionToQueue( [this, User, RemoveObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		RemoveMemberships_priv(User, RemoveObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::RemoveMemberships(FString User, TArray<FString> Channels, FOnRemoveMembershipsResponse OnRemoveMembershipResponse, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	RemoveMembershipsRaw(User, UPubnubJsonUtilities::GetJsonFromMembershipsToRemove(Channels), OnRemoveMembershipResponse, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::RemoveMemberships(FString User, TArray<FString> Channels, FOnRemoveMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	RemoveMembershipsRaw(User, UPubnubJsonUtilities::GetJsonFromMembershipsToRemove(Channels), NativeCallback, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::GetChannelMembersRaw(FString Channel, FOnGetChannelMembersResponse OnGetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    FOnGetChannelMembersResponseNative NativeCallback;
    NativeCallback.BindLambda([OnGetMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
    {
        OnGetMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
    });
    GetChannelMembersRaw(Channel, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::GetChannelMembersRaw(FString Channel, FOnGetChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubChannelMemberData>(), FString(), FString());

    QuickActionThread->AddFunctionToQueue([this, Channel, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count]
    {
        GetChannelMembers_priv(Channel, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
    });
}

void UPubnubSubsystem::GetChannelMembers(FString Channel, FOnGetChannelMembersResponse OnGetMembersResponse, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    GetChannelMembersRaw(Channel, OnGetMembersResponse, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::GetChannelMembers(FString Channel, FOnGetChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    GetChannelMembersRaw(Channel, NativeCallback, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::SetChannelMembersRaw(FString Channel, FString SetObj, FOnSetChannelMembersResponse OnSetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    FOnSetChannelMembersResponseNative NativeCallback;
    NativeCallback.BindLambda([OnSetMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
    {
        OnSetMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
    });
    SetChannelMembersRaw(Channel, SetObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::SetChannelMembersRaw(FString Channel, FString SetObj, FOnSetChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubChannelMemberData>(), FString(), FString());

    QuickActionThread->AddFunctionToQueue([this, Channel, SetObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count]
    {
        SetChannelMembers_priv(Channel, SetObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
    });
}

void UPubnubSubsystem::SetChannelMembers(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FOnSetChannelMembersResponse OnSetMembersResponse, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    SetChannelMembersRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersDataArray(Users), OnSetMembersResponse, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::SetChannelMembers(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FOnSetChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    SetChannelMembersRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersDataArray(Users), NativeCallback, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
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
    PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubChannelMemberData>(), FString(), FString());

    QuickActionThread->AddFunctionToQueue([this, Channel, RemoveObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count]
    {
        RemoveChannelMembers_priv(Channel, RemoveObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
    });
}

void UPubnubSubsystem::RemoveChannelMembers(FString Channel, TArray<FString> Users, FOnRemoveChannelMembersResponse OnRemoveMembersResponse, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    RemoveChannelMembersRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersToRemove(Users), OnRemoveMembersResponse, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubSubsystem::RemoveChannelMembers(FString Channel, TArray<FString> Users, FOnRemoveChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    RemoveChannelMembersRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersToRemove(Users), NativeCallback, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubMessageActionData());
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, MessageTimetoken, ActionType, Value, NativeCallback]
	{
		AddMessageAction_priv(Channel, MessageTimetoken, ActionType, Value, NativeCallback);
	});
}

void UPubnubSubsystem::GetMessageActions(FString Channel, FOnGetMessageActionsResponse OnGetMessageActionsResponse, FString Start, FString End, int Limit)
{
	FOnGetMessageActionsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetMessageActionsResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMessageActionData>& MessageActions)
	{
		OnGetMessageActionsResponse.ExecuteIfBound(Result, MessageActions);
	});
	GetMessageActions(Channel, NativeCallback, Start, End, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit));
}

void UPubnubSubsystem::GetMessageActions(FString Channel, FOnGetMessageActionsResponseNative NativeCallback, FString Start, FString End, int Limit)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubMessageActionData>());
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, Start, End, Limit, NativeCallback]
	{
		GetMessageActions_priv(Channel, NativeCallback, Start, End, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit));
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
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	QuickActionThread->AddFunctionToQueue( [this, Channel, MessageTimetoken, ActionTimetoken, NativeCallback]
	{
		RemoveMessageAction_priv(Channel, MessageTimetoken, ActionTimetoken, NativeCallback);
	});
}

void UPubnubSubsystem::ReconnectSubscriptions()
{
	pubnub_reconnect(ctx_ee, nullptr);
}

void UPubnubSubsystem::DisconnectSubscriptions()
{
	pubnub_disconnect(ctx_ee);
}

void UPubnubSubsystem::SetCryptoModule(TScriptInterface<IPubnubCryptoProviderInterface> CryptoModule)
{
	// Clean up previous crypto bridge if it was already set.
	if(CryptoBridge)
	{
		CryptoBridge->CleanUpCryptoBridge();
	}

	// If empty object is given, just clean up the module
	UObject* CryptorObject = CryptoModule.GetObject();
	if(!CryptorObject)
	{
		pubnub_set_crypto_module(ctx_pub, nullptr);
		pubnub_set_crypto_module(ctx_ee, nullptr);
		CryptoBridge = nullptr;
	}
	else
	{
		CryptoBridge = NewObject<UPubnubCryptoBridge>(this);
		CryptoBridge->InitCryptoBridge(CryptoModule);

		pubnub_set_crypto_module(ctx_pub, CryptoBridge->GetProvider());
		pubnub_set_crypto_module(ctx_ee, CryptoBridge->GetProvider());
	}
}

TScriptInterface<IPubnubCryptoProviderInterface> UPubnubSubsystem::GetCryptoModule()
{
	if(CryptoBridge)
	{
		return CryptoBridge->GetUECryptoModule();
	}

	return nullptr;
}

FString UPubnubSubsystem::GetLastResponse(pubnub_t* context)
{
	FString Response;
	
	if(!context)
	{return Response;}
	
	pubnub_res PubnubResponse = pubnub_await(context);
	if (PNR_OK == PubnubResponse) {

		//Convert it keeping UTF8 characters valid
		const char* CharResponse = pubnub_get(context);
		FUTF8ToTCHAR Converter(CharResponse);
		Response = FString(Converter.Length(), Converter.Get());
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
		
		//Convert it keeping UTF8 characters valid
		const char* CharResponse = pubnub_get_channel(context);
		FUTF8ToTCHAR Converter(CharResponse);
		Response = FString(Converter.Length(), Converter.Get());
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

	FUTF8StringHolder UserIDHolder(UserID);
	pubnub_set_user_id(ctx_pub, UserIDHolder.Get());
	pubnub_set_user_id(ctx_ee, UserIDHolder.Get());

	IsUserIDSet = true;
}

void UPubnubSubsystem::PublishMessage_priv(FString Channel, FString Message, FOnPublishMessageResponseNative OnPublishMessageResponse, FPubnubPublishSettings PublishSettings)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnPublishMessageResponse, FPubnubMessageData());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnPublishMessageResponse, FPubnubMessageData());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Message, OnPublishMessageResponse, FPubnubMessageData());

	FString FinalMessage = Message;

	//If provided string is not a valid Json object or array, we treat it as literal string and serialize it
	if(!UPubnubJsonUtilities::IsCorrectJsonString(Message, false))
	{
		FinalMessage = UPubnubJsonUtilities::SerializeString(FinalMessage);
	}

	FUTF8StringHolder MessageHolder(FinalMessage);
	FUTF8StringHolder ChannelHolder(Channel);
	
	//Convert all UE PublishSettings to Pubnub PublishOptions
	
	//Converted char needs to live in function scope, so we need to create it here
	pubnub_publish_options PubnubOptions;
	
	FUTF8StringHolder MetaHolder(PublishSettings.MetaData);
	FUTF8StringHolder CustomMessageTypeHolder(PublishSettings.CustomMessageType);
	PubnubOptions.meta = MetaHolder.Get();
	PubnubOptions.custom_message_type = CustomMessageTypeHolder.Get();
	
	PublishUESettingsToPubnubPublishOptions(PublishSettings, PubnubOptions);
	pubnub_publish_ex(ctx_pub, ChannelHolder.Get(), MessageHolder.Get(), PubnubOptions);

	pubnub_res PublishResultStatus = pubnub_await(ctx_pub);
	
	FPubnubMessageData PublishedMessage;
	FPubnubOperationResult PublishResult;

	//Fill data about Publish Result
	PublishResult.Status = pubnub_last_http_code(ctx_pub);
	PublishResult.ErrorMessage = pubnub_last_publish_result(ctx_pub);
	PublishResult.Error = PublishResultStatus != PNR_OK;

	//In case error message is empty, we just put status there, it might be more useful than nothing
	if(PublishResult.ErrorMessage.IsEmpty())
	{
		PublishResult.ErrorMessage = pubnub_res_2_string(PublishResultStatus);
	}
	
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

	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnPublishMessageResponse, PublishResult, PublishedMessage);
}

void UPubnubSubsystem::Signal_priv(FString Channel, FString Message, FOnSignalResponseNative OnSignalResponse, FPubnubSignalSettings SignalSettings)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnSignalResponse, FPubnubMessageData());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnSignalResponse, FPubnubMessageData());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Message, OnSignalResponse, FPubnubMessageData());

	FString FinalMessage = Message;
	//If provided string is not a valid Json object or array, we treat it as literal string and serialize it
	if(!UPubnubJsonUtilities::IsCorrectJsonString(Message, false))
	{
		FinalMessage = UPubnubJsonUtilities::SerializeString(FinalMessage);
	}
	
	FUTF8StringHolder MessageHolder(FinalMessage);
	FUTF8StringHolder ChannelHolder(Channel);
	
	pubnub_signal_options PubnubOptions = pubnub_signal_defopts();
	FUTF8StringHolder CustomMessageTypeHolder(SignalSettings.CustomMessageType);
	PubnubOptions.custom_message_type = SignalSettings.CustomMessageType.IsEmpty() ? NULL : CustomMessageTypeHolder.Get();
	pubnub_signal_ex(ctx_pub, ChannelHolder.Get(), MessageHolder.Get(), PubnubOptions);
	
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

	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnSignalResponse, PublishResult, SignalMessage);
}

void UPubnubSubsystem::SubscribeToChannel_priv(FString Channel, FOnSubscribeOperationResponseNative OnSubscribeToChannelResponse, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnSubscribeToChannelResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnSubscribeToChannelResponse);

	if(ChannelSubscriptions.Contains(Channel))
	{
		PubnubError("[SubscribeToChannel]: Already subscribed to this channel. Aborting operation.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSubscribeToChannelResponse, "[SubscribeToChannel]: Already subscribed to chis channel. Aborting operation.");
		return;
	}

	//All subscription related operations are non blocking, so we lock ActionThread manually,
	//make it wait with calling other function until we have subscription result
	QuickActionThread->LockForSubscribeOperation();

	//Create subscription for channel entity
	pubnub_subscription_t* Subscription = UPubnubUtilities::EEGetSubscriptionForChannel(ctx_ee, Channel, SubscribeSettings);

	if(nullptr == Subscription)
	{
		PubnubError("[SubscribeToChannel]: Failed to subscribe to channel. Pubnub_subscription_alloc didn't create subscription.");
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSubscribeToChannelResponse, "[SubscribeToChannel]: Failed to subscribe to channel. Pubnub_subscription_alloc didn't create subscription.");
		QuickActionThread->UnlockAfterSubscriptionOperationFinished();
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
		PubnubError("[SubscribeToChannel]: Failed to subscribe to channel.");
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSubscribeToChannelResponse, "[SubscribeToChannel]: Failed to subscribe to channel.");
		QuickActionThread->UnlockAfterSubscriptionOperationFinished();
		return;
	}

	//Save Callback and Subscription, so later we can use it to unsubscribe
	CCoreSubscriptionData SubscriptionData{Callback, Subscription};
	ChannelSubscriptions.Add(Channel, SubscriptionData);
}

void UPubnubSubsystem::SubscribeToGroup_priv(FString ChannelGroup, FOnSubscribeOperationResponseNative OnSubscribeToGroupResponse, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnSubscribeToGroupResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(ChannelGroup, OnSubscribeToGroupResponse);

	if(ChannelGroupSubscriptions.Contains(ChannelGroup))
	{
		PubnubError("[SubscribeToGroup]: Already subscribed to this channel group. Aborting operation.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSubscribeToGroupResponse, "[SubscribeToGroup]: Already subscribed to chis channel group. Aborting operation.");
		return;
	}

	//All subscription related operations are non blocking, so we lock ActionThread manually,
	//make it wait with calling other function until we have subscription result
	QuickActionThread->LockForSubscribeOperation();
	
	//Create subscription for channel group entity
	pubnub_subscription_t* Subscription = UPubnubUtilities::EEGetSubscriptionForChannelGroup(ctx_ee, ChannelGroup, SubscribeSettings);

	if(nullptr == Subscription)
	{
		PubnubError("[SubscribeToGroup]: Failed to subscribe to channel group. Pubnub_subscription_alloc didn't create subscription.");
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSubscribeToGroupResponse, "[SubscribeToGroup]: Failed to subscribe to channel group. Pubnub_subscription_alloc didn't create subscription.");
		QuickActionThread->UnlockAfterSubscriptionOperationFinished();
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
		PubnubError("[SubscribeToGroup]: Failed to subscribe to channel group.");
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSubscribeToGroupResponse, "[SubscribeToGroup]: Failed to subscribe to channel group.");
		QuickActionThread->UnlockAfterSubscriptionOperationFinished();
		return;
	}

	//Save Callback and Subscription, so later we can use it to unsubscribe
	CCoreSubscriptionData SubscriptionData{Callback, Subscription};
	ChannelGroupSubscriptions.Add(ChannelGroup, SubscriptionData);
}

void UPubnubSubsystem::UnsubscribeFromChannel_priv(FString Channel, FOnSubscribeOperationResponseNative OnUnsubscribeFromChannelResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnUnsubscribeFromChannelResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnUnsubscribeFromChannelResponse);
	
	CCoreSubscriptionData* SubscriptionData =  ChannelSubscriptions.Find(Channel);
	if(!SubscriptionData)
	{
		PubnubError("[UnsubscribeFromChannel]: There is no such subscription. Aborting operation.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnUnsubscribeFromChannelResponse, "[UnsubscribeFromChannel]: There is no such subscription. Aborting operation.");
		return;
	}

	//All subscription related operations are non blocking, so we lock ActionThread manually,
	//make it wait with calling other function until we have subscription result
	QuickActionThread->LockForSubscribeOperation();

	//Remove subscription listener and unsubscribe with subscription
	if(!UPubnubUtilities::EERemoveListenerAndUnsubscribe(&SubscriptionData->Subscription, SubscriptionData->Callback, this))
	{
		PubnubError("[UnsubscribeFromChannel]: Failed to unsubscribe.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnUnsubscribeFromChannelResponse, "[UnsubscribeFromChannel]: Failed to unsubscribe.");
		QuickActionThread->UnlockAfterSubscriptionOperationFinished();
		return;
	}

	//Free subscription memory
	pubnub_subscription_free(&SubscriptionData->Subscription);

	ChannelSubscriptions.Remove(Channel);
}

void UPubnubSubsystem::UnsubscribeFromGroup_priv(FString ChannelGroup, FOnSubscribeOperationResponseNative OnUnsubscribeFromGroupResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnUnsubscribeFromGroupResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(ChannelGroup, OnUnsubscribeFromGroupResponse);

	CCoreSubscriptionData* SubscriptionData =  ChannelGroupSubscriptions.Find(ChannelGroup);
	if(!SubscriptionData)
	{
		PubnubError("[UnsubscribeFromGroup]: There is no such subscription. Aborting operation.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnUnsubscribeFromGroupResponse, "[UnsubscribeFromGroup]: There is no such subscription. Aborting operation.");
		return;
	}

	//All subscription related operations are non blocking, so we lock ActionThread manually,
	//make it wait with calling other function until we have subscription result
	QuickActionThread->LockForSubscribeOperation();
	
	//Remove subscription listener and unsubscribe with subscription
	if(!UPubnubUtilities::EERemoveListenerAndUnsubscribe(&SubscriptionData->Subscription, SubscriptionData->Callback, this))
	{
		PubnubError("[UnsubscribeFromGroup]: Failed to unsubscribe.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnUnsubscribeFromGroupResponse, "[UnsubscribeFromGroup]: Failed to unsubscribe.");
		QuickActionThread->UnlockAfterSubscriptionOperationFinished();
		return;
	}

	//Free subscription memory
	pubnub_subscription_free(&SubscriptionData->Subscription);

	ChannelGroupSubscriptions.Remove(ChannelGroup);
}

void UPubnubSubsystem::UnsubscribeFromAll_priv(FOnSubscribeOperationResponseNative OnUnsubscribeFromAllResponse)
{
	if(ChannelSubscriptions.IsEmpty() && ChannelGroupSubscriptions.IsEmpty())
	{
		UPubnubUtilities::CallPubnubDelegate(OnUnsubscribeFromAllResponse, FPubnubOperationResult({200, false, ""}));
		return;
	}

	PUBNUB_ENSURE_USER_ID_IS_SET(OnUnsubscribeFromAllResponse);

	//All subscription related operations are non blocking, so we lock ActionThread manually,
	//make it wait with calling other function until we have subscription result
	QuickActionThread->LockForSubscribeOperation();

	pubnub_unsubscribe_all(ctx_ee);
	
	ChannelSubscriptions.Empty();
	ChannelGroupSubscriptions.Empty();
}

void UPubnubSubsystem::AddChannelToGroup_priv(FString Channel, FString ChannelGroup, FOnAddChannelToGroupResponseNative OnAddChannelToGroupResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnAddChannelToGroupResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnAddChannelToGroupResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(ChannelGroup, OnAddChannelToGroupResponse);

	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);
	FUTF8StringHolder ChannelHolder(Channel);
	
	pubnub_add_channel_to_group(ctx_pub, ChannelHolder.Get(), ChannelGroupHolder.Get());

	//This is just to clear the C-Core response buffer, but it doesn't return the server response
	GetLastResponse(ctx_pub);
	//So we need to get the response separately
	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);

	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnAddChannelToGroupResponse, UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse));
}

void UPubnubSubsystem::RemoveChannelFromGroup_priv(FString Channel, FString ChannelGroup, FOnRemoveChannelFromGroupResponseNative OnRemoveChannelFromGroupResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnRemoveChannelFromGroupResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnRemoveChannelFromGroupResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(ChannelGroup, OnRemoveChannelFromGroupResponse);

	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);
	FUTF8StringHolder ChannelHolder(Channel);

	pubnub_remove_channel_from_group(ctx_pub, ChannelHolder.Get(), ChannelGroupHolder.Get());

	//This is just to clear the C-Core response buffer, but it doesn't return the server response
	GetLastResponse(ctx_pub);
	//So we need to get the response separately
	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);

	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnRemoveChannelFromGroupResponse, UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse));
}

void UPubnubSubsystem::ListChannelsFromGroup_priv(FString ChannelGroup, FOnListChannelsFromGroupResponseNative OnListChannelsResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnListChannelsResponse, TArray<FString>{});
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(ChannelGroup, OnListChannelsResponse, TArray<FString>{});
	
	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);
	
	pubnub_list_channel_group(ctx_pub, ChannelGroupHolder.Get());
	
	FString JsonResponse = GetLastChannelResponse(ctx_pub);

	//Execute provided delegate with results
	FPubnubOperationResult Result;
	TArray<FString> Channels;
	UPubnubJsonUtilities::ListChannelsFromGroupJsonToData(JsonResponse, Result, Channels);
	UPubnubUtilities::CallPubnubDelegate(OnListChannelsResponse, Result, Channels);
}

void UPubnubSubsystem::RemoveChannelGroup_priv(FString ChannelGroup, FOnRemoveChannelGroupResponseNative OnRemoveChannelGroupResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnRemoveChannelGroupResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(ChannelGroup, OnRemoveChannelGroupResponse);

	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);

	pubnub_remove_channel_group(ctx_pub, ChannelGroupHolder.Get());
	
	//This is just to clear the C-Core response buffer, but it doesn't return the server response
	GetLastResponse(ctx_pub);
	//So we need to get the response separately
	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnRemoveChannelGroupResponse, UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse));
}

void UPubnubSubsystem::ListUsersFromChannel_priv(FString Channel, FOnListUsersFromChannelResponseNative ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(ListUsersFromChannelResponse, FPubnubListUsersFromChannelWrapper());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, ListUsersFromChannelResponse, FPubnubListUsersFromChannelWrapper());

	//Set all options from ListUsersFromChannelSettings
	FUTF8StringHolder ChannelHolder(Channel);
	
	//Converted char needs to live in function scope, so we need to create it here
	pubnub_here_now_options HereNowOptions;
	FUTF8StringHolder ChannelGroupHolder(ListUsersFromChannelSettings.ChannelGroup);
	HereNowOptions.channel_group = ChannelGroupHolder.Get();
	
	HereNowUESettingsToPubnubHereNowOptions(ListUsersFromChannelSettings, HereNowOptions);
	
	pubnub_here_now_ex(ctx_pub, ChannelHolder.Get(), HereNowOptions);
	
	FString JsonResponse = GetLastResponse(ctx_pub);
	
	//Execute provided delegate with results
	FPubnubOperationResult Result;
	FPubnubListUsersFromChannelWrapper Data;
	UPubnubJsonUtilities::ListUsersFromChannelJsonToData(JsonResponse, Result, Data);
	UPubnubUtilities::CallPubnubDelegate(ListUsersFromChannelResponse, Result, Data);
}

void UPubnubSubsystem::ListUserSubscribedChannels_priv(FString UserID, FOnListUsersSubscribedChannelsResponseNative ListUserSubscribedChannelsResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(ListUserSubscribedChannelsResponse, TArray<FString>{});
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(UserID, ListUserSubscribedChannelsResponse, TArray<FString>{});

	FUTF8StringHolder UserIDHolder(UserID);
	pubnub_where_now(ctx_pub, UserIDHolder.Get());

	FString JsonResponse = GetLastResponse(ctx_pub);
	
	//Execute provided delegate with results
	FPubnubOperationResult Result;
	TArray<FString> Channels;
	UPubnubJsonUtilities::ListUserSubscribedChannelsJsonToData(JsonResponse, Result, Channels);
	UPubnubUtilities::CallPubnubDelegate(ListUserSubscribedChannelsResponse, Result, Channels);
}

void UPubnubSubsystem::SetState_priv(FString Channel, FString StateJson, FOnSetStateResponseNative OnSetStateResponse, FPubnubSetStateSettings SetStateSettings)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnSetStateResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnSetStateResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(StateJson, OnSetStateResponse);

	if(!UPubnubJsonUtilities::IsCorrectJsonString(StateJson, false))
	{
		PubnubError("[SetState]: StateJson has to be a correct Json Object. Aborting operation.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSetStateResponse, "[SetState]: StateJson has to be a correct Json Object. Operation aborted.");
		return;
	}
	
	//Set all options from SetStateSettings

	//Converted char needs to live in function scope, so we need to create it here
	pubnub_set_state_options SetStateOptions;
	FUTF8StringHolder ChannelGroupHolder(SetStateSettings.ChannelGroup);
	SetStateOptions.channel_group = ChannelGroupHolder.Get();
	FUTF8StringHolder UserIDHolder(SetStateSettings.UserID);
	SetStateOptions.user_id = UserIDHolder.Get();

	SetStateUESettingsToPubnubSetStateOptions(SetStateSettings, SetStateOptions);

	FUTF8StringHolder StateJsonHolder(StateJson);
	FUTF8StringHolder ChannelHolder(Channel);
	
	pubnub_set_state_ex(ctx_pub, ChannelHolder.Get(), StateJsonHolder.Get(), SetStateOptions);
	
	//This is just to clear the C-Core response buffer, but it doesn't return the server response
	GetLastResponse(ctx_pub);
	//So we need to get the response separately
	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);

	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnSetStateResponse, UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse));
}

void UPubnubSubsystem::GetState_priv(FString Channel, FString ChannelGroup, FString UserID, FOnGetStateResponseNative OnGetStateResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnGetStateResponse, "");
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnGetStateResponse, "");

	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder UserIDHolder(UserID);

	pubnub_state_get(ctx_pub, ChannelHolder.Get(), ChannelGroupHolder.Get(), UserIDHolder.Get());
	FString JsonResponse = GetLastResponse(ctx_pub);
	
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}

	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnGetStateResponse, UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse), JsonResponse);
}

void UPubnubSubsystem::Heartbeat_priv(FString Channel, FString ChannelGroup)
{
	PUBNUB_RETURN_IF_USER_ID_NOT_SET();
	
	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);
	FUTF8StringHolder ChannelHolder(Channel);

	pubnub_heartbeat(ctx_pub, ChannelHolder.Get(), ChannelGroupHolder.Get());

	GetLastResponse(ctx_pub);
}

void UPubnubSubsystem::GrantToken_priv(FString PermissionObject, FOnGrantTokenResponseNative OnGrantTokenResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnGrantTokenResponse, FString());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(PermissionObject, OnGrantTokenResponse, FString());

	FUTF8StringHolder PermissionObjectHolder(PermissionObject);
	
	pubnub_grant_token(ctx_pub, PermissionObjectHolder.Get());

	pubnub_await(ctx_pub);
	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	//Access Manager has similar result structure to AppContext, so we use the same getter
	FPubnubOperationResult Result = UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(JsonResponse);
	FString Token = "";
	if(Result.Status == 200)
	{
		pubnub_chamebl_t grant_token_resp = pubnub_get_grant_token(ctx_pub);
		Token = UPubnubUtilities::PubnubCharMemBlockToString(grant_token_resp);
	}
	
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnGrantTokenResponse, Result, Token);
}

void UPubnubSubsystem::RevokeToken_priv(FString Token, FOnRevokeTokenResponseNative OnRevokeTokenResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnRevokeTokenResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Token, OnRevokeTokenResponse);

	FUTF8StringHolder TokenHolder(Token);
	
	pubnub_revoke_token(ctx_pub, TokenHolder.Get());

	FString JsonResponse = GetLastResponse(ctx_pub);

	//If response is empty, there was server error. 
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Execute provided delegate with results
	//Access Manager has similar result structure to AppContext, so we use the same getter
	UPubnubUtilities::CallPubnubDelegate(OnRevokeTokenResponse, UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(JsonResponse));
}

FString UPubnubSubsystem::ParseToken_priv(FString Token)
{
	PUBNUB_RETURN_IF_USER_ID_NOT_SET("");
	PUBNUB_RETURN_IF_FIELD_EMPTY(Token, "");

	FUTF8StringHolder TokenHolder(Token);
	
	char* TokenResponse = pubnub_parse_token(ctx_pub, TokenHolder.Get());
	FUTF8ToTCHAR Converter(TokenResponse);
	FString ParsedToken(Converter.Length(), Converter.Get());
	
	//Free this char, as it's allocated with malloc inside of pubnub_parse_token
	free(TokenResponse);

	//Rework parsed token into more human readable form
	return UPubnubTokenUtilities::ReworkParsedToken(ParsedToken);
}

void UPubnubSubsystem::FetchHistory_priv(FString Channel, FOnFetchHistoryResponseNative OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnFetchHistoryResponse, TArray<FPubnubHistoryMessageData>());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnFetchHistoryResponse, TArray<FPubnubHistoryMessageData>());
	
	//Set all options from HistorySettings

	//Converted char needs to live in function scope, so we need to create it here
	pubnub_fetch_history_options FetchHistoryOptions;
	FUTF8StringHolder StartHolder(FetchHistorySettings.Start);
	FUTF8StringHolder EndHolder(FetchHistorySettings.End);
	FetchHistoryOptions.start = StartHolder.Get();
	FetchHistoryOptions.end = EndHolder.Get();

	FetchHistoryUESettingsToPbFetchHistoryOptions(FetchHistorySettings, FetchHistoryOptions);

	FUTF8StringHolder ChannelHolder(Channel);
	
	pubnub_fetch_history(ctx_pub, ChannelHolder.Get(), FetchHistoryOptions);
	
	//FString JsonResponse = GetLastResponse(ctx_pub);


	FString HistoryResponse = "";
	
	
	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if (PNR_OK == PubnubResponse) {

		//Convert it keeping UTF8 characters valid
		pubnub_chamebl_t HistoryMemBlock = pubnub_get_fetch_history(ctx_pub);
		HistoryResponse = UPubnubUtilities::PubnubCharMemBlockToString(HistoryMemBlock);
	}
	else
	{
		PubnubResponseError(PubnubResponse, "Failed to get last response.");
	}


	

	//If response is empty, there was server error. 
	if(HistoryResponse.IsEmpty())
	{
		HistoryResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Parse Json response into data
	FPubnubOperationResult Result;
	TArray<FPubnubHistoryMessageData> Messages;
	UPubnubJsonUtilities::FetchHistoryJsonToData(HistoryResponse, Result, Messages);
	DecryptHistoryMessages(Messages);
				
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnFetchHistoryResponse, Result, Messages);
}

void UPubnubSubsystem::DeleteMessages_priv(FString Channel, FOnDeleteMessagesResponseNative OnDeleteMessagesResponse, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnDeleteMessagesResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnDeleteMessagesResponse);

	pubnub_delete_messages_options DeleteMessagesOptions = pubnub_delete_messages_defopts();
	FUTF8StringHolder StartHolder(DeleteMessagesSettings.Start);
	FUTF8StringHolder EndHolder(DeleteMessagesSettings.End);
	DeleteMessagesOptions.start = StartHolder.Get();
	DeleteMessagesOptions.end = EndHolder.Get();

	FUTF8StringHolder ChannelHolder(Channel);

	pubnub_delete_messages(ctx_pub, ChannelHolder.Get(), DeleteMessagesOptions);
	
	FString JsonResponse = GetLastResponse(ctx_pub);

	//If response is empty, there was server error. 
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}

	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnDeleteMessagesResponse, UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse));
}

void UPubnubSubsystem::MessageCounts_priv(FString Channel, FString Timetoken, FOnMessageCountsResponseNative OnMessageCountsResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnMessageCountsResponse, 0);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnMessageCountsResponse, 0);

	FUTF8StringHolder TimetokenHolder(Timetoken);
	FUTF8StringHolder ChannelHolder(Channel);
	
	pubnub_message_counts(ctx_pub, ChannelHolder.Get(), TimetokenHolder.Get());

	pubnub_await(ctx_pub);

	int MessageCountsNumber = 0;
	pubnub_get_message_counts(ctx_pub, ChannelHolder.Get(), &MessageCountsNumber);

	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnMessageCountsResponse, UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse), MessageCountsNumber);
}

void UPubnubSubsystem::GetAllUserMetadata_priv(FOnGetAllUserMetadataResponseNative OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnGetAllUserMetadataResponse, TArray<FPubnubUserData>(), FString(), FString());

	pubnub_getall_metadata_opts PubnubOptions = pubnub_getall_metadata_defopts();
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(PageNext);
	FUTF8StringHolder PagePrevHolder(PagePrev);
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  PagePrevHolder.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;
	
	pubnub_getall_uuidmetadata_ex(ctx_pub, PubnubOptions);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Parse Json response into data
	FPubnubOperationResult Result;
	TArray<FPubnubUserData> UsersData;
	FString ResultPageNext;
	FString ResultPagePrev;
	UPubnubJsonUtilities::GetAllUserMetadataJsonToData(JsonResponse, Result, UsersData, ResultPageNext, ResultPagePrev);
	
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnGetAllUserMetadataResponse, Result, UsersData, ResultPageNext, ResultPagePrev);
}

void UPubnubSubsystem::SetUserMetadata_priv(FString User, FString UserMetadataObj, FOnSetUserMetadataResponseNative OnSetUserMetadataResponse, FString Include)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnSetUserMetadataResponse, FPubnubUserData());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(User, OnSetUserMetadataResponse, FPubnubUserData());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(UserMetadataObj, OnSetUserMetadataResponse, FPubnubUserData());

	if(!UPubnubJsonUtilities::IsCorrectJsonString(UserMetadataObj, false))
	{
		PubnubError("[SetUserMetadata]: UserMetadataObj has to be a correct Json Object. Aborting operation.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSetUserMetadataResponse, "[SetUserMetadata]: UserMetadataObj has to be a correct Json Object. Operation aborted.", FPubnubUserData());
		return;
	}
	
	FUTF8StringHolder UserHolder(User);
	FUTF8StringHolder UserMetadataObjHolder(UserMetadataObj);
	FUTF8StringHolder IncludeHolder(Include);
	
	pubnub_set_uuidmetadata(ctx_pub, UserHolder.Get(), IncludeHolder.Get(), UserMetadataObjHolder.Get());

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Parse Json response into data
	FPubnubOperationResult Result;
	FPubnubUserData UserData;
	UPubnubJsonUtilities::GetUserMetadataJsonToData(JsonResponse, Result, UserData);
								
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnSetUserMetadataResponse, Result, UserData);
}

void UPubnubSubsystem::GetUserMetadata_priv(FString User, FOnGetUserMetadataResponseNative OnGetUserMetadataResponse, FString Include)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnGetUserMetadataResponse, FPubnubUserData());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(User, OnGetUserMetadataResponse, FPubnubUserData());

	FUTF8StringHolder UserHolder(User);
	FUTF8StringHolder IncludeHolder(Include);
	pubnub_get_uuidmetadata(ctx_pub, IncludeHolder.Get(), UserHolder.Get());

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}

	//Parse Json response into data
	FPubnubOperationResult Result;
	FPubnubUserData UserData;
	UPubnubJsonUtilities::GetUserMetadataJsonToData(JsonResponse, Result, UserData);
								
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnGetUserMetadataResponse, Result, UserData);
}

void UPubnubSubsystem::RemoveUserMetadata_priv(FString User, FOnRemoveUserMetadataResponseNative OnRemoveUserMetadataResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnRemoveUserMetadataResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(User, OnRemoveUserMetadataResponse);

	FUTF8StringHolder UserHolder(User);
	
	pubnub_remove_uuidmetadata(ctx_pub, UserHolder.Get());

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnRemoveUserMetadataResponse, UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(JsonResponse));
}

void UPubnubSubsystem::GetAllChannelMetadata_priv(FOnGetAllChannelMetadataResponseNative OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnGetAllChannelMetadataResponse, TArray<FPubnubChannelData>(), FString(), FString());
	
	pubnub_getall_metadata_opts PubnubOptions = pubnub_getall_metadata_defopts();
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(PageNext);
	FUTF8StringHolder PagePrevHolder(PagePrev);
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  PagePrevHolder.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;
	
	pubnub_getall_channelmetadata_ex(ctx_pub, PubnubOptions);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Parse Json response into data
	FPubnubOperationResult Result;
	TArray<FPubnubChannelData> ChannelsData;
	FString ResultPageNext;
	FString ResultPagePrev;
	UPubnubJsonUtilities::GetAllChannelMetadataJsonToData(JsonResponse, Result, ChannelsData, ResultPageNext, ResultPagePrev);
	
	//Execute provided delegate with results
    UPubnubUtilities::CallPubnubDelegate(OnGetAllChannelMetadataResponse, Result, ChannelsData, ResultPageNext, ResultPagePrev);
}

void UPubnubSubsystem::SetChannelMetadata_priv(FString Channel, FString ChannelMetadataObj, FOnSetChannelMetadataResponseNative OnSetChannelMetadataResponse, FString Include)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnSetChannelMetadataResponse, FPubnubChannelData());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnSetChannelMetadataResponse, FPubnubChannelData());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(ChannelMetadataObj, OnSetChannelMetadataResponse, FPubnubChannelData());

	if(!UPubnubJsonUtilities::IsCorrectJsonString(ChannelMetadataObj, false))
	{
		PubnubError("[SetChannelMetadata]: ChannelMetadataObj has to be a correct Json Object. Aborting operation.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSetChannelMetadataResponse, "[SetChannelMetadata]: ChannelMetadataObj has to be a correct Json Object. Operation aborted.", FPubnubChannelData());
		return;
	}
	
	FUTF8StringHolder ChannelMetadataObjHolder(ChannelMetadataObj);
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder IncludeHolder(Include);
	
	pubnub_set_channelmetadata(ctx_pub, ChannelHolder.Get(), IncludeHolder.Get(), ChannelMetadataObjHolder.Get());

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Parse Json response into data
	FPubnubOperationResult Result;
	FPubnubChannelData ChannelData;
	UPubnubJsonUtilities::GetChannelMetadataJsonToData(JsonResponse, Result, ChannelData);
								
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnSetChannelMetadataResponse, Result, ChannelData);
}

void UPubnubSubsystem::GetChannelMetadata_priv(FString Channel, FOnGetChannelMetadataResponseNative OnGetChannelMetadataResponse, FString Include)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnGetChannelMetadataResponse, FPubnubChannelData());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnGetChannelMetadataResponse, FPubnubChannelData());

	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder IncludeHolder(Include);
	
	pubnub_get_channelmetadata(ctx_pub, IncludeHolder.Get(), ChannelHolder.Get());

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}

	//Parse Json response into data
	FPubnubOperationResult Result;
	FPubnubChannelData ChannelData;
	UPubnubJsonUtilities::GetChannelMetadataJsonToData(JsonResponse, Result, ChannelData);
								
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnGetChannelMetadataResponse, Result, ChannelData);
}

void UPubnubSubsystem::RemoveChannelMetadata_priv(FString Channel, FOnRemoveChannelMetadataResponseNative OnRemoveChannelMetadataResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnRemoveChannelMetadataResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnRemoveChannelMetadataResponse);

	FUTF8StringHolder ChannelHolder(Channel);

	pubnub_remove_channelmetadata(ctx_pub, ChannelHolder.Get());

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnRemoveChannelMetadataResponse, UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(JsonResponse));
}

void UPubnubSubsystem::GetMemberships_priv(FString User, FOnGetMembershipsResponseNative OnGetMembershipsResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnGetMembershipsResponse, TArray<FPubnubMembershipData>(), FString(), FString());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(User, OnGetMembershipsResponse, TArray<FPubnubMembershipData>(), FString(), FString());
	
	pubnub_membership_opts PubnubOptions = pubnub_membership_opts();
	FUTF8StringHolder UserHolder(User);
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(PageNext);
	FUTF8StringHolder PagePrevHolder(PagePrev);
	PubnubOptions.uuid = User.IsEmpty() ? NULL : UserHolder.Get();
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  PagePrevHolder.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;
	
	pubnub_get_memberships_ex(ctx_pub, PubnubOptions);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Parse Json response into data
	FPubnubOperationResult Result;
	TArray<FPubnubMembershipData> MembershipsData;
	FString ResultPageNext;
	FString ResultPagePrev;
	UPubnubJsonUtilities::GetMembershipsJsonToData(JsonResponse, Result, MembershipsData, ResultPageNext, ResultPagePrev);
	
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnGetMembershipsResponse, Result, MembershipsData, ResultPageNext, ResultPagePrev);
}

void UPubnubSubsystem::SetMemberships_priv(FString User, FString SetObj, FOnSetMembershipsResponseNative OnSetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnSetMembershipResponse, TArray<FPubnubMembershipData>(), FString(), FString());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(User, OnSetMembershipResponse, TArray<FPubnubMembershipData>(), FString(), FString());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(SetObj, OnSetMembershipResponse, TArray<FPubnubMembershipData>(), FString(), FString());

	if(!UPubnubJsonUtilities::IsCorrectJsonString(SetObj, false))
	{
		PubnubError("[SetMemberships]: SetObj has to be a correct Json Object. Aborting operation.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSetMembershipResponse, "[SetMemberships]: SetObj has to be a correct Json Object. Operation aborted.", TArray<FPubnubMembershipData>(), FString(), FString());
		return;
	}

	pubnub_membership_opts PubnubOptions = pubnub_membership_opts();
	FUTF8StringHolder UserHolder(User);
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(PageNext);
	FUTF8StringHolder PagePrevHolder(PagePrev);
	PubnubOptions.uuid = User.IsEmpty() ? NULL : UserHolder.Get();
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  PagePrevHolder.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;

	FUTF8StringHolder SetObjHolder(SetObj);
	pubnub_set_memberships_ex(ctx_pub, SetObjHolder.Get(), PubnubOptions);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}

	//Parse Json response into data
	FPubnubOperationResult Result;
	TArray<FPubnubMembershipData> MembershipsData;
	FString ResultPageNext;
	FString ResultPagePrev;
	UPubnubJsonUtilities::GetMembershipsJsonToData(JsonResponse, Result, MembershipsData, ResultPageNext, ResultPagePrev);
	
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnSetMembershipResponse, Result, MembershipsData, ResultPageNext, ResultPagePrev);
}

void UPubnubSubsystem::RemoveMemberships_priv(FString User, FString RemoveObj, FOnRemoveMembershipsResponseNative OnRemoveMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnRemoveMembershipResponse, TArray<FPubnubMembershipData>(), FString(), FString());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(User, OnRemoveMembershipResponse, TArray<FPubnubMembershipData>(), FString(), FString());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(RemoveObj, OnRemoveMembershipResponse, TArray<FPubnubMembershipData>(), FString(), FString());

	if(!UPubnubJsonUtilities::IsCorrectJsonString(RemoveObj, false))
	{
		PubnubError("[RemoveMemberships]: RemoveObj has to be a correct Json Object. Aborting operation.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnRemoveMembershipResponse, "[RemoveMemberships]: RemoveObj has to be a correct Json Object. Operation aborted.", TArray<FPubnubMembershipData>(), FString(), FString());
		return;
	}

	pubnub_membership_opts PubnubOptions = pubnub_membership_opts();
	FUTF8StringHolder UserHolder(User);
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(PageNext);
	FUTF8StringHolder PagePrevHolder(PagePrev);
	PubnubOptions.uuid = User.IsEmpty() ? NULL : UserHolder.Get();
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  PagePrevHolder.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;

	FUTF8StringHolder RemoveObjHolder(RemoveObj);
	pubnub_remove_memberships_ex(ctx_pub, RemoveObjHolder.Get(), PubnubOptions);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}

	//Parse Json response into data
	FPubnubOperationResult Result;
	TArray<FPubnubMembershipData> MembershipsData;
	FString ResultPageNext;
	FString ResultPagePrev;
	UPubnubJsonUtilities::GetMembershipsJsonToData(JsonResponse, Result, MembershipsData, ResultPageNext, ResultPagePrev);

	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnRemoveMembershipResponse, Result, MembershipsData, ResultPageNext, ResultPagePrev);
}

void UPubnubSubsystem::GetChannelMembers_priv(FString Channel, FOnGetChannelMembersResponseNative OnGetMembersResponse, FString Include, int Limit,
	FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnGetMembersResponse, TArray<FPubnubChannelMemberData>(), FString(), FString());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnGetMembersResponse, TArray<FPubnubChannelMemberData>(), FString(), FString());
	
	pubnub_members_opts PubnubOptions = pubnub_members_opts();
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(PageNext);
	FUTF8StringHolder PagePrevHolder(PagePrev);
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  PagePrevHolder.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;

	FUTF8StringHolder ChannelHolder(Channel);
	pubnub_get_members_ex(ctx_pub, ChannelHolder.Get(), PubnubOptions);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}

	//Parse Json response into data
	FPubnubOperationResult Result;
	TArray<FPubnubChannelMemberData> MembersData;
	FString ResultPageNext;
	FString ResultPagePrev;
	UPubnubJsonUtilities::GetChannelMembersJsonToData(JsonResponse, Result, MembersData, ResultPageNext, ResultPagePrev);
	
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnGetMembersResponse, Result, MembersData, ResultPageNext, ResultPagePrev);
}

void UPubnubSubsystem::SetChannelMembers_priv(FString Channel, FString SetObj, FOnSetChannelMembersResponseNative OnSetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnSetMembersResponse, TArray<FPubnubChannelMemberData>(), FString(), FString());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnSetMembersResponse, TArray<FPubnubChannelMemberData>(), FString(), FString());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(SetObj, OnSetMembersResponse, TArray<FPubnubChannelMemberData>(), FString(), FString());

	if(!UPubnubJsonUtilities::IsCorrectJsonString(SetObj, false))
	{
		PubnubError("[SetChannelMembers]: SetObj has to be a correct Json Object. Aborting operation.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSetMembersResponse, "[SetChannelMembers]: SetObj has to be a correct Json Object. Operation aborted.", TArray<FPubnubChannelMemberData>(), FString(), FString());
		return;
	}

	pubnub_members_opts PubnubOptions = pubnub_members_opts();
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(PageNext);
	FUTF8StringHolder PagePrevHolder(PagePrev);
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  PagePrevHolder.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;

	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder SetObjHolder(SetObj);
	pubnub_set_members_ex(ctx_pub, ChannelHolder.Get(), SetObjHolder.Get(), PubnubOptions);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Parse Json response into data
	FPubnubOperationResult Result;
	TArray<FPubnubChannelMemberData> MembersData;
	FString ResultPageNext;
	FString ResultPagePrev;
	UPubnubJsonUtilities::GetChannelMembersJsonToData(JsonResponse, Result, MembersData, ResultPageNext, ResultPagePrev);

	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnSetMembersResponse, Result, MembersData, ResultPageNext, ResultPagePrev);
}

void UPubnubSubsystem::RemoveChannelMembers_priv(FString Channel, FString RemoveObj, FOnRemoveChannelMembersResponseNative OnRemoveMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnRemoveMembersResponse, TArray<FPubnubChannelMemberData>(), FString(), FString());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnRemoveMembersResponse, TArray<FPubnubChannelMemberData>(), FString(), FString());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(RemoveObj, OnRemoveMembersResponse, TArray<FPubnubChannelMemberData>(), FString(), FString());

	if(!UPubnubJsonUtilities::IsCorrectJsonString(RemoveObj, false))
	{
		PubnubError("[RemoveChannelMembers]: RemoveObj has to be a correct Json Object. Aborting operation.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnRemoveMembersResponse, "[RemoveChannelMembers]: RemoveObj has to be a correct Json Object. Operation aborted.", TArray<FPubnubChannelMemberData>(), FString(), FString());
		return;
	}

	pubnub_members_opts PubnubOptions = pubnub_members_opts();
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(PageNext);
	FUTF8StringHolder PagePrevHolder(PagePrev);
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	PubnubOptions.page.next = PageNext.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = PagePrev.IsEmpty() ? NULL :  PagePrevHolder.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;

	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder RemoveObjHolder(RemoveObj);
	pubnub_remove_members_ex(ctx_pub, ChannelHolder.Get(), RemoveObjHolder.Get(), PubnubOptions);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Parse Json response into data
	FPubnubOperationResult Result;
	TArray<FPubnubChannelMemberData> MembersData;
	FString ResultPageNext;
	FString ResultPagePrev;
	UPubnubJsonUtilities::GetChannelMembersJsonToData(JsonResponse, Result, MembersData, ResultPageNext, ResultPagePrev);

	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnRemoveMembersResponse, Result, MembersData, ResultPageNext, ResultPagePrev);
}

void UPubnubSubsystem::AddMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnAddMessageActionResponseNative AddMessageActionResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(AddMessageActionResponse, FPubnubMessageActionData());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, AddMessageActionResponse, FPubnubMessageActionData());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(MessageTimetoken, AddMessageActionResponse, FPubnubMessageActionData());
	
	//Add quotes to these fields as they are required by C-Core
	FString FinalActionType = UPubnubUtilities::AddQuotesToString(ActionType);
	FString FinalValue = UPubnubUtilities::AddQuotesToString(Value);

	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder MessageTimetokenHolder(MessageTimetoken);
	FUTF8StringHolder FinalActionTypeHolder(FinalActionType);
	FUTF8StringHolder FinalValueHolder(FinalValue);
	
	pubnub_add_message_action_str(ctx_pub, ChannelHolder.Get(), MessageTimetokenHolder.Get(), FinalActionTypeHolder.Get(),  FinalValueHolder.Get());
	
	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	//Parse Json response into data
	FPubnubOperationResult Result;
	FPubnubMessageActionData MessageActionData;
	UPubnubJsonUtilities::AddMessageActionJsonToData(JsonResponse, Result, MessageActionData);
	
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(AddMessageActionResponse, Result, MessageActionData);
}

void UPubnubSubsystem::RemoveMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnRemoveMessageActionResponseNative OnRemoveMessageActionResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnRemoveMessageActionResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnRemoveMessageActionResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(MessageTimetoken, OnRemoveMessageActionResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(ActionTimetoken, OnRemoveMessageActionResponse);
	
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

	FUTF8StringHolder ChannelHolder(Channel);
	
	pubnub_remove_message_action(ctx_pub, ChannelHolder.Get(), message_timetoken_chamebl, action_timetoken_chamebl);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}

	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnRemoveMessageActionResponse, UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(JsonResponse));
}

void UPubnubSubsystem::GetMessageActions_priv(FString Channel, FOnGetMessageActionsResponseNative OnGetMessageActionsResponse, FString Start, FString End, int Limit)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnGetMessageActionsResponse, TArray<FPubnubMessageActionData>());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnGetMessageActionsResponse, TArray<FPubnubMessageActionData>());
	
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder StartHolder(Start);
	FUTF8StringHolder EndHolder(End);
	pubnub_get_message_actions(ctx_pub, ChannelHolder.Get(), StartHolder.Get(), EndHolder.Get(), Limit);
	
	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}

	//Parse Json response into data
	FPubnubOperationResult Result;
	TArray<FPubnubMessageActionData> MessageActions;
	UPubnubJsonUtilities::GetMessageActionsJsonToData(JsonResponse, Result, MessageActions);
										
	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnGetMessageActionsResponse, Result, MessageActions);
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

void UPubnubSubsystem::DecryptHistoryMessages(TArray<FPubnubHistoryMessageData>& Messages)
{
	//If crypto module is not set, we can't encrypt anything
	if(!CryptoBridge || !CryptoBridge->GetUECryptoModule() || !CryptoBridge->GetUECryptoModule().GetObject())
	{ return; }

	for(auto& Message : Messages)
	{
		FString ReworkedMessage = IPubnubCryptoProviderInterface::Execute_ProviderDecrypt(CryptoBridge->GetUECryptoModule().GetObject(), Message.Message);
		
		// If encryption failed - for example when history message was not encrypted, but crypto module is set, just leave the message as it is
		if(ReworkedMessage.IsEmpty())
		{ continue; }

		//Not encrypted messages are deserialized automatically, but in case of encrypted once we need to Deserialize them ourselves
		ReworkedMessage = UPubnubJsonUtilities::DeserializeString(ReworkedMessage);
		Message.Message = ReworkedMessage;
	}
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

//Logs from C-Core that are false warnings as they are sent during normal C-Core operations flow
TArray<FString> UPubnubSubsystem::FalseCCoreLogPhrases =
	{
		"errno=0('No error')",
		"errno=9('Bad file descriptor')",
		"errno=2('No such file or directory')",
		"errno=35('Resource temporarily unavailable')"
	};

bool UPubnubSubsystem::ShouldCCoreLogBeSkipped(FString Message)
{
	for(FString& LogSkipPhrases : FalseCCoreLogPhrases)
	{
		if(Message.Contains(LogSkipPhrases))
		{
			return true;
		}
	}
	return false;
}

void UPubnubSubsystem::PubnubSDKLogConverter(enum pubnub_log_level log_level, const char* message)
{
	//This is temporal solution to skip false warnings from C-Core.
	//It should be fixed on C-Core level, but until it's done we filter them out here
	if(ShouldCCoreLogBeSkipped(FString(message)))
	{
		return;
	}
	
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

void UPubnubSubsystem::OnCCoreSubscriptionStatusReceived(const pubnub_subscription_status status, const pubnub_subscription_status_data_t status_data)
{
	if(!SubscriptionResultDelegates.IsEmpty())
	{
		FPubnubOperationResult Result;
		Result.Error = status == PNSS_SUBSCRIPTION_STATUS_CONNECTION_ERROR || status == PNSS_SUBSCRIPTION_STATUS_DISCONNECTED_UNEXPECTEDLY;
		Result.Status = Result.Error ? 503 : 200;
		Result.ErrorMessage = pubnub_res_2_string(status_data.reason);
		SubscriptionResultDelegates[0].ExecuteIfBound(Result);

		SubscriptionResultDelegates.RemoveAt(0);
	}

	QuickActionThread->UnlockAfterSubscriptionOperationFinished();
	
	//Don't waste resources to translate data if there is no delegate bound to it
	if(!OnSubscriptionStatusChanged.IsBound() && !OnSubscriptionStatusChangedNative.IsBound())
	{return;}

	FPubnubSubscriptionStatusData SubscriptionStatusData;
	SubscriptionStatusData.Reason = pubnub_res_2_string(status_data.reason);

	//If status is disconnected we don't need to give subscribed channels
	if(status != PNSS_SUBSCRIPTION_STATUS_DISCONNECTED)
	{
		//Fill channels and channel groups data, from C-Core
		if (NULL != status_data.channels)
		{
			FUTF8ToTCHAR Converter(status_data.channels);
			FString Channels(Converter.Length(), Converter.Get());
			Channels.ParseIntoArray(SubscriptionStatusData.Channels, TEXT(","));
		}
		if (NULL != status_data.channel_groups)
		{
			FUTF8ToTCHAR Converter(status_data.channel_groups);
			FString ChannelGroups(Converter.Length(), Converter.Get());
			ChannelGroups.ParseIntoArray(SubscriptionStatusData.ChannelGroups, TEXT(","));
		}
	}
	
	OnSubscriptionStatusChanged.Broadcast((EPubnubSubscriptionStatus)status, SubscriptionStatusData);
	OnSubscriptionStatusChangedNative.Broadcast((EPubnubSubscriptionStatus)status, SubscriptionStatusData);
}
