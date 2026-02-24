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

FPubnubPublishMessageResult UPubnubClient::PublishMessage(FString Channel, FString Message, FPubnubPublishSettings PublishSettings)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubPublishMessageResult());
	
	return PublishMessage_priv(Channel, Message, PublishSettings);
}

void UPubnubClient::PublishMessageAsync(FString Channel, FString Message, FOnPubnubPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings)
{
	FOnPubnubPublishMessageResponseNative NativeCallback;
	NativeCallback.BindLambda([OnPublishMessageResponse](FPubnubOperationResult Result, FPubnubMessageData PublishedMessage)
	{
		OnPublishMessageResponse.ExecuteIfBound(Result, PublishedMessage);
	});

	PublishMessageAsync(Channel, Message, NativeCallback, PublishSettings);
}

void UPubnubClient::PublishMessageAsync(FString Channel, FString Message, FOnPubnubPublishMessageResponseNative NativeCallback, FPubnubPublishSettings PublishSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubMessageData());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, Message, NativeCallback, PublishSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubPublishMessageResult PublishMessageResult = WeakThis.Get()->PublishMessage_priv(Channel, Message, PublishSettings);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, PublishMessageResult.Result, PublishMessageResult.PublishedMessage);
	});
}

void UPubnubClient::PublishMessageAsync(FString Channel, FString Message, FPubnubPublishSettings PublishSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, Message, PublishSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->PublishMessage_priv(Channel, Message, PublishSettings);
	});
}

FPubnubSignalResult UPubnubClient::Signal(FString Channel, FString Message, FPubnubSignalSettings SignalSettings)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubSignalResult());
	
	return Signal_priv(Channel, Message, SignalSettings);
}

void UPubnubClient::SignalAsync(FString Channel, FString Message, FOnPubnubSignalResponse OnSignalResponse, FPubnubSignalSettings SignalSettings)
{
	FOnPubnubSignalResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSignalResponse](const FPubnubOperationResult& Result, const FPubnubMessageData& SignalMessage)
	{
		OnSignalResponse.ExecuteIfBound(Result, SignalMessage);
	});

	SignalAsync(Channel, Message, NativeCallback, SignalSettings);
}

void UPubnubClient::SignalAsync(FString Channel, FString Message, FOnPubnubSignalResponseNative NativeCallback, FPubnubSignalSettings SignalSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubMessageData());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, Message, NativeCallback, SignalSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubSignalResult SignalResult = WeakThis.Get()->Signal_priv(Channel, Message, SignalSettings);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, SignalResult.Result, SignalResult.SignalMessage);
	});
}

void UPubnubClient::SignalAsync(FString Channel, FString Message, FPubnubSignalSettings SignalSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, Message, SignalSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->Signal_priv(Channel, Message, SignalSettings);
	});
}

FPubnubOperationResult UPubnubClient::SubscribeToChannel(FString Channel, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	return SubscribeToChannel_priv(Channel, SubscribeSettings);
}


void UPubnubClient::SubscribeToChannelAsync(FString Channel, FOnPubnubSubscribeOperationResponse OnSubscribeToChannelResponse, FPubnubSubscribeSettings SubscribeSettings)
{
	FOnPubnubSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSubscribeToChannelResponse](FPubnubOperationResult Result)
	{
		OnSubscribeToChannelResponse.ExecuteIfBound(Result);
	});

	SubscribeToChannelAsync(Channel, NativeCallback, SubscribeSettings);
}

void UPubnubClient::SubscribeToChannelAsync(FString Channel, FOnPubnubSubscribeOperationResponseNative NativeCallback, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, NativeCallback, SubscribeSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult SubscribeResult = WeakThis.Get()->SubscribeToChannel_priv(Channel, SubscribeSettings);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, SubscribeResult);
	});
}

void UPubnubClient::SubscribeToChannelAsync(FString Channel, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	SubscribeToChannelAsync(Channel, nullptr, SubscribeSettings);
}

FPubnubOperationResult UPubnubClient::SubscribeToGroup(FString ChannelGroup, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	return SubscribeToGroup_priv(ChannelGroup, SubscribeSettings);
}

void UPubnubClient::SubscribeToGroupAsync(FString ChannelGroup, FOnPubnubSubscribeOperationResponse OnSubscribeToGroupResponse, FPubnubSubscribeSettings SubscribeSettings)
{
	FOnPubnubSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSubscribeToGroupResponse](FPubnubOperationResult Result)
	{
		OnSubscribeToGroupResponse.ExecuteIfBound(Result);
	});

	SubscribeToGroupAsync(ChannelGroup, NativeCallback, SubscribeSettings);
}

void UPubnubClient::SubscribeToGroupAsync(FString ChannelGroup, FOnPubnubSubscribeOperationResponseNative NativeCallback, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, ChannelGroup, NativeCallback, SubscribeSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult SubscribeResult = WeakThis.Get()->SubscribeToGroup_priv(ChannelGroup, SubscribeSettings);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, SubscribeResult);
	});
}

void UPubnubClient::SubscribeToGroupAsync(FString ChannelGroup, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	SubscribeToGroupAsync(ChannelGroup, nullptr, SubscribeSettings);
}

FPubnubOperationResult UPubnubClient::UnsubscribeFromChannel(FString Channel)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	return UnsubscribeFromChannel_priv(Channel);
}

void UPubnubClient::UnsubscribeFromChannelAsync(FString Channel, FOnPubnubSubscribeOperationResponse OnUnsubscribeFromChannelResponse)
{
	FOnPubnubSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeFromChannelResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromChannelResponse.ExecuteIfBound(Result);
	});

	UnsubscribeFromChannelAsync(Channel, NativeCallback);
}

void UPubnubClient::UnsubscribeFromChannelAsync(FString Channel, FOnPubnubSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult UnsubscribeResult = WeakThis.Get()->UnsubscribeFromChannel_priv(Channel);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, UnsubscribeResult);
	});
}

FPubnubOperationResult UPubnubClient::UnsubscribeFromGroup(FString ChannelGroup)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	return UnsubscribeFromGroup_priv(ChannelGroup);
}

void UPubnubClient::UnsubscribeFromGroupAsync(FString ChannelGroup, FOnPubnubSubscribeOperationResponse OnUnsubscribeFromGroupResponse)
{
	FOnPubnubSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeFromGroupResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromGroupResponse.ExecuteIfBound(Result);
	});

	UnsubscribeFromGroupAsync(ChannelGroup, NativeCallback);
}

void UPubnubClient::UnsubscribeFromGroupAsync(FString ChannelGroup, FOnPubnubSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, ChannelGroup, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult UnsubscribeResult = WeakThis.Get()->UnsubscribeFromGroup_priv(ChannelGroup);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, UnsubscribeResult);
	});
}

FPubnubOperationResult UPubnubClient::UnsubscribeFromAll()
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	return UnsubscribeFromAll_priv();
}

void UPubnubClient::UnsubscribeFromAllAsync(FOnPubnubSubscribeOperationResponse OnUnsubscribeFromAllResponse)
{
	FOnPubnubSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeFromAllResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromAllResponse.ExecuteIfBound(Result);
	});

	UnsubscribeFromAllAsync(NativeCallback);
}

void UPubnubClient::UnsubscribeFromAllAsync(FOnPubnubSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult UnsubscribeResult = WeakThis.Get()->UnsubscribeFromAll_priv();

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, UnsubscribeResult);
	});
}

FPubnubOperationResult UPubnubClient::AddChannelToGroup(FString Channel, FString ChannelGroup)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	
	return AddChannelToGroup_priv(Channel, ChannelGroup);
}

void UPubnubClient::AddChannelToGroupAsync(FString Channel, FString ChannelGroup, FOnPubnubAddChannelToGroupResponse OnAddChannelToGroupResponse)
{
	FOnPubnubAddChannelToGroupResponseNative NativeCallback;
	NativeCallback.BindLambda([OnAddChannelToGroupResponse](const FPubnubOperationResult& Result)
	{
		OnAddChannelToGroupResponse.ExecuteIfBound(Result);
	});
	AddChannelToGroupAsync(Channel, ChannelGroup, NativeCallback);
}

void UPubnubClient::AddChannelToGroupAsync(FString Channel, FString ChannelGroup, FOnPubnubAddChannelToGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, ChannelGroup, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult Result = WeakThis.Get()->AddChannelToGroup_priv(Channel, ChannelGroup);
		
		//Execute provided delegate with results
        UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result);
	});
}

FPubnubOperationResult UPubnubClient::RemoveChannelFromGroup(FString Channel, FString ChannelGroup)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	
	return RemoveChannelFromGroup_priv(Channel, ChannelGroup);
}

void UPubnubClient::RemoveChannelFromGroupAsync(FString Channel, FString ChannelGroup, FOnPubnubRemoveChannelFromGroupResponse OnRemoveChannelFromGroupResponse)
{
	FOnPubnubRemoveChannelFromGroupResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveChannelFromGroupResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveChannelFromGroupResponse.ExecuteIfBound(Result);
	});
	RemoveChannelFromGroupAsync(Channel, ChannelGroup, NativeCallback);
}

void UPubnubClient::RemoveChannelFromGroupAsync(FString Channel, FString ChannelGroup, FOnPubnubRemoveChannelFromGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, ChannelGroup, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult Result = WeakThis.Get()->RemoveChannelFromGroup_priv(Channel, ChannelGroup);
		
		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result);
	});
}

FPubnubListChannelsFromGroupResult UPubnubClient::ListChannelsFromGroup(FString ChannelGroup)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubListChannelsFromGroupResult());
	
	return ListChannelsFromGroup_priv(ChannelGroup);
}

void UPubnubClient::ListChannelsFromGroupAsync(FString ChannelGroup, FOnPubnubListChannelsFromGroupResponse OnListChannelsResponse)
{
	FOnPubnubListChannelsFromGroupResponseNative NativeCallback;
	NativeCallback.BindLambda([OnListChannelsResponse](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		OnListChannelsResponse.ExecuteIfBound(Result, Channels);
	});

	ListChannelsFromGroupAsync(ChannelGroup, NativeCallback);
}

void UPubnubClient::ListChannelsFromGroupAsync(FString ChannelGroup, FOnPubnubListChannelsFromGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FString>{});
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, ChannelGroup, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubListChannelsFromGroupResult Result = WeakThis.Get()->ListChannelsFromGroup_priv(ChannelGroup);
		
		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result.Result, Result.Channels);
	});
}

FPubnubOperationResult UPubnubClient::RemoveChannelGroup(FString ChannelGroup)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	
	return RemoveChannelGroup_priv(ChannelGroup);
}

void UPubnubClient::RemoveChannelGroupAsync(FString ChannelGroup, FOnPubnubRemoveChannelGroupResponse OnRemoveChannelGroupResponse)
{
	FOnPubnubRemoveChannelGroupResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveChannelGroupResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveChannelGroupResponse.ExecuteIfBound(Result);
	});
	RemoveChannelGroupAsync(ChannelGroup, NativeCallback);
}

void UPubnubClient::RemoveChannelGroupAsync(FString ChannelGroup, FOnPubnubRemoveChannelGroupResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, ChannelGroup, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult Result = WeakThis.Get()->RemoveChannelGroup_priv(ChannelGroup);
		
		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result);
	});
}

FPubnubListUsersFromChannelResult UPubnubClient::ListUsersFromChannel(FString Channel, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubListUsersFromChannelResult());
	
	return ListUsersFromChannel_priv(Channel, ListUsersFromChannelSettings);
}

void UPubnubClient::ListUsersFromChannelAsync(FString Channel, FOnPubnubListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	FOnPubnubListUsersFromChannelResponseNative NativeCallback;
	NativeCallback.BindLambda([ListUsersFromChannelResponse](const FPubnubOperationResult& Result, FPubnubListUsersFromChannelWrapper Data)
	{
		ListUsersFromChannelResponse.ExecuteIfBound(Result, Data);
	});

	ListUsersFromChannelAsync(Channel, NativeCallback, ListUsersFromChannelSettings);
}

void UPubnubClient::ListUsersFromChannelAsync(FString Channel, FOnPubnubListUsersFromChannelResponseNative NativeCallback, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubListUsersFromChannelWrapper());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, NativeCallback, ListUsersFromChannelSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubListUsersFromChannelResult Result = WeakThis.Get()->ListUsersFromChannel_priv(Channel, ListUsersFromChannelSettings);
		
		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result.Result, Result.Data);
	});
}

FPubnubListUsersSubscribedChannelsResult UPubnubClient::ListUserSubscribedChannels(FString UserID)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubListUsersSubscribedChannelsResult());
	
	return ListUserSubscribedChannels_priv(UserID);
}

void UPubnubClient::ListUserSubscribedChannelsAsync(FString UserID, FOnPubnubListUsersSubscribedChannelsResponse ListUserSubscribedChannelsResponse)
{
	FOnPubnubListUsersSubscribedChannelsResponseNative NativeCallback;
	NativeCallback.BindLambda([ListUserSubscribedChannelsResponse](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		ListUserSubscribedChannelsResponse.ExecuteIfBound(Result, Channels);
	});

	ListUserSubscribedChannelsAsync(UserID, NativeCallback);
}

void UPubnubClient::ListUserSubscribedChannelsAsync(FString UserID, FOnPubnubListUsersSubscribedChannelsResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FString>{});
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, UserID, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubListUsersSubscribedChannelsResult Result = WeakThis.Get()->ListUserSubscribedChannels_priv(UserID);
		
		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result.Result, Result.Channels);
	});
}

FPubnubOperationResult UPubnubClient::SetState(FString Channel, FString StateJson, FPubnubSetStateSettings SetStateSettings)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	
	return SetState_priv(Channel, StateJson, SetStateSettings);
}

void UPubnubClient::SetStateAsync(FString Channel, FString StateJson, FOnPubnubSetStateResponse OnSetStateResponse, FPubnubSetStateSettings SetStateSettings)
{
	FOnPubnubSetStateResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSetStateResponse](const FPubnubOperationResult& Result)
	{
		OnSetStateResponse.ExecuteIfBound(Result);
	});
	SetStateAsync(Channel, StateJson, NativeCallback, SetStateSettings);
}

void UPubnubClient::SetStateAsync(FString Channel, FString StateJson, FOnPubnubSetStateResponseNative NativeCallback, FPubnubSetStateSettings SetStateSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, StateJson, NativeCallback, SetStateSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult Result = WeakThis.Get()->SetState_priv(Channel, StateJson, SetStateSettings);
		
		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result);
	});
}

void UPubnubClient::SetStateAsync(FString Channel, FString StateJson, FPubnubSetStateSettings SetStateSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, StateJson, SetStateSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->SetState_priv(Channel, StateJson, SetStateSettings);
	});
}

FPubnubGetStateResult UPubnubClient::GetState(FString Channel, FString ChannelGroup, FString UserID)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubGetStateResult());
	
	return GetState_priv(Channel, ChannelGroup, UserID);
}

void UPubnubClient::GetStateAsync(FString Channel, FString ChannelGroup, FString UserID, FOnPubnubGetStateResponse OnGetStateResponse)
{
	FOnPubnubGetStateResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetStateResponse](const FPubnubOperationResult& Result, FString JsonResponse)
	{
		OnGetStateResponse.ExecuteIfBound(Result, JsonResponse);
	});

	GetStateAsync(Channel, ChannelGroup, UserID, NativeCallback);
}

void UPubnubClient::GetStateAsync(FString Channel, FString ChannelGroup, FString UserID, FOnPubnubGetStateResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FString());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, ChannelGroup, UserID, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubGetStateResult Result = WeakThis.Get()->GetState_priv(Channel, ChannelGroup, UserID);
		
		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result.Result, Result.StateResponse);
	});
}

FPubnubOperationResult UPubnubClient::Heartbeat(FString Channel, FString ChannelGroup)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	
	return Heartbeat_priv(Channel, ChannelGroup);
}

void UPubnubClient::HeartbeatAsync(FString Channel, FString ChannelGroup)
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

FPubnubGrantTokenResult UPubnubClient::GrantToken(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FString Meta)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubGrantTokenResult());
	
	return GrantToken_priv(UPubnubTokenUtilities::CreateGrantTokenPermissionObjectString(Ttl, AuthorizedUser, Permissions, Meta));
}

void UPubnubClient::GrantTokenAsync(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FOnPubnubGrantTokenResponse OnGrantTokenResponse, FString Meta)
{
	FOnPubnubGrantTokenResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGrantTokenResponse](const FPubnubOperationResult& Result, FString Token)
	{
		OnGrantTokenResponse.ExecuteIfBound(Result, Token);
	});

	GrantTokenAsync(Ttl, AuthorizedUser, Permissions, NativeCallback, Meta);
}

void UPubnubClient::GrantTokenAsync(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FOnPubnubGrantTokenResponseNative NativeCallback, FString Meta)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FString());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Ttl, AuthorizedUser, Permissions, NativeCallback, Meta]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubGrantTokenResult Result = WeakThis.Get()->GrantToken_priv(UPubnubTokenUtilities::CreateGrantTokenPermissionObjectString(Ttl, AuthorizedUser, Permissions, Meta));
		
		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result.Result, Result.Token);
	});
}

FPubnubOperationResult UPubnubClient::RevokeToken(FString Token)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	
	return RevokeToken_priv(Token);
}

void UPubnubClient::RevokeTokenAsync(FString Token, FOnPubnubRevokeTokenResponse OnRevokeTokenResponse)
{
	FOnPubnubRevokeTokenResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRevokeTokenResponse](const FPubnubOperationResult& Result)
	{
		OnRevokeTokenResponse.ExecuteIfBound(Result);
	});
	RevokeTokenAsync(Token, NativeCallback);
}

void UPubnubClient::RevokeTokenAsync(FString Token, FOnPubnubRevokeTokenResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Token, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult Result = WeakThis.Get()->RevokeToken_priv(Token);
		
		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result);
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

int UPubnubClient::SetOrigin(FString Origin)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED(-1);

	return SetOrigin_priv(Origin);
}

FString UPubnubClient::GetOrigin() const
{
	return pubnub_get_origin(ctx_pub);
}

FPubnubFetchHistoryResult UPubnubClient::FetchHistory(FString Channel, FPubnubFetchHistorySettings FetchHistorySettings)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubFetchHistoryResult());
	
	return FetchHistory_priv(Channel, FetchHistorySettings);
}

void UPubnubClient::FetchHistoryAsync(FString Channel, FOnPubnubFetchHistoryResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings)
{
	FOnPubnubFetchHistoryResponseNative NativeCallback;
	NativeCallback.BindLambda([OnFetchHistoryResponse](const FPubnubOperationResult& Result, const TArray<FPubnubHistoryMessageData>& Messages)
	{
		OnFetchHistoryResponse.ExecuteIfBound(Result, Messages);
	});

	FetchHistoryAsync(Channel, NativeCallback, FetchHistorySettings);
}

void UPubnubClient::FetchHistoryAsync(FString Channel, FOnPubnubFetchHistoryResponseNative NativeCallback, FPubnubFetchHistorySettings FetchHistorySettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubHistoryMessageData>());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, NativeCallback, FetchHistorySettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubFetchHistoryResult Result = WeakThis.Get()->FetchHistory_priv(Channel, FetchHistorySettings);
		
		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result.Result, Result.Messages);
	});
}

FPubnubOperationResult UPubnubClient::DeleteMessages(FString Channel, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	
	return DeleteMessages_priv(Channel, DeleteMessagesSettings);
}

void UPubnubClient::DeleteMessagesAsync(FString Channel, FOnPubnubDeleteMessagesResponse OnDeleteMessagesResponse, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	FOnPubnubDeleteMessagesResponseNative NativeCallback;
	NativeCallback.BindLambda([OnDeleteMessagesResponse](FPubnubOperationResult Result)
	{
		OnDeleteMessagesResponse.ExecuteIfBound(Result);
	});

	DeleteMessagesAsync(Channel, NativeCallback, DeleteMessagesSettings);
}

void UPubnubClient::DeleteMessagesAsync(FString Channel, FOnPubnubDeleteMessagesResponseNative NativeCallback, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, NativeCallback, DeleteMessagesSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult Result = WeakThis.Get()->DeleteMessages_priv(Channel, DeleteMessagesSettings);
		
		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result);
	});
}

void UPubnubClient::DeleteMessagesAsync(FString Channel, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, DeleteMessagesSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		WeakThis.Get()->DeleteMessages_priv(Channel, DeleteMessagesSettings);
	});
}

FPubnubMessageCountsResult UPubnubClient::MessageCounts(FString Channel, FString Timetoken)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubMessageCountsResult());
	
	return MessageCounts_priv(Channel, Timetoken);
}

void UPubnubClient::MessageCountsAsync(FString Channel, FString Timetoken, FOnPubnubMessageCountsResponse OnMessageCountsResponse)
{
	FOnPubnubMessageCountsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnMessageCountsResponse](const FPubnubOperationResult& Result, int MessageCounts)
	{
		OnMessageCountsResponse.ExecuteIfBound(Result, MessageCounts);
	});

	MessageCountsAsync(Channel, Timetoken, NativeCallback);
}

void UPubnubClient::MessageCountsAsync(FString Channel, FString Timetoken, FOnPubnubMessageCountsResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, 0);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, Timetoken, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubMessageCountsResult Result = WeakThis.Get()->MessageCounts_priv(Channel, Timetoken);
		
		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result.Result, Result.MessageCounts);
	});
}

FPubnubMessageCountsMultipleResult UPubnubClient::MessageCountsMultiple(TArray<FString> Channels, TArray<FString> Timetokens)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubMessageCountsMultipleResult());
	
	return MessageCountsMultiple_priv(Channels, Timetokens);
}

void UPubnubClient::MessageCountsMultipleAsync(TArray<FString> Channels, TArray<FString> Timetokens, FOnPubnubMessageCountsMultipleResponse OnMessageCountsMultipleResponse)
{
	FOnPubnubMessageCountsMultipleResponseNative NativeCallback;
	NativeCallback.BindLambda([OnMessageCountsMultipleResponse](const FPubnubMessageCountsMultipleResult& Result)
	{
		OnMessageCountsMultipleResponse.ExecuteIfBound(Result);
	});

	MessageCountsMultipleAsync(Channels, Timetokens, NativeCallback);
}

void UPubnubClient::MessageCountsMultipleAsync(TArray<FString> Channels, TArray<FString> Timetokens, FOnPubnubMessageCountsMultipleResponseNative NativeCallback)
{
	if (!IsInitialized)
	{
		PubnubError(FString::Printf(TEXT("[%s]: PubnubClient is not initialized. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__))));
		FPubnubMessageCountsMultipleResult ErrorResult;
		ErrorResult.Result.Error = true;
		ErrorResult.Result.ErrorMessage = TEXT("PubnubClient is not initialized.");
		if (NativeCallback.IsBound())
		{
			NativeCallback.Execute(ErrorResult);
		}
		return;
	}
	if (!PubnubCallsThread)
	{
		PubnubError(FString::Printf(TEXT("[%s]: PubnubCallsThread is invalid. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__))));
		FPubnubMessageCountsMultipleResult ErrorResult;
		ErrorResult.Result.Error = true;
		ErrorResult.Result.ErrorMessage = TEXT("PubnubCallsThread is invalid.");
		if (NativeCallback.IsBound())
		{
			NativeCallback.Execute(ErrorResult);
		}
		return;
	}
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channels, Timetokens, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubMessageCountsMultipleResult Result = WeakThis.Get()->MessageCountsMultiple_priv(Channels, Timetokens);
		
		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result);
	});
}

FPubnubGetAllUserMetadataResult UPubnubClient::GetAllUserMetadataRaw(FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubGetAllUserMetadataResult());
	
	return GetAllUserMetadata_priv(Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::GetAllUserMetadataRawAsync(FOnPubnubGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FOnPubnubGetAllUserMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetAllUserMetadataResponse](const FPubnubOperationResult& Result, const TArray<FPubnubUserData>& UsersData, FPubnubPage Page, int TotalCount)
	{
		OnGetAllUserMetadataResponse.ExecuteIfBound(Result, UsersData, Page, TotalCount);
	});
	GetAllUserMetadataRawAsync(NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::GetAllUserMetadataRawAsync(FOnPubnubGetAllUserMetadataResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubUserData>(), FPubnubPage(), 0);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, NativeCallback, Include, Limit, Filter, Sort, Page,  Count]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubGetAllUserMetadataResult GetAllUserMetadataResult = WeakThis.Get()->GetAllUserMetadata_priv(Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page,  Count);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, GetAllUserMetadataResult.Result, GetAllUserMetadataResult.UsersData, GetAllUserMetadataResult.Page, GetAllUserMetadataResult.TotalCount);
	});
}

FPubnubGetAllUserMetadataResult UPubnubClient::GetAllUserMetadata(FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FPubnubPage Page)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubGetAllUserMetadataResult());
	
	return GetAllUserMetadata_priv(UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetAllUserMetadataAsync(FOnPubnubGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FPubnubPage Page)
{
	GetAllUserMetadataRawAsync(OnGetAllUserMetadataResponse, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), Page,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetAllUserMetadataAsync(FOnPubnubGetAllUserMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FPubnubPage Page)
{
	GetAllUserMetadataRawAsync(NativeCallback, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), Page,  (EPubnubTribool)Include.IncludeTotalCount);
}

FPubnubUserMetadataResult UPubnubClient::SetUserMetadataRaw(FString User, FString UserMetadataObj, FString Include)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubUserMetadataResult());
	
	return SetUserMetadata_priv(User, UserMetadataObj, Include);
}

void UPubnubClient::SetUserMetadataRawAsync(FString User, FString UserMetadataObj, FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse, FString Include)
{
	FOnPubnubSetUserMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSetUserMetadataResponse](const FPubnubOperationResult& Result, FPubnubUserData UserData)
	{
		OnSetUserMetadataResponse.ExecuteIfBound(Result, UserData);
	});
	SetUserMetadataRawAsync(User, UserMetadataObj, NativeCallback, Include);
}

void UPubnubClient::SetUserMetadataRawAsync(FString User, FString UserMetadataObj, FOnPubnubSetUserMetadataResponseNative NativeCallback, FString Include)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubUserData());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, User, UserMetadataObj, NativeCallback, Include]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubUserMetadataResult SetUserMetadataResult = WeakThis.Get()->SetUserMetadata_priv(User, UserMetadataObj, Include);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, SetUserMetadataResult.Result, SetUserMetadataResult.UserData);
	});
}

FPubnubUserMetadataResult UPubnubClient::SetUserMetadata(FString User, FPubnubUserInputData UserMetadata, FPubnubGetMetadataInclude Include)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubUserMetadataResult());
	
	return SetUserMetadata_priv(User, UPubnubJsonUtilities::GetJsonFromUserData(User, UserMetadata), UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubClient::SetUserMetadataAsync(FString User, FPubnubUserInputData UserMetadata, FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	SetUserMetadataRawAsync(User, UPubnubJsonUtilities::GetJsonFromUserData(User, UserMetadata), OnSetUserMetadataResponse, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubClient::SetUserMetadataAsync(FString User, FPubnubUserInputData UserMetadata, FOnPubnubSetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	SetUserMetadataRawAsync(User, UPubnubJsonUtilities::GetJsonFromUserData(User, UserMetadata), NativeCallback, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

FPubnubUserMetadataResult UPubnubClient::GetUserMetadataRaw(FString User, FString Include)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubUserMetadataResult());
	
	return GetUserMetadata_priv(User, Include);
}

void UPubnubClient::GetUserMetadataRawAsync(FString User, FOnPubnubGetUserMetadataResponse OnGetUserMetadataResponse, FString Include)
{
	FOnPubnubGetUserMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetUserMetadataResponse](const FPubnubOperationResult& Result, FPubnubUserData UserData)
	{
		OnGetUserMetadataResponse.ExecuteIfBound(Result, UserData);
	});
	GetUserMetadataRawAsync(User, NativeCallback, Include);
}

void UPubnubClient::GetUserMetadataRawAsync(FString User, FOnPubnubGetUserMetadataResponseNative NativeCallback, FString Include)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubUserData());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, User, NativeCallback, Include]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubUserMetadataResult GetUserMetadataResult = WeakThis.Get()->GetUserMetadata_priv(User, Include);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, GetUserMetadataResult.Result, GetUserMetadataResult.UserData);
	});
}

FPubnubUserMetadataResult UPubnubClient::GetUserMetadata(FString User, FPubnubGetMetadataInclude Include)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubUserMetadataResult());
	
	return GetUserMetadata_priv(User, UPubnubUtilities::GetMetadataIncludeToString(Include));
}


void UPubnubClient::GetUserMetadataAsync(FString User, FOnPubnubGetUserMetadataResponse OnGetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	GetUserMetadataRawAsync(User, OnGetUserMetadataResponse, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubClient::GetUserMetadataAsync(FString User, FOnPubnubGetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	GetUserMetadataRawAsync(User, NativeCallback, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

FPubnubOperationResult UPubnubClient::RemoveUserMetadata(FString User)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	
	return RemoveUserMetadata_priv(User);
}

void UPubnubClient::RemoveUserMetadataAsync(FString User, FOnPubnubRemoveUserMetadataResponse OnRemoveUserMetadataResponse)
{
	FOnPubnubRemoveUserMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveUserMetadataResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveUserMetadataResponse.ExecuteIfBound(Result);
	});
	RemoveUserMetadataAsync(User, NativeCallback);
}

void UPubnubClient::RemoveUserMetadataAsync(FString User, FOnPubnubRemoveUserMetadataResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, User, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult RemoveUserMetadataResult = WeakThis.Get()->RemoveUserMetadata_priv(User);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, RemoveUserMetadataResult);
	});
}

FPubnubGetAllChannelMetadataResult UPubnubClient::GetAllChannelMetadataRaw(FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubGetAllChannelMetadataResult());
	
	return GetAllChannelMetadata_priv(Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::GetAllChannelMetadataRawAsync(FOnPubnubGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FOnPubnubGetAllChannelMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetAllChannelMetadataResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelData>& ChannelsData, FPubnubPage Page, int TotalCount)
	{
		OnGetAllChannelMetadataResponse.ExecuteIfBound(Result, ChannelsData, Page, TotalCount);
	});
	GetAllChannelMetadataRawAsync(NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::GetAllChannelMetadataRawAsync(FOnPubnubGetAllChannelMetadataResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubChannelData>(), FPubnubPage(), 0);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, NativeCallback, Include, Limit, Filter, Sort, Page,  Count]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubGetAllChannelMetadataResult GetAllChannelMetadataResult = WeakThis.Get()->GetAllChannelMetadata_priv(Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page,  Count);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, GetAllChannelMetadataResult.Result, GetAllChannelMetadataResult.ChannelsData, GetAllChannelMetadataResult.Page, GetAllChannelMetadataResult.TotalCount);
	});
}

FPubnubGetAllChannelMetadataResult UPubnubClient::GetAllChannelMetadata(FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FPubnubPage Page)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubGetAllChannelMetadataResult());
	
	return GetAllChannelMetadata_priv(UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetAllChannelMetadataAsync(FOnPubnubGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FPubnubPage Page)
{
	GetAllChannelMetadataRawAsync(OnGetAllChannelMetadataResponse, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), Page,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetAllChannelMetadataAsync(FOnPubnubGetAllChannelMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FPubnubPage Page)
{
	GetAllChannelMetadataRawAsync(NativeCallback, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), Page,  (EPubnubTribool)Include.IncludeTotalCount);
}

FPubnubChannelMetadataResult UPubnubClient::SetChannelMetadataRaw(FString Channel, FString ChannelMetadataObj, FString Include)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubChannelMetadataResult());
	
	return SetChannelMetadata_priv(Channel, ChannelMetadataObj, Include);
}

void UPubnubClient::SetChannelMetadataRawAsync(FString Channel, FString ChannelMetadataObj, FOnPubnubSetChannelMetadataResponse OnSetChannelMetadataResponse, FString Include)
{
	FOnPubnubSetChannelMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSetChannelMetadataResponse](const FPubnubOperationResult& Result, FPubnubChannelData ChannelData)
	{
		OnSetChannelMetadataResponse.ExecuteIfBound(Result, ChannelData);
	});
	SetChannelMetadataRawAsync(Channel, ChannelMetadataObj, NativeCallback, Include);
}

void UPubnubClient::SetChannelMetadataRawAsync(FString Channel, FString ChannelMetadataObj, FOnPubnubSetChannelMetadataResponseNative NativeCallback, FString Include)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubChannelData());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, ChannelMetadataObj, NativeCallback, Include]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubChannelMetadataResult SetChannelMetadataResult = WeakThis.Get()->SetChannelMetadata_priv(Channel, ChannelMetadataObj, Include);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, SetChannelMetadataResult.Result, SetChannelMetadataResult.ChannelData);
	});
}

FPubnubChannelMetadataResult UPubnubClient::SetChannelMetadata(FString Channel, FPubnubChannelInputData ChannelMetadata, FPubnubGetMetadataInclude Include)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubChannelMetadataResult());
	
	return SetChannelMetadata_priv(Channel, UPubnubJsonUtilities::GetJsonFromChannelData(Channel, ChannelMetadata), UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubClient::SetChannelMetadataAsync(FString Channel, FPubnubChannelInputData ChannelMetadata, FOnPubnubSetChannelMetadataResponse OnSetChannelMetadataResponse, FPubnubGetMetadataInclude Include)
{
	SetChannelMetadataRawAsync(Channel, UPubnubJsonUtilities::GetJsonFromChannelData(Channel, ChannelMetadata), OnSetChannelMetadataResponse, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubClient::SetChannelMetadataAsync(FString Channel, FPubnubChannelInputData ChannelMetadata, FOnPubnubSetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	SetChannelMetadataRawAsync(Channel, UPubnubJsonUtilities::GetJsonFromChannelData(Channel, ChannelMetadata), NativeCallback, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

FPubnubChannelMetadataResult UPubnubClient::GetChannelMetadataRaw(FString Channel, FString Include)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubChannelMetadataResult());
	
	return GetChannelMetadata_priv(Channel, Include);
}

void UPubnubClient::GetChannelMetadataRawAsync(FString Channel, FOnPubnubGetChannelMetadataResponse OnGetChannelMetadataResponse, FString Include)
{
    FOnPubnubGetChannelMetadataResponseNative NativeCallback;
    NativeCallback.BindLambda([OnGetChannelMetadataResponse](const FPubnubOperationResult& Result, FPubnubChannelData ChannelData)
    {
        OnGetChannelMetadataResponse.ExecuteIfBound(Result, ChannelData);
    });
    GetChannelMetadataRawAsync(Channel, NativeCallback, Include);
}

void UPubnubClient::GetChannelMetadataRawAsync(FString Channel, FOnPubnubGetChannelMetadataResponseNative NativeCallback, FString Include)
{
    PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubChannelData());

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

    PubnubCallsThread->AddFunctionToQueue([WeakThis, Channel, NativeCallback, Include]
    {
		if(!WeakThis.IsValid())
		{return;}
		
        FPubnubChannelMetadataResult GetChannelMetadataResult = WeakThis.Get()->GetChannelMetadata_priv(Channel, Include);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, GetChannelMetadataResult.Result, GetChannelMetadataResult.ChannelData);
    });
}

FPubnubChannelMetadataResult UPubnubClient::GetChannelMetadata(FString Channel, FPubnubGetMetadataInclude Include)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubChannelMetadataResult());
	
	return GetChannelMetadata_priv(Channel, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubClient::GetChannelMetadataAsync(FString Channel, FOnPubnubGetChannelMetadataResponse OnGetChannelMetadataResponse, FPubnubGetMetadataInclude Include)
{
	GetChannelMetadataRawAsync(Channel, OnGetChannelMetadataResponse, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

void UPubnubClient::GetChannelMetadataAsync(FString Channel, FOnPubnubGetChannelMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	GetChannelMetadataRawAsync(Channel, NativeCallback, UPubnubUtilities::GetMetadataIncludeToString(Include));
}

FPubnubOperationResult UPubnubClient::RemoveChannelMetadata(FString Channel)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	
	return RemoveChannelMetadata_priv(Channel);
}

void UPubnubClient::RemoveChannelMetadataAsync(FString Channel, FOnPubnubRemoveChannelMetadataResponse OnRemoveChannelMetadataResponse)
{
	FOnPubnubRemoveChannelMetadataResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveChannelMetadataResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveChannelMetadataResponse.ExecuteIfBound(Result);
	});
	RemoveChannelMetadataAsync(Channel, NativeCallback);
}

void UPubnubClient::RemoveChannelMetadataAsync(FString Channel, FOnPubnubRemoveChannelMetadataResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult RemoveChannelMetadataResult = WeakThis.Get()->RemoveChannelMetadata_priv(Channel);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, RemoveChannelMetadataResult);
	});
}

FPubnubMembershipsResult UPubnubClient::GetMembershipsRaw(FString User, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubMembershipsResult());
	return GetMemberships_priv(User, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::GetMembershipsRawAsync(FString User, FOnPubnubGetMembershipsResponse OnGetMembershipsResponse, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FOnPubnubGetMembershipsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetMembershipsResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
	{
		OnGetMembershipsResponse.ExecuteIfBound(Result, MembershipsData, Page, TotalCount);
	});

	GetMembershipsRawAsync(User, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page,  Count);
}

void UPubnubClient::GetMembershipsRawAsync(FString User, FOnPubnubGetMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FPubnubPage(), 0);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, User, NativeCallback, Include, Limit, Filter, Sort, Page,  Count]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubMembershipsResult GetMembershipsResult = WeakThis.Get()->GetMemberships_priv(User, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page,  Count);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, GetMembershipsResult.Result, GetMembershipsResult.MembershipsData, GetMembershipsResult.Page, GetMembershipsResult.TotalCount);
	});
}

FPubnubMembershipsResult UPubnubClient::GetMemberships(FString User, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubMembershipsResult());
	return GetMemberships_priv(User, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetMembershipsAsync(FString User, FOnPubnubGetMembershipsResponse OnGetMembershipsResponse, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	GetMembershipsRawAsync(User, OnGetMembershipsResponse, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetMembershipsAsync(FString User, FOnPubnubGetMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	GetMembershipsRawAsync(User, NativeCallback, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page,  (EPubnubTribool)Include.IncludeTotalCount);
}

FPubnubMembershipsResult UPubnubClient::SetMembershipsRaw(FString User, FString SetObj, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubMembershipsResult());
	return SetMemberships_priv(User, SetObj, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::SetMembershipsRawAsync(FString User, FString SetObj, FOnPubnubSetMembershipsResponse OnSetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FOnPubnubSetMembershipsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSetMembershipResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
	{
		OnSetMembershipResponse.ExecuteIfBound(Result, MembershipsData, Page, TotalCount);
	});

	SetMembershipsRawAsync(User, SetObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page,  Count);
}

void UPubnubClient::SetMembershipsRawAsync(FString User, FString SetObj, FOnPubnubSetMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FPubnubPage(), 0);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, User, SetObj, NativeCallback, Include, Limit, Filter, Sort, Page,  Count]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubMembershipsResult SetMembershipsResult = WeakThis.Get()->SetMemberships_priv(User, SetObj, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page,  Count);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, SetMembershipsResult.Result, SetMembershipsResult.MembershipsData, SetMembershipsResult.Page, SetMembershipsResult.TotalCount);
	});
}

FPubnubMembershipsResult UPubnubClient::SetMemberships(FString User, TArray<FPubnubMembershipInputData> Channels, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubMembershipsResult());
	return SetMemberships_priv(User, UPubnubJsonUtilities::GetJsonFromMembershipsDataArray(Channels), UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::SetMembershipsAsync(FString User, TArray<FPubnubMembershipInputData> Channels, FOnPubnubSetMembershipsResponse OnSetMembershipResponse, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	SetMembershipsRawAsync(User, UPubnubJsonUtilities::GetJsonFromMembershipsDataArray(Channels), OnSetMembershipResponse, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::SetMembershipsAsync(FString User, TArray<FPubnubMembershipInputData> Channels, FOnPubnubSetMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	SetMembershipsRawAsync(User, UPubnubJsonUtilities::GetJsonFromMembershipsDataArray(Channels), NativeCallback, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page,  (EPubnubTribool)Include.IncludeTotalCount);
}

FPubnubMembershipsResult UPubnubClient::RemoveMembershipsRaw(FString User, FString RemoveObj, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubMembershipsResult());
	return RemoveMemberships_priv(User, RemoveObj, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::RemoveMembershipsRawAsync(FString User, FString RemoveObj, FOnPubnubRemoveMembershipsResponse OnRemoveMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FOnPubnubRemoveMembershipsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveMembershipResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
	{
		OnRemoveMembershipResponse.ExecuteIfBound(Result, MembershipsData, Page, TotalCount);
	});

	RemoveMembershipsRawAsync(User, RemoveObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page,  Count);
}

void UPubnubClient::RemoveMembershipsRawAsync(FString User, FString RemoveObj, FOnPubnubRemoveMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FPubnubPage(), 0);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, User, RemoveObj, NativeCallback, Include, Limit, Filter, Sort, Page,  Count]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubMembershipsResult RemoveMembershipsResult = WeakThis.Get()->RemoveMemberships_priv(User, RemoveObj, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page,  Count);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, RemoveMembershipsResult.Result, RemoveMembershipsResult.MembershipsData, RemoveMembershipsResult.Page, RemoveMembershipsResult.TotalCount);
	});
}

FPubnubMembershipsResult UPubnubClient::RemoveMemberships(FString User, TArray<FString> Channels, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubMembershipsResult());
	
	return RemoveMemberships_priv(User, UPubnubJsonUtilities::GetJsonFromMembershipsToRemove(Channels), UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::RemoveMembershipsAsync(FString User, TArray<FString> Channels, FOnPubnubRemoveMembershipsResponse OnRemoveMembershipResponse, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	RemoveMembershipsRawAsync(User, UPubnubJsonUtilities::GetJsonFromMembershipsToRemove(Channels), OnRemoveMembershipResponse, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page,  (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::RemoveMembershipsAsync(FString User, TArray<FString> Channels, FOnPubnubRemoveMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	RemoveMembershipsRawAsync(User, UPubnubJsonUtilities::GetJsonFromMembershipsToRemove(Channels), NativeCallback, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page,  (EPubnubTribool)Include.IncludeTotalCount);
}

FPubnubChannelMembersResult UPubnubClient::GetChannelMembersRaw(FString Channel, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubChannelMembersResult());
	
	return GetChannelMembers_priv(Channel, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::GetChannelMembersRawAsync(FString Channel, FOnPubnubGetChannelMembersResponse OnGetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
    FOnPubnubGetChannelMembersResponseNative NativeCallback;
    NativeCallback.BindLambda([OnGetMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
    {
        OnGetMembersResponse.ExecuteIfBound(Result, MembersData, Page, TotalCount);
    });
    GetChannelMembersRawAsync(Channel, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::GetChannelMembersRawAsync(FString Channel, FOnPubnubGetChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
    PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubChannelMemberData>(), FPubnubPage(), 0);

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

    PubnubCallsThread->AddFunctionToQueue([WeakThis, Channel, NativeCallback, Include, Limit, Filter, Sort, Page, Count]
    {
		if(!WeakThis.IsValid())
		{return;}
		
        FPubnubChannelMembersResult GetChannelMembersResult = WeakThis.Get()->GetChannelMembers_priv(Channel, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, GetChannelMembersResult.Result, GetChannelMembersResult.MembersData, GetChannelMembersResult.Page, GetChannelMembersResult.TotalCount);
    });
}

FPubnubChannelMembersResult UPubnubClient::GetChannelMembers(FString Channel, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubChannelMembersResult());
	
	return GetChannelMembers_priv(Channel, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetChannelMembersAsync(FString Channel, FOnPubnubGetChannelMembersResponse OnGetMembersResponse, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
    GetChannelMembersRawAsync(Channel, OnGetMembersResponse, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetChannelMembersAsync(FString Channel, FOnPubnubGetChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
    GetChannelMembersRawAsync(Channel, NativeCallback, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

FPubnubChannelMembersResult UPubnubClient::SetChannelMembersRaw(FString Channel, FString SetObj, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubChannelMembersResult());
	
	return SetChannelMembers_priv(Channel, SetObj, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::SetChannelMembersRawAsync(FString Channel, FString SetObj, FOnPubnubSetChannelMembersResponse OnSetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
    FOnPubnubSetChannelMembersResponseNative NativeCallback;
    NativeCallback.BindLambda([OnSetMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
    {
        OnSetMembersResponse.ExecuteIfBound(Result, MembersData, Page, TotalCount);
    });
    SetChannelMembersRawAsync(Channel, SetObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::SetChannelMembersRawAsync(FString Channel, FString SetObj, FOnPubnubSetChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
    PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubChannelMemberData>(), FPubnubPage(), 0);

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

    PubnubCallsThread->AddFunctionToQueue([WeakThis, Channel, SetObj, NativeCallback, Include, Limit, Filter, Sort, Page, Count]
    {
		if(!WeakThis.IsValid())
		{return;}
		
        FPubnubChannelMembersResult SetChannelMembersResult = WeakThis.Get()->SetChannelMembers_priv(Channel, SetObj, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, SetChannelMembersResult.Result, SetChannelMembersResult.MembersData, SetChannelMembersResult.Page, SetChannelMembersResult.TotalCount);
    });
}

FPubnubChannelMembersResult UPubnubClient::SetChannelMembers(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubChannelMembersResult());
	
	return SetChannelMembers_priv(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersDataArray(Users), UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::SetChannelMembersAsync(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FOnPubnubSetChannelMembersResponse OnSetMembersResponse, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
    SetChannelMembersRawAsync(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersDataArray(Users), OnSetMembersResponse, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::SetChannelMembersAsync(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FOnPubnubSetChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
    SetChannelMembersRawAsync(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersDataArray(Users), NativeCallback, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

FPubnubChannelMembersResult UPubnubClient::RemoveChannelMembersRaw(FString Channel, FString RemoveObj, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubChannelMembersResult());
	
	return RemoveChannelMembers_priv(Channel, RemoveObj, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::RemoveChannelMembersRawAsync(FString Channel, FString RemoveObj, FOnPubnubRemoveChannelMembersResponse OnRemoveMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
    FOnPubnubRemoveChannelMembersResponseNative NativeCallback;
    NativeCallback.BindLambda([OnRemoveMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
    {
        OnRemoveMembersResponse.ExecuteIfBound(Result, MembersData, Page, TotalCount);
    });
    RemoveChannelMembersRawAsync(Channel, RemoveObj, NativeCallback, Include, Limit, Filter, Sort, Page, Count);
}

void UPubnubClient::RemoveChannelMembersRawAsync(FString Channel, FString RemoveObj, FOnPubnubRemoveChannelMembersResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
    PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubChannelMemberData>(), FPubnubPage(), 0);

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

    PubnubCallsThread->AddFunctionToQueue([WeakThis, Channel, RemoveObj, NativeCallback, Include, Limit, Filter, Sort, Page, Count]
    {
		if(!WeakThis.IsValid())
		{return;}
		
        FPubnubChannelMembersResult RemoveChannelMembersResult = WeakThis.Get()->RemoveChannelMembers_priv(Channel, RemoveObj, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, RemoveChannelMembersResult.Result, RemoveChannelMembersResult.MembersData, RemoveChannelMembersResult.Page, RemoveChannelMembersResult.TotalCount);
    });
}

FPubnubChannelMembersResult UPubnubClient::RemoveChannelMembers(FString Channel, TArray<FString> Users, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubChannelMembersResult());
	
	return RemoveChannelMembers_priv(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersToRemove(Users), UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::RemoveChannelMembersAsync(FString Channel, TArray<FString> Users, FOnPubnubRemoveChannelMembersResponse OnRemoveMembersResponse, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
    RemoveChannelMembersRawAsync(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersToRemove(Users), OnRemoveMembersResponse, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::RemoveChannelMembersAsync(FString Channel, TArray<FString> Users, FOnPubnubRemoveChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
    RemoveChannelMembersRawAsync(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersToRemove(Users), NativeCallback, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

FPubnubAddMessageActionResult UPubnubClient::AddMessageAction(FString Channel, FString MessageTimetoken, FString ActionType, FString Value)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubAddMessageActionResult());
	return AddMessageAction_priv(Channel, MessageTimetoken, ActionType, Value);
}

void UPubnubClient::AddMessageActionAsync(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnPubnubAddMessageActionResponse OnAddMessageActionResponse)
{
	FOnPubnubAddMessageActionResponseNative NativeCallback;
	NativeCallback.BindLambda([OnAddMessageActionResponse](const FPubnubOperationResult& Result, FPubnubMessageActionData MessageActionData)
	{
		OnAddMessageActionResponse.ExecuteIfBound(Result, MessageActionData);
	});
	AddMessageActionAsync(Channel, MessageTimetoken, ActionType, Value, NativeCallback);
}

void UPubnubClient::AddMessageActionAsync(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value, FOnPubnubAddMessageActionResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubMessageActionData());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, MessageTimetoken, ActionType, Value, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubAddMessageActionResult AddMessageActionResult = WeakThis.Get()->AddMessageAction_priv(Channel, MessageTimetoken, ActionType, Value);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, AddMessageActionResult.Result, AddMessageActionResult.MessageActionData);
	});
}

FPubnubGetMessageActionsResult UPubnubClient::GetMessageActions(FString Channel, FString Start, FString End, int Limit)
{
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FPubnubGetMessageActionsResult());
	return GetMessageActions_priv(Channel, Start, End, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit));
}

void UPubnubClient::GetMessageActionsAsync(FString Channel, FOnPubnubGetMessageActionsResponse OnGetMessageActionsResponse, FString Start, FString End, int Limit)
{
	FOnPubnubGetMessageActionsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetMessageActionsResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMessageActionData>& MessageActions)
	{
		OnGetMessageActionsResponse.ExecuteIfBound(Result, MessageActions);
	});
	GetMessageActionsAsync(Channel, NativeCallback, Start, End, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit));
}

void UPubnubClient::GetMessageActionsAsync(FString Channel, FOnPubnubGetMessageActionsResponseNative NativeCallback, FString Start, FString End, int Limit)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubMessageActionData>());
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, Start, End, Limit, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubGetMessageActionsResult GetMessageActionsResult = WeakThis.Get()->GetMessageActions_priv(Channel, Start, End, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit));

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, GetMessageActionsResult.Result, GetMessageActionsResult.MessageActions);
	});
}

FPubnubOperationResult UPubnubClient::RemoveMessageAction(FString Channel, FString MessageTimetoken, FString ActionTimetoken)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	return RemoveMessageAction_priv(Channel, MessageTimetoken, ActionTimetoken);
}

void UPubnubClient::RemoveMessageActionAsync(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnPubnubRemoveMessageActionResponse OnRemoveMessageActionResponse)
{
	FOnPubnubRemoveMessageActionResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveMessageActionResponse](const FPubnubOperationResult& Result)
	{
		OnRemoveMessageActionResponse.ExecuteIfBound(Result);
	});
	RemoveMessageActionAsync(Channel, MessageTimetoken, ActionTimetoken, NativeCallback);
}

void UPubnubClient::RemoveMessageActionAsync(FString Channel, FString MessageTimetoken, FString ActionTimetoken, FOnPubnubRemoveMessageActionResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, MessageTimetoken, ActionTimetoken, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult RemoveMessageActionResult = WeakThis.Get()->RemoveMessageAction_priv(Channel, MessageTimetoken, ActionTimetoken);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, RemoveMessageActionResult);
	});
}

FPubnubOperationResult UPubnubClient::ReconnectSubscriptions(FString Timetoken)
{
	enum pubnub_res ReconnectResult;
	if (Timetoken.IsEmpty())
	{
		ReconnectResult = pubnub_reconnect(ctx_ee, nullptr);
	}
	else
	{
		FUTF8StringHolder ChannelHolder(Timetoken);
		pubnub_subscribe_cursor_t cursor = pubnub_subscribe_cursor(ChannelHolder.Get());
		ReconnectResult = pubnub_reconnect(ctx_ee, &cursor);
	}

	FPubnubOperationResult FinalResult;
	
	if (PNR_OK != ReconnectResult)
	{
		FinalResult.Error = true;
		FinalResult.ErrorMessage = pubnub_res_2_string(ReconnectResult);
	}
	else
	{
		FinalResult.Status = 200;
	}
	
	return FinalResult;
}

FPubnubOperationResult UPubnubClient::DisconnectSubscriptions()
{
	enum pubnub_res DisconnectResult = pubnub_disconnect(ctx_ee);
	
	FPubnubOperationResult FinalResult;
	
	if (PNR_OK != DisconnectResult)
	{
		FinalResult.Error = true;
		FinalResult.ErrorMessage = pubnub_res_2_string(DisconnectResult);
	}
	else
	{
		FinalResult.Status = 200;
	}
	
	return FinalResult;
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
	if(IsInitialized.load(std::memory_order_acquire))
	{
		//Create new thread to queue all pubnub operations
		PubnubCallsThread = new FPubnubFunctionThread;
	}
}

void UPubnubClient::BeginDestroy()
{
	if(IsInitialized.load(std::memory_order_acquire))
	{
		DeinitializeClient();
	}
	
	Super::BeginDestroy();
}

void UPubnubClient::DeinitializeClient()
{
	if(!IsInitialized.load(std::memory_order_acquire))
	{return;}

	CancelPendingSubscriptionOperation(TEXT("Subscription operation cancelled because PubnubClient is being deinitialized."));

	if(PubnubCallsThread)
	{
		PubnubCallsThread->Stop();
	}

	{
		FScopeLock SubscriptionExecutionLock(&SubscriptionOperationExecutionMutex);
		//Unsubscribe from all channels and groups so this user will not be visible for others anymore
		UnsubscribeAllForDeinit();
	}
	
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
	delete[] OriginBuffer;
	OriginBuffer = nullptr;
	OriginLength = 0;
	delete PubnubCallsThread;
	PubnubCallsThread = nullptr;

	//Notify that Deinitialization is finished
	OnClientDeinitialized.Broadcast();
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

FPubnubOperationResult UPubnubClient::ExecuteSerializedSubscriptionOperation(const FString& StartFailureMessage, const FString& TimeoutMessage, TFunctionRef<bool()> StartOperation)
{
	FScopeLock SubscriptionExecutionLock(&SubscriptionOperationExecutionMutex);

	FEvent* CompletionEvent = FPlatformProcess::GetSynchEventFromPool(false);
	const int32 OperationId = NextSubscriptionOperationId++;
	ActivatePendingSubscriptionOperation(CompletionEvent, OperationId);

	if(!StartOperation())
	{
		ClearPendingSubscriptionOperation();
		FPlatformProcess::ReturnSynchEventToPool(CompletionEvent);
		return FPubnubOperationResult({0, true, StartFailureMessage});
	}

	const bool bCompleted = CompletionEvent->Wait(SubscriptionOperationTimeout);
	if(!bCompleted)
	{
		ClearPendingSubscriptionOperation();
		FPlatformProcess::ReturnSynchEventToPool(CompletionEvent);
		return FPubnubOperationResult({408, true, TimeoutMessage});
	}

	FPubnubOperationResult OperationResult;
	{
		FScopeLock PendingOperationLock(&PendingSubscriptionOperationMutex);
		OperationResult = PendingSubscriptionOperation.Result;
	}

	ClearPendingSubscriptionOperation();
	FPlatformProcess::ReturnSynchEventToPool(CompletionEvent);
	return OperationResult;
}

void UPubnubClient::ActivatePendingSubscriptionOperation(FEvent* CompletionEvent, int32 OperationId)
{
	FScopeLock PendingOperationLock(&PendingSubscriptionOperationMutex);
	PendingSubscriptionOperation.OperationId = OperationId;
	PendingSubscriptionOperation.CompletionEvent = CompletionEvent;
	PendingSubscriptionOperation.Result = FPubnubOperationResult();
	PendingSubscriptionOperation.bIsActive = true;
}

bool UPubnubClient::CompletePendingSubscriptionOperation(const FPubnubOperationResult& Result)
{
	FScopeLock PendingOperationLock(&PendingSubscriptionOperationMutex);
	if(!PendingSubscriptionOperation.bIsActive || !PendingSubscriptionOperation.CompletionEvent)
	{
		return false;
	}

	PendingSubscriptionOperation.Result = Result;
	PendingSubscriptionOperation.CompletionEvent->Trigger();
	return true;
}

void UPubnubClient::ClearPendingSubscriptionOperation()
{
	FScopeLock PendingOperationLock(&PendingSubscriptionOperationMutex);
	PendingSubscriptionOperation.OperationId = INDEX_NONE;
	PendingSubscriptionOperation.CompletionEvent = nullptr;
	PendingSubscriptionOperation.Result = FPubnubOperationResult();
	PendingSubscriptionOperation.bIsActive = false;
}

void UPubnubClient::CancelPendingSubscriptionOperation(const FString& CancelReason)
{
	FScopeLock PendingOperationLock(&PendingSubscriptionOperationMutex);
	if(!PendingSubscriptionOperation.bIsActive || !PendingSubscriptionOperation.CompletionEvent)
	{
		return;
	}

	PendingSubscriptionOperation.Result = FPubnubOperationResult({499, true, CancelReason});
	PendingSubscriptionOperation.CompletionEvent->Trigger();
}

void UPubnubClient::OnCCoreSubscriptionStatusReceived(int StatusEnum, const void* StatusData)
{
	//Cast data back to C-Core types
	pubnub_subscription_status status = static_cast<pubnub_subscription_status>(StatusEnum);
	const pubnub_subscription_status_data_t* status_data = static_cast<const pubnub_subscription_status_data_t*>(StatusData);

	FPubnubOperationResult Result;
	Result.Error = status == PNSS_SUBSCRIPTION_STATUS_CONNECTION_ERROR || status == PNSS_SUBSCRIPTION_STATUS_DISCONNECTED_UNEXPECTEDLY;
	Result.Status = Result.Error ? 503 : 200;
	Result.ErrorMessage = status_data ? FString(pubnub_res_2_string(status_data->reason)) : TEXT("No status data.");
	CompletePendingSubscriptionOperation(Result);
	
	//Don't waste resources to translate data if there is no delegate bound to it
	if(!OnSubscriptionStatusChanged.IsBound() && !OnSubscriptionStatusChangedNative.IsBound())
	{return;}

	FPubnubSubscriptionStatusData SubscriptionStatusData;
	SubscriptionStatusData.Reason = status_data ? FString(pubnub_res_2_string(status_data->reason)) : TEXT("No status data.");

	//If status is disconnected we don't need to give subscribed channels
	if(status_data && status != PNSS_SUBSCRIPTION_STATUS_DISCONNECTED)
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
	OnSubscriptionStatusChanged.Broadcast((EPubnubSubscriptionStatus)status, SubscriptionStatusData);
	OnSubscriptionStatusChangedNative.Broadcast((EPubnubSubscriptionStatus)status, SubscriptionStatusData);
}

FString UPubnubClient::GetLastResponse(pubnub_t* context)
{
	FString Response;
	
	if(!context)
	{return Response;}
	
	pubnub_res PubnubResponse = pubnub_await(context);
	if (PNR_OK == PubnubResponse)
	{

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
	if (PNR_OK == PubnubResponse)
	{
		
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

	PubnubOperationMutex.Lock();
	
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
		SetUserID_priv(Config.UserID);
	}
	
	if(PubnubConfig.SetSecretKeyAutomatically)
	{
		SetSecretKey_priv();
	}
	
	PubnubOperationMutex.Unlock();
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


FPubnubPublishMessageResult UPubnubClient::PublishMessage_priv(FString Channel, FString Message, FPubnubPublishSettings PublishSettings)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubPublishMessageResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FPubnubPublishMessageResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Message, FPubnubPublishMessageResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubPublishMessageResult());

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
	
	PubnubOperationMutex.Unlock();
	
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
	return FPubnubPublishMessageResult({PublishResult, PublishedMessage});
}

FPubnubSignalResult UPubnubClient::Signal_priv(FString Channel, FString Message, FPubnubSignalSettings SignalSettings)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubSignalResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FPubnubSignalResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Message, FPubnubSignalResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubSignalResult());
	
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

	PubnubOperationMutex.Unlock();

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

	return FPubnubSignalResult({PublishResult, SignalMessage});
}


FPubnubOperationResult UPubnubClient::SubscribeToChannel_priv(FString Channel, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);

	//Create subscription for channel entity
	pubnub_subscription_t* Subscription = UPubnubInternalUtilities::EEGetSubscriptionForEntity(ctx_ee, Channel, EPubnubEntityType::PEnT_Channel, SubscribeSettings);
	//Return if created subscription is invalid
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS((Subscription != nullptr), TEXT("Failed to subscribe to channel. Pubnub_subscription_alloc didn't create subscription."));
	
	//Create callback that will be triggered by the c-core event engine
	pubnub_subscribe_message_callback_t Callback = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		TWeakObjectPtr<UPubnubClient> ThisClientWeak = MakeWeakObjectPtr<UPubnubClient>(static_cast<UPubnubClient*>(user_data));
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, ThisClientWeak]()
		{
			if(ThisClientWeak.IsValid())
			{
				ThisClientWeak.Get()->OnMessageReceived.Broadcast(MessageData);
				ThisClientWeak.Get()->OnMessageReceivedNative.Broadcast(MessageData);
			}
		});
	};

	FString StartFailureMessage = TEXT("Failed to subscribe to channel.");
	return ExecuteSerializedSubscriptionOperation(
		StartFailureMessage,
		TEXT("Subscribe operation timed out"),
		[&]()
		{
			if(ChannelSubscriptions.Contains(Channel))
			{
				StartFailureMessage = TEXT("Already subscribed to this channel. Aborting operation.");
				pubnub_subscription_free(&Subscription);
				return false;
			}

			if(!UPubnubInternalUtilities::EEAddListenerAndSubscribe(Subscription, Callback, this))
			{
				PubnubError("[SubscribeToChannel]: Failed to subscribe to channel.");
				pubnub_subscription_free(&Subscription);
				return false;
			}

			//Save callback and subscription so it can be unsubscribed later.
			CCoreSubscriptionCallback* SubscriptionData = new CCoreSubscriptionCallback{Callback, Subscription};
			ChannelSubscriptions.Add(Channel, SubscriptionData);
			return true;
		});
}


FPubnubOperationResult UPubnubClient::SubscribeToGroup_priv(FString ChannelGroup, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(ChannelGroup);
	
	//Create subscription for channel group entity
	pubnub_subscription_t* Subscription = UPubnubInternalUtilities::EEGetSubscriptionForEntity(ctx_ee, ChannelGroup, EPubnubEntityType::PEnT_ChannelGroup, SubscribeSettings);
	//Return if created subscription is invalid
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS((Subscription != nullptr), TEXT("Failed to subscribe to group. Pubnub_subscription_alloc didn't create subscription."));
	
	//Create callback that will be triggered by the c-core event engine
	pubnub_subscribe_message_callback_t Callback = +[](const pubnub_t* pb, struct pubnub_v2_message message, void* user_data)
	{
		TWeakObjectPtr<UPubnubClient> ThisClientWeak = MakeWeakObjectPtr<UPubnubClient>(static_cast<UPubnubClient*>(user_data));
		FPubnubMessageData MessageData = UPubnubUtilities::UEMessageFromPubnubMessage(message); 
		AsyncTask(ENamedThreads::GameThread, [MessageData, ThisClientWeak]()
		{
			if(ThisClientWeak.IsValid())
			{
				ThisClientWeak.Get()->OnMessageReceived.Broadcast(MessageData);
				ThisClientWeak.Get()->OnMessageReceivedNative.Broadcast(MessageData);
			}
		});
	};

	FString StartFailureMessage = TEXT("Failed to subscribe to channel group.");
	return ExecuteSerializedSubscriptionOperation(
		StartFailureMessage,
		TEXT("Subscribe operation timed out"),
		[&]()
		{
			if(ChannelGroupSubscriptions.Contains(ChannelGroup))
			{
				StartFailureMessage = TEXT("Already subscribed to this channel group. Aborting operation.");
				pubnub_subscription_free(&Subscription);
				return false;
			}

			if(!UPubnubInternalUtilities::EEAddListenerAndSubscribe(Subscription, Callback, this))
			{
				PubnubError("[SubscribeToGroup]: Failed to subscribe to channel group.");
				pubnub_subscription_free(&Subscription);
				return false;
			}

			//Save callback and subscription so it can be unsubscribed later.
			CCoreSubscriptionCallback* SubscriptionData = new CCoreSubscriptionCallback{Callback, Subscription};
			ChannelGroupSubscriptions.Add(ChannelGroup, SubscriptionData);
			return true;
		});
}

FPubnubOperationResult UPubnubClient::UnsubscribeFromChannel_priv(FString Channel)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);

	FScopeLock SubscriptionExecutionLock(&SubscriptionOperationExecutionMutex);

	CCoreSubscriptionCallback* SubscriptionData = ChannelSubscriptions.FindRef(Channel);
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(SubscriptionData, TEXT("There is no such subscription. Aborting operation."));

	if(!UPubnubInternalUtilities::EERemoveListenerAndUnsubscribe(&SubscriptionData->Subscription, SubscriptionData->Callback, this))
	{
		PubnubError("[UnsubscribeFromChannel]: Failed to unsubscribe.", EPubnubErrorType::PET_Warning);
		return FPubnubOperationResult({0, true, "Failed to unsubscribe."});
	}

	//Free subscription memory and remove local tracking.
	pubnub_subscription_free(&SubscriptionData->Subscription);
	ChannelSubscriptions.Remove(Channel);
	delete SubscriptionData;

	return FPubnubOperationResult({200, false, ""});
}

FPubnubOperationResult UPubnubClient::UnsubscribeFromGroup_priv(FString ChannelGroup)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(ChannelGroup);

	FScopeLock SubscriptionExecutionLock(&SubscriptionOperationExecutionMutex);

	CCoreSubscriptionCallback* SubscriptionData = ChannelGroupSubscriptions.FindRef(ChannelGroup);
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(SubscriptionData, TEXT("There is no such subscription. Aborting operation."));

	if(!UPubnubInternalUtilities::EERemoveListenerAndUnsubscribe(&SubscriptionData->Subscription, SubscriptionData->Callback, this))
	{
		PubnubError("[UnsubscribeFromGroup]: Failed to unsubscribe.", EPubnubErrorType::PET_Warning);
		return FPubnubOperationResult({0, true, "Failed to unsubscribe."});
	}

	//Free subscription memory and remove local tracking.
	pubnub_subscription_free(&SubscriptionData->Subscription);
	ChannelGroupSubscriptions.Remove(ChannelGroup);
	delete SubscriptionData;

	return FPubnubOperationResult({200, false, ""});
}

FPubnubOperationResult UPubnubClient::UnsubscribeFromAll_priv()
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	FScopeLock SubscriptionExecutionLock(&SubscriptionOperationExecutionMutex);

	//If there are no subscriptions, return success immediately.
	if(ChannelSubscriptions.IsEmpty() && ChannelGroupSubscriptions.IsEmpty())
	{
		return FPubnubOperationResult({200, false, ""});
	}

	const enum pubnub_res UnsubscribeAllResult = pubnub_unsubscribe_all(ctx_ee);
	if(UnsubscribeAllResult != PNR_OK)
	{
		return FPubnubOperationResult({0, true, FString::Printf(TEXT("Failed to unsubscribe all. Error: %s"), UTF8_TO_TCHAR(pubnub_res_2_string(UnsubscribeAllResult)))});
	}

	CleanUpAllSubscriptions();
	return FPubnubOperationResult({200, false, ""});
}


FPubnubOperationResult UPubnubClient::AddChannelToGroup_priv(FString Channel, FString ChannelGroup)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(ChannelGroup);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);
	FUTF8StringHolder ChannelHolder(Channel);
	
	pubnub_add_channel_to_group(ctx_pub, ChannelHolder.Get(), ChannelGroupHolder.Get());

	//This is just to clear the C-Core response buffer, but it doesn't return the server response
	GetLastResponse(ctx_pub);
	//So we need to get the response separately
	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);

	PubnubOperationMutex.Unlock();

	return UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse);
}

FPubnubOperationResult UPubnubClient::RemoveChannelFromGroup_priv(FString Channel, FString ChannelGroup)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(ChannelGroup);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);
	FUTF8StringHolder ChannelHolder(Channel);

	pubnub_remove_channel_from_group(ctx_pub, ChannelHolder.Get(), ChannelGroupHolder.Get());

	//This is just to clear the C-Core response buffer, but it doesn't return the server response
	GetLastResponse(ctx_pub);
	//So we need to get the response separately
	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);

	PubnubOperationMutex.Unlock();

	return UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse);
}

FPubnubListChannelsFromGroupResult UPubnubClient::ListChannelsFromGroup_priv(FString ChannelGroup)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubListChannelsFromGroupResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(ChannelGroup, FPubnubListChannelsFromGroupResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubListChannelsFromGroupResult());
	
	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);
	
	pubnub_list_channel_group(ctx_pub, ChannelGroupHolder.Get());
	
	FString JsonResponse = GetLastChannelResponse(ctx_pub);

	PubnubOperationMutex.Unlock();

	FPubnubOperationResult Result;
	TArray<FString> Channels;
	UPubnubJsonUtilities::ListChannelsFromGroupJsonToData(JsonResponse, Result, Channels);
	
	return FPubnubListChannelsFromGroupResult({Result, Channels});
}

FPubnubOperationResult UPubnubClient::RemoveChannelGroup_priv(FString ChannelGroup)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(ChannelGroup);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);

	pubnub_remove_channel_group(ctx_pub, ChannelGroupHolder.Get());
	
	//This is just to clear the C-Core response buffer, but it doesn't return the server response
	GetLastResponse(ctx_pub);
	//So we need to get the response separately
	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	
	PubnubOperationMutex.Unlock();
	
	return UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse);
}

FPubnubListUsersFromChannelResult UPubnubClient::ListUsersFromChannel_priv(FString Channel, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubListUsersFromChannelResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FPubnubListUsersFromChannelResult());
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS((ListUsersFromChannelSettings.Limit >= 0), TEXT("Limit can't be below 0."), FPubnubListUsersFromChannelResult());
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS((ListUsersFromChannelSettings.Offset >= 0), TEXT("Offset can't be below 0."), FPubnubListUsersFromChannelResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubListUsersFromChannelResult());

	//Set all options from ListUsersFromChannelSettings
	FUTF8StringHolder ChannelHolder(Channel);
	
	//Converted char needs to live in function scope, so we need to create it here
	pubnub_here_now_options HereNowOptions;
	FUTF8StringHolder ChannelGroupHolder(ListUsersFromChannelSettings.ChannelGroup);
	HereNowOptions.channel_group = ChannelGroupHolder.Get();
	
	UPubnubInternalUtilities::HereNowUESettingsToPubnubHereNowOptions(ListUsersFromChannelSettings, HereNowOptions);
	
	pubnub_here_now_ex(ctx_pub, ChannelHolder.Get(), HereNowOptions);
	
	FString JsonResponse = GetLastResponse(ctx_pub);

	PubnubOperationMutex.Unlock();
	
	FPubnubOperationResult Result;
	
	//If response is empty, there was server error. 
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
		//Presence api doesn't provide status code in the response, so we need to get it manually
		Result.Status = pubnub_last_http_code(ctx_pub);
	}
	
	FPubnubListUsersFromChannelWrapper Data;
	UPubnubJsonUtilities::ListUsersFromChannelJsonToData(JsonResponse, Result, Data);
	
	return FPubnubListUsersFromChannelResult({Result, Data});
}

FPubnubListUsersSubscribedChannelsResult UPubnubClient::ListUserSubscribedChannels_priv(FString UserID)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubListUsersSubscribedChannelsResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(UserID, FPubnubListUsersSubscribedChannelsResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubListUsersSubscribedChannelsResult());

	FUTF8StringHolder UserIDHolder(UserID);
	pubnub_where_now(ctx_pub, UserIDHolder.Get());

	FString JsonResponse = GetLastResponse(ctx_pub);
	
	PubnubOperationMutex.Unlock();
	
	FPubnubOperationResult Result;
	TArray<FString> Channels;
	UPubnubJsonUtilities::ListUserSubscribedChannelsJsonToData(JsonResponse, Result, Channels);
	
	return FPubnubListUsersSubscribedChannelsResult({Result, Channels});
}

FPubnubOperationResult UPubnubClient::SetState_priv(FString Channel, FString StateJson, FPubnubSetStateSettings SetStateSettings)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(StateJson);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	if(!UPubnubJsonUtilities::IsCorrectJsonString(StateJson, false))
	{
		PubnubError("[SetState]: StateJson has to be a correct Json Object. Aborting operation.", EPubnubErrorType::PET_Warning);
		FPubnubOperationResult Result;
		Result.Error = true;
		Result.ErrorMessage = "[SetState]: StateJson has to be a correct Json Object. Operation aborted.";
		return Result;
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

	PubnubOperationMutex.Unlock();

	return UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse);
}

FPubnubGetStateResult UPubnubClient::GetState_priv(FString Channel, FString ChannelGroup, FString UserID)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubGetStateResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FPubnubGetStateResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubGetStateResult());

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

	PubnubOperationMutex.Unlock();

	return FPubnubGetStateResult({UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse), JsonResponse});
}

FPubnubOperationResult UPubnubClient::Heartbeat_priv(FString Channel, FString ChannelGroup)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();
	
	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);
	FUTF8StringHolder ChannelHolder(Channel);

	pubnub_heartbeat(ctx_pub, ChannelHolder.Get(), ChannelGroupHolder.Get());

	GetLastResponse(ctx_pub);
	
	PubnubOperationMutex.Unlock();
	
	FPubnubOperationResult Result;
	Result.Error = false;
	return Result;
}

FPubnubGrantTokenResult UPubnubClient::GrantToken_priv(FString PermissionObject)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubGrantTokenResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(PermissionObject, FPubnubGrantTokenResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubGrantTokenResult());

	FUTF8StringHolder PermissionObjectHolder(PermissionObject);
	
	pubnub_grant_token(ctx_pub, PermissionObjectHolder.Get());

	pubnub_await(ctx_pub);
	
	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	
	PubnubOperationMutex.Unlock();
	
	//Access Manager has similar result structure to AppContext, so we use the same getter
	FPubnubOperationResult Result = UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(JsonResponse);
	FString Token = "";
	if(Result.Status == 200)
	{
		pubnub_chamebl_t grant_token_resp = pubnub_get_grant_token(ctx_pub);
		Token = UPubnubUtilities::PubnubCharMemBlockToString(grant_token_resp);
	}
	
	return FPubnubGrantTokenResult({Result, Token});
}

FPubnubOperationResult UPubnubClient::RevokeToken_priv(FString Token)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Token);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	FUTF8StringHolder TokenHolder(Token);
	
	pubnub_revoke_token(ctx_pub, TokenHolder.Get());

	FString JsonResponse = GetLastResponse(ctx_pub);

	//If response is empty, there was server error. 
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	PubnubOperationMutex.Unlock();
	
	//Access Manager has similar result structure to AppContext, so we use the same getter
	return UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(JsonResponse);
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
	pubnub_set_auth_token(ctx_ee, AuthTokenBuffer);
}

int UPubnubClient::SetOrigin_priv(FString Origin)
{
	PUBNUB_RETURN_IF_USER_ID_NOT_SET(-1);

	int Result = 0;

	//If origin is empty, pass null to pubnub_origin_set
	if (Origin.IsEmpty())
	{
		delete[] OriginBuffer;
		OriginBuffer = nullptr;
		OriginLength = 0;
		
		Result = pubnub_origin_set(ctx_pub, nullptr);
		return Result;
	}

	//Origin has to be kept alive for the lifetime of the sdk, so we copy it into OriginBuffer
	FTCHARToUTF8 Converter(*Origin);
	OriginLength = Converter.Length();
	delete[] OriginBuffer;
	OriginBuffer = new char[OriginLength + 1];
	FMemory::Memcpy(OriginBuffer, Converter.Get(), OriginLength);
	OriginBuffer[OriginLength] = '\0';
	
	//This is just a setter, so no need to call it on a separate thread
	Result = pubnub_origin_set(ctx_pub, OriginBuffer);
	return Result;
}

FPubnubFetchHistoryResult UPubnubClient::FetchHistory_priv(FString Channel, FPubnubFetchHistorySettings FetchHistorySettings)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubFetchHistoryResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FPubnubFetchHistoryResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubFetchHistoryResult());
	
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
	
	PubnubOperationMutex.Unlock();
	
	//Parse Json response into data
	FPubnubOperationResult Result;
	TArray<FPubnubHistoryMessageData> Messages;
	UPubnubJsonUtilities::FetchHistoryJsonToData(HistoryResponse, Result, Messages);
	DecryptHistoryMessages(Messages);
			
	return FPubnubFetchHistoryResult({Result, Messages});
}

FPubnubOperationResult UPubnubClient::DeleteMessages_priv(FString Channel, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

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

	PubnubOperationMutex.Unlock();

	return UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse);
}

FPubnubMessageCountsResult UPubnubClient::MessageCounts_priv(FString Channel, FString Timetoken)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubMessageCountsResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FPubnubMessageCountsResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubMessageCountsResult());

	FUTF8StringHolder TimetokenHolder(Timetoken);
	FUTF8StringHolder ChannelHolder(Channel);
	
	pubnub_message_counts(ctx_pub, ChannelHolder.Get(), TimetokenHolder.Get());

	pubnub_await(ctx_pub);

	int MessageCountsNumber = 0;
	pubnub_get_message_counts(ctx_pub, ChannelHolder.Get(), &MessageCountsNumber);

	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	
	PubnubOperationMutex.Unlock();
	
	return FPubnubMessageCountsResult({UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse), MessageCountsNumber});
}

FPubnubMessageCountsMultipleResult UPubnubClient::MessageCountsMultiple_priv(TArray<FString> Channels, TArray<FString> Timetokens)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubMessageCountsMultipleResult());
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS((!Channels.IsEmpty()), TEXT("Channels array cannot be empty."), FPubnubMessageCountsMultipleResult());
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS((Channels.Num() == Timetokens.Num()), TEXT("Number of channels must match number of timetokens."), FPubnubMessageCountsMultipleResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubMessageCountsMultipleResult());

	FUTF8StringHolder TimetokensHolder(UPubnubUtilities::ArrayOfStringsToCommaSeparatedString(Timetokens));
	FUTF8StringHolder ChannelHolder(UPubnubUtilities::ArrayOfStringsToCommaSeparatedString(Channels));
	
	pubnub_message_counts(ctx_pub, ChannelHolder.Get(), TimetokensHolder.Get());

	pubnub_await(ctx_pub);
	
	int Size = Channels.Num();
	TArray<int> MessageCountsReturn;
	MessageCountsReturn.SetNumZeroed(Size);
	
	int GetResponse = pubnub_get_message_counts(ctx_pub, ChannelHolder.Get(), MessageCountsReturn.GetData());
	
	TMap<FString, int> MessageCountsPerChannel;
	for(int i = 0; i < Channels.Num(); i++)
	{
		MessageCountsPerChannel.Add(Channels[i], GetResponse >= 0 ? MessageCountsReturn[i] : 0);
	}

	FString JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	
	PubnubOperationMutex.Unlock();
	
	FPubnubMessageCountsMultipleResult Result;
	Result.Result = UPubnubJsonUtilities::GetOperationResultFromJson(JsonResponse);
	Result.MessageCountsPerChannel = MessageCountsPerChannel;
	return Result;
}

FPubnubGetAllUserMetadataResult UPubnubClient::GetAllUserMetadata_priv(FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubGetAllUserMetadataResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubGetAllUserMetadataResult());

	pubnub_getall_metadata_opts PubnubOptions = pubnub_getall_metadata_defopts();
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	// If both Next and Prev are provided, Next takes precedence
	PubnubOptions.page.next = Page.Next.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = (Page.Next.IsEmpty() && !Page.Prev.IsEmpty()) ? PagePrevHolder.Get() : NULL;
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;
	
	pubnub_getall_uuidmetadata_ex(ctx_pub, PubnubOptions);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	PubnubOperationMutex.Unlock();
	
	//Parse Json response into data
	FPubnubGetAllUserMetadataResult GetAllUserMetadataResult;
	UPubnubJsonUtilities::GetAllUserMetadataJsonToData(JsonResponse, GetAllUserMetadataResult.Result, GetAllUserMetadataResult.UsersData, GetAllUserMetadataResult.Page, GetAllUserMetadataResult.TotalCount);
	
	return GetAllUserMetadataResult;
}

FPubnubUserMetadataResult UPubnubClient::SetUserMetadata_priv(FString User, FString UserMetadataObj, FString Include)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubUserMetadataResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(User, FPubnubUserMetadataResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(UserMetadataObj, FPubnubUserMetadataResult());
	//Make sure that provided UserMetadataObj is a correct Json string
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(UPubnubJsonUtilities::IsCorrectJsonString(UserMetadataObj, false), TEXT("UserMetadataObj has to be a correct Json Object. Operation aborted."),  FPubnubUserMetadataResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubUserMetadataResult());
	
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
	
	PubnubOperationMutex.Unlock();
	
	//Parse Json response into data
	FPubnubUserMetadataResult SetUserMetadataResult;
	UPubnubJsonUtilities::GetUserMetadataJsonToData(JsonResponse, SetUserMetadataResult.Result, SetUserMetadataResult.UserData);
							
	return SetUserMetadataResult;
}

FPubnubUserMetadataResult UPubnubClient::GetUserMetadata_priv(FString User, FString Include)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubUserMetadataResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(User, FPubnubUserMetadataResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubUserMetadataResult());

	FUTF8StringHolder UserHolder(User);
	FUTF8StringHolder IncludeHolder(Include);
	pubnub_get_uuidmetadata(ctx_pub, IncludeHolder.Get(), UserHolder.Get());

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}

	PubnubOperationMutex.Unlock();
	
	//Parse Json response into data
	FPubnubUserMetadataResult GetUserMetadataResult;
	UPubnubJsonUtilities::GetUserMetadataJsonToData(JsonResponse, GetUserMetadataResult.Result, GetUserMetadataResult.UserData);
							
	return GetUserMetadataResult;
}

FPubnubOperationResult UPubnubClient::RemoveUserMetadata_priv(FString User)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(User);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	FUTF8StringHolder UserHolder(User);
	
	pubnub_remove_uuidmetadata(ctx_pub, UserHolder.Get());

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	PubnubOperationMutex.Unlock();
	
	return UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(JsonResponse);
}

FPubnubGetAllChannelMetadataResult UPubnubClient::GetAllChannelMetadata_priv(FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubGetAllChannelMetadataResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubGetAllChannelMetadataResult());
	
	pubnub_getall_metadata_opts PubnubOptions = pubnub_getall_metadata_defopts();
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	// If both Next and Prev are provided, Next takes precedence
	PubnubOptions.page.next = Page.Next.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = (Page.Next.IsEmpty() && !Page.Prev.IsEmpty()) ? PagePrevHolder.Get() : NULL;
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
	PubnubOperationMutex.Unlock();
	
	FPubnubGetAllChannelMetadataResult GetAllChannelMetadataResult;
	UPubnubJsonUtilities::GetAllChannelMetadataJsonToData(JsonResponse, GetAllChannelMetadataResult.Result, GetAllChannelMetadataResult.ChannelsData, GetAllChannelMetadataResult.Page, GetAllChannelMetadataResult.TotalCount);
	
	return GetAllChannelMetadataResult;
}

FPubnubChannelMetadataResult UPubnubClient::SetChannelMetadata_priv(FString Channel, FString ChannelMetadataObj, FString Include)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubChannelMetadataResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FPubnubChannelMetadataResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(ChannelMetadataObj, FPubnubChannelMetadataResult());
	//Make sure that provided ChannelMetadataObj is a correct Json string
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(UPubnubJsonUtilities::IsCorrectJsonString(ChannelMetadataObj, false), TEXT("ChannelMetadataObj has to be a correct Json Object. Operation aborted."),  FPubnubChannelMetadataResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubChannelMetadataResult());
	
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
	
	PubnubOperationMutex.Unlock();
	
	//Parse Json response into data
	FPubnubChannelMetadataResult SetChannelMetadataResult;
	UPubnubJsonUtilities::GetChannelMetadataJsonToData(JsonResponse, SetChannelMetadataResult.Result, SetChannelMetadataResult.ChannelData);
							
	return SetChannelMetadataResult;
}

FPubnubChannelMetadataResult UPubnubClient::GetChannelMetadata_priv(FString Channel, FString Include)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubChannelMetadataResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FPubnubChannelMetadataResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubChannelMetadataResult());

	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder IncludeHolder(Include);
	
	pubnub_get_channelmetadata(ctx_pub, IncludeHolder.Get(), ChannelHolder.Get());

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}

	PubnubOperationMutex.Unlock();
	
	//Parse Json response into data
	FPubnubChannelMetadataResult GetChannelMetadataResult;
	UPubnubJsonUtilities::GetChannelMetadataJsonToData(JsonResponse, GetChannelMetadataResult.Result, GetChannelMetadataResult.ChannelData);
							
	return GetChannelMetadataResult;
}

FPubnubOperationResult UPubnubClient::RemoveChannelMetadata_priv(FString Channel)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	FUTF8StringHolder ChannelHolder(Channel);

	pubnub_remove_channelmetadata(ctx_pub, ChannelHolder.Get());

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	PubnubOperationMutex.Unlock();
	
	return UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(JsonResponse);
}

FPubnubMembershipsResult UPubnubClient::GetMemberships_priv(FString User, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubMembershipsResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(User, FPubnubMembershipsResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubMembershipsResult());
	
	pubnub_membership_opts PubnubOptions = pubnub_membership_opts();
	FUTF8StringHolder UserHolder(User);
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);
	PubnubOptions.uuid = User.IsEmpty() ? NULL : UserHolder.Get();
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	PubnubOptions.page.next = Page.Next.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = Page.Prev.IsEmpty() ? NULL :  PagePrevHolder.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;
	
	pubnub_get_memberships_ex(ctx_pub, PubnubOptions);

	FString JsonResponse = GetLastResponse(ctx_pub);
	//If last response is empty, it means that there was an error, so return server response instead
	if(JsonResponse.IsEmpty())
	{
		JsonResponse = UPubnubUtilities::PubnubGetLastServerHttpResponse(ctx_pub);
	}
	
	PubnubOperationMutex.Unlock();
	
	//Parse Json response into data
	FPubnubMembershipsResult GetMembershipsResult;
	UPubnubJsonUtilities::GetMembershipsJsonToData(JsonResponse, GetMembershipsResult.Result, GetMembershipsResult.MembershipsData, GetMembershipsResult.Page, GetMembershipsResult.TotalCount);
	
	return GetMembershipsResult;
}

FPubnubMembershipsResult UPubnubClient::SetMemberships_priv(FString User, FString SetObj, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubMembershipsResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(User, FPubnubMembershipsResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(SetObj, FPubnubMembershipsResult());
	//Make sure that provided SetObj is a correct Json string
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(UPubnubJsonUtilities::IsCorrectJsonString(SetObj, false), TEXT("SetObj has to be a correct Json Object. Operation aborted."),  FPubnubMembershipsResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubMembershipsResult());

	pubnub_membership_opts PubnubOptions = pubnub_membership_opts();
	FUTF8StringHolder UserHolder(User);
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);
	PubnubOptions.uuid = User.IsEmpty() ? NULL : UserHolder.Get();
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	PubnubOptions.page.next = Page.Next.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = Page.Prev.IsEmpty() ? NULL :  PagePrevHolder.Get();
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
	
	PubnubOperationMutex.Unlock();
	
	//Parse Json response into data
	FPubnubMembershipsResult SetMembershipsResult;
	UPubnubJsonUtilities::GetMembershipsJsonToData(JsonResponse, SetMembershipsResult.Result, SetMembershipsResult.MembershipsData, SetMembershipsResult.Page, SetMembershipsResult.TotalCount);
	
	return SetMembershipsResult;
}

FPubnubMembershipsResult UPubnubClient::RemoveMemberships_priv(FString User, FString RemoveObj, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubMembershipsResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(User, FPubnubMembershipsResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(RemoveObj, FPubnubMembershipsResult());
	//Make sure that provided RemoveObj is a correct Json string
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(UPubnubJsonUtilities::IsCorrectJsonString(RemoveObj, false), TEXT("RemoveObj has to be a correct Json Object. Operation aborted."),  FPubnubMembershipsResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubMembershipsResult());

	pubnub_membership_opts PubnubOptions = pubnub_membership_opts();
	FUTF8StringHolder UserHolder(User);
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);
	PubnubOptions.uuid = User.IsEmpty() ? NULL : UserHolder.Get();
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	PubnubOptions.page.next = Page.Next.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = Page.Prev.IsEmpty() ? NULL :  PagePrevHolder.Get();
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
	
	PubnubOperationMutex.Unlock();
	
	//Parse Json response into data
	FPubnubMembershipsResult RemoveMembershipsResult;
	UPubnubJsonUtilities::GetMembershipsJsonToData(JsonResponse, RemoveMembershipsResult.Result, RemoveMembershipsResult.MembershipsData, RemoveMembershipsResult.Page, RemoveMembershipsResult.TotalCount);
	
	return RemoveMembershipsResult;
}

FPubnubChannelMembersResult UPubnubClient::GetChannelMembers_priv(FString Channel, FString Include, int Limit,
	FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubChannelMembersResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FPubnubChannelMembersResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubChannelMembersResult());
	
	pubnub_members_opts PubnubOptions = pubnub_members_opts();
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	PubnubOptions.page.next = Page.Next.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = Page.Prev.IsEmpty() ? NULL :  PagePrevHolder.Get();
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

	PubnubOperationMutex.Unlock();
	
	//Parse Json response into data
	FPubnubChannelMembersResult GetChannelMembersResult;
	UPubnubJsonUtilities::GetChannelMembersJsonToData(JsonResponse, GetChannelMembersResult.Result, GetChannelMembersResult.MembersData, GetChannelMembersResult.Page, GetChannelMembersResult.TotalCount);
	
	return GetChannelMembersResult;
}

FPubnubChannelMembersResult UPubnubClient::SetChannelMembers_priv(FString Channel, FString SetObj, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubChannelMembersResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FPubnubChannelMembersResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(SetObj, FPubnubChannelMembersResult());
	//Make sure that provided SetObj is a correct Json string
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(UPubnubJsonUtilities::IsCorrectJsonString(SetObj, false), TEXT("SetObj has to be a correct Json Object. Operation aborted."),  FPubnubChannelMembersResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubChannelMembersResult());

	pubnub_members_opts PubnubOptions = pubnub_members_opts();
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	PubnubOptions.page.next = Page.Next.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = Page.Prev.IsEmpty() ? NULL :  PagePrevHolder.Get();
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
	
	PubnubOperationMutex.Unlock();
	
	//Parse Json response into data
	FPubnubChannelMembersResult SetChannelMembersResult;
	UPubnubJsonUtilities::GetChannelMembersJsonToData(JsonResponse, SetChannelMembersResult.Result, SetChannelMembersResult.MembersData, SetChannelMembersResult.Page, SetChannelMembersResult.TotalCount);

	return SetChannelMembersResult;
}

FPubnubChannelMembersResult UPubnubClient::RemoveChannelMembers_priv(FString Channel, FString RemoveObj, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubChannelMembersResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FPubnubChannelMembersResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(RemoveObj, FPubnubChannelMembersResult());
	//Make sure that provided RemoveObj is a correct Json string
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(UPubnubJsonUtilities::IsCorrectJsonString(RemoveObj, false), TEXT("RemoveObj has to be a correct Json Object. Operation aborted."),  FPubnubChannelMembersResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubChannelMembersResult());

	pubnub_members_opts PubnubOptions = pubnub_members_opts();
	FUTF8StringHolder IncludeHolder(Include);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);
	PubnubOptions.include = Include.IsEmpty() ? NULL : IncludeHolder.Get();
	PubnubOptions.filter = Filter.IsEmpty() ? NULL :  FilterHolder.Get();
	PubnubOptions.sort = Sort.IsEmpty() ? NULL :  SortHolder.Get();
	PubnubOptions.page.next = Page.Next.IsEmpty() ? NULL :  PageNextHolder.Get();
	PubnubOptions.page.prev = Page.Prev.IsEmpty() ? NULL :  PagePrevHolder.Get();
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
	
	PubnubOperationMutex.Unlock();
	
	//Parse Json response into data
	FPubnubChannelMembersResult RemoveChannelMembersResult;
	UPubnubJsonUtilities::GetChannelMembersJsonToData(JsonResponse, RemoveChannelMembersResult.Result, RemoveChannelMembersResult.MembersData, RemoveChannelMembersResult.Page, RemoveChannelMembersResult.TotalCount);

	return RemoveChannelMembersResult;
}

FPubnubAddMessageActionResult UPubnubClient::AddMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionType,  FString Value)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubAddMessageActionResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FPubnubAddMessageActionResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(MessageTimetoken, FPubnubAddMessageActionResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(ActionType, FPubnubAddMessageActionResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Value, FPubnubAddMessageActionResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubAddMessageActionResult());
	
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
	
	PubnubOperationMutex.Unlock();
	
	//Parse Json response into data
	FPubnubAddMessageActionResult AddMessageActionResult;
	UPubnubJsonUtilities::AddMessageActionJsonToData(JsonResponse, AddMessageActionResult.Result, AddMessageActionResult.MessageActionData);
	
	return AddMessageActionResult;
}

FPubnubOperationResult UPubnubClient::RemoveMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionTimetoken)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(MessageTimetoken);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(ActionTimetoken);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();
	
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

	PubnubOperationMutex.Unlock();

	return UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(JsonResponse);
}

FPubnubGetMessageActionsResult UPubnubClient::GetMessageActions_priv(FString Channel, FString Start, FString End, int Limit)
{
	PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(FPubnubGetMessageActionsResult());
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FPubnubGetMessageActionsResult());
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FPubnubGetMessageActionsResult());
	
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

	PubnubOperationMutex.Unlock();

	//Parse Json response into data
	FPubnubGetMessageActionsResult GetMessageActionsResult;
	UPubnubJsonUtilities::GetMessageActionsJsonToData(JsonResponse, GetMessageActionsResult.Result, GetMessageActionsResult.MessageActions);
										
	return GetMessageActionsResult;
}


void UPubnubClient::SubscribeWithSubscriptionAsync(UPubnubSubscription* Subscription, FPubnubSubscriptionCursor Cursor, FOnPubnubSubscribeOperationResponseNative OnSubscribeResponse)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(OnSubscribeResponse);

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Subscription, Cursor, OnSubscribeResponse]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult SubscribeResult = WeakThis.Get()->SubscribeWithSubscription(Subscription, Cursor);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(OnSubscribeResponse, SubscribeResult);
	});
}

FPubnubOperationResult UPubnubClient::SubscribeWithSubscription(UPubnubSubscription* Subscription, FPubnubSubscriptionCursor Cursor)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(Subscription, TEXT("Subscription is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(Subscription->CCoreSubscription, TEXT("CCoreSubscription is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(!Subscription->bIsSubscribed, TEXT("Subscription is already subscribed."));

	FPubnubOperationResult SubscribeResult = ExecuteSerializedSubscriptionOperation(
		TEXT("Failed to subscribe with Subscription."),
		TEXT("Subscribe operation timed out"),
		[&]()
		{
			if(!UPubnubInternalUtilities::EESubscribeWithSubscription(Subscription->CCoreSubscription, Cursor))
			{
				PubnubError("[SubscribeWithSubscription]: Failed to subscribe with subscription.");
				return false;
			}
			return true;
		});

	if(!SubscribeResult.Error)
	{
		Subscription->bIsSubscribed = true;
		//C-Core subscribe operation needs some time to operate, so it's not hanged in case of immediate unsubscribe
		FPlatformProcess::Sleep(0.05f);
	}

	return SubscribeResult;
}

void UPubnubClient::SubscribeWithSubscriptionSetAsync(UPubnubSubscriptionSet* SubscriptionSet, FPubnubSubscriptionCursor Cursor, FOnPubnubSubscribeOperationResponseNative OnSubscribeResponse)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(OnSubscribeResponse);

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, SubscriptionSet, Cursor, OnSubscribeResponse]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult SubscribeResult = WeakThis.Get()->SubscribeWithSubscriptionSet(SubscriptionSet, Cursor);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(OnSubscribeResponse, SubscribeResult);
	});
}

FPubnubOperationResult UPubnubClient::SubscribeWithSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet, FPubnubSubscriptionCursor Cursor)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(SubscriptionSet, TEXT("SubscriptionSet is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(SubscriptionSet->CCoreSubscriptionSet, TEXT("CCoreSubscriptionSet is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(!SubscriptionSet->bIsSubscribed, TEXT("SubscriptionSet is already subscribed."));

	FPubnubOperationResult SubscribeResult = ExecuteSerializedSubscriptionOperation(
		TEXT("Failed to subscribe with SubscriptionSet."),
		TEXT("Subscribe operation timed out"),
		[&]()
		{
			if(!UPubnubInternalUtilities::EESubscribeWithSubscriptionSet(SubscriptionSet->CCoreSubscriptionSet, Cursor))
			{
				PubnubError("[SubscribeWithSubscriptionSet]: Failed to subscribe with subscription set.");
				return false;
			}
			return true;
		});

	if(!SubscribeResult.Error)
	{
		SubscriptionSet->bIsSubscribed = true;
		//C-Core subscribe operation needs some time to operate, so it's not hanged in case of immediate unsubscribe
		FPlatformProcess::Sleep(0.05f);
	}

	return SubscribeResult;
}

void UPubnubClient::UnsubscribeWithSubscriptionAsync(UPubnubSubscription* Subscription, FOnPubnubSubscribeOperationResponseNative OnUnsubscribeResponse)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(OnUnsubscribeResponse);

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Subscription, OnUnsubscribeResponse]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult UnsubscribeResult = WeakThis.Get()->UnsubscribeWithSubscription(Subscription);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(OnUnsubscribeResponse, UnsubscribeResult);
	});
}

FPubnubOperationResult UPubnubClient::UnsubscribeWithSubscription(UPubnubSubscription* Subscription)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(Subscription, TEXT("Subscription is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(Subscription->CCoreSubscription, TEXT("Subscription CCoreSubscription is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(Subscription->bIsSubscribed, TEXT("Subscription is not subscribed."));

	FScopeLock SubscriptionExecutionLock(&SubscriptionOperationExecutionMutex);
	if(!UPubnubInternalUtilities::EEUnsubscribeWithSubscription(&Subscription->CCoreSubscription))
	{
		PubnubError("[UnsubscribeWithSubscription]: Failed to unsubscribe with subscription.");
		return FPubnubOperationResult({0, true, "Failed to unsubscribe with Subscription."});
	}

	Subscription->bIsSubscribed = false;
	return FPubnubOperationResult({200, false, ""});
}

void UPubnubClient::UnsubscribeWithSubscriptionSetAsync(UPubnubSubscriptionSet* SubscriptionSet, FOnPubnubSubscribeOperationResponseNative OnUnsubscribeResponse)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(OnUnsubscribeResponse);

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, SubscriptionSet, OnUnsubscribeResponse]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubOperationResult UnsubscribeResult = WeakThis.Get()->UnsubscribeWithSubscriptionSet(SubscriptionSet);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(OnUnsubscribeResponse, UnsubscribeResult);
	});
}

FPubnubOperationResult UPubnubClient::UnsubscribeWithSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET();
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(SubscriptionSet, TEXT("SubscriptionSet is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(SubscriptionSet->CCoreSubscriptionSet, TEXT("SubscriptionSet CCoreSubscriptionSet is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(SubscriptionSet->bIsSubscribed, TEXT("SubscriptionSet is not subscribed."));

	FScopeLock SubscriptionExecutionLock(&SubscriptionOperationExecutionMutex);
	if(!UPubnubInternalUtilities::EEUnsubscribeWithSubscriptionSet(&SubscriptionSet->CCoreSubscriptionSet))
	{
		PubnubError("[UnsubscribeWithSubscriptionSet]: Failed to unsubscribe with subscription set.");
		return FPubnubOperationResult({0, true, "Failed to unsubscribe with SubscriptionSet."});
	}

	SubscriptionSet->bIsSubscribed = false;
	return FPubnubOperationResult({200, false, ""});
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
