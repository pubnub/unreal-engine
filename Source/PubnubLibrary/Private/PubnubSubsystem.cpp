// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"

#include "Config/PubnubSettings.h"
#include "PubnubInternalMacros.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "FunctionLibraries/PubnubUtilities.h"
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
		UE_LOG(PubnubLog, Warning, TEXT("Pubnub is already initialized. Disable InitializeAutomatically in Pubnub SDK Project Settings to be able to Init Pubnub manually"));
		return;
	}
	
	DefaultClient = CreatePubnubClient(Config);

	//Bind delegates to the default client, so subsystem can also call them
	TWeakObjectPtr<UPubnubSubsystem> WeakThis = MakeWeakObjectPtr(this);
	DefaultClient->OnPubnubMessageReceivedNative.AddLambda([WeakThis](FPubnubMessageData MessageData)
	{
		if(WeakThis.IsValid())
		{
			WeakThis.Get()->OnMessageReceived.Broadcast(MessageData);
			WeakThis.Get()->OnMessageReceivedNative.Broadcast(MessageData);
		}

	});

	DefaultClient->OnPubnubErrorNative.AddLambda([WeakThis](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		if(WeakThis.IsValid())
		{
			WeakThis.Get()->OnPubnubError.Broadcast(ErrorMessage, ErrorType);
			WeakThis.Get()->OnPubnubErrorNative.Broadcast(ErrorMessage, ErrorType);
		}

	});

	DefaultClient->OnPubnubSubscriptionStatusChangedNative.AddLambda([WeakThis](EPubnubSubscriptionStatus Status, FPubnubSubscriptionStatusData StatusData)
	{
		if(WeakThis.IsValid())
		{
			WeakThis.Get()->OnSubscriptionStatusChanged.Broadcast(Status, StatusData);
			WeakThis.Get()->OnSubscriptionStatusChangedNative.Broadcast(Status, StatusData);
		}

	});
	
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
	
	FOnPubnubPublishMessageResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnPublishMessageResponse](const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
	{
		OnPublishMessageResponse.ExecuteIfBound(Result, PublishedMessage);
	});

	DefaultClient->PublishMessage(Channel, Message, ConvertedCallback, PublishSettings);
}

void UPubnubSubsystem::PublishMessage(FString Channel, FString Message, FOnPublishMessageResponseNative NativeCallback, FPubnubPublishSettings PublishSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubMessageData());

	FOnPubnubPublishMessageResponseNative ConvertedCallback;
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
	
	FOnPubnubSignalResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSignalResponse](const FPubnubOperationResult& Result, const FPubnubMessageData& SignalMessage)
	{
		OnSignalResponse.ExecuteIfBound(Result, SignalMessage);
	});

	DefaultClient->Signal(Channel, Message, ConvertedCallback, SignalSettings);
}

void UPubnubSubsystem::Signal(FString Channel, FString Message, FOnSignalResponseNative NativeCallback, FPubnubSignalSettings SignalSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubMessageData());
	
	FOnPubnubSignalResponseNative ConvertedCallback;
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
	
	FOnPubnubSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSubscribeToChannelResponse](FPubnubOperationResult Result)
	{
		OnSubscribeToChannelResponse.ExecuteIfBound(Result);
	});

	DefaultClient->SubscribeToChannel(Channel, ConvertedCallback, SubscribeSettings);
}

void UPubnubSubsystem::SubscribeToChannel(FString Channel, FOnSubscribeOperationResponseNative NativeCallback, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FOnPubnubSubscribeOperationResponseNative ConvertedCallback;
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
	
	FOnPubnubSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSubscribeToGroupResponse](FPubnubOperationResult Result)
	{
		OnSubscribeToGroupResponse.ExecuteIfBound(Result);
	});

	DefaultClient->SubscribeToGroup(ChannelGroup, ConvertedCallback, SubscribeSettings);
}

void UPubnubSubsystem::SubscribeToGroup(FString ChannelGroup, FOnSubscribeOperationResponseNative NativeCallback, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FOnPubnubSubscribeOperationResponseNative ConvertedCallback;
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
	
	FOnPubnubSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnUnsubscribeFromChannelResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromChannelResponse.ExecuteIfBound(Result);
	});

	DefaultClient->UnsubscribeFromChannel(Channel, ConvertedCallback);
}

void UPubnubSubsystem::UnsubscribeFromChannel(FString Channel, FOnSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FOnPubnubSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](FPubnubOperationResult Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});

	DefaultClient->UnsubscribeFromChannel(Channel, ConvertedCallback);
}

void UPubnubSubsystem::UnsubscribeFromGroup(FString ChannelGroup, FOnSubscribeOperationResponse OnUnsubscribeFromGroupResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnUnsubscribeFromGroupResponse);
	
	FOnPubnubSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnUnsubscribeFromGroupResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromGroupResponse.ExecuteIfBound(Result);
	});

	DefaultClient->UnsubscribeFromGroup(ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::UnsubscribeFromGroup(FString ChannelGroup, FOnSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FOnPubnubSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](FPubnubOperationResult Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});

	DefaultClient->UnsubscribeFromGroup(ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::UnsubscribeFromAll(FOnSubscribeOperationResponse OnUnsubscribeFromAllResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnUnsubscribeFromAllResponse);
	
	FOnPubnubSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnUnsubscribeFromAllResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromAllResponse.ExecuteIfBound(Result);
	});

	DefaultClient->UnsubscribeFromAll(ConvertedCallback);
}

void UPubnubSubsystem::UnsubscribeFromAll(FOnSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FOnPubnubSubscribeOperationResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](FPubnubOperationResult Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});

	DefaultClient->UnsubscribeFromAll(ConvertedCallback);
}

void UPubnubSubsystem::AddChannelToGroup(FString Channel, FString ChannelGroup, FOnAddChannelToGroupResponse OnAddChannelToGroupResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnAddChannelToGroupResponse);
	
	FOnPubnubAddChannelToGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnAddChannelToGroupResponse](const FPubnubOperationResult& Result)
	{
		OnAddChannelToGroupResponse.ExecuteIfBound(Result);
	});
	
	DefaultClient->AddChannelToGroup(Channel, ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::AddChannelToGroup(FString Channel, FString ChannelGroup, FOnAddChannelToGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FOnPubnubAddChannelToGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});
	
	DefaultClient->AddChannelToGroup(Channel, ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::RemoveChannelFromGroup(FString Channel, FString ChannelGroup, FOnRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnRemoveChannelFromGroupResponse);
	
	FOnPubnubRemoveChannelFromGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnRemoveChannelFromGroupResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveChannelFromGroupResponse.ExecuteIfBound(Result);
	});
	
	DefaultClient->RemoveChannelFromGroup(Channel, ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::RemoveChannelFromGroup(FString Channel, FString ChannelGroup, FOnRemoveChannelFromGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FOnPubnubRemoveChannelFromGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});
	
	DefaultClient->RemoveChannelFromGroup(Channel, ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::ListChannelsFromGroup(FString ChannelGroup, FOnListChannelsFromGroupResponse OnListChannelsResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnListChannelsResponse, TArray<FString>{});
	
	FOnPubnubListChannelsFromGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnListChannelsResponse](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		OnListChannelsResponse.ExecuteIfBound(Result, Channels);
	});

	DefaultClient->ListChannelsFromGroup(ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::ListChannelsFromGroup(FString ChannelGroup, FOnListChannelsFromGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FString>{});
	
	FOnPubnubListChannelsFromGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		NativeCallback.ExecuteIfBound(Result, Channels);
	});
	
	DefaultClient->ListChannelsFromGroup(ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::RemoveChannelGroup(FString ChannelGroup, FOnRemoveChannelGroupResponse OnRemoveChannelGroupResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnRemoveChannelGroupResponse);
	
	FOnPubnubRemoveChannelGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnRemoveChannelGroupResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveChannelGroupResponse.ExecuteIfBound(Result);
	});
	
	DefaultClient->RemoveChannelGroup(ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::RemoveChannelGroup(FString ChannelGroup, FOnRemoveChannelGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FOnPubnubRemoveChannelGroupResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});
	
	DefaultClient->RemoveChannelGroup(ChannelGroup, ConvertedCallback);
}

void UPubnubSubsystem::ListUsersFromChannel(FString Channel, FOnListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	PUBNUB_ENSURE_INITIALIZED(ListUsersFromChannelResponse, FPubnubListUsersFromChannelWrapper());
	
	FOnPubnubListUsersFromChannelResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([ListUsersFromChannelResponse](const FPubnubOperationResult& Result, FPubnubListUsersFromChannelWrapper Data)
	{
		ListUsersFromChannelResponse.ExecuteIfBound(Result, Data);
	});

	DefaultClient->ListUsersFromChannel(Channel, ConvertedCallback, ListUsersFromChannelSettings);
}

void UPubnubSubsystem::ListUsersFromChannel(FString Channel, FOnListUsersFromChannelResponseNative NativeCallback, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubListUsersFromChannelWrapper());
	
	FOnPubnubListUsersFromChannelResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, FPubnubListUsersFromChannelWrapper Data)
	{
		NativeCallback.ExecuteIfBound(Result, Data);
	});
	
	DefaultClient->ListUsersFromChannel(Channel, ConvertedCallback, ListUsersFromChannelSettings);
}

void UPubnubSubsystem::ListUserSubscribedChannels(FString UserID, FOnListUsersSubscribedChannelsResponse ListUserSubscribedChannelsResponse)
{
	PUBNUB_ENSURE_INITIALIZED(ListUserSubscribedChannelsResponse, TArray<FString>{});
	
	FOnPubnubListUsersSubscribedChannelsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([ListUserSubscribedChannelsResponse](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		ListUserSubscribedChannelsResponse.ExecuteIfBound(Result, Channels);
	});

	DefaultClient->ListUserSubscribedChannels(UserID, ConvertedCallback);
}

void UPubnubSubsystem::ListUserSubscribedChannels(FString UserID, FOnListUsersSubscribedChannelsResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FString>{});
	
	FOnPubnubListUsersSubscribedChannelsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		NativeCallback.ExecuteIfBound(Result, Channels);
	});
	
	DefaultClient->ListUserSubscribedChannels(UserID, ConvertedCallback);
}

void UPubnubSubsystem::SetState(FString Channel, FString StateJson, FOnSetStateResponse OnSetStateResponse, FPubnubSetStateSettings SetStateSettings)
{
	PUBNUB_ENSURE_INITIALIZED(OnSetStateResponse);
	
	FOnPubnubSetStateResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSetStateResponse](const FPubnubOperationResult& Result)
	{
		OnSetStateResponse.ExecuteIfBound(Result);
	});
	
	DefaultClient->SetState(Channel, StateJson, ConvertedCallback, SetStateSettings);
}

void UPubnubSubsystem::SetState(FString Channel, FString StateJson, FOnSetStateResponseNative NativeCallback, FPubnubSetStateSettings SetStateSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FOnPubnubSetStateResponseNative ConvertedCallback;
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
	
	FOnPubnubGetStateResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGetStateResponse](const FPubnubOperationResult& Result, FString JsonResponse)
	{
		OnGetStateResponse.ExecuteIfBound(Result, JsonResponse);
	});

	DefaultClient->GetState(Channel, ChannelGroup, UserID, ConvertedCallback);
}

void UPubnubSubsystem::GetState(FString Channel, FString ChannelGroup, FString UserID, FOnGetStateResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FString());
	
	FOnPubnubGetStateResponseNative ConvertedCallback;
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

	FOnPubnubGrantTokenResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGrantTokenResponse](const FPubnubOperationResult& Result, FString Token)
	{
		OnGrantTokenResponse.ExecuteIfBound(Result, Token);
	});

	DefaultClient->GrantToken(Ttl, AuthorizedUser, Permissions, ConvertedCallback, Meta);
}

void UPubnubSubsystem::GrantToken(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FOnGrantTokenResponseNative NativeCallback, FString Meta)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FString());
	
	FOnPubnubGrantTokenResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, FString Token)
	{
		NativeCallback.ExecuteIfBound(Result, Token);
	});
	
	DefaultClient->GrantToken(Ttl, AuthorizedUser, Permissions, ConvertedCallback, Meta);
}

void UPubnubSubsystem::RevokeToken(FString Token, FOnRevokeTokenResponse OnRevokeTokenResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnRevokeTokenResponse);
	
	FOnPubnubRevokeTokenResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnRevokeTokenResponse](const FPubnubOperationResult& Result)
	{
		OnRevokeTokenResponse.ExecuteIfBound(Result);
	});
	
	DefaultClient->RevokeToken(Token, ConvertedCallback);
}

void UPubnubSubsystem::RevokeToken(FString Token, FOnRevokeTokenResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FOnPubnubRevokeTokenResponseNative ConvertedCallback;
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

	FOnPubnubFetchHistoryResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnFetchHistoryResponse](const FPubnubOperationResult& Result, const TArray<FPubnubHistoryMessageData>& Messages)
	{
		OnFetchHistoryResponse.ExecuteIfBound(Result, Messages);
	});

	DefaultClient->FetchHistory(Channel, ConvertedCallback, FetchHistorySettings);
}

void UPubnubSubsystem::FetchHistory(FString Channel, FOnFetchHistoryResponseNative NativeCallback, FPubnubFetchHistorySettings FetchHistorySettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubHistoryMessageData>());
	
	FOnPubnubFetchHistoryResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const TArray<FPubnubHistoryMessageData>& Messages)
	{
		NativeCallback.ExecuteIfBound(Result, Messages);
	});
	
	DefaultClient->FetchHistory(Channel, ConvertedCallback, FetchHistorySettings);
}

void UPubnubSubsystem::DeleteMessages(FString Channel, FOnDeleteMessagesResponse OnDeleteMessagesResponse, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	PUBNUB_ENSURE_INITIALIZED(OnDeleteMessagesResponse);

	FOnPubnubDeleteMessagesResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnDeleteMessagesResponse](FPubnubOperationResult Result)
	{
		OnDeleteMessagesResponse.ExecuteIfBound(Result);
	});

	DefaultClient->DeleteMessages(Channel, ConvertedCallback, DeleteMessagesSettings);
}

void UPubnubSubsystem::DeleteMessages(FString Channel, FOnDeleteMessagesResponseNative NativeCallback, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FOnPubnubDeleteMessagesResponseNative ConvertedCallback;
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

	FOnPubnubMessageCountsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnMessageCountsResponse](const FPubnubOperationResult& Result, int MessageCounts)
	{
		OnMessageCountsResponse.ExecuteIfBound(Result, MessageCounts);
	});

	DefaultClient->MessageCounts(Channel, Timetoken, ConvertedCallback);
}

void UPubnubSubsystem::MessageCounts(FString Channel, FString Timetoken, FOnMessageCountsResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, 0);
	
	FOnPubnubMessageCountsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, int MessageCounts)
	{
		NativeCallback.ExecuteIfBound(Result, MessageCounts);
	});
	
	DefaultClient->MessageCounts(Channel, Timetoken, ConvertedCallback);
}

void UPubnubSubsystem::GetAllUserMetadataRaw(FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(OnGetAllUserMetadataResponse, TArray<FPubnubUserData>(), FString(), FString());

	FOnPubnubGetAllUserMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGetAllUserMetadataResponse](const FPubnubOperationResult& Result, const TArray<FPubnubUserData>& UsersData, FString PageNext, FString PagePrev)
	{
		OnGetAllUserMetadataResponse.ExecuteIfBound(Result, UsersData, PageNext, PagePrev);
	});

	DefaultClient->GetAllUserMetadataRaw(ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::GetAllUserMetadataRaw(FOnGetAllUserMetadataResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubUserData>(), FString(), FString());
	
	FOnPubnubGetAllUserMetadataResponseNative ConvertedCallback;
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

	FOnPubnubSetUserMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSetUserMetadataResponse](const FPubnubOperationResult& Result, FPubnubUserData UserData)
	{
		OnSetUserMetadataResponse.ExecuteIfBound(Result, UserData);
	});

	DefaultClient->SetUserMetadataRaw(User, UserMetadataObj, ConvertedCallback, Include);
}

void UPubnubSubsystem::SetUserMetadataRaw(FString User, FString UserMetadataObj, FOnSetUserMetadataResponseNative NativeCallback, FString Include)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubUserData());
	
	FOnPubnubSetUserMetadataResponseNative ConvertedCallback;
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

	FOnPubnubGetUserMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGetUserMetadataResponse](const FPubnubOperationResult& Result, FPubnubUserData UserData)
	{
		OnGetUserMetadataResponse.ExecuteIfBound(Result, UserData);
	});

	DefaultClient->GetUserMetadataRaw(User, ConvertedCallback, Include);
}

void UPubnubSubsystem::GetUserMetadataRaw(FString User, FOnGetUserMetadataResponseNative NativeCallback, FString Include)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubUserData());
	
	FOnPubnubGetUserMetadataResponseNative ConvertedCallback;
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

	FOnPubnubRemoveUserMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnRemoveUserMetadataResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveUserMetadataResponse.ExecuteIfBound(Result);
	});

	DefaultClient->RemoveUserMetadata(User, ConvertedCallback);
}

void UPubnubSubsystem::RemoveUserMetadata(FString User, FOnRemoveUserMetadataResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FOnPubnubRemoveUserMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});
	
	DefaultClient->RemoveUserMetadata(User, ConvertedCallback);
}

void UPubnubSubsystem::GetAllChannelMetadataRaw(FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(OnGetAllChannelMetadataResponse, TArray<FPubnubChannelData>(), FString(), FString());

	FOnPubnubGetAllChannelMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGetAllChannelMetadataResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelData>& ChannelsData, FString PageNext, FString PagePrev)
	{
		OnGetAllChannelMetadataResponse.ExecuteIfBound(Result, ChannelsData, PageNext, PagePrev);
	});

	DefaultClient->GetAllChannelMetadataRaw(ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::GetAllChannelMetadataRaw(FOnGetAllChannelMetadataResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubChannelData>(), FString(), FString());
	
	FOnPubnubGetAllChannelMetadataResponseNative ConvertedCallback;
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

	FOnPubnubSetChannelMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSetChannelMetadataResponse](const FPubnubOperationResult& Result, FPubnubChannelData ChannelData)
	{
		OnSetChannelMetadataResponse.ExecuteIfBound(Result, ChannelData);
	});

	DefaultClient->SetChannelMetadataRaw(Channel, ChannelMetadataObj, ConvertedCallback, Include);
}

void UPubnubSubsystem::SetChannelMetadataRaw(FString Channel, FString ChannelMetadataObj, FOnSetChannelMetadataResponseNative NativeCallback, FString Include)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubChannelData());
	
	FOnPubnubSetChannelMetadataResponseNative ConvertedCallback;
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

	FOnPubnubGetChannelMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGetChannelMetadataResponse](const FPubnubOperationResult& Result, FPubnubChannelData ChannelData)
	{
		OnGetChannelMetadataResponse.ExecuteIfBound(Result, ChannelData);
	});

	DefaultClient->GetChannelMetadataRaw(Channel, ConvertedCallback, Include);
}

void UPubnubSubsystem::GetChannelMetadataRaw(FString Channel, FOnGetChannelMetadataResponseNative NativeCallback, FString Include)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubChannelData());

	FOnPubnubGetChannelMetadataResponseNative ConvertedCallback;
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

	FOnPubnubRemoveChannelMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnRemoveChannelMetadataResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveChannelMetadataResponse.ExecuteIfBound(Result);
	});

	DefaultClient->RemoveChannelMetadata(Channel, ConvertedCallback);
}

void UPubnubSubsystem::RemoveChannelMetadata(FString Channel, FOnRemoveChannelMetadataResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FOnPubnubRemoveChannelMetadataResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});
	
	DefaultClient->RemoveChannelMetadata(Channel, ConvertedCallback);
}

void UPubnubSubsystem::GetMembershipsRaw(FString User, FOnGetMembershipsResponse OnGetMembershipsResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(OnGetMembershipsResponse, TArray<FPubnubMembershipData>(), FString(), FString());

	FOnPubnubGetMembershipsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGetMembershipsResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		OnGetMembershipsResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});

	DefaultClient->GetMembershipsRaw(User, ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::GetMembershipsRaw(FString User, FOnGetMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FString(), FString());
	
	FOnPubnubGetMembershipsResponseNative ConvertedCallback;
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

	FOnPubnubSetMembershipsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSetMembershipResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		OnSetMembershipResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});

	DefaultClient->SetMembershipsRaw(User, SetObj, ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::SetMembershipsRaw(FString User, FString SetObj, FOnSetMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FString(), FString());
	
	FOnPubnubSetMembershipsResponseNative ConvertedCallback;
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

	FOnPubnubRemoveMembershipsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnRemoveMembershipResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		OnRemoveMembershipResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});

	DefaultClient->RemoveMembershipsRaw(User, RemoveObj, ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::RemoveMembershipsRaw(FString User, FString RemoveObj, FOnRemoveMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FString(), FString());
	
	FOnPubnubRemoveMembershipsResponseNative ConvertedCallback;
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

	FOnPubnubGetChannelMembersResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGetMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
	{
		OnGetMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
	});

	DefaultClient->GetChannelMembersRaw(Channel, ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::GetChannelMembersRaw(FString Channel, FOnGetChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubChannelMemberData>(), FString(), FString());

	FOnPubnubGetChannelMembersResponseNative ConvertedCallback;
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

	FOnPubnubSetChannelMembersResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnSetMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
	{
		OnSetMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
	});

	DefaultClient->SetChannelMembersRaw(Channel, SetObj, ConvertedCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::SetChannelMembersRaw(FString Channel, FString SetObj, FOnSetChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubChannelMemberData>(), FString(), FString());

	FOnPubnubSetChannelMembersResponseNative ConvertedCallback;
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

	FOnPubnubRemoveChannelMembersResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnRemoveMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
	{
		OnRemoveMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
	});

	DefaultClient->RemoveChannelMembersRaw(Channel, RemoveObj, ConvertedCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubSubsystem::RemoveChannelMembersRaw(FString Channel, FString RemoveObj, FOnRemoveChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubChannelMemberData>(), FString(), FString());

	FOnPubnubRemoveChannelMembersResponseNative ConvertedCallback;
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
	PUBNUB_ENSURE_INITIALIZED(OnAddMessageActionResponse, FPubnubMessageActionData());

	FOnPubnubAddMessageActionResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnAddMessageActionResponse](const FPubnubOperationResult& Result, FPubnubMessageActionData MessageActionData)
	{
		OnAddMessageActionResponse.ExecuteIfBound(Result, MessageActionData);
	});

	DefaultClient->AddMessageAction(Channel, MessageTimetoken, ActionType, Value, ConvertedCallback);
}

void UPubnubSubsystem::AddMessageAction(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnAddMessageActionResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, FPubnubMessageActionData());
	
	FOnPubnubAddMessageActionResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, FPubnubMessageActionData MessageActionData)
	{
		NativeCallback.ExecuteIfBound(Result, MessageActionData);
	});
	
	DefaultClient->AddMessageAction(Channel, MessageTimetoken, ActionType, Value, ConvertedCallback);
}

void UPubnubSubsystem::GetMessageActions(FString Channel, FOnGetMessageActionsResponse OnGetMessageActionsResponse, FString Start, FString End, int Limit)
{
	PUBNUB_ENSURE_INITIALIZED(OnGetMessageActionsResponse, TArray<FPubnubMessageActionData>());

	FOnPubnubGetMessageActionsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnGetMessageActionsResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMessageActionData>& MessageActions)
	{
		OnGetMessageActionsResponse.ExecuteIfBound(Result, MessageActions);
	});

	DefaultClient->GetMessageActions(Channel, ConvertedCallback, Start, End, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit));
}

void UPubnubSubsystem::GetMessageActions(FString Channel, FOnGetMessageActionsResponseNative NativeCallback, FString Start, FString End, int Limit)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback, TArray<FPubnubMessageActionData>());
	
	FOnPubnubGetMessageActionsResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result, const TArray<FPubnubMessageActionData>& MessageActions)
	{
		NativeCallback.ExecuteIfBound(Result, MessageActions);
	});
	
	DefaultClient->GetMessageActions(Channel, ConvertedCallback, Start, End, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit));
}

void UPubnubSubsystem::RemoveMessageAction(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnRemoveMessageActionResponse OnRemoveMessageActionResponse)
{
	PUBNUB_ENSURE_INITIALIZED(OnRemoveMessageActionResponse);

	FOnPubnubRemoveMessageActionResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([OnRemoveMessageActionResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveMessageActionResponse.ExecuteIfBound(Result);
	});

	DefaultClient->RemoveMessageAction(Channel, MessageTimetoken, ActionTimetoken, ConvertedCallback);
}

void UPubnubSubsystem::RemoveMessageAction(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnRemoveMessageActionResponseNative NativeCallback)
{
	PUBNUB_ENSURE_INITIALIZED(NativeCallback);
	
	FOnPubnubRemoveMessageActionResponseNative ConvertedCallback;
	ConvertedCallback.BindLambda([NativeCallback](const FPubnubOperationResult& Result)
	{
		NativeCallback.ExecuteIfBound(Result);
	});
	
	DefaultClient->RemoveMessageAction(Channel, MessageTimetoken, ActionTimetoken, ConvertedCallback);
}

void UPubnubSubsystem::ReconnectSubscriptions()
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
	DefaultClient->ReconnectSubscriptions();
}

void UPubnubSubsystem::DisconnectSubscriptions()
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
	DefaultClient->DisconnectSubscriptions();
}

void UPubnubSubsystem::SetCryptoModule(TScriptInterface<IPubnubCryptoProviderInterface> CryptoModule)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED();
	
	DefaultClient->SetCryptoModule(CryptoModule);
}

TScriptInterface<IPubnubCryptoProviderInterface> UPubnubSubsystem::GetCryptoModule()
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED(nullptr);
	
	return DefaultClient->GetCryptoModule();
}

UPubnubChannelEntity* UPubnubSubsystem::CreateChannelEntity(FString Channel)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED(nullptr);
	
	return DefaultClient->CreateChannelEntity(Channel);
}

UPubnubChannelGroupEntity* UPubnubSubsystem::CreateChannelGroupEntity(FString ChannelGroup)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED(nullptr);
	
	return DefaultClient->CreateChannelGroupEntity(ChannelGroup);
}

UPubnubChannelMetadataEntity* UPubnubSubsystem::CreateChannelMetadataEntity(FString Channel)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED(nullptr);
	
	return DefaultClient->CreateChannelMetadataEntity(Channel);
}

UPubnubUserMetadataEntity* UPubnubSubsystem::CreateUserMetadataEntity(FString User)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED(nullptr);
	
	return DefaultClient->CreateUserMetadataEntity(User);
}

UPubnubSubscriptionSet* UPubnubSubsystem::CreateSubscriptionSet(TArray<FString> Channels, TArray<FString> ChannelGroups, FPubnubSubscribeSettings SubscriptionSettings)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED(nullptr);
	
	return DefaultClient->CreateSubscriptionSet(Channels, ChannelGroups, SubscriptionSettings);
}

UPubnubSubscriptionSet* UPubnubSubsystem::CreateSubscriptionSetFromEntities(TArray<UPubnubBaseEntity*> Entities, FPubnubSubscribeSettings SubscriptionSettings)
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED(nullptr);
	
	return DefaultClient->CreateSubscriptionSetFromEntities(Entities, SubscriptionSettings);
}

TArray<UPubnubSubscription*> UPubnubSubsystem::GetActiveSubscriptions()
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED(TArray<UPubnubSubscription*>());
	
	return DefaultClient->GetActiveSubscriptions();
}

TArray<UPubnubSubscriptionSet*> UPubnubSubsystem::GetActiveSubscriptionSets()
{
	PUBNUB_RETURN_IF_NOT_INITIALIZED(TArray<UPubnubSubscriptionSet*>());
	
	return DefaultClient->GetActiveSubscriptionSets();
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

	TWeakObjectPtr<UPubnubSubsystem> WeakThis = MakeWeakObjectPtr(this);

	//Errors has to be broadcasted on GameThread, otherwise engine will crash if someone uses them for example with widgets
	AsyncTask(ENamedThreads::GameThread, [WeakThis, ErrorMessage, ErrorType]()
	{
		if(WeakThis.IsValid())
		{
			//Broadcast bound delegate with JsonResponse
			WeakThis.Get()->OnPubnubError.Broadcast(ErrorMessage, ErrorType);
			WeakThis.Get()->OnPubnubErrorNative.Broadcast(ErrorMessage, ErrorType);
		}
	});
}

void UPubnubSubsystem::LoadPluginSettings()
{
	//Save all settings
	PubnubPluginSettings = GetMutableDefault<UPubnubSettings>();
}
