// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Config/PubnubSettings.h"
#include "Threads/PubnubFunctionThread.h"
#include "PubnubInternalMacros.h"
#include "Crypto/PubnubAesCryptor.h"
#include "FunctionLibraries/PubnubTokenUtilities.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "FunctionLibraries/PubnubUtilities.h"
#include "FunctionLibraries/PubnubInternalUtilities.h"
#include "Entities/PubnubBaseEntity.h"
#include "Entities/PubnubChannelEntity.h"
#include "Entities/PubnubChannelGroupEntity.h"
#include "Entities/PubnubChannelMetadataEntity.h"
#include "Entities/PubnubUserMetadataEntity.h"
#include "Entities/PubnubSubscription.h"
#include "PubnubClient.h"

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

UPubnubClient* UPubnubSubsystem::CreatePubnubClient(FPubnubConfig Config, FString DebugName)
{
	UPubnubClient* PubnubClient = NewObject<UPubnubClient>(this);

	int NewID = NextClientID++;

	PubnubClient->InitWithConfig(this, Config, NewID, DebugName);
	
	PubnubClients.Add(NewID, PubnubClient);

	return PubnubClient;
}

UPubnubClient* UPubnubSubsystem::GetPubnubClient(int ClientID)
{
	return PubnubClients.FindRef(ClientID);
}

bool UPubnubSubsystem::DestroyPubnubClient(UPubnubClient* ClientToDestroy)
{
	if(!ClientToDestroy)
	{return false;}
	
	if(PubnubClients.Find(ClientToDestroy->GetClientID()))
	{
		PubnubClients.Remove(ClientToDestroy->GetClientID());
		ClientToDestroy->DeinitializeClient();
		return true;
	}
	
	return false;
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
	DefaultClient = CreatePubnubClient(Config);
	IsInitialized = true;
}

void UPubnubSubsystem::DeinitPubnub()
{
	if(!IsInitialized)
	{return;}
	
	DefaultClient->DeinitializeClient();
	DefaultClient = nullptr;
	
	IsInitialized = false;

	//Notify that Deinitialization is finished
	OnPubnubSubsystemDeinitialized.Broadcast();
}

void UPubnubSubsystem::SetUserID(FString UserID)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
	DefaultClient->SetUserID(UserID);
}

FString UPubnubSubsystem::GetUserID()
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED("");
	
	return DefaultClient->GetUserID();
}

void UPubnubSubsystem::SetSecretKey()
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();

	DefaultClient->SetSecretKey();
}

void UPubnubSubsystem::PublishMessage(FString Channel, FString Message, FOnPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings)
{
	PUBNUB_ENSURE_INITIALIZED(OnPublishMessageResponse, FPubnubMessageData());
	
	FPubnubOnPublishMessageResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnPublishMessageResponse](const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
	{
		OnPublishMessageResponse.ExecuteIfBound(Result, PublishedMessage);
	});

	DefaultClient->PublishMessage(Channel, Message, ConvertedCallback, PublishSettings);
}

void UPubnubSubsystem::PublishMessage(FString Channel, FString Message, FOnPublishMessageResponseNative NativeCallback, FPubnubPublishSettings PublishSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubMessageData());

	FPubnubOnPublishMessageResponseNative ConvertedCallback;
    ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
    {
        NativeCallback.ExecuteIfBound(Result, PublishedMessage);
    });

	DefaultClient->PublishMessage(Channel, Message, ConvertedCallback, PublishSettings);
}

void UPubnubSubsystem::PublishMessage(FString Channel, FString Message, FPubnubPublishSettings PublishSettings)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
	DefaultClient->PublishMessage(Channel, Message, nullptr, PublishSettings);
}

void UPubnubSubsystem::Signal(FString Channel, FString Message, FOnSignalResponse OnSignalResponse, FPubnubSignalSettings SignalSettings)
{
	PUBNUB_ENSURE_INITIALIZED(OnSignalResponse, FPubnubMessageData());
	
	FPubnubOnSignalResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSignalResponse](const FPubnubOperationResult& Result, const FPubnubMessageData& SignalMessage)
	{
		OnSignalResponse.ExecuteIfBound(Result, SignalMessage);
	});

	DefaultClient->Signal(Channel, Message, ConvertedCallback, SignalSettings);
}

void UPubnubSubsystem::Signal(FString Channel, FString Message, FOnSignalResponseNative NativeCallback, FPubnubSignalSettings SignalSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubMessageData());
	
	FPubnubOnSignalResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const FPubnubMessageData& SignalMessage)
	{
		NativeCallback.ExecuteIfBound(Result, SignalMessage);
	});

	DefaultClient->Signal(Channel, Message, ConvertedCallback, SignalSettings);
}

void UPubnubSubsystem::Signal(FString Channel, FString Message, FPubnubSignalSettings SignalSettings)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
	DefaultClient->Signal(Channel, Message, nullptr, SignalSettings);
}

void UPubnubSubsystem::SubscribeToChannel(FString Channel, FOnSubscribeOperationResponse OnSubscribeToChannelResponse, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_INITIALIZED(OnSubscribeToChannelResponse);
	
	FPubnubOnSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSubscribeToChannelResponse](FPubnubOperationResult Result)
	{
		OnSubscribeToChannelResponse.ExecuteIfBound(Result);
	});

	DefaultClient->SubscribeToChannel(Channel, ConvertedCallback, SubscribeSettings);
}

void UPubnubSubsystem::SubscribeToChannel(FString Channel, FOnSubscribeOperationResponseNative NativeCallback, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FPubnubOnSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](FPubnubOperationResult Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});

	DefaultClient->SubscribeToChannel(Channel, ConvertedCallback, SubscribeSettings);
}

void UPubnubSubsystem::SubscribeToChannel(FString Channel, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
	DefaultClient->SubscribeToChannel(Channel, nullptr, SubscribeSettings);
}

void UPubnubSubsystem::SubscribeToGroup(FString ChannelGroup, FOnSubscribeOperationResponse OnSubscribeToGroupResponse, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_INITIALIZED(OnSubscribeToGroupResponse);
	
	FPubnubOnSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSubscribeToGroupResponse](FPubnubOperationResult Result)
	{
		OnSubscribeToGroupResponse.ExecuteIfBound(Result);
	});

	DefaultClient->SubscribeToGroup(ChannelGroup, ConvertedCallback, SubscribeSettings);
}

void UPubnubSubsystem::SubscribeToGroup(FString ChannelGroup, FOnSubscribeOperationResponseNative NativeCallback, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FPubnubOnSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](FPubnubOperationResult Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});

	DefaultClient->SubscribeToGroup(ChannelGroup, ConvertedCallback, SubscribeSettings);
}

void UPubnubSubsystem::SubscribeToGroup(FString ChannelGroup, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
	DefaultClient->SubscribeToGroup(ChannelGroup, nullptr, SubscribeSettings);
}

void UPubnubSubsystem::UnsubscribeFromChannel(FString Channel, FOnSubscribeOperationResponse OnUnsubscribeFromChannelResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnUnsubscribeFromChannelResponse);
	
	FPubnubOnSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnUnsubscribeFromChannelResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromChannelResponse.ExecuteIfBound(Result);
	});

	DefaultClient->UnsubscribeFromChannel(Channel, ConvertedCallback);
}

void UPubnubSubsystem::UnsubscribeFromChannel(FString Channel, FOnSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FPubnubOnSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](FPubnubOperationResult Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});

	DefaultClient->UnsubscribeFromChannel(Channel, ConvertedCallback);
}

void UPubnubSubsystem::UnsubscribeFromGroup(FString ChannelGroup, FOnSubscribeOperationResponse OnUnsubscribeFromGroupResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnUnsubscribeFromGroupResponse);
	
	FPubnubOnSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnUnsubscribeFromGroupResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromGroupResponse.ExecuteIfBound(Result);
	});

	DefaultClient->UnsubscribeFromGroup(ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::UnsubscribeFromGroup(FString ChannelGroup, FOnSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FPubnubOnSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](FPubnubOperationResult Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});

	DefaultClient->UnsubscribeFromGroup(ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::UnsubscribeFromAll(FOnSubscribeOperationResponse OnUnsubscribeFromAllResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnUnsubscribeFromAllResponse);
	
	FPubnubOnSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnUnsubscribeFromAllResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromAllResponse.ExecuteIfBound(Result);
	});

	DefaultClient->UnsubscribeFromAll(ConvertedCallback);
}

void UPubnubSubsystem::UnsubscribeFromAll(FOnSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FPubnubOnSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](FPubnubOperationResult Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});

	DefaultClient->UnsubscribeFromAll(ConvertedCallback);
}

void UPubnubSubsystem::AddChannelToGroup(FString Channel, FString ChannelGroup, FOnAddChannelToGroupResponse OnAddChannelToGroupResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnAddChannelToGroupResponse);
	
	FPubnubOnAddChannelToGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnAddChannelToGroupResponse](const FPubnubOperationResult& Result)
	{
		OnAddChannelToGroupResponse.ExecuteIfBound(Result);
	});
	
	DefaultClient->AddChannelToGroup(Channel, ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::AddChannelToGroup(FString Channel, FString ChannelGroup, FOnAddChannelToGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FPubnubOnAddChannelToGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});
	
	DefaultClient->AddChannelToGroup(Channel, ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::RemoveChannelFromGroup(FString Channel, FString ChannelGroup, FOnRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnRemoveChannelFromGroupResponse);
	
	FPubnubOnRemoveChannelFromGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnRemoveChannelFromGroupResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveChannelFromGroupResponse.ExecuteIfBound(Result);
	});
	
	DefaultClient->RemoveChannelFromGroup(Channel, ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::RemoveChannelFromGroup(FString Channel, FString ChannelGroup, FOnRemoveChannelFromGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FPubnubOnRemoveChannelFromGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});
	
	DefaultClient->RemoveChannelFromGroup(Channel, ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::ListChannelsFromGroup(FString ChannelGroup, FOnListChannelsFromGroupResponse OnListChannelsResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnListChannelsResponse, TArray<FString>{});
	
	FPubnubOnListChannelsFromGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnListChannelsResponse](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		OnListChannelsResponse.ExecuteIfBound(Result, Channels);
	});

	DefaultClient->ListChannelsFromGroup(ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::ListChannelsFromGroup(FString ChannelGroup, FOnListChannelsFromGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FString>{});
	
	FPubnubOnListChannelsFromGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		NativeCallback.ExecuteIfBound(Result, Channels);
	});
	
	DefaultClient->ListChannelsFromGroup(ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::RemoveChannelGroup(FString ChannelGroup, FOnRemoveChannelGroupResponse OnRemoveChannelGroupResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnRemoveChannelGroupResponse);
	
	FPubnubOnRemoveChannelGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnRemoveChannelGroupResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveChannelGroupResponse.ExecuteIfBound(Result);
	});
	
	DefaultClient->RemoveChannelGroup(ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::RemoveChannelGroup(FString ChannelGroup, FOnRemoveChannelGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FPubnubOnRemoveChannelGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});
	
	DefaultClient->RemoveChannelGroup(ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::ListUsersFromChannel(FString Channel, FOnListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	PUBNUB_ENSURE_INITIALIZED(ListUsersFromChannelResponse, FPubnubListUsersFromChannelWrapper());
	
	FPubnubOnListUsersFromChannelResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([ListUsersFromChannelResponse](const FPubnubOperationResult& Result, FPubnubListUsersFromChannelWrapper Data)
	{
		ListUsersFromChannelResponse.ExecuteIfBound(Result, Data);
	});

	DefaultClient->ListUsersFromChannel(Channel, ConvertedCallback, ListUsersFromChannelSettings);
}

void UPubnubSubsystem::ListUsersFromChannel(FString Channel, FOnListUsersFromChannelResponseNative NativeCallback, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubListUsersFromChannelWrapper());
	
	FPubnubOnListUsersFromChannelResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, FPubnubListUsersFromChannelWrapper Data)
	{
		NativeCallback.ExecuteIfBound(Result, Data);
	});
	
	DefaultClient->ListUsersFromChannel(Channel, ConvertedCallback, ListUsersFromChannelSettings);
}

void UPubnubSubsystem::ListUserSubscribedChannels(FString UserID, FOnListUsersSubscribedChannelsResponse ListUserSubscribedChannelsResponse)
{
	PUBNUB_ENSURE_INITIALIZED(ListUserSubscribedChannelsResponse, TArray<FString>{});
	
	FPubnubOnListUsersSubscribedChannelsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([ListUserSubscribedChannelsResponse](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		ListUserSubscribedChannelsResponse.ExecuteIfBound(Result, Channels);
	});

	DefaultClient->ListUserSubscribedChannels(UserID, ConvertedCallback);
}

void UPubnubSubsystem::ListUserSubscribedChannels(FString UserID, FOnListUsersSubscribedChannelsResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FString>{});
	
	FPubnubOnListUsersSubscribedChannelsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		NativeCallback.ExecuteIfBound(Result, Channels);
	});
	
	DefaultClient->ListUserSubscribedChannels(UserID, ConvertedCallback);
}

void UPubnubSubsystem::SetState(FString Channel, FString StateJson, FOnSetStateResponse OnSetStateResponse, FPubnubSetStateSettings SetStateSettings)
{
	PUBNUB_ENSURE_INITIALIZED(OnSetStateResponse);
	
	FPubnubOnSetStateResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSetStateResponse](const FPubnubOperationResult& Result)
	{
		OnSetStateResponse.ExecuteIfBound(Result);
	});
	
	DefaultClient->SetState(Channel, StateJson, ConvertedCallback, SetStateSettings);
}

void UPubnubSubsystem::SetState(FString Channel, FString StateJson, FOnSetStateResponseNative NativeCallback, FPubnubSetStateSettings SetStateSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FPubnubOnSetStateResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});
	
	DefaultClient->SetState(Channel, StateJson, ConvertedCallback, SetStateSettings);
}

void UPubnubSubsystem::SetState(FString Channel, FString StateJson, FPubnubSetStateSettings SetStateSettings)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
	DefaultClient->SetState(Channel, StateJson, nullptr, SetStateSettings);
}

void UPubnubSubsystem::GetState(FString Channel, FString ChannelGroup, FString UserID, FOnGetStateResponse OnGetStateResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnGetStateResponse, FString());
	
	FPubnubOnGetStateResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGetStateResponse](const FPubnubOperationResult& Result, FString JsonResponse)
	{
		OnGetStateResponse.ExecuteIfBound(Result, JsonResponse);
	});

	DefaultClient->GetState(Channel, ChannelGroup, UserID, ConvertedCallback);
}

void UPubnubSubsystem::GetState(FString Channel, FString ChannelGroup, FString UserID, FOnGetStateResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FString());
	
	FPubnubOnGetStateResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, FString JsonResponse)
	{
		NativeCallback.ExecuteIfBound(Result, JsonResponse);
	});
	
	DefaultClient->GetState(Channel, ChannelGroup, UserID, ConvertedCallback);
}

void UPubnubSubsystem::Heartbeat(FString Channel, FString ChannelGroup)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
	DefaultClient->Heartbeat(Channel, ChannelGroup);
}

void UPubnubSubsystem::GrantToken(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FOnGrantTokenResponse OnGrantTokenResponse, FString Meta)
{
	PUBNUB_ENSURE_INITIALIZED(OnGrantTokenResponse, FString());

	FPubnubOnGrantTokenResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGrantTokenResponse](const FPubnubOperationResult& Result, FString Token)
	{
		OnGrantTokenResponse.ExecuteIfBound(Result, Token);
	});

	DefaultClient->GrantToken(Ttl, AuthorizedUser, Permissions, ConvertedCallback, Meta);
}

void UPubnubSubsystem::GrantToken(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FOnGrantTokenResponseNative NativeCallback, FString Meta)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FString());
	
	FPubnubOnGrantTokenResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, FString Token)
	{
		NativeCallback.ExecuteIfBound(Result, Token);
	});
	
	DefaultClient->GrantToken(Ttl, AuthorizedUser, Permissions, ConvertedCallback, Meta);
}

void UPubnubSubsystem::RevokeToken(FString Token, FOnRevokeTokenResponse OnRevokeTokenResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnRevokeTokenResponse);
	
	FPubnubOnRevokeTokenResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnRevokeTokenResponse](const FPubnubOperationResult& Result)
	{
		OnRevokeTokenResponse.ExecuteIfBound(Result);
	});
	
	DefaultClient->RevokeToken(Token, ConvertedCallback);
}

void UPubnubSubsystem::RevokeToken(FString Token, FOnRevokeTokenResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FPubnubOnRevokeTokenResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});
	
	DefaultClient->RevokeToken(Token, ConvertedCallback);
}

FString UPubnubSubsystem::ParseToken(FString Token)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED("");

	return DefaultClient->ParseToken(Token);
}

void UPubnubSubsystem::SetAuthToken(FString Token)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();

	DefaultClient->SetAuthToken(Token);
}

void UPubnubSubsystem::FetchHistory(FString Channel, FOnFetchHistoryResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings)
{
	PUBNUB_ENSURE_INITIALIZED(OnFetchHistoryResponse, TArray<FPubnubHistoryMessageData>());

	FPubnubOnFetchHistoryResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnFetchHistoryResponse](const FPubnubOperationResult& Result, const TArray<FPubnubHistoryMessageData>& Messages)
	{
		OnFetchHistoryResponse.ExecuteIfBound(Result, Messages);
	});

	DefaultClient->FetchHistory(Channel, ConvertedCallback, FetchHistorySettings);
}

void UPubnubSubsystem::FetchHistory(FString Channel, FOnFetchHistoryResponseNative NativeCallback, FPubnubFetchHistorySettings FetchHistorySettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubHistoryMessageData>());
	
	FPubnubOnFetchHistoryResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const TArray<FPubnubHistoryMessageData>& Messages)
	{
		NativeCallback.ExecuteIfBound(Result, Messages);
	});
	
	DefaultClient->FetchHistory(Channel, ConvertedCallback, FetchHistorySettings);
}

void UPubnubSubsystem::DeleteMessages(FString Channel, FOnDeleteMessagesResponse OnDeleteMessagesResponse, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	PUBNUB_ENSURE_INITIALIZED(OnDeleteMessagesResponse);

	FPubnubOnDeleteMessagesResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnDeleteMessagesResponse](FPubnubOperationResult Result)
	{
		OnDeleteMessagesResponse.ExecuteIfBound(Result);
	});

	DefaultClient->DeleteMessages(Channel, ConvertedCallback, DeleteMessagesSettings);
}

void UPubnubSubsystem::DeleteMessages(FString Channel, FOnDeleteMessagesResponseNative NativeCallback, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FPubnubOnDeleteMessagesResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](FPubnubOperationResult Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});
	
	DefaultClient->DeleteMessages(Channel, ConvertedCallback, DeleteMessagesSettings);
}

void UPubnubSubsystem::DeleteMessages(FString Channel, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
	DefaultClient->DeleteMessages(Channel, DeleteMessagesSettings);
}

void UPubnubSubsystem::MessageCounts(FString Channel, FString Timetoken, FOnMessageCountsResponse OnMessageCountsResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnMessageCountsResponse, 0);

	FPubnubOnMessageCountsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnMessageCountsResponse](const FPubnubOperationResult& Result, int MessageCounts)
	{
		OnMessageCountsResponse.ExecuteIfBound(Result, MessageCounts);
	});

	DefaultClient->MessageCounts(Channel, Timetoken, ConvertedCallback);
}

void UPubnubSubsystem::MessageCounts(FString Channel, FString Timetoken, FOnMessageCountsResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, 0);
	
	FPubnubOnMessageCountsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, int MessageCounts)
	{
		NativeCallback.ExecuteIfBound(Result, MessageCounts);
	});
	
	DefaultClient->MessageCounts(Channel, Timetoken, ConvertedCallback);
}

void UPubnubSubsystem::GetAllUserMetadataRaw(FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(OnGetAllUserMetadataResponse, TArray<FPubnubUserData>(), FString(), FString());

	FPubnubOnGetAllUserMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGetAllUserMetadataResponse](const FPubnubOperationResult& Result, const TArray<FPubnubUserData>& UsersData, FString PageNext, FString PagePrev)
	{
		OnGetAllUserMetadataResponse.ExecuteIfBound(Result, UsersData, PageNext, PagePrev);
	});

	DefaultClient->GetAllUserMetadataRaw(ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::GetAllUserMetadataRaw(FOnGetAllUserMetadataResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubUserData>(), FString(), FString());
	
	FPubnubOnGetAllUserMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const TArray<FPubnubUserData>& UsersData, FString PageNext, FString PagePrev)
	{
		NativeCallback.ExecuteIfBound(Result, UsersData, PageNext, PagePrev);
	});
	
	DefaultClient->GetAllUserMetadataRaw(ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
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
	PUBNUB_ENSURE_INITIALIZED(OnSetUserMetadataResponse, FPubnubUserData());

	FPubnubOnSetUserMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSetUserMetadataResponse](const FPubnubOperationResult& Result, FPubnubUserData UserData)
	{
		OnSetUserMetadataResponse.ExecuteIfBound(Result, UserData);
	});

	DefaultClient->SetUserMetadataRaw(User, UserMetadataObj, ConvertedCallback, Include);
}

void UPubnubSubsystem::SetUserMetadataRaw(FString User, FString UserMetadataObj, FOnSetUserMetadataResponseNative NativeCallback, FString Include)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubUserData());
	
	FPubnubOnSetUserMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, FPubnubUserData UserData)
	{
		NativeCallback.ExecuteIfBound(Result, UserData);
	});
	
	DefaultClient->SetUserMetadataRaw(User, UserMetadataObj, ConvertedCallback, Include);
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
	PUBNUB_ENSURE_INITIALIZED(OnGetUserMetadataResponse, FPubnubUserData());

	FPubnubOnGetUserMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGetUserMetadataResponse](const FPubnubOperationResult& Result, FPubnubUserData UserData)
	{
		OnGetUserMetadataResponse.ExecuteIfBound(Result, UserData);
	});

	DefaultClient->GetUserMetadataRaw(User, ConvertedCallback, Include);
}

void UPubnubSubsystem::GetUserMetadataRaw(FString User, FOnGetUserMetadataResponseNative NativeCallback, FString Include)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubUserData());
	
	FPubnubOnGetUserMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, FPubnubUserData UserData)
	{
		NativeCallback.ExecuteIfBound(Result, UserData);
	});
	
	DefaultClient->GetUserMetadataRaw(User, ConvertedCallback, Include);
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
	PUBNUB_ENSURE_INITIALIZED(OnRemoveUserMetadataResponse);

	FPubnubOnRemoveUserMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnRemoveUserMetadataResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveUserMetadataResponse.ExecuteIfBound(Result);
	});

	DefaultClient->RemoveUserMetadata(User, ConvertedCallback);
}

void UPubnubSubsystem::RemoveUserMetadata(FString User, FOnRemoveUserMetadataResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FPubnubOnRemoveUserMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});
	
	DefaultClient->RemoveUserMetadata(User, ConvertedCallback);
}

void UPubnubSubsystem::GetAllChannelMetadataRaw(FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(OnGetAllChannelMetadataResponse, TArray<FPubnubChannelData>(), FString(), FString());

	FPubnubOnGetAllChannelMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGetAllChannelMetadataResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelData>& ChannelsData, FString PageNext, FString PagePrev)
	{
		OnGetAllChannelMetadataResponse.ExecuteIfBound(Result, ChannelsData, PageNext, PagePrev);
	});

	DefaultClient->GetAllChannelMetadataRaw(ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::GetAllChannelMetadataRaw(FOnGetAllChannelMetadataResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubChannelData>(), FString(), FString());
	
	FPubnubOnGetAllChannelMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const TArray<FPubnubChannelData>& ChannelsData, FString PageNext, FString PagePrev)
	{
		NativeCallback.ExecuteIfBound(Result, ChannelsData, PageNext, PagePrev);
	});
	
	DefaultClient->GetAllChannelMetadataRaw(ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
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
	PUBNUB_ENSURE_INITIALIZED(OnSetChannelMetadataResponse, FPubnubChannelData());

	FPubnubOnSetChannelMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSetChannelMetadataResponse](const FPubnubOperationResult& Result, FPubnubChannelData ChannelData)
	{
		OnSetChannelMetadataResponse.ExecuteIfBound(Result, ChannelData);
	});

	DefaultClient->SetChannelMetadataRaw(Channel, ChannelMetadataObj, ConvertedCallback, Include);
}

void UPubnubSubsystem::SetChannelMetadataRaw(FString Channel, FString ChannelMetadataObj, FOnSetChannelMetadataResponseNative NativeCallback, FString Include)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubChannelData());
	
	FPubnubOnSetChannelMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, FPubnubChannelData ChannelData)
	{
		NativeCallback.ExecuteIfBound(Result, ChannelData);
	});
	
	DefaultClient->SetChannelMetadataRaw(Channel, ChannelMetadataObj, ConvertedCallback, Include);
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
	PUBNUB_ENSURE_INITIALIZED(OnGetChannelMetadataResponse, FPubnubChannelData());

	FPubnubOnGetChannelMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGetChannelMetadataResponse](const FPubnubOperationResult& Result, FPubnubChannelData ChannelData)
	{
		OnGetChannelMetadataResponse.ExecuteIfBound(Result, ChannelData);
	});

	DefaultClient->GetChannelMetadataRaw(Channel, ConvertedCallback, Include);
}

void UPubnubSubsystem::GetChannelMetadataRaw(FString Channel, FOnGetChannelMetadataResponseNative NativeCallback, FString Include)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubChannelData());

	FPubnubOnGetChannelMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, FPubnubChannelData ChannelData)
	{
		NativeCallback.ExecuteIfBound(Result, ChannelData);
	});

	DefaultClient->GetChannelMetadataRaw(Channel, ConvertedCallback, Include);
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
	PUBNUB_ENSURE_INITIALIZED(OnRemoveChannelMetadataResponse);

	FPubnubOnRemoveChannelMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnRemoveChannelMetadataResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveChannelMetadataResponse.ExecuteIfBound(Result);
	});

	DefaultClient->RemoveChannelMetadata(Channel, ConvertedCallback);
}

void UPubnubSubsystem::RemoveChannelMetadata(FString Channel, FOnRemoveChannelMetadataResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FPubnubOnRemoveChannelMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});
	
	DefaultClient->RemoveChannelMetadata(Channel, ConvertedCallback);
}

void UPubnubSubsystem::GetMembershipsRaw(FString User, FOnGetMembershipsResponse OnGetMembershipsResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(OnGetMembershipsResponse, TArray<FPubnubMembershipData>(), FString(), FString());

	FPubnubOnGetMembershipsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGetMembershipsResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		OnGetMembershipsResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});

	DefaultClient->GetMembershipsRaw(User, ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::GetMembershipsRaw(FString User, FOnGetMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FString(), FString());
	
	FPubnubOnGetMembershipsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		NativeCallback.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});
	
	DefaultClient->GetMembershipsRaw(User, ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
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
	PUBNUB_ENSURE_INITIALIZED(OnSetMembershipResponse, TArray<FPubnubMembershipData>(), FString(), FString());

	FPubnubOnSetMembershipsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSetMembershipResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		OnSetMembershipResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});

	DefaultClient->SetMembershipsRaw(User, SetObj, ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::SetMembershipsRaw(FString User, FString SetObj, FOnSetMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FString(), FString());
	
	FPubnubOnSetMembershipsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		NativeCallback.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});
	
	DefaultClient->SetMembershipsRaw(User, SetObj, ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
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
	PUBNUB_ENSURE_INITIALIZED(OnRemoveMembershipResponse, TArray<FPubnubMembershipData>(), FString(), FString());

	FPubnubOnRemoveMembershipsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnRemoveMembershipResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		OnRemoveMembershipResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});

	DefaultClient->RemoveMembershipsRaw(User, RemoveObj, ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::RemoveMembershipsRaw(FString User, FString RemoveObj, FOnRemoveMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FString(), FString());
	
	FPubnubOnRemoveMembershipsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		NativeCallback.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});
	
	DefaultClient->RemoveMembershipsRaw(User, RemoveObj, ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
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
	PUBNUB_ENSURE_INITIALIZED(OnGetMembersResponse, TArray<FPubnubChannelMemberData>(), FString(), FString());

	FPubnubOnGetChannelMembersResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGetMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
	{
		OnGetMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
	});

	DefaultClient->GetChannelMembersRaw(Channel, ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::GetChannelMembersRaw(FString Channel, FOnGetChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubChannelMemberData>(), FString(), FString());

	FPubnubOnGetChannelMembersResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
	{
		NativeCallback.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
	});

	DefaultClient->GetChannelMembersRaw(Channel, ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
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
	PUBNUB_ENSURE_INITIALIZED(OnSetMembersResponse, TArray<FPubnubChannelMemberData>(), FString(), FString());

	FPubnubOnSetChannelMembersResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSetMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
	{
		OnSetMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
	});

	DefaultClient->SetChannelMembersRaw(Channel, SetObj, ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::SetChannelMembersRaw(FString Channel, FString SetObj, FOnSetChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubChannelMemberData>(), FString(), FString());

	FPubnubOnSetChannelMembersResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
	{
		NativeCallback.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
	});

	DefaultClient->SetChannelMembersRaw(Channel, SetObj, ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
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
	PUBNUB_ENSURE_INITIALIZED(OnRemoveMembersResponse, TArray<FPubnubChannelMemberData>(), FString(), FString());

	FPubnubOnRemoveChannelMembersResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnRemoveMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
	{
		OnRemoveMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
	});

	DefaultClient->RemoveChannelMembersRaw(Channel, RemoveObj, ConvertedCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::RemoveChannelMembersRaw(FString Channel, FString RemoveObj, FOnRemoveChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubChannelMemberData>(), FString(), FString());

	FPubnubOnRemoveChannelMembersResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
	{
		NativeCallback.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
	});

	DefaultClient->RemoveChannelMembersRaw(Channel, RemoveObj, ConvertedCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);
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

UPubnubChannelEntity* UPubnubSubsystem::CreateChannelEntity(FString Channel)
{
	PUBNUB_RETURN_IF_FIELD_EMPTY(Channel, nullptr);
	
	UPubnubChannelEntity* ChannelEntity = NewObject<UPubnubChannelEntity>(this);
	ChannelEntity->InitEntity(this);
	ChannelEntity->EntityID = Channel;
	return ChannelEntity;
}

UPubnubChannelGroupEntity* UPubnubSubsystem::CreateChannelGroupEntity(FString ChannelGroup)
{
	PUBNUB_RETURN_IF_FIELD_EMPTY(ChannelGroup, nullptr);
	
	UPubnubChannelGroupEntity* ChannelGroupEntity = NewObject<UPubnubChannelGroupEntity>(this);
	ChannelGroupEntity->InitEntity(this);
	ChannelGroupEntity->EntityID = ChannelGroup;
	return ChannelGroupEntity;
}

UPubnubChannelMetadataEntity* UPubnubSubsystem::CreateChannelMetadataEntity(FString Channel)
{
	PUBNUB_RETURN_IF_FIELD_EMPTY(Channel, nullptr);
	
	UPubnubChannelMetadataEntity* ChannelMetadataEntity = NewObject<UPubnubChannelMetadataEntity>(this);
	ChannelMetadataEntity->InitEntity(this);
	ChannelMetadataEntity->EntityID = Channel;
	return ChannelMetadataEntity;
}

UPubnubUserMetadataEntity* UPubnubSubsystem::CreateUserMetadataEntity(FString User)
{
	PUBNUB_RETURN_IF_FIELD_EMPTY(User, nullptr);
	
	UPubnubUserMetadataEntity* UserMetadataEntity = NewObject<UPubnubUserMetadataEntity>(this);
	UserMetadataEntity->InitEntity(this);
	UserMetadataEntity->EntityID = User;
	return UserMetadataEntity;
}

UPubnubSubscriptionSet* UPubnubSubsystem::CreateSubscriptionSet(TArray<FString> Channels, TArray<FString> ChannelGroups, FPubnubSubscribeSettings SubscriptionSettings)
{
	if(Channels.IsEmpty() && ChannelGroups.IsEmpty())
	{
		PubnubError("[CreateSubscriptionSet]: at least one Channel or ChannelGroup is needed to create SubscriptionSet.", EPubnubErrorType::PET_Warning);
	}
	UPubnubSubscriptionSet* SubscriptionSet = NewObject<UPubnubSubscriptionSet>(this);
	SubscriptionSet->InitSubscriptionSet(this, Channels, ChannelGroups, SubscriptionSettings);
	return SubscriptionSet;
}

UPubnubSubscriptionSet* UPubnubSubsystem::CreateSubscriptionSetFromEntities(TArray<UPubnubBaseEntity*> Entities, FPubnubSubscribeSettings SubscriptionSettings)
{
	if(Entities.IsEmpty())
	{
		PubnubError("[CreateSubscriptionSetFromEntities]: at least one Entity is needed to create SubscriptionSet.", EPubnubErrorType::PET_Warning);
	}
	TArray<FString> Channels, ChannelGroups;

	//Group up entities for those that subscribe to Channel and ChannelGroup
	for(auto Entity : Entities)
	{
		Entity->EntityType == EPubnubEntityType::PEnT_ChannelGroup? ChannelGroups.Add(Entity->EntityID) : Channels.Add(Entity->EntityID);
	}
	
	UPubnubSubscriptionSet* SubscriptionSet = NewObject<UPubnubSubscriptionSet>(this);
	SubscriptionSet->InitSubscriptionSet(this, Channels, ChannelGroups, SubscriptionSettings);
	return SubscriptionSet;
}

TArray<UPubnubSubscription*> UPubnubSubsystem::GetActiveSubscriptions()
{
	size_t Count;
	pubnub_subscription** CCoreSubs =  pubnub_subscriptions(ctx_ee, &Count);
	if (!CCoreSubs || Count == 0) {
		return {};
	}

	//Free CCoreSubs when the function ends
	ON_SCOPE_EXIT { free(CCoreSubs); };

	TArray<UPubnubSubscription*> Subscriptions;

	for(pubnub_subscription_t* CCoreSub : MakeArrayView(CCoreSubs, Count))
	{
		UPubnubSubscription* Subscription = NewObject<UPubnubSubscription>(this);
		Subscription->InitWithCCoreSubscription(this, CCoreSub);
		Subscriptions.Add(Subscription);
	}

	return Subscriptions;
}

TArray<UPubnubSubscriptionSet*> UPubnubSubsystem::GetActiveSubscriptionSets()
{
	size_t Count;
	pubnub_subscription_set** CCoreSubSets =  pubnub_subscription_sets(ctx_ee, &Count);
	if (!CCoreSubSets || Count == 0) {
		return {};
	}

	//Free CCoreSubs when the function ends
	ON_SCOPE_EXIT { free(CCoreSubSets); };

	TArray<UPubnubSubscriptionSet*> SubscriptionSets;

	for(pubnub_subscription_set_t* CCoreSubsSet : MakeArrayView(CCoreSubSets, Count))
	{
		UPubnubSubscriptionSet* SubscriptionSet = NewObject<UPubnubSubscriptionSet>(this);
		SubscriptionSet->InitWithCCoreSubscriptionSet(this, CCoreSubsSet);
		SubscriptionSets.Add(SubscriptionSet);
		
		size_t SubsCount;

		pubnub_subscription** CCoreSubs = pubnub_subscription_set_subscriptions(CCoreSubsSet, &SubsCount);
		if (!CCoreSubs || Count == 0) {
			continue;
		}

		//Free CCoreSubs when the function ends
		ON_SCOPE_EXIT { free(CCoreSubs); };

		for(pubnub_subscription_t* CCoreSub : MakeArrayView(CCoreSubs, Count))
		{
			UPubnubSubscription* Subscription = NewObject<UPubnubSubscription>(this);
			Subscription->InitWithCCoreSubscription(this, CCoreSub);
			SubscriptionSet->Subscriptions.Add(Subscription);
		}
	}

	return SubscriptionSets;
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
	
	// Safely copy all keys using the utility function
	UPubnubUtilities::SafeCopyFStringToCharBuffer(PublishKey, PublishKeySize + 1, Config.PublishKey, TEXT("PublishKey"));
	UPubnubUtilities::SafeCopyFStringToCharBuffer(SubscribeKey, PublishKeySize + 1, Config.SubscribeKey, TEXT("SubscribeKey"));
	UPubnubUtilities::SafeCopyFStringToCharBuffer(SecretKey, SecretKeySize + 1, Config.SecretKey, TEXT("SecretKey"));
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

/* PRIV FUNCTIONS */


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

void UPubnubSubsystem::SubscribeWithSubscription(UPubnubSubscription* Subscription, FPubnubSubscriptionCursor Cursor, FOnSubscribeOperationResponseNative OnSubscribeResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnSubscribeResponse);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(OnSubscribeResponse);

	QuickActionThread->AddFunctionToQueue( [this, Subscription, Cursor, OnSubscribeResponse]
	{
		if(!UPubnubInternalUtilities::EESubscribeWithSubscription(Subscription->CCoreSubscription, Cursor))
		{
			PubnubError("[SubscribeWithSubscription]: Failed to subscribe with subscription..");
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSubscribeResponse, "[Subscribe]: Failed to subscribe with Subscription.");
			QuickActionThread->UnlockAfterSubscriptionOperationFinished();
		}
	});
}

void UPubnubSubsystem::SubscribeWithSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet, FPubnubSubscriptionCursor Cursor, FOnSubscribeOperationResponseNative OnSubscribeResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnSubscribeResponse);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(OnSubscribeResponse);

	QuickActionThread->AddFunctionToQueue( [this, SubscriptionSet, Cursor, OnSubscribeResponse]
	{
		if(!UPubnubInternalUtilities::EESubscribeWithSubscriptionSet(SubscriptionSet->CCoreSubscriptionSet, Cursor))
		{
			PubnubError("[SubscribeWithSubscription]: Failed to subscribe with subscription..");
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSubscribeResponse, "[Subscribe]: Failed to subscribe with Subscription.");
			QuickActionThread->UnlockAfterSubscriptionOperationFinished();
		}
	});
}

void UPubnubSubsystem::UnsubscribeWithSubscription(UPubnubSubscription* Subscription, FOnSubscribeOperationResponseNative OnUnsubscribeResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnUnsubscribeResponse);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(OnUnsubscribeResponse);

	QuickActionThread->AddFunctionToQueue( [this, Subscription, OnUnsubscribeResponse]
	{
		if(!UPubnubInternalUtilities::EEUnsubscribeWithSubscription(&Subscription->CCoreSubscription))
		{
			PubnubError("[SubscribeWithSubscription]: Failed to subscribe with subscription..");
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnUnsubscribeResponse, "[Subscribe]: Failed to subscribe with Subscription.");
			QuickActionThread->UnlockAfterSubscriptionOperationFinished();
		}
	});
}

void UPubnubSubsystem::UnsubscribeWithSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet, FOnSubscribeOperationResponseNative OnUnsubscribeResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnUnsubscribeResponse);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(OnUnsubscribeResponse);

	QuickActionThread->AddFunctionToQueue( [this, SubscriptionSet, OnUnsubscribeResponse]
	{
		if(!UPubnubInternalUtilities::EEUnsubscribeWithSubscriptionSet(&SubscriptionSet->CCoreSubscriptionSet))
		{
			PubnubError("[SubscribeWithSubscription]: Failed to subscribe with subscription..");
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnUnsubscribeResponse, "[Subscribe]: Failed to subscribe with Subscription.");
			QuickActionThread->UnlockAfterSubscriptionOperationFinished();
		}
	});
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
