// Fill out your copyright notice in the Description page of Project Settings.


#include "PubnubClient.h"
#include "PubNub.h"
#include "PubnubInternalMacros.h"
#include "PubnubSubsystem.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "Threads/PubnubFunctionThread.h"
#include "FunctionLibraries/PubnubUtilities.h"
#include "FunctionLibraries/PubnubInternalUtilities.h"
#include "FunctionLibraries/PubnubTokenUtilities.h"
#include "Entities/PubnubBaseEntity.h"
#include "Entities/PubnubChannelEntity.h"
#include "Entities/PubnubChannelGroupEntity.h"
#include "Entities/PubnubChannelMetadataEntity.h"
#include "Entities/PubnubUserMetadataEntity.h"
#include "Entities/PubnubSubscription.h"


struct CCoreSubscriptionCallback
{
	pubnub_subscribe_message_callback_t Callback;
	pubnub_subscription_t* Subscription;
};

//TODO:: Move this to logger
//Logs from C-Core that are false warnings as they are sent during normal C-Core operations flow
static TArray<FString> FalseCCoreLogPhrases =
	{
	"errno=0('No error')",
	"errno=9('Bad file descriptor')",
	"errno=2('No such file or directory')",
	"errno=35('Resource temporarily unavailable')"
};

//TODO:: Move this to logger
static bool ShouldCCoreLogBeSkipped(FString Message)
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

//TODO:: Move this to logger
static void PubnubSDKLogConverter(enum pubnub_log_level log_level, const char* message)
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
}

//TODO:: Move this to logger
void UPubnubClient::PubnubError(FString ErrorMessage, EPubnubErrorType ErrorType)
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

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	//Errors has to be broadcasted on GameThread, otherwise engine will crash if someone uses them for example with widgets
	AsyncTask(ENamedThreads::GameThread, [WeakThis, ErrorMessage, ErrorType]()
	{
		if(!WeakThis.IsValid())
		{return;}
		
		//Broadcast bound delegate with JsonResponse
		WeakThis.Get()->OnPubnubError.Broadcast(ErrorMessage, ErrorType);
		WeakThis.Get()->OnPubnubErrorNative.Broadcast(ErrorMessage, ErrorType);
	});
}

//TODO:: Move this to logger
void UPubnubClient::PubnubResponseError(int PubnubResponse, FString ErrorMessage)
{
	//Convert all error data into single string
	FString ResponseString(pubnub_res_2_string(static_cast<pubnub_res>(PubnubResponse)));
	FString FinalErrorMessage = FString::Printf(TEXT("%s Error: %s."), *ErrorMessage, *ResponseString);

	PubnubError(FinalErrorMessage,EPubnubErrorType::PET_Error);
}

void UPubnubClient::DestroyClient()
{
	if(!PubnubSubsystem)
	{return;}

	PubnubSubsystem->DestroyPubnubClient(this);
}

void UPubnubClient::SetUserID(FString UserID)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();

	SetUserID_priv(UserID);
}

FString UPubnubClient::GetUserID()
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED("");

	return GetUserID_priv();
}

void UPubnubClient::SetSecretKey()
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();

	SetSecretKey_priv();
}

void UPubnubClient::PublishMessage(FString Channel, FString Message, FOnPubnubPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings)
{
	FOnPubnubPublishMessageResponseNative NativeCallback;
	NativeCallback.BindLambda([OnPublishMessageResponse](FPubnubOperationResult Result, FPubnubMessageData PublishedMessage)
	{
		OnPublishMessageResponse.ExecuteIfBound(Result, PublishedMessage);
	});

	PublishMessage(Channel, Message, NativeCallback, PublishSettings);
}

void UPubnubClient::PublishMessage(FString Channel, FString Message, FOnPubnubPublishMessageResponseNative NativeCallback, FPubnubPublishSettings PublishSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubMessageData());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, Message, NativeCallback, PublishSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->PublishMessage_priv(Channel, Message, NativeCallback, PublishSettings);
	});
}

void UPubnubClient::PublishMessage(FString Channel, FString Message, FPubnubPublishSettings PublishSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, Message, PublishSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->PublishMessage_priv(Channel, Message, nullptr, PublishSettings);
	});
}

void UPubnubClient::Signal(FString Channel, FString Message, FOnPubnubSignalResponse OnSignalResponse, FPubnubSignalSettings SignalSettings)
{
	FOnPubnubSignalResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSignalResponse](const FPubnubOperationResult& Result, const FPubnubMessageData& SignalMessage)
	{
		OnSignalResponse.ExecuteIfBound(Result, SignalMessage);
	});

	Signal(Channel, Message, NativeCallback, SignalSettings);
}

void UPubnubClient::Signal(FString Channel, FString Message, FOnPubnubSignalResponseNative NativeCallback, FPubnubSignalSettings SignalSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubMessageData());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, Message, NativeCallback, SignalSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->Signal_priv(Channel, Message, NativeCallback, SignalSettings);
	});
}

void UPubnubClient::Signal(FString Channel, FString Message, FPubnubSignalSettings SignalSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, Message, SignalSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->Signal_priv(Channel, Message, nullptr, SignalSettings);
	});
}


void UPubnubClient::SubscribeToChannel(FString Channel, FOnPubnubSubscribeOperationResponse OnSubscribeToChannelResponse, FPubnubSubscribeSettings SubscribeSettings)
{
	FOnPubnubSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSubscribeToChannelResponse](FPubnubOperationResult Result)
	{
		OnSubscribeToChannelResponse.ExecuteIfBound(Result);
	});

	SubscribeToChannel(Channel, NativeCallback, SubscribeSettings);
}

void UPubnubClient::SubscribeToChannel(FString Channel, FOnPubnubSubscribeOperationResponseNative NativeCallback, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, NativeCallback, SubscribeSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->SubscribeToChannel_priv(Channel, NativeCallback, SubscribeSettings);
	});
}

void UPubnubClient::SubscribeToChannel(FString Channel, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	SubscribeToChannel(Channel, nullptr, SubscribeSettings);
}

void UPubnubClient::SubscribeToGroup(FString ChannelGroup, FOnPubnubSubscribeOperationResponse OnSubscribeToGroupResponse, FPubnubSubscribeSettings SubscribeSettings)
{
	FOnPubnubSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSubscribeToGroupResponse](FPubnubOperationResult Result)
	{
		OnSubscribeToGroupResponse.ExecuteIfBound(Result);
	});

	SubscribeToGroup(ChannelGroup, NativeCallback, SubscribeSettings);
}

void UPubnubClient::SubscribeToGroup(FString ChannelGroup, FOnPubnubSubscribeOperationResponseNative NativeCallback, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, ChannelGroup, NativeCallback, SubscribeSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->SubscribeToGroup_priv(ChannelGroup, NativeCallback, SubscribeSettings);
	});
}

void UPubnubClient::SubscribeToGroup(FString ChannelGroup, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	SubscribeToGroup(ChannelGroup, nullptr, SubscribeSettings);
}

void UPubnubClient::UnsubscribeFromChannel(FString Channel, FOnPubnubSubscribeOperationResponse OnUnsubscribeFromChannelResponse)
{
	FOnPubnubSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeFromChannelResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromChannelResponse.ExecuteIfBound(Result);
	});

	UnsubscribeFromChannel(Channel, NativeCallback);
}

void UPubnubClient::UnsubscribeFromChannel(FString Channel, FOnPubnubSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->UnsubscribeFromChannel_priv(Channel, NativeCallback);
	});
}

void UPubnubClient::UnsubscribeFromGroup(FString ChannelGroup, FOnPubnubSubscribeOperationResponse OnUnsubscribeFromGroupResponse)
{
	FOnPubnubSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeFromGroupResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromGroupResponse.ExecuteIfBound(Result);
	});

	UnsubscribeFromGroup(ChannelGroup, NativeCallback);
}

void UPubnubClient::UnsubscribeFromGroup(FString ChannelGroup, FOnPubnubSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, ChannelGroup, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->UnsubscribeFromGroup_priv(ChannelGroup, NativeCallback);
	});
}

void UPubnubClient::UnsubscribeFromAll(FOnPubnubSubscribeOperationResponse OnUnsubscribeFromAllResponse)
{
	FOnPubnubSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeFromAllResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromAllResponse.ExecuteIfBound(Result);
	});

	UnsubscribeFromAll(NativeCallback);
}

void UPubnubClient::UnsubscribeFromAll(FOnPubnubSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->UnsubscribeFromAll_priv(NativeCallback);
	});
}

void UPubnubClient::AddChannelToGroup(FString Channel, FString ChannelGroup, FOnPubnubAddChannelToGroupResponse OnAddChannelToGroupResponse)
{
	FOnPubnubAddChannelToGroupResponseNative NativeCallback;
	NativeCallback.BindLambda([OnAddChannelToGroupResponse](const FPubnubOperationResult& Result)
	{
		OnAddChannelToGroupResponse.ExecuteIfBound(Result);
	});
	AddChannelToGroup(Channel, ChannelGroup, NativeCallback);
}

void UPubnubClient::AddChannelToGroup(FString Channel, FString ChannelGroup, FOnPubnubAddChannelToGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, ChannelGroup, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->AddChannelToGroup_priv(Channel, ChannelGroup, NativeCallback);
	});
}

void UPubnubClient::RemoveChannelFromGroup(FString Channel, FString ChannelGroup, FOnPubnubRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse)
{
	FOnPubnubRemoveChannelFromGroupResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveChannelFromGroupResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveChannelFromGroupResponse.ExecuteIfBound(Result);
	});
	RemoveChannelFromGroup(Channel, ChannelGroup, NativeCallback);
}

void UPubnubClient::RemoveChannelFromGroup(FString Channel, FString ChannelGroup, FOnPubnubRemoveChannelFromGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, ChannelGroup, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->RemoveChannelFromGroup_priv(Channel, ChannelGroup, NativeCallback);
	});
}

void UPubnubClient::ListChannelsFromGroup(FString ChannelGroup, FOnPubnubListChannelsFromGroupResponse OnListChannelsResponse)
{
	FOnPubnubListChannelsFromGroupResponseNative NativeCallback;
	NativeCallback.BindLambda([OnListChannelsResponse](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		OnListChannelsResponse.ExecuteIfBound(Result, Channels);
	});

	ListChannelsFromGroup(ChannelGroup, NativeCallback);
}

void UPubnubClient::ListChannelsFromGroup(FString ChannelGroup, FOnPubnubListChannelsFromGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FString>{});
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, ChannelGroup, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->ListChannelsFromGroup_priv(ChannelGroup, NativeCallback);
	});
}

void UPubnubClient::RemoveChannelGroup(FString ChannelGroup, FOnPubnubRemoveChannelGroupResponse OnRemoveChannelGroupResponse)
{
	FOnPubnubRemoveChannelGroupResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveChannelGroupResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveChannelGroupResponse.ExecuteIfBound(Result);
	});
	RemoveChannelGroup(ChannelGroup, NativeCallback);
}

void UPubnubClient::RemoveChannelGroup(FString ChannelGroup, FOnPubnubRemoveChannelGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, ChannelGroup, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->RemoveChannelGroup_priv(ChannelGroup, NativeCallback);
	});
}

void UPubnubClient::ListUsersFromChannel(FString Channel, FOnPubnubListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	FOnPubnubListUsersFromChannelResponseNative NativeCallback;
	NativeCallback.BindLambda([ListUsersFromChannelResponse](const FPubnubOperationResult& Result, FPubnubListUsersFromChannelWrapper Data)
	{
		ListUsersFromChannelResponse.ExecuteIfBound(Result, Data);
	});

	ListUsersFromChannel(Channel, NativeCallback, ListUsersFromChannelSettings);
}

void UPubnubClient::ListUsersFromChannel(FString Channel, FOnPubnubListUsersFromChannelResponseNative NativeCallback, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubListUsersFromChannelWrapper());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, NativeCallback, ListUsersFromChannelSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->ListUsersFromChannel_priv(Channel, NativeCallback, ListUsersFromChannelSettings);
	});
}

void UPubnubClient::ListUserSubscribedChannels(FString UserID, FOnPubnubListUsersSubscribedChannelsResponse ListUserSubscribedChannelsResponse)
{
	FOnPubnubListUsersSubscribedChannelsResponseNative NativeCallback;
	NativeCallback.BindLambda([ListUserSubscribedChannelsResponse](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		ListUserSubscribedChannelsResponse.ExecuteIfBound(Result, Channels);
	});

	ListUserSubscribedChannels(UserID, NativeCallback);
}

void UPubnubClient::ListUserSubscribedChannels(FString UserID, FOnPubnubListUsersSubscribedChannelsResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FString>{});
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, UserID, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->ListUserSubscribedChannels_priv(UserID, NativeCallback);
	});
}

void UPubnubClient::SetState(FString Channel, FString StateJson, FOnPubnubSetStateResponse OnSetStateResponse, FPubnubSetStateSettings SetStateSettings)
{
	FOnPubnubSetStateResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSetStateResponse](const FPubnubOperationResult& Result)
	{
		OnSetStateResponse.ExecuteIfBound(Result);
	});
	SetState(Channel, StateJson, NativeCallback, SetStateSettings);
}

void UPubnubClient::SetState(FString Channel, FString StateJson, FOnPubnubSetStateResponseNative NativeCallback, FPubnubSetStateSettings SetStateSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, StateJson, NativeCallback, SetStateSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->SetState_priv(Channel, StateJson, NativeCallback, SetStateSettings);
	});
}

void UPubnubClient::SetState(FString Channel, FString StateJson, FPubnubSetStateSettings SetStateSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, StateJson, SetStateSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->SetState_priv(Channel, StateJson, nullptr, SetStateSettings);
	});
}

void UPubnubClient::GetState(FString Channel, FString ChannelGroup, FString UserID, FOnPubnubGetStateResponse OnGetStateResponse)
{
	FOnPubnubGetStateResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetStateResponse](const FPubnubOperationResult& Result, FString JsonResponse)
	{
		OnGetStateResponse.ExecuteIfBound(Result, JsonResponse);
	});

	GetState(Channel, ChannelGroup, UserID, NativeCallback);
}

void UPubnubClient::GetState(FString Channel, FString ChannelGroup, FString UserID, FOnPubnubGetStateResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FString());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, ChannelGroup, UserID, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->GetState_priv(Channel, ChannelGroup, UserID, NativeCallback);
	});
}

void UPubnubClient::Heartbeat(FString Channel, FString ChannelGroup)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, ChannelGroup]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->Heartbeat_priv(Channel, ChannelGroup);
	});
}

void UPubnubClient::GrantToken(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FOnPubnubGrantTokenResponse OnGrantTokenResponse, FString Meta)
{
	FOnPubnubGrantTokenResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGrantTokenResponse](const FPubnubOperationResult& Result, FString Token)
	{
		OnGrantTokenResponse.ExecuteIfBound(Result, Token);
	});

	GrantToken(Ttl, AuthorizedUser, Permissions, NativeCallback, Meta);
}

void UPubnubClient::GrantToken(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FOnPubnubGrantTokenResponseNative NativeCallback, FString Meta)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FString());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Ttl, AuthorizedUser, Permissions, NativeCallback, Meta]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->GrantToken_priv(UPubnubTokenUtilities::CreateGrantTokenPermissionObjectString(Ttl, AuthorizedUser, Permissions, Meta), NativeCallback);
	});
}

void UPubnubClient::RevokeToken(FString Token, FOnPubnubRevokeTokenResponse OnRevokeTokenResponse)
{
	FOnPubnubRevokeTokenResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRevokeTokenResponse](const FPubnubOperationResult& Result)
	{
		OnRevokeTokenResponse.ExecuteIfBound(Result);
	});
	RevokeToken(Token, NativeCallback);
}

void UPubnubClient::RevokeToken(FString Token, FOnPubnubRevokeTokenResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Token, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->RevokeToken_priv(Token, NativeCallback);
	});
}

FString UPubnubClient::ParseToken(FString Token)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED("");

	return ParseToken_priv(Token);
}

void UPubnubClient::SetAuthToken(FString Token)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();

	SetAuthToken_priv(Token);
}

void UPubnubClient::FetchHistory(FString Channel, FOnPubnubFetchHistoryResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings)
{
	FOnPubnubFetchHistoryResponseNative NativeCallback;
	NativeCallback.BindLambda([OnFetchHistoryResponse](const FPubnubOperationResult& Result, const TArray<FPubnubHistoryMessageData>& Messages)
	{
		OnFetchHistoryResponse.ExecuteIfBound(Result, Messages);
	});

	FetchHistory(Channel, NativeCallback, FetchHistorySettings);
}

void UPubnubClient::FetchHistory(FString Channel, FOnPubnubFetchHistoryResponseNative NativeCallback, FPubnubFetchHistorySettings FetchHistorySettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubHistoryMessageData>());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, NativeCallback, FetchHistorySettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->FetchHistory_priv(Channel, NativeCallback, FetchHistorySettings);
	});
}

void UPubnubClient::DeleteMessages(FString Channel, FOnPubnubDeleteMessagesResponse OnDeleteMessagesResponse, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	FOnPubnubDeleteMessagesResponseNative NativeCallback;
	NativeCallback.BindLambda([OnDeleteMessagesResponse](FPubnubOperationResult Result)
	{
		OnDeleteMessagesResponse.ExecuteIfBound(Result);
	});

	DeleteMessages(Channel, NativeCallback, DeleteMessagesSettings);
}

void UPubnubClient::DeleteMessages(FString Channel, FOnPubnubDeleteMessagesResponseNative NativeCallback, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, NativeCallback, DeleteMessagesSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->DeleteMessages_priv(Channel, NativeCallback, DeleteMessagesSettings);
	});
}

void UPubnubClient::DeleteMessages(FString Channel, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, DeleteMessagesSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->DeleteMessages_priv(Channel, nullptr, DeleteMessagesSettings);
	});
}

void UPubnubClient::MessageCounts(FString Channel, FString Timetoken, FOnPubnubMessageCountsResponse OnMessageCountsResponse)
{
	FOnPubnubMessageCountsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnMessageCountsResponse](const FPubnubOperationResult& Result, int MessageCounts)
	{
		OnMessageCountsResponse.ExecuteIfBound(Result, MessageCounts);
	});

	MessageCounts(Channel, Timetoken, NativeCallback);
}

void UPubnubClient::MessageCounts(FString Channel, FString Timetoken, FOnPubnubMessageCountsResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, 0);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, Timetoken, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->MessageCounts_priv(Channel, Timetoken, NativeCallback);
	});
}

void UPubnubClient::GetAllUserMetadataRaw(FOnPubnubGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	FOnPubnubGetAllUserMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetAllUserMetadataResponse](const FPubnubOperationResult& Result, const TArray<FPubnubUserData>& UsersData, FString PageNext, FString PagePrev)
	{
		OnGetAllUserMetadataResponse.ExecuteIfBound(Result, UsersData, PageNext, PagePrev);
	});
	GetAllUserMetadataRaw(NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubClient::GetAllUserMetadataRaw(FOnPubnubGetAllUserMetadataResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubUserData>(), FString(), FString());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->GetAllUserMetadata_priv(NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubClient::GetAllUserMetadata(FOnPubnubGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FString PageNext, FString PagePrev)
{
	GetAllUserMetadataRaw(OnGetAllUserMetadataResponse, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetAllUserMetadata(FOnPubnubGetAllUserMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FString PageNext, FString PagePrev)
{
	GetAllUserMetadataRaw(NativeCallback, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::SetUserMetadataRaw(FString User, FString UserMetadataObj, FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse, FString Include)
{
	FOnPubnubSetUserMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSetUserMetadataResponse](const FPubnubOperationResult& Result, FPubnubUserData UserData)
	{
		OnSetUserMetadataResponse.ExecuteIfBound(Result, UserData);
	});
	SetUserMetadataRaw(User, UserMetadataObj, NativeCallback, Include);
}

void UPubnubClient::SetUserMetadataRaw(FString User, FString UserMetadataObj, FOnPubnubSetUserMetadataResponseNative NativeCallback, FString Include)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubUserData());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, User, UserMetadataObj, NativeCallback, Include]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->SetUserMetadata_priv(User, UserMetadataObj, NativeCallback, Include);
	});
}

void UPubnubClient::SetUserMetadata(FString User, FPubnubUserData UserMetadata, FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	SetUserMetadataRaw(User, UPubnubJsonUtilities::GetJsonFromUserData(UserMetadata), OnSetUserMetadataResponse, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubClient::SetUserMetadata(FString User, FPubnubUserData UserMetadata, FOnPubnubSetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	SetUserMetadataRaw(User, UPubnubJsonUtilities::GetJsonFromUserData(UserMetadata), NativeCallback, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubClient::GetUserMetadataRaw(FString User, FOnPubnubGetUserMetadataResponse OnGetUserMetadataResponse, FString Include)
{
	FOnPubnubGetUserMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetUserMetadataResponse](const FPubnubOperationResult& Result, FPubnubUserData UserData)
	{
		OnGetUserMetadataResponse.ExecuteIfBound(Result, UserData);
	});
	GetUserMetadataRaw(User, NativeCallback, Include);
}

void UPubnubClient::GetUserMetadataRaw(FString User, FOnPubnubGetUserMetadataResponseNative NativeCallback, FString Include)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubUserData());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, User, NativeCallback, Include]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->GetUserMetadata_priv(User, NativeCallback, Include);
	});
}

void UPubnubClient::GetUserMetadata(FString User, FOnPubnubGetUserMetadataResponse OnGetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	GetUserMetadataRaw(User, OnGetUserMetadataResponse, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubClient::GetUserMetadata(FString User, FOnPubnubGetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	GetUserMetadataRaw(User, NativeCallback, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubClient::RemoveUserMetadata(FString User, FOnPubnubRemoveUserMetadataResponse OnRemoveUserMetadataResponse)
{
	FOnPubnubRemoveUserMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveUserMetadataResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveUserMetadataResponse.ExecuteIfBound(Result);
	});
	RemoveUserMetadata(User, NativeCallback);
}

void UPubnubClient::RemoveUserMetadata(FString User, FOnPubnubRemoveUserMetadataResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, User, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->RemoveUserMetadata_priv(User, NativeCallback);
	});
}

void UPubnubClient::GetAllChannelMetadataRaw(FOnPubnubGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	FOnPubnubGetAllChannelMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetAllChannelMetadataResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelData>& ChannelsData, FString PageNext, FString PagePrev)
	{
		OnGetAllChannelMetadataResponse.ExecuteIfBound(Result, ChannelsData, PageNext, PagePrev);
	});
	GetAllChannelMetadataRaw(NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubClient::GetAllChannelMetadataRaw(FOnPubnubGetAllChannelMetadataResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubChannelData>(), FString(), FString());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->GetAllChannelMetadata_priv(NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubClient::GetAllChannelMetadata(FOnPubnubGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FString PageNext, FString PagePrev)
{
	GetAllChannelMetadataRaw(OnGetAllChannelMetadataResponse, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetAllChannelMetadata(FOnPubnubGetAllChannelMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FString PageNext, FString PagePrev)
{
	GetAllChannelMetadataRaw(NativeCallback, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::SetChannelMetadataRaw(FString Channel, FString ChannelMetadataObj, FOnPubnubSetChannelMetadataResponse OnSetChannelMetadataResponse, FString Include)
{
	FOnPubnubSetChannelMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSetChannelMetadataResponse](const FPubnubOperationResult& Result, FPubnubChannelData ChannelData)
	{
		OnSetChannelMetadataResponse.ExecuteIfBound(Result, ChannelData);
	});
	SetChannelMetadataRaw(Channel, ChannelMetadataObj, NativeCallback, Include);
}

void UPubnubClient::SetChannelMetadataRaw(FString Channel, FString ChannelMetadataObj, FOnPubnubSetChannelMetadataResponseNative NativeCallback, FString Include)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubChannelData());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, ChannelMetadataObj, NativeCallback, Include]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->SetChannelMetadata_priv(Channel, ChannelMetadataObj, NativeCallback, Include);
	});
}

void UPubnubClient::SetChannelMetadata(FString Channel, FPubnubChannelData ChannelMetadata, FOnPubnubSetChannelMetadataResponse OnSetChannelMetadataResponse, FPubnubGetMetadataInclude Include)
{
	SetChannelMetadataRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelData(ChannelMetadata), OnSetChannelMetadataResponse, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubClient::SetChannelMetadata(FString Channel, FPubnubChannelData ChannelMetadata, FOnPubnubSetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	SetChannelMetadataRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelData(ChannelMetadata), NativeCallback, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubClient::GetChannelMetadataRaw(FString Channel, FOnPubnubGetChannelMetadataResponse OnGetChannelMetadataResponse, FString Include)
{
    FOnPubnubGetChannelMetadataResponseNative NativeCallback;
    NativeCallback.BindLambda([OnGetChannelMetadataResponse](const FPubnubOperationResult& Result, FPubnubChannelData ChannelData)
    {
        OnGetChannelMetadataResponse.ExecuteIfBound(Result, ChannelData);
    });
    GetChannelMetadataRaw(Channel, NativeCallback, Include);
}

void UPubnubClient::GetChannelMetadataRaw(FString Channel, FOnPubnubGetChannelMetadataResponseNative NativeCallback, FString Include)
{
    PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubChannelData());

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

    PubnubCallsThread->AddFunctionToQueue([WeakThis, Channel, NativeCallback, Include]
    {
		if(!WeakThis.IsValid())
		{return;}
		
        WeakThis.Get()->GetChannelMetadata_priv(Channel, NativeCallback, Include);
    });
}

void UPubnubClient::GetChannelMetadata(FString Channel, FOnPubnubGetChannelMetadataResponse OnGetChannelMetadataResponse, FPubnubGetMetadataInclude Include)
{
	GetChannelMetadataRaw(Channel, OnGetChannelMetadataResponse, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubClient::GetChannelMetadata(FString Channel, FOnPubnubGetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	GetChannelMetadataRaw(Channel, NativeCallback, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubClient::RemoveChannelMetadata(FString Channel, FOnPubnubRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse)
{
	FOnPubnubRemoveChannelMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveChannelMetadataResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveChannelMetadataResponse.ExecuteIfBound(Result);
	});
	RemoveChannelMetadata(Channel, NativeCallback);
}

void UPubnubClient::RemoveChannelMetadata(FString Channel, FOnPubnubRemoveChannelMetadataResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->RemoveChannelMetadata_priv(Channel, NativeCallback);
	});
}

void UPubnubClient::GetMembershipsRaw(FString User, FOnPubnubGetMembershipsResponse OnGetMembershipsResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	FOnPubnubGetMembershipsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetMembershipsResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		OnGetMembershipsResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});

	GetMembershipsRaw(User, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
}

void UPubnubClient::GetMembershipsRaw(FString User, FOnPubnubGetMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FString(), FString());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, User, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->GetMemberships_priv(User, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubClient::GetMemberships(FString User, FOnPubnubGetMembershipsResponse OnGetMembershipsResponse, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	GetMembershipsRaw(User, OnGetMembershipsResponse, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetMemberships(FString User, FOnPubnubGetMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	GetMembershipsRaw(User, NativeCallback, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::SetMembershipsRaw(FString User, FString SetObj, FOnPubnubSetMembershipsResponse OnSetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	FOnPubnubGetMembershipsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSetMembershipResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		OnSetMembershipResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});

	SetMembershipsRaw(User, SetObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
}

void UPubnubClient::SetMembershipsRaw(FString User, FString SetObj, FOnPubnubSetMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FString(), FString());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, User, SetObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->SetMemberships_priv(User, SetObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubClient::SetMemberships(FString User, TArray<FPubnubMembershipInputData> Channels, FOnPubnubSetMembershipsResponse OnSetMembershipResponse, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	SetMembershipsRaw(User, UPubnubJsonUtilities::GetJsonFromMembershipsDataArray(Channels), OnSetMembershipResponse, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::SetMemberships(FString User, TArray<FPubnubMembershipInputData> Channels, FOnPubnubSetMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	SetMembershipsRaw(User, UPubnubJsonUtilities::GetJsonFromMembershipsDataArray(Channels), NativeCallback, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::RemoveMembershipsRaw(FString User, FString RemoveObj, FOnPubnubRemoveMembershipsResponse OnRemoveMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	FOnPubnubGetMembershipsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveMembershipResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
	{
		OnRemoveMembershipResponse.ExecuteIfBound(Result, MembershipsData, PageNext, PagePrev);
	});

	RemoveMembershipsRaw(User, RemoveObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
}

void UPubnubClient::RemoveMembershipsRaw(FString User, FString RemoveObj, FOnPubnubRemoveMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FString(), FString());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, User, RemoveObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->RemoveMemberships_priv(User, RemoveObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubClient::RemoveMemberships(FString User, TArray<FString> Channels, FOnPubnubRemoveMembershipsResponse OnRemoveMembershipResponse, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	RemoveMembershipsRaw(User, UPubnubJsonUtilities::GetJsonFromMembershipsToRemove(Channels), OnRemoveMembershipResponse, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::RemoveMemberships(FString User, TArray<FString> Channels, FOnPubnubRemoveMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FString PageNext, FString PagePrev)
{
	RemoveMembershipsRaw(User, UPubnubJsonUtilities::GetJsonFromMembershipsToRemove(Channels), NativeCallback, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), PageNext, PagePrev,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetChannelMembersRaw(FString Channel, FOnPubnubGetChannelMembersResponse OnGetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    FOnPubnubGetChannelMembersResponseNative NativeCallback;
    NativeCallback.BindLambda([OnGetMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
    {
        OnGetMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
    });
    GetChannelMembersRaw(Channel, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubClient::GetChannelMembersRaw(FString Channel, FOnPubnubGetChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubChannelMemberData>(), FString(), FString());

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

    PubnubCallsThread->AddFunctionToQueue([WeakThis, Channel, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count]
    {
		if(!WeakThis.IsValid())
		{return;}
		
        WeakThis.Get()->GetChannelMembers_priv(Channel, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
    });
}

void UPubnubClient::GetChannelMembers(FString Channel, FOnPubnubGetChannelMembersResponse OnGetMembersResponse, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    GetChannelMembersRaw(Channel, OnGetMembersResponse, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetChannelMembers(FString Channel, FOnPubnubGetChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    GetChannelMembersRaw(Channel, NativeCallback, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::SetChannelMembersRaw(FString Channel, FString SetObj, FOnPubnubSetChannelMembersResponse OnSetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    FOnPubnubSetChannelMembersResponseNative NativeCallback;
    NativeCallback.BindLambda([OnSetMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
    {
        OnSetMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
    });
    SetChannelMembersRaw(Channel, SetObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubClient::SetChannelMembersRaw(FString Channel, FString SetObj, FOnPubnubSetChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubChannelMemberData>(), FString(), FString());

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

    PubnubCallsThread->AddFunctionToQueue([WeakThis, Channel, SetObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count]
    {
		if(!WeakThis.IsValid())
		{return;}
		
        WeakThis.Get()->SetChannelMembers_priv(Channel, SetObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
    });
}

void UPubnubClient::SetChannelMembers(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FOnPubnubSetChannelMembersResponse OnSetMembersResponse, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    SetChannelMembersRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersDataArray(Users), OnSetMembersResponse, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::SetChannelMembers(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FOnPubnubSetChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    SetChannelMembersRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersDataArray(Users), NativeCallback, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::RemoveChannelMembersRaw(FString Channel, FString RemoveObj, FOnPubnubRemoveChannelMembersResponse OnRemoveMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    FOnPubnubRemoveChannelMembersResponseNative NativeCallback;
    NativeCallback.BindLambda([OnRemoveMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
    {
        OnRemoveMembersResponse.ExecuteIfBound(Result, MembersData, PageNext, PagePrev);
    });
    RemoveChannelMembersRaw(Channel, RemoveObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);
}

void UPubnubClient::RemoveChannelMembersRaw(FString Channel, FString RemoveObj, FOnPubnubRemoveChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
    PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubChannelMemberData>(), FString(), FString());

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

    PubnubCallsThread->AddFunctionToQueue([WeakThis, Channel, RemoveObj, NativeCallback, Include, Limit, Filter, Sort, PageNext, PagePrev, Count]
    {
		if(!WeakThis.IsValid())
		{return;}
		
        WeakThis.Get()->RemoveChannelMembers_priv(Channel, RemoveObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, PageNext, PagePrev, Count);
    });
}

void UPubnubClient::RemoveChannelMembers(FString Channel, TArray<FString> Users, FOnPubnubRemoveChannelMembersResponse OnRemoveMembersResponse, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    RemoveChannelMembersRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersToRemove(Users), OnRemoveMembersResponse, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::RemoveChannelMembers(FString Channel, TArray<FString> Users, FOnPubnubRemoveChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FString PageNext, FString PagePrev)
{
    RemoveChannelMembersRaw(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersToRemove(Users), NativeCallback, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), PageNext, PagePrev, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::AddMessageAction(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnPubnubAddMessageActionResponse OnAddMessageActionResponse)
{
	FOnPubnubAddMessageActionResponseNative NativeCallback;
	NativeCallback.BindLambda([OnAddMessageActionResponse](const FPubnubOperationResult& Result, FPubnubMessageActionData MessageActionData)
	{
		OnAddMessageActionResponse.ExecuteIfBound(Result, MessageActionData);
	});
	AddMessageAction(Channel, MessageTimetoken, ActionType, Value, NativeCallback);
}

void UPubnubClient::AddMessageAction(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnPubnubAddMessageActionResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubMessageActionData());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, MessageTimetoken, ActionType, Value, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->AddMessageAction_priv(Channel, MessageTimetoken, ActionType, Value, NativeCallback);
	});
}

void UPubnubClient::GetMessageActions(FString Channel, FOnPubnubGetMessageActionsResponse OnGetMessageActionsResponse, FString Start, FString End, int Limit)
{
	FOnPubnubGetMessageActionsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetMessageActionsResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMessageActionData>& MessageActions)
	{
		OnGetMessageActionsResponse.ExecuteIfBound(Result, MessageActions);
	});
	GetMessageActions(Channel, NativeCallback, Start, End, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit));
}

void UPubnubClient::GetMessageActions(FString Channel, FOnPubnubGetMessageActionsResponseNative NativeCallback, FString Start, FString End, int Limit)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubMessageActionData>());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, Start, End, Limit, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->GetMessageActions_priv(Channel, NativeCallback, Start, End, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit));
	});
}

void UPubnubClient::RemoveMessageAction(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnPubnubRemoveMessageActionResponse OnRemoveMessageActionResponse)
{
	FOnPubnubRemoveMessageActionResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveMessageActionResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveMessageActionResponse.ExecuteIfBound(Result);
	});
	RemoveMessageAction(Channel, MessageTimetoken, ActionTimetoken, NativeCallback);
}

void UPubnubClient::RemoveMessageAction(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnPubnubRemoveMessageActionResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, MessageTimetoken, ActionTimetoken, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->RemoveMessageAction_priv(Channel, MessageTimetoken, ActionTimetoken, NativeCallback);
	});
}

void UPubnubClient::ReconnectSubscriptions()
{
	pubnub_reconnect(ctx_ee, nullptr);
}

void UPubnubClient::DisconnectSubscriptions()
{
	pubnub_disconnect(ctx_ee);
}

void UPubnubClient::SetCryptoModule(TScriptInterface<IPubnubCryptoProviderInterface> CryptoModule)
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

TScriptInterface<IPubnubCryptoProviderInterface> UPubnubClient::GetCryptoModule()
{
	if(CryptoBridge)
	{
		return CryptoBridge->GetUECryptoModule();
	}

	return nullptr;
}

UPubnubChannelEntity* UPubnubClient::CreateChannelEntity(FString Channel)
{
	PUBNUB_RETURN_IF_FIELD_EMPTY(Channel, nullptr);
	
	UPubnubChannelEntity* ChannelEntity = NewObject<UPubnubChannelEntity>(this);
	ChannelEntity->InitEntity(this);
	ChannelEntity->EntityID = Channel;
	return ChannelEntity;
}

UPubnubChannelGroupEntity* UPubnubClient::CreateChannelGroupEntity(FString ChannelGroup)
{
	PUBNUB_RETURN_IF_FIELD_EMPTY(ChannelGroup, nullptr);
	
	UPubnubChannelGroupEntity* ChannelGroupEntity = NewObject<UPubnubChannelGroupEntity>(this);
	ChannelGroupEntity->InitEntity(this);
	ChannelGroupEntity->EntityID = ChannelGroup;
	return ChannelGroupEntity;
}

UPubnubChannelMetadataEntity* UPubnubClient::CreateChannelMetadataEntity(FString Channel)
{
	PUBNUB_RETURN_IF_FIELD_EMPTY(Channel, nullptr);
	
	UPubnubChannelMetadataEntity* ChannelMetadataEntity = NewObject<UPubnubChannelMetadataEntity>(this);
	ChannelMetadataEntity->InitEntity(this);
	ChannelMetadataEntity->EntityID = Channel;
	return ChannelMetadataEntity;
}

UPubnubUserMetadataEntity* UPubnubClient::CreateUserMetadataEntity(FString User)
{
	PUBNUB_RETURN_IF_FIELD_EMPTY(User, nullptr);
	
	UPubnubUserMetadataEntity* UserMetadataEntity = NewObject<UPubnubUserMetadataEntity>(this);
	UserMetadataEntity->InitEntity(this);
	UserMetadataEntity->EntityID = User;
	return UserMetadataEntity;
}

UPubnubSubscriptionSet* UPubnubClient::CreateSubscriptionSet(TArray<FString> Channels, TArray<FString> ChannelGroups, FPubnubSubscribeSettings SubscriptionSettings)
{
	if(Channels.IsEmpty() && ChannelGroups.IsEmpty())
	{
		PubnubError("[CreateSubscriptionSet]: at least one Channel or ChannelGroup is needed to create SubscriptionSet.", EPubnubErrorType::PET_Warning);
	}
	UPubnubSubscriptionSet* SubscriptionSet = NewObject<UPubnubSubscriptionSet>(this);
	SubscriptionSet->InitSubscriptionSet(this, Channels, ChannelGroups, SubscriptionSettings);
	return SubscriptionSet;
}

UPubnubSubscriptionSet* UPubnubClient::CreateSubscriptionSetFromEntities(TArray<UPubnubBaseEntity*> Entities, FPubnubSubscribeSettings SubscriptionSettings)
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

TArray<UPubnubSubscription*> UPubnubClient::GetActiveSubscriptions()
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

TArray<UPubnubSubscriptionSet*> UPubnubClient::GetActiveSubscriptionSets()
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

void UPubnubClient::InitWithConfig(UPubnubSubsystem* InPubnubSubsystem, FPubnubConfig InConfig, int InClientID, FString InDebugName )
{
	PubnubSubsystem = InPubnubSubsystem;
	ClientID = InClientID;
	DebugName = InDebugName;

	SavePubnubConfig(InConfig);
	
	InitPubnub_priv(InConfig);

	//If initialized correctly, create required thread.
	if(IsInitialized)
	{
		//Create new thread to queue all pubnub operations
		PubnubCallsThread = new FPubnubFunctionThread;
	}
}

void UPubnubClient::DeinitializeClient()
{
	if(!IsInitialized)
	{return;}

	if(PubnubCallsThread)
	{
		PubnubCallsThread->Stop();
	}
	
	//Unsubscribe from all channels and groups so this user will not be visible for others anymore
	UnsubscribeAllForDeinit();
	
	IsInitialized = false;
	PubnubSubsystem = nullptr;

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

		//Clean up C-core pubnub contexts
		pubnub_free(ctx_pub);
		pubnub_free(ctx_ee);
		ctx_pub = nullptr;
		ctx_ee = nullptr;
	}
	
	IsUserIDSet = false;
	delete[] AuthTokenBuffer;
	AuthTokenBuffer = nullptr;
	AuthTokenLength = 0;
	delete PubnubCallsThread;
	PubnubCallsThread = nullptr;

	SubscriptionResultDelegates.Empty();

	//Notify that Deinitialization is finished
	OnPubnubClientDeinitialized.Broadcast();
}

void UPubnubClient::DecryptHistoryMessages(TArray<FPubnubHistoryMessageData>& Messages)
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

void UPubnubClient::SavePubnubConfig(const FPubnubConfig& InConfig)
{
	PubnubConfig = InConfig;
	
	//Safely copy all keys using the utility function
	UPubnubUtilities::SafeCopyFStringToCharBuffer(PublishKey, PublishKeySize + 1, InConfig.PublishKey, TEXT("PublishKey"));
	UPubnubUtilities::SafeCopyFStringToCharBuffer(SubscribeKey, PublishKeySize + 1, InConfig.SubscribeKey, TEXT("SubscribeKey"));
	UPubnubUtilities::SafeCopyFStringToCharBuffer(SecretKey, SecretKeySize + 1, InConfig.SecretKey, TEXT("SecretKey"));
}

void UPubnubClient::OnCCoreSubscriptionStatusReceived(int StatusEnum, const void* StatusData)
{
	//Cast data back to C-Core types
	pubnub_subscription_status status = static_cast<pubnub_subscription_status>(StatusEnum);
	const pubnub_subscription_status_data_t* status_data = static_cast<const pubnub_subscription_status_data_t*>(StatusData);

	//Call and remove the first subscription result delegate in queue
	if(!SubscriptionResultDelegates.IsEmpty())
	{
		FPubnubOperationResult Result;
		Result.Error = status == PNSS_SUBSCRIPTION_STATUS_CONNECTION_ERROR || status == PNSS_SUBSCRIPTION_STATUS_DISCONNECTED_UNEXPECTEDLY;
		Result.Status = Result.Error ? 503 : 200;
		Result.ErrorMessage = pubnub_res_2_string(status_data->reason);
		SubscriptionResultDelegates[0].ExecuteIfBound(Result);

		SubscriptionResultDelegates.RemoveAt(0);
	}

	//Unlock the thread to proceed with queue - thread might be invalid if this is after deinitialization
	if(PubnubCallsThread)
	{
		PubnubCallsThread->UnlockAfterSubscriptionOperationFinished();
	}
	
	//Don't waste resources to translate data if there is no delegate bound to it
	if(!OnPubnubSubscriptionStatusChanged.IsBound() && !OnPubnubSubscriptionStatusChangedNative.IsBound())
	{return;}

	FPubnubSubscriptionStatusData SubscriptionStatusData;
	SubscriptionStatusData.Reason = pubnub_res_2_string(status_data->reason);

	//If status is disconnected we don't need to give subscribed channels
	if(status != PNSS_SUBSCRIPTION_STATUS_DISCONNECTED)
	{
		//Fill channels and channel groups data, from C-Core
		if (NULL != status_data->channels)
		{
			FUTF8ToTCHAR Converter(status_data->channels);
			FString Channels(Converter.Length(), Converter.Get());
			Channels.ParseIntoArray(SubscriptionStatusData.Channels, TEXT(","));
		}
		if (NULL != status_data->channel_groups)
		{
			FUTF8ToTCHAR Converter(status_data->channel_groups);
			FString ChannelGroups(Converter.Length(), Converter.Get());
			ChannelGroups.ParseIntoArray(SubscriptionStatusData.ChannelGroups, TEXT(","));
		}
	}

	//Call SubscriptionStatusChanged delegates 
	OnPubnubSubscriptionStatusChanged.Broadcast((EPubnubSubscriptionStatus)status, SubscriptionStatusData);
	OnPubnubSubscriptionStatusChangedNative.Broadcast((EPubnubSubscriptionStatus)status, SubscriptionStatusData);
}

FString UPubnubClient::GetLastResponse(pubnub_t* context)
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

FString UPubnubClient::GetLastChannelResponse(pubnub_t* context)
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

void UPubnubClient::InitPubnub_priv(const FPubnubConfig& Config)
{
	if(IsInitialized)
	{return;}
	
	//Make sure that keys are filled
	if(PublishKey[0] == '\0')
	{
		PubnubError("Publish key is empty, can't initialize Pubnub");
		return;
	}

	if(SubscribeKey[0] == '\0')
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
		UPubnubClient* ThisClient = static_cast<UPubnubClient*>(_data);
		if(!ThisClient)
		{return;}

		ThisClient->OnCCoreSubscriptionStatusReceived(status, &status_data);
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

void UPubnubClient::SetUserID_priv(FString UserID)
{
	PUBNUB_RETURN_IF_FIELD_EMPTY(UserID);

	FUTF8StringHolder UserIDHolder(UserID);
	pubnub_set_user_id(ctx_pub, UserIDHolder.Get());
	pubnub_set_user_id(ctx_ee, UserIDHolder.Get());

	IsUserIDSet = true;
}

FString UPubnubClient::GetUserID_priv()
{
	if(const char* UserIDChar = pubnub_user_id_get(ctx_pub))
	{
		FString UserIDString(UserIDChar);
		return UserIDString;
	}

	return "";
}

void UPubnubClient::SetSecretKey_priv()
{
	if(SecretKey[0] == '\0')
	{
		PubnubError("Can't set Secret Key. Secret Key is empty.");
		return;
	}
	
	pubnub_set_secret_key(ctx_pub, SecretKey);
	pubnub_set_secret_key(ctx_ee, SecretKey);
}


void UPubnubClient::PublishMessage_priv(FString Channel, FString Message, FOnPubnubPublishMessageResponseNative OnPublishMessageResponse, FPubnubPublishSettings PublishSettings)
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
	
	UPubnubInternalUtilities::PublishUESettingsToPubnubPublishOptions(PublishSettings, PubnubOptions);
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
		PublishedMessage.UserID = GetUserID_priv();
		PublishedMessage.Timetoken = pubnub_last_publish_timetoken(ctx_pub);
		PublishedMessage.Metadata = PublishSettings.MetaData;
		PublishedMessage.MessageType = EPubnubMessageType::PMT_Published;
		PublishedMessage.CustomMessageType = PublishSettings.CustomMessageType;
	}

	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnPublishMessageResponse, PublishResult, PublishedMessage);
}

void UPubnubClient::Signal_priv(FString Channel, FString Message, FOnPubnubSignalResponseNative OnSignalResponse, FPubnubSignalSettings SignalSettings)
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
		SignalMessage.UserID = GetUserID_priv();
		SignalMessage.Timetoken = pubnub_last_publish_timetoken(ctx_pub);
		SignalMessage.Metadata = ""; // Signals don't have metadata
		SignalMessage.MessageType = EPubnubMessageType::PMT_Signal;
		SignalMessage.CustomMessageType = SignalSettings.CustomMessageType;
	}

	//Execute provided delegate with results
	UPubnubUtilities::CallPubnubDelegate(OnSignalResponse, PublishResult, SignalMessage);
}


void UPubnubClient::SubscribeToChannel_priv(FString Channel, FOnPubnubSubscribeOperationResponseNative OnSubscribeToChannelResponse, FPubnubSubscribeSettings SubscribeSettings)
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
	PubnubCallsThread->LockForSubscribeOperation();

	//Create subscription for channel entity
	pubnub_subscription_t* Subscription = UPubnubInternalUtilities::EEGetSubscriptionForEntity(ctx_ee, Channel, EPubnubEntityType::PEnT_Channel, SubscribeSettings);

	if(nullptr == Subscription)
	{
		PubnubError("[SubscribeToChannel]: Failed to subscribe to channel. Pubnub_subscription_alloc didn't create subscription.");
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSubscribeToChannelResponse, "[SubscribeToChannel]: Failed to subscribe to channel. Pubnub_subscription_alloc didn't create subscription.");
		PubnubCallsThread->UnlockAfterSubscriptionOperationFinished();
		return;
	}
	
	//Create callback that will be triggered by the c-core event engine
	pubnub_subscribe_message_callback_t Callback = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		TWeakObjectPtr<UPubnubClient> ThisClientWeak = MakeWeakObjectPtr<UPubnubClient>(static_cast<UPubnubClient*>(user_data));
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, ThisClientWeak]()
		{
			if(ThisClientWeak.IsValid())
			{
				ThisClientWeak.Get()->OnPubnubMessageReceived.Broadcast(MessageData);
				ThisClientWeak.Get()->OnPubnubMessageReceivedNative.Broadcast(MessageData);
			}
		});
	};

	//Add subscription listener and subscribe with subscription
	if(!UPubnubInternalUtilities::EEAddListenerAndSubscribe(Subscription, Callback, this))
	{
		PubnubError("[SubscribeToChannel]: Failed to subscribe to channel.");
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSubscribeToChannelResponse, "[SubscribeToChannel]: Failed to subscribe to channel.");
		PubnubCallsThread->UnlockAfterSubscriptionOperationFinished();
		return;
	}

	//Save Callback and Subscription, so later we can use it to unsubscribe
	CCoreSubscriptionCallback* SubscriptionData = new CCoreSubscriptionCallback{Callback, Subscription};
	ChannelSubscriptions.Add(Channel, SubscriptionData);
}


void UPubnubClient::SubscribeToGroup_priv(FString ChannelGroup, FOnPubnubSubscribeOperationResponseNative OnSubscribeToGroupResponse, FPubnubSubscribeSettings SubscribeSettings)
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
	PubnubCallsThread->LockForSubscribeOperation();
	
	//Create subscription for channel group entity
	pubnub_subscription_t* Subscription = UPubnubInternalUtilities::EEGetSubscriptionForEntity(ctx_ee, ChannelGroup, EPubnubEntityType::PEnT_ChannelGroup, SubscribeSettings);

	if(nullptr == Subscription)
	{
		PubnubError("[SubscribeToGroup]: Failed to subscribe to channel group. Pubnub_subscription_alloc didn't create subscription.");
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSubscribeToGroupResponse, "[SubscribeToGroup]: Failed to subscribe to channel group. Pubnub_subscription_alloc didn't create subscription.");
		PubnubCallsThread->UnlockAfterSubscriptionOperationFinished();
		return;
	}

	//Create callback that will be triggered by the c-core event engine
	pubnub_subscribe_message_callback_t Callback = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		TWeakObjectPtr<UPubnubClient> ThisClientWeak = MakeWeakObjectPtr<UPubnubClient>(static_cast<UPubnubClient*>(user_data));
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, ThisClientWeak]()
		{
			if(ThisClientWeak.IsValid())
			{
				ThisClientWeak.Get()->OnPubnubMessageReceived.Broadcast(MessageData);
				ThisClientWeak.Get()->OnPubnubMessageReceivedNative.Broadcast(MessageData);
			}
		});
	};

	//Add subscription listener and subscribe with subscription
	if(!UPubnubInternalUtilities::EEAddListenerAndSubscribe(Subscription, Callback, this))
	{
		PubnubError("[SubscribeToGroup]: Failed to subscribe to channel group.");
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSubscribeToGroupResponse, "[SubscribeToGroup]: Failed to subscribe to channel group.");
		PubnubCallsThread->UnlockAfterSubscriptionOperationFinished();
		return;
	}

	//Save Callback and Subscription, so later we can use it to unsubscribe
	CCoreSubscriptionCallback* SubscriptionData = new CCoreSubscriptionCallback{Callback, Subscription};
	ChannelGroupSubscriptions.Add(ChannelGroup, SubscriptionData);
}

void UPubnubClient::UnsubscribeFromChannel_priv(FString Channel, FOnPubnubSubscribeOperationResponseNative OnUnsubscribeFromChannelResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnUnsubscribeFromChannelResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, OnUnsubscribeFromChannelResponse);
	
	CCoreSubscriptionCallback* SubscriptionData =  ChannelSubscriptions.FindRef(Channel);
	if(!SubscriptionData)
	{
		PubnubError("[UnsubscribeFromChannel]: There is no such subscription. Aborting operation.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnUnsubscribeFromChannelResponse, "[UnsubscribeFromChannel]: There is no such subscription. Aborting operation.");
		return;
	}

	//All subscription related operations are non blocking, so we lock ActionThread manually,
	//make it wait with calling other function until we have subscription result
	PubnubCallsThread->LockForSubscribeOperation();

	//Remove subscription listener and unsubscribe with subscription
	if(!UPubnubInternalUtilities::EERemoveListenerAndUnsubscribe(&SubscriptionData->Subscription, SubscriptionData->Callback, this))
	{
		PubnubError("[UnsubscribeFromChannel]: Failed to unsubscribe.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnUnsubscribeFromChannelResponse, "[UnsubscribeFromChannel]: Failed to unsubscribe.");
		PubnubCallsThread->UnlockAfterSubscriptionOperationFinished();
		return;
	}

	//Free subscription memory
	pubnub_subscription_free(&SubscriptionData->Subscription);

	ChannelSubscriptions.Remove(Channel);
}

void UPubnubClient::UnsubscribeFromGroup_priv(FString ChannelGroup, FOnPubnubSubscribeOperationResponseNative OnUnsubscribeFromGroupResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnUnsubscribeFromGroupResponse);
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(ChannelGroup, OnUnsubscribeFromGroupResponse);

	CCoreSubscriptionCallback* SubscriptionData =  ChannelGroupSubscriptions.FindRef(ChannelGroup);
	if(!SubscriptionData)
	{
		PubnubError("[UnsubscribeFromGroup]: There is no such subscription. Aborting operation.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnUnsubscribeFromGroupResponse, "[UnsubscribeFromGroup]: There is no such subscription. Aborting operation.");
		return;
	}

	//All subscription related operations are non blocking, so we lock ActionThread manually,
	//make it wait with calling other function until we have subscription result
	PubnubCallsThread->LockForSubscribeOperation();
	
	//Remove subscription listener and unsubscribe with subscription
	if(!UPubnubInternalUtilities::EERemoveListenerAndUnsubscribe(&SubscriptionData->Subscription, SubscriptionData->Callback, this))
	{
		PubnubError("[UnsubscribeFromGroup]: Failed to unsubscribe.", EPubnubErrorType::PET_Warning);
		UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnUnsubscribeFromGroupResponse, "[UnsubscribeFromGroup]: Failed to unsubscribe.");
		PubnubCallsThread->UnlockAfterSubscriptionOperationFinished();
		return;
	}

	//Free subscription memory
	pubnub_subscription_free(&SubscriptionData->Subscription);

	ChannelGroupSubscriptions.Remove(ChannelGroup);
}

void UPubnubClient::UnsubscribeFromAll_priv(FOnPubnubSubscribeOperationResponseNative OnUnsubscribeFromAllResponse)
{
	if(ChannelSubscriptions.IsEmpty() && ChannelGroupSubscriptions.IsEmpty())
	{
		UPubnubUtilities::CallPubnubDelegate(OnUnsubscribeFromAllResponse, FPubnubOperationResult({200, false, ""}));
		return;
	}

	PUBNUB_ENSURE_USER_ID_IS_SET(OnUnsubscribeFromAllResponse);

	//All subscription related operations are non blocking, so we lock ActionThread manually,
	//make it wait with calling other function until we have subscription result
	PubnubCallsThread->LockForSubscribeOperation();

	pubnub_unsubscribe_all(ctx_ee);
	
	CleanUpAllSubscriptions();
}


void UPubnubClient::AddChannelToGroup_priv(FString Channel, FString ChannelGroup, FOnPubnubAddChannelToGroupResponseNative OnAddChannelToGroupResponse)
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

void UPubnubClient::RemoveChannelFromGroup_priv(FString Channel, FString ChannelGroup, FOnPubnubRemoveChannelFromGroupResponseNative OnRemoveChannelFromGroupResponse)
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

void UPubnubClient::ListChannelsFromGroup_priv(FString ChannelGroup, FOnPubnubListChannelsFromGroupResponseNative OnListChannelsResponse)
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

void UPubnubClient::RemoveChannelGroup_priv(FString ChannelGroup, FOnPubnubRemoveChannelGroupResponseNative OnRemoveChannelGroupResponse)
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

void UPubnubClient::ListUsersFromChannel_priv(FString Channel, FOnPubnubListUsersFromChannelResponseNative ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(ListUsersFromChannelResponse, FPubnubListUsersFromChannelWrapper());
	PUBNUB_ENSURE_FIELD_NOT_EMPTY(Channel, ListUsersFromChannelResponse, FPubnubListUsersFromChannelWrapper());
	PUBNUB_ENSURE_CONDITION(ListUsersFromChannelSettings.Limit >= 0, TEXT("Limit can't be below 0."), ListUsersFromChannelResponse, FPubnubListUsersFromChannelWrapper());
	PUBNUB_ENSURE_CONDITION(ListUsersFromChannelSettings.Offset >= 0, TEXT("Offset can't be below 0."), ListUsersFromChannelResponse, FPubnubListUsersFromChannelWrapper());

	//Set all options from ListUsersFromChannelSettings
	FUTF8StringHolder ChannelHolder(Channel);
	
	//Converted char needs to live in function scope, so we need to create it here
	pubnub_here_now_options HereNowOptions;
	FUTF8StringHolder ChannelGroupHolder(ListUsersFromChannelSettings.ChannelGroup);
	HereNowOptions.channel_group = ChannelGroupHolder.Get();
	
	UPubnubInternalUtilities::HereNowUESettingsToPubnubHereNowOptions(ListUsersFromChannelSettings, HereNowOptions);
	
	pubnub_here_now_ex(ctx_pub, ChannelHolder.Get(), HereNowOptions);
	
	FString JsonResponse = GetLastResponse(ctx_pub);
	
	FPubnubOperationResult Result;
	
	//If response is empty, there was server error. 
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
		//Presence api doesn't provide status code in the response, so we need to get it manually
		Result.Status = pubnub_last_http_code(ctx_pub);
	}
	
	//Execute provided delegate with results
	FPubnubListUsersFromChannelWrapper Data;
	UPubnubJsonUtilities::ListUsersFromChannelJsonToData(JsonResponse, Result, Data);
	UPubnubUtilities::CallPubnubDelegate(ListUsersFromChannelResponse, Result, Data);
}

void UPubnubClient::ListUserSubscribedChannels_priv(FString UserID, FOnPubnubListUsersSubscribedChannelsResponseNative ListUserSubscribedChannelsResponse)
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

void UPubnubClient::SetState_priv(FString Channel, FString StateJson, FOnPubnubSetStateResponseNative OnSetStateResponse, FPubnubSetStateSettings SetStateSettings)
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

	UPubnubInternalUtilities::SetStateUESettingsToPubnubSetStateOptions(SetStateSettings, SetStateOptions);

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

void UPubnubClient::GetState_priv(FString Channel, FString ChannelGroup, FString UserID, FOnPubnubGetStateResponseNative OnGetStateResponse)
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

void UPubnubClient::Heartbeat_priv(FString Channel, FString ChannelGroup)
{
	PUBNUB_RETURN_IF_USER_ID_NOT_SET();
	
	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);
	FUTF8StringHolder ChannelHolder(Channel);

	pubnub_heartbeat(ctx_pub, ChannelHolder.Get(), ChannelGroupHolder.Get());

	GetLastResponse(ctx_pub);
}

void UPubnubClient::GrantToken_priv(FString PermissionObject, FOnPubnubGrantTokenResponseNative OnGrantTokenResponse)
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

void UPubnubClient::RevokeToken_priv(FString Token, FOnPubnubRevokeTokenResponseNative OnRevokeTokenResponse)
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

FString UPubnubClient::ParseToken_priv(FString Token)
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

void UPubnubClient::SetAuthToken_priv(FString Token)
{
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

void UPubnubClient::FetchHistory_priv(FString Channel, FOnPubnubFetchHistoryResponseNative OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings)
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

	UPubnubInternalUtilities::FetchHistoryUESettingsToPbFetchHistoryOptions(FetchHistorySettings, FetchHistoryOptions);

	FUTF8StringHolder ChannelHolder(Channel);
	
	pubnub_fetch_history(ctx_pub, ChannelHolder.Get(), FetchHistoryOptions);

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

void UPubnubClient::DeleteMessages_priv(FString Channel, FOnPubnubDeleteMessagesResponseNative OnDeleteMessagesResponse, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
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

void UPubnubClient::MessageCounts_priv(FString Channel, FString Timetoken, FOnPubnubMessageCountsResponseNative OnMessageCountsResponse)
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

void UPubnubClient::GetAllUserMetadata_priv(FOnPubnubGetAllUserMetadataResponseNative OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
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

void UPubnubClient::SetUserMetadata_priv(FString User, FString UserMetadataObj, FOnPubnubSetUserMetadataResponseNative OnSetUserMetadataResponse, FString Include)
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

void UPubnubClient::GetUserMetadata_priv(FString User, FOnPubnubGetUserMetadataResponseNative OnGetUserMetadataResponse, FString Include)
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

void UPubnubClient::RemoveUserMetadata_priv(FString User, FOnPubnubRemoveUserMetadataResponseNative OnRemoveUserMetadataResponse)
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

void UPubnubClient::GetAllChannelMetadata_priv(FOnPubnubGetAllChannelMetadataResponseNative OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
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

void UPubnubClient::SetChannelMetadata_priv(FString Channel, FString ChannelMetadataObj, FOnPubnubSetChannelMetadataResponseNative OnSetChannelMetadataResponse, FString Include)
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

void UPubnubClient::GetChannelMetadata_priv(FString Channel, FOnPubnubGetChannelMetadataResponseNative OnGetChannelMetadataResponse, FString Include)
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

void UPubnubClient::RemoveChannelMetadata_priv(FString Channel, FOnPubnubRemoveChannelMetadataResponseNative OnRemoveChannelMetadataResponse)
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

void UPubnubClient::GetMemberships_priv(FString User, FOnPubnubGetMembershipsResponseNative OnGetMembershipsResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
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

void UPubnubClient::SetMemberships_priv(FString User, FString SetObj, FOnPubnubSetMembershipsResponseNative OnSetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
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

void UPubnubClient::RemoveMemberships_priv(FString User, FString RemoveObj, FOnPubnubRemoveMembershipsResponseNative OnRemoveMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
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

void UPubnubClient::GetChannelMembers_priv(FString Channel, FOnPubnubGetChannelMembersResponseNative OnGetMembersResponse, FString Include, int Limit,
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

void UPubnubClient::SetChannelMembers_priv(FString Channel, FString SetObj, FOnPubnubSetChannelMembersResponseNative OnSetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
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

void UPubnubClient::RemoveChannelMembers_priv(FString Channel, FString RemoveObj, FOnPubnubRemoveChannelMembersResponseNative OnRemoveMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
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

void UPubnubClient::AddMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnPubnubAddMessageActionResponseNative AddMessageActionResponse)
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

void UPubnubClient::RemoveMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnPubnubRemoveMessageActionResponseNative OnRemoveMessageActionResponse)
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

void UPubnubClient::GetMessageActions_priv(FString Channel, FOnPubnubGetMessageActionsResponseNative OnGetMessageActionsResponse, FString Start, FString End, int Limit)
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


void UPubnubClient::SubscribeWithSubscription(UPubnubSubscription* Subscription, FPubnubSubscriptionCursor Cursor, FOnPubnubSubscribeOperationResponseNative OnSubscribeResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnSubscribeResponse);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(OnSubscribeResponse);

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Subscription, Cursor, OnSubscribeResponse]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		if(!UPubnubInternalUtilities::EESubscribeWithSubscription(Subscription->CCoreSubscription, Cursor))
		{
			WeakThis.Get()->PubnubError("[SubscribeWithSubscription]: Failed to subscribe with subscription..");
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSubscribeResponse, "[Subscribe]: Failed to subscribe with Subscription.");
			WeakThis.Get()->PubnubCallsThread->UnlockAfterSubscriptionOperationFinished();
		}
	});
}

void UPubnubClient::SubscribeWithSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet, FPubnubSubscriptionCursor Cursor, FOnPubnubSubscribeOperationResponseNative OnSubscribeResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnSubscribeResponse);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(OnSubscribeResponse);

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, SubscriptionSet, Cursor, OnSubscribeResponse]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		if(!UPubnubInternalUtilities::EESubscribeWithSubscriptionSet(SubscriptionSet->CCoreSubscriptionSet, Cursor))
		{
			WeakThis.Get()->PubnubError("[SubscribeWithSubscription]: Failed to subscribe with subscription..");
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnSubscribeResponse, "[Subscribe]: Failed to subscribe with Subscription.");
			WeakThis.Get()->PubnubCallsThread->UnlockAfterSubscriptionOperationFinished();
		}
	});
}

void UPubnubClient::UnsubscribeWithSubscription(UPubnubSubscription* Subscription, FOnPubnubSubscribeOperationResponseNative OnUnsubscribeResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnUnsubscribeResponse);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(OnUnsubscribeResponse);

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Subscription, OnUnsubscribeResponse]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		if(!UPubnubInternalUtilities::EEUnsubscribeWithSubscription(&Subscription->CCoreSubscription))
		{
			WeakThis.Get()->PubnubError("[SubscribeWithSubscription]: Failed to subscribe with subscription..");
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnUnsubscribeResponse, "[Subscribe]: Failed to subscribe with Subscription.");
			WeakThis.Get()->PubnubCallsThread->UnlockAfterSubscriptionOperationFinished();
		}
	});
}

void UPubnubClient::UnsubscribeWithSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet, FOnPubnubSubscribeOperationResponseNative OnUnsubscribeResponse)
{
	PUBNUB_ENSURE_USER_ID_IS_SET(OnUnsubscribeResponse);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(OnUnsubscribeResponse);

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, SubscriptionSet, OnUnsubscribeResponse]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		if(!UPubnubInternalUtilities::EEUnsubscribeWithSubscriptionSet(&SubscriptionSet->CCoreSubscriptionSet))
		{
			WeakThis.Get()->PubnubError("[SubscribeWithSubscription]: Failed to subscribe with subscription..");
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(OnUnsubscribeResponse, "[Subscribe]: Failed to subscribe with Subscription.");
			WeakThis.Get()->PubnubCallsThread->UnlockAfterSubscriptionOperationFinished();
		}
	});
}

void UPubnubClient::CleanUpAllSubscriptions()
{
	for(auto& Pair : ChannelSubscriptions)
	{
		if(Pair.Value)
		{
			if(Pair.Value->Subscription)
			{
				pubnub_subscription_free(&Pair.Value->Subscription);
			}
			delete Pair.Value;
		}
	}
	for(auto& Pair : ChannelGroupSubscriptions)
	{
		if(Pair.Value)
		{
			if(Pair.Value->Subscription)
			{
				pubnub_subscription_free(&Pair.Value->Subscription);
			}
			delete Pair.Value;
		}
	}

	ChannelSubscriptions.Empty();
	ChannelGroupSubscriptions.Empty();
}

void UPubnubClient::UnsubscribeAllForDeinit()
{
	if(ChannelSubscriptions.IsEmpty() && ChannelGroupSubscriptions.IsEmpty())
	{return;}
	
	pubnub_unsubscribe_all(ctx_ee);
	CleanUpAllSubscriptions();
}