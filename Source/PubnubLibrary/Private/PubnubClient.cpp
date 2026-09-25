// Copyright 2026 PubNub Inc. All Rights Reserved.


#include "PubnubClient.h"
#include "PubNub.h"
#include "PubnubInternalMacros.h"
#include "Logging/PubnubLogManager.h"
#include "FunctionLibraries/PubnubInternalUtilities.h"
#include "PubnubInternalStructLibrary.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "FunctionLibraries/PubnubTokenUtilities.h"
#include "FunctionLibraries/PubnubUtilities.h"
#include "PubnubDefaultLogger.h"
#include "Threads/PubnubFunctionThread.h"
#include "PubnubSubsystem.h"

#include "pubnub/future.h"
#include "Entities/PubnubBaseEntity.h"
#include "Entities/PubnubChannelEntity.h"
#include "Entities/PubnubChannelGroupEntity.h"
#include "Entities/PubnubChannelMetadataEntity.h"
#include "Entities/PubnubUserMetadataEntity.h"
#include "Entities/PubnubSubscription.h"
#include "Crypto/PubnubCryptoModule.h"


namespace
{
	/** C++-safe equivalent of PUBNUB_FUTURE_INVALID. */
	pubnub_future_t MakeInvalidFuture()
	{
		pubnub_future_t future = {};
		future.ctx = nullptr;
		future.slot_id = PUBNUB_SLOT_ID_INVALID;
		future.generation = 0;
		future.status = PUBNUB_ERR_INVALID_ARGUMENT;
		return future;
	}

	void DestroySubscriptionMap(TMap<FString, pubnub_subscription_t>& Subscriptions)
	{
		for (auto& Pair : Subscriptions)
		{
			if (Pair.Value)
			{
				pubnub_subscription_destroy(Pair.Value);
			}
		}
		Subscriptions.Empty();
	}
}

void UPubnubClient::DestroyClient()
{
	if(!PubnubSubsystem)
	{return;}

	PubnubSubsystem->DestroyPubnubClient(this);
}

void UPubnubClient::BeginDestroy()
{
	if(IsInitialized.load(std::memory_order_acquire))
	{
		DeinitializeClient();
	}
	
	Super::BeginDestroy();
}


FPubnubOperationResult UPubnubClient::SetUserID(FString UserID)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return SetUserID_priv(UserID);
}

FString UPubnubClient::GetUserID()
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED("");
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return GetUserID_priv();
}


FPubnubPublishMessageResult UPubnubClient::PublishMessage(FString Channel, FString Message, FPubnubPublishSettings PublishSettings)
{
	FPubnubPublishMessageResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	
	return PublishMessage_priv(Channel, Message, PublishSettings);
}

void UPubnubClient::PublishMessageAsync(FString Channel, FString Message, FOnPubnubPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings)
{
	FOnPubnubPublishMessageResponseNative NativeCallback;
	NativeCallback.BindLambda([OnPublishMessageResponse](const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
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
	FPubnubSignalResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	
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
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
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
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
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
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
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
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
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
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
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
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	
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
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	
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
	FPubnubListChannelsFromGroupResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	
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
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	
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
	FPubnubListUsersFromChannelResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	
	return ListUsersFromChannel_priv(Channel, ListUsersFromChannelSettings);
}

void UPubnubClient::ListUsersFromChannelAsync(FString Channel, FOnPubnubListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	FOnPubnubListUsersFromChannelResponseNative NativeCallback;
	NativeCallback.BindLambda([ListUsersFromChannelResponse](const FPubnubOperationResult& Result, int TotalOccupancy, int TotalChannels, const TArray<FPubnubUsersFromChannel>& Channels)
	{
		ListUsersFromChannelResponse.ExecuteIfBound(Result, TotalOccupancy, TotalChannels, Channels);
	});

	ListUsersFromChannelAsync(Channel, NativeCallback, ListUsersFromChannelSettings);
}

void UPubnubClient::ListUsersFromChannelAsync(FString Channel, FOnPubnubListUsersFromChannelResponseNative NativeCallback, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, 0, 0, TArray<FPubnubUsersFromChannel>{});
	
	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, NativeCallback, ListUsersFromChannelSettings]
	{
		if(!WeakThis.IsValid())
		{return;}
		
		FPubnubListUsersFromChannelResult Result = WeakThis.Get()->ListUsersFromChannel_priv(Channel, ListUsersFromChannelSettings);
		
		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result.Result, Result.TotalOccupancy, Result.TotalChannels, Result.Channels);
	});
}

FPubnubListUsersSubscribedChannelsResult UPubnubClient::ListUserSubscribedChannels(FString UserID)
{
	FPubnubListUsersSubscribedChannelsResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	
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
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	
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
	FPubnubGetStateResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return GetState_priv(Channel, ChannelGroup, UserID);
}

void UPubnubClient::GetStateAsync(FString Channel, FString ChannelGroup, FString UserID, FOnPubnubGetStateResponse OnGetStateResponse)
{
	FOnPubnubGetStateResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetStateResponse](const FPubnubOperationResult& Result, const TArray<FPubnubUserStateOnChannel>& States)
	{
		OnGetStateResponse.ExecuteIfBound(Result, States);
	});

	GetStateAsync(Channel, ChannelGroup, UserID, NativeCallback);
}

void UPubnubClient::GetStateAsync(FString Channel, FString ChannelGroup, FString UserID, FOnPubnubGetStateResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubUserStateOnChannel>{});

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, Channel, ChannelGroup, UserID, NativeCallback]
	{
		if(!WeakThis.IsValid())
		{return;}

		FPubnubGetStateResult Result = WeakThis.Get()->GetState_priv(Channel, ChannelGroup, UserID);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result.Result, Result.States);
	});
}

FPubnubGrantTokenResult UPubnubClient::GrantToken(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FString Meta)
{
	FPubnubGrantTokenResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return GrantToken_priv(Ttl, AuthorizedUser, Permissions, Meta);
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

		FPubnubGrantTokenResult Result = WeakThis.Get()->GrantToken_priv(Ttl, AuthorizedUser, Permissions, Meta);

		//Execute provided delegate with results
		UPubnubUtilities::CallPubnubDelegate(NativeCallback, Result.Result, Result.Token);
	});
}

FPubnubOperationResult UPubnubClient::RevokeToken(FString Token)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

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
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return ParseToken_priv(Token);
}

void UPubnubClient::SetAuthToken(FString Token)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	SetAuthToken_priv(Token);
}

void UPubnubClient::SetAuthTokenAsync(FString Token)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue([WeakThis, Token]
	{
		if(!WeakThis.IsValid())
		{return;}

		WeakThis.Get()->SetAuthToken_priv(Token);
	});
}

FPubnubOperationResult UPubnubClient::SetOrigin(FString Origin)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return SetOrigin_priv(Origin);
}

FString UPubnubClient::GetOrigin() const
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED("");

	return GetOrigin_priv();
}

FPubnubFetchHistoryResult UPubnubClient::FetchHistory(FString Channel, FPubnubFetchHistorySettings FetchHistorySettings)
{
	FPubnubFetchHistoryResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	
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
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	
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
	FPubnubMessageCountsResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	
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
	FPubnubMessageCountsMultipleResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	
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
		PUBNUB_LOG_FUNCTION_ERROR(FString::Printf(TEXT("[%s]: PubnubClient is not initialized. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__))));
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
		PUBNUB_LOG_FUNCTION_ERROR(FString::Printf(TEXT("[%s]: PubnubCallsThread is invalid. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__))));
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
	FPubnubGetAllUserMetadataResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

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

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, NativeCallback, Include, Limit, Filter, Sort, Page, Count]
	{
		if(!WeakThis.IsValid())
		{return;}

		FPubnubGetAllUserMetadataResult GetAllUserMetadataResult = WeakThis.Get()->GetAllUserMetadata_priv(Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);

		UPubnubUtilities::CallPubnubDelegate(NativeCallback, GetAllUserMetadataResult.Result, GetAllUserMetadataResult.UsersData, GetAllUserMetadataResult.Page, GetAllUserMetadataResult.TotalCount);
	});
}

FPubnubGetAllUserMetadataResult UPubnubClient::GetAllUserMetadata(FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FPubnubPage Page)
{
	FPubnubGetAllUserMetadataResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return GetAllUserMetadata_priv(UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetAllUserMetadataAsync(FOnPubnubGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FPubnubPage Page)
{
	GetAllUserMetadataRawAsync(OnGetAllUserMetadataResponse, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetAllUserMetadataAsync(FOnPubnubGetAllUserMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FPubnubPage Page)
{
	GetAllUserMetadataRawAsync(NativeCallback, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

FPubnubUserMetadataResult UPubnubClient::SetUserMetadataRaw(FString User, FString UserMetadataObj, FString Include)
{
	FPubnubUserMetadataResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

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

		UPubnubUtilities::CallPubnubDelegate(NativeCallback, SetUserMetadataResult.Result, SetUserMetadataResult.UserData);
	});
}

FPubnubUserMetadataResult UPubnubClient::SetUserMetadata(FString User, FPubnubUserInputData UserMetadata, FPubnubGetMetadataInclude Include)
{
	FPubnubUserMetadataResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

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
	FPubnubUserMetadataResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

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

		UPubnubUtilities::CallPubnubDelegate(NativeCallback, GetUserMetadataResult.Result, GetUserMetadataResult.UserData);
	});
}

FPubnubUserMetadataResult UPubnubClient::GetUserMetadata(FString User, FPubnubGetMetadataInclude Include)
{
	FPubnubUserMetadataResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

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
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

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

		UPubnubUtilities::CallPubnubDelegate(NativeCallback, RemoveUserMetadataResult);
	});
}

FPubnubGetAllChannelMetadataResult UPubnubClient::GetAllChannelMetadataRaw(FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FPubnubGetAllChannelMetadataResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

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

	PubnubCallsThread->AddFunctionToQueue( [WeakThis, NativeCallback, Include, Limit, Filter, Sort, Page, Count]
	{
		if(!WeakThis.IsValid())
		{return;}

		FPubnubGetAllChannelMetadataResult GetAllChannelMetadataResult = WeakThis.Get()->GetAllChannelMetadata_priv(Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);

		UPubnubUtilities::CallPubnubDelegate(NativeCallback, GetAllChannelMetadataResult.Result, GetAllChannelMetadataResult.ChannelsData, GetAllChannelMetadataResult.Page, GetAllChannelMetadataResult.TotalCount);
	});
}

FPubnubGetAllChannelMetadataResult UPubnubClient::GetAllChannelMetadata(FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FPubnubPage Page)
{
	FPubnubGetAllChannelMetadataResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return GetAllChannelMetadata_priv(UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetAllChannelMetadataAsync(FOnPubnubGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FPubnubPage Page)
{
	GetAllChannelMetadataRawAsync(OnGetAllChannelMetadataResponse, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetAllChannelMetadataAsync(FOnPubnubGetAllChannelMetadataResponseNative NativeCallback, FPubnubGetAllInclude Include, int Limit, FString Filter, FPubnubGetAllSort Sort, FPubnubPage Page)
{
	GetAllChannelMetadataRawAsync(NativeCallback, UPubnubUtilities::GetAllIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::GetAllSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

FPubnubChannelMetadataResult UPubnubClient::SetChannelMetadataRaw(FString Channel, FString ChannelMetadataObj, FString Include)
{
	FPubnubChannelMetadataResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

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

		UPubnubUtilities::CallPubnubDelegate(NativeCallback, SetChannelMetadataResult.Result, SetChannelMetadataResult.ChannelData);
	});
}

FPubnubChannelMetadataResult UPubnubClient::SetChannelMetadata(FString Channel, FPubnubChannelInputData ChannelMetadata, FPubnubGetMetadataInclude Include)
{
	FPubnubChannelMetadataResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

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
	FPubnubChannelMetadataResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

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

		UPubnubUtilities::CallPubnubDelegate(NativeCallback, GetChannelMetadataResult.Result, GetChannelMetadataResult.ChannelData);
	});
}

FPubnubChannelMetadataResult UPubnubClient::GetChannelMetadata(FString Channel, FPubnubGetMetadataInclude Include)
{
	FPubnubChannelMetadataResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

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
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

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

		UPubnubUtilities::CallPubnubDelegate(NativeCallback, RemoveChannelMetadataResult);
	});
}

FPubnubMembershipsResult UPubnubClient::GetMembershipsRaw(FString User, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FPubnubMembershipsResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return GetMemberships_priv(User, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::GetMembershipsRawAsync(FString User, FOnPubnubGetMembershipsResponse OnGetMembershipsResponse, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FOnPubnubGetMembershipsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnGetMembershipsResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
	{
		OnGetMembershipsResponse.ExecuteIfBound(Result, MembershipsData, Page, TotalCount);
	});
	GetMembershipsRawAsync(User, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::GetMembershipsRawAsync(FString User, FOnPubnubGetMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FPubnubPage(), 0);

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue([WeakThis, User, NativeCallback, Include, Limit, Filter, Sort, Page, Count]
	{
		if(!WeakThis.IsValid())
		{return;}

		FPubnubMembershipsResult GetMembershipsResult = WeakThis.Get()->GetMemberships_priv(User, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);

		UPubnubUtilities::CallPubnubDelegate(NativeCallback, GetMembershipsResult.Result, GetMembershipsResult.MembershipsData, GetMembershipsResult.Page, GetMembershipsResult.TotalCount);
	});
}

FPubnubMembershipsResult UPubnubClient::GetMemberships(FString User, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	FPubnubMembershipsResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return GetMemberships_priv(User, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetMembershipsAsync(FString User, FOnPubnubGetMembershipsResponse OnGetMembershipsResponse, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	GetMembershipsRawAsync(User, OnGetMembershipsResponse, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::GetMembershipsAsync(FString User, FOnPubnubGetMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	GetMembershipsRawAsync(User, NativeCallback, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

FPubnubMembershipsResult UPubnubClient::SetMembershipsRaw(FString User, FString SetObj, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FPubnubMembershipsResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return SetMemberships_priv(User, SetObj, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::SetMembershipsRawAsync(FString User, FString SetObj, FOnPubnubSetMembershipsResponse OnSetMembershipsResponse, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FOnPubnubSetMembershipsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSetMembershipsResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
	{
		OnSetMembershipsResponse.ExecuteIfBound(Result, MembershipsData, Page, TotalCount);
	});
	SetMembershipsRawAsync(User, SetObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::SetMembershipsRawAsync(FString User, FString SetObj, FOnPubnubSetMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FPubnubPage(), 0);

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue([WeakThis, User, SetObj, NativeCallback, Include, Limit, Filter, Sort, Page, Count]
	{
		if(!WeakThis.IsValid())
		{return;}

		FPubnubMembershipsResult SetMembershipsResult = WeakThis.Get()->SetMemberships_priv(User, SetObj, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);

		UPubnubUtilities::CallPubnubDelegate(NativeCallback, SetMembershipsResult.Result, SetMembershipsResult.MembershipsData, SetMembershipsResult.Page, SetMembershipsResult.TotalCount);
	});
}

FPubnubMembershipsResult UPubnubClient::SetMemberships(FString User, TArray<FPubnubMembershipInputData> Channels, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	FPubnubMembershipsResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return SetMemberships_priv(User, UPubnubJsonUtilities::GetJsonFromMembershipsDataArray(Channels), UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::SetMembershipsAsync(FString User, TArray<FPubnubMembershipInputData> Channels, FOnPubnubSetMembershipsResponse OnSetMembershipsResponse, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	SetMembershipsRawAsync(User, UPubnubJsonUtilities::GetJsonFromMembershipsDataArray(Channels), OnSetMembershipsResponse, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::SetMembershipsAsync(FString User, TArray<FPubnubMembershipInputData> Channels, FOnPubnubSetMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	SetMembershipsRawAsync(User, UPubnubJsonUtilities::GetJsonFromMembershipsDataArray(Channels), NativeCallback, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

FPubnubMembershipsResult UPubnubClient::RemoveMembershipsRaw(FString User, FString RemoveObj, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FPubnubMembershipsResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return RemoveMemberships_priv(User, RemoveObj, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::RemoveMembershipsRawAsync(FString User, FString RemoveObj, FOnPubnubRemoveMembershipsResponse OnRemoveMembershipsResponse, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FOnPubnubRemoveMembershipsResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveMembershipsResponse](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FPubnubPage Page, int TotalCount)
	{
		OnRemoveMembershipsResponse.ExecuteIfBound(Result, MembershipsData, Page, TotalCount);
	});
	RemoveMembershipsRawAsync(User, RemoveObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::RemoveMembershipsRawAsync(FString User, FString RemoveObj, FOnPubnubRemoveMembershipsResponseNative NativeCallback, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, TArray<FPubnubMembershipData>(), FPubnubPage(), 0);

	TWeakObjectPtr<UPubnubClient> WeakThis = MakeWeakObjectPtr<UPubnubClient>(this);

	PubnubCallsThread->AddFunctionToQueue([WeakThis, User, RemoveObj, NativeCallback, Include, Limit, Filter, Sort, Page, Count]
	{
		if(!WeakThis.IsValid())
		{return;}

		FPubnubMembershipsResult RemoveMembershipsResult = WeakThis.Get()->RemoveMemberships_priv(User, RemoveObj, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);

		UPubnubUtilities::CallPubnubDelegate(NativeCallback, RemoveMembershipsResult.Result, RemoveMembershipsResult.MembershipsData, RemoveMembershipsResult.Page, RemoveMembershipsResult.TotalCount);
	});
}

FPubnubMembershipsResult UPubnubClient::RemoveMemberships(FString User, TArray<FString> Channels, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	FPubnubMembershipsResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return RemoveMemberships_priv(User, UPubnubJsonUtilities::GetJsonFromMembershipsToRemove(Channels), UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::RemoveMembershipsAsync(FString User, TArray<FString> Channels, FOnPubnubRemoveMembershipsResponse OnRemoveMembershipsResponse, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	RemoveMembershipsRawAsync(User, UPubnubJsonUtilities::GetJsonFromMembershipsToRemove(Channels), OnRemoveMembershipsResponse, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::RemoveMembershipsAsync(FString User, TArray<FString> Channels, FOnPubnubRemoveMembershipsResponseNative NativeCallback, FPubnubMembershipInclude Include, int Limit, FString Filter, FPubnubMembershipSort Sort, FPubnubPage Page)
{
	RemoveMembershipsRawAsync(User, UPubnubJsonUtilities::GetJsonFromMembershipsToRemove(Channels), NativeCallback, UPubnubUtilities::MembershipIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MembershipSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

FPubnubChannelMembersResult UPubnubClient::GetChannelMembersRaw(FString Channel, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FPubnubChannelMembersResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

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

		UPubnubUtilities::CallPubnubDelegate(NativeCallback, GetChannelMembersResult.Result, GetChannelMembersResult.MembersData, GetChannelMembersResult.Page, GetChannelMembersResult.TotalCount);
	});
}

FPubnubChannelMembersResult UPubnubClient::GetChannelMembers(FString Channel, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
	FPubnubChannelMembersResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

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
	FPubnubChannelMembersResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return SetChannelMembers_priv(Channel, SetObj, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::SetChannelMembersRawAsync(FString Channel, FString SetObj, FOnPubnubSetChannelMembersResponse OnSetChannelMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FOnPubnubSetChannelMembersResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSetChannelMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
	{
		OnSetChannelMembersResponse.ExecuteIfBound(Result, MembersData, Page, TotalCount);
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

		UPubnubUtilities::CallPubnubDelegate(NativeCallback, SetChannelMembersResult.Result, SetChannelMembersResult.MembersData, SetChannelMembersResult.Page, SetChannelMembersResult.TotalCount);
	});
}

FPubnubChannelMembersResult UPubnubClient::SetChannelMembers(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
	FPubnubChannelMembersResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return SetChannelMembers_priv(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersDataArray(Users), UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::SetChannelMembersAsync(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FOnPubnubSetChannelMembersResponse OnSetChannelMembersResponse, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
	SetChannelMembersRawAsync(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersDataArray(Users), OnSetChannelMembersResponse, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::SetChannelMembersAsync(FString Channel, TArray<FPubnubChannelMemberInputData> Users, FOnPubnubSetChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
	SetChannelMembersRawAsync(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersDataArray(Users), NativeCallback, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

FPubnubChannelMembersResult UPubnubClient::RemoveChannelMembersRaw(FString Channel, FString RemoveObj, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FPubnubChannelMembersResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return RemoveChannelMembers_priv(Channel, RemoveObj, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
}

void UPubnubClient::RemoveChannelMembersRawAsync(FString Channel, FString RemoveObj, FOnPubnubRemoveChannelMembersResponse OnRemoveChannelMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	FOnPubnubRemoveChannelMembersResponseNative NativeCallback;
	NativeCallback.BindLambda([OnRemoveChannelMembersResponse](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FPubnubPage Page, int TotalCount)
	{
		OnRemoveChannelMembersResponse.ExecuteIfBound(Result, MembersData, Page, TotalCount);
	});
	RemoveChannelMembersRawAsync(Channel, RemoveObj, NativeCallback, Include, UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, Sort, Page, Count);
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

		UPubnubUtilities::CallPubnubDelegate(NativeCallback, RemoveChannelMembersResult.Result, RemoveChannelMembersResult.MembersData, RemoveChannelMembersResult.Page, RemoveChannelMembersResult.TotalCount);
	});
}

FPubnubChannelMembersResult UPubnubClient::RemoveChannelMembers(FString Channel, TArray<FString> Users, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
	FPubnubChannelMembersResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();

	return RemoveChannelMembers_priv(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersToRemove(Users), UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::RemoveChannelMembersAsync(FString Channel, TArray<FString> Users, FOnPubnubRemoveChannelMembersResponse OnRemoveChannelMembersResponse, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
	RemoveChannelMembersRawAsync(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersToRemove(Users), OnRemoveChannelMembersResponse, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

void UPubnubClient::RemoveChannelMembersAsync(FString Channel, TArray<FString> Users, FOnPubnubRemoveChannelMembersResponseNative NativeCallback, FPubnubMemberInclude Include, int Limit, FString Filter, FPubnubMemberSort Sort, FPubnubPage Page)
{
	RemoveChannelMembersRawAsync(Channel, UPubnubJsonUtilities::GetJsonFromChannelMembersToRemove(Users), NativeCallback, UPubnubUtilities::MemberIncludeToString(Include), UPubnubUtilities::RoundLimitForPubnubFunctions(Limit), Filter, UPubnubUtilities::MemberSortToString(Sort), Page, (EPubnubTribool)Include.IncludeTotalCount);
}

FPubnubAddMessageActionResult UPubnubClient::AddMessageAction(FString Channel, FString MessageTimetoken, FString ActionType, FString Value)
{
	FPubnubAddMessageActionResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
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
	FPubnubGetMessageActionsResult FinalResult;
	PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(FinalResult);
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
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
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
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
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	return ReconnectSubscriptions_priv(Timetoken);
}

FPubnubOperationResult UPubnubClient::DisconnectSubscriptions()
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	return DisconnectSubscriptions_priv();
}

void UPubnubClient::AddLogger(TScriptInterface<IPubnubLoggerInterface> Logger)
{
	if (!LoggerManager)
	{
		return;
	}
	LoggerManager->AddLogger(Logger);
}

void UPubnubClient::RemoveLogger(TScriptInterface<IPubnubLoggerInterface> Logger)
{
	if (!LoggerManager)
	{
		return;
	}
	LoggerManager->RemoveLogger(Logger);
}

void UPubnubClient::ClearLoggers()
{
	if (!LoggerManager)
	{
		return;
	}
	LoggerManager->ClearLoggers();
}

TArray<TScriptInterface<IPubnubLoggerInterface>> UPubnubClient::GetLoggers()
{
	if (!LoggerManager)
	{
		return {};
	}
	return LoggerManager->GetLoggers();
}

bool UPubnubClient::InitWithConfig(UPubnubSubsystem* InPubnubSubsystem, FPubnubConfig InConfig, int InClientID, FString InDebugName )
{
	if(IsInitialized)
	{return false;}

	if(!ValidateConfig(InConfig))
	{return false;}

	PubnubSubsystem = InPubnubSubsystem;
	ClientID = InClientID;
	DebugName = InDebugName;
	PubnubConfig = InConfig;
	InFlightFuture = new pubnub_future_t(MakeInvalidFuture());

	InitLoggerManager(InConfig);
	PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("initializing pubnub client. ClientID=%d, DebugName=%s, Config=%s"), ClientID, *DebugName, *UPubnubLogUtilities::LogConfigToString(InConfig)));
	
	FScopeLock OperationLock(&PubnubOperationMutex);
	
	FUTF8StringHolder UserIDHolder(InConfig.UserID);
	FUTF8StringHolder PublishKeyHolder(InConfig.PublishKey);
	FUTF8StringHolder SubscribeKeyHolder(InConfig.SubscribeKey);
	FUTF8StringHolder SecretKeyHolder(InConfig.SecretKey);
	FUTF8StringHolder OriginHolder(InConfig.Origin);
	FString FinalPNSdk = InConfig.PNSdkOverride.IsEmpty() ? UPubnubInternalUtilities::GetPubnubSdkVersionSuffix() : InConfig.PNSdkOverride;
	FUTF8StringHolder PnsdkHolder(FinalPNSdk);
	
	pubnub_config_t config = pubnub_config_defaults();

	config.user_id = UserIDHolder.Get();
	config.publish_key = PublishKeyHolder.Get();
	config.subscribe_key = SubscribeKeyHolder.Get();
	config.secret_key = SecretKeyHolder.GetOrNull();
	config.origin = OriginHolder.Get();
	config.pnsdk_override = PnsdkHolder.Get();
	config.logger = LoggerManager->GetCCoreLoggerProvider();
	config.log_level = PUBNUB_LOG_LEVEL_TRACE;

	if (InConfig.CryptoModule)
	{
		config.crypto_module = InConfig.CryptoModule->GetCCoreModule();
		if (!config.crypto_module)
		{
			UE_LOG(PubnubLog, Error, TEXT("CryptoModule is set but InitCryptoModule has not created a C-Core module."));
			delete InFlightFuture;
			InFlightFuture = nullptr;
			DefaultLogger = nullptr;
			LoggerManager = nullptr;
			return false;
		}
	}

	pubnub_context = pubnub_create(&config);
	
	if (!pubnub_context)
	{
		delete InFlightFuture;
		InFlightFuture = nullptr;
		DefaultLogger = nullptr;
		LoggerManager = nullptr;
		UE_LOG(PubnubLog, Error, TEXT("Creating C-Core context failed."));
		return false;
	}

	if (!AddSubscribeListenerToPubnubContext())
	{
		PUBNUB_LOG_FUNCTION_ERROR(TEXT("failed to register subscription listener. Aborting client initialization."));
		pubnub_destroy(pubnub_context);
		pubnub_context = nullptr;
		delete InFlightFuture;
		InFlightFuture = nullptr;
		DefaultLogger = nullptr;
		LoggerManager = nullptr;
		return false;
	}

	IsInitialized.store(true, std::memory_order_release);

	PubnubCallsThread = new FPubnubFunctionThread;
	PUBNUB_LOG_FUNCTION_DEBUG_TEXT(TEXT("pubnub calls thread created."));
	PUBNUB_LOG_FUNCTION_INFO(FString::Printf(TEXT("client ready. ClientID=%d, DebugName=%s"), ClientID, *DebugName));
	
	return true;
}


bool UPubnubClient::ValidateConfig(const FPubnubConfig& InConfig)
{
	if(InConfig.PublishKey.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Publish key is empty, can't initialize Pubnub"));
		return false;
	}

	if(InConfig.SubscribeKey.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Subscribe key is empty, can't initialize Pubnub"));
		return false;
	}

	if(InConfig.UserID.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("User ID is empty, can't initialize Pubnub"));
		return false;
	}

	return true;
}

void UPubnubClient::InitLoggerManager(const FPubnubConfig& InConfig)
{
	LoggerManager = UPubnubInternalUtilities::SafeNewObject<UPubnubLogManager>(this);

	const FString EmitterID = DebugName.IsEmpty()
		? FString::Printf(TEXT("PubNub-%d"), ClientID)
		: FString::Printf(TEXT("PubNub-%d(\"%s\")"), ClientID, *DebugName);
	LoggerManager->SetUESdkEmitterID(EmitterID);

	if (InConfig.LoggerConfig.bEnableDefaultLogger)
	{
		DefaultLogger = UPubnubInternalUtilities::SafeNewObject<UPubnubDefaultLogger>(this);
		IPubnubLoggerInterface::Execute_SetMinimumLogLevel(DefaultLogger, InConfig.LoggerConfig.DefaultLoggerMinLevel);
		IPubnubLoggerInterface::Execute_SetMinimumCCoreLogLevel(DefaultLogger, InConfig.LoggerConfig.DefaultLoggerMinCCoreLevel);

		TScriptInterface<IPubnubLoggerInterface> DefaultLoggerInterface;
		DefaultLoggerInterface.SetObject(DefaultLogger);
		DefaultLoggerInterface.SetInterface(Cast<IPubnubLoggerInterface>(DefaultLogger));
		LoggerManager->AddLogger(DefaultLoggerInterface);
	}

	for (UObject* LoggerObject : InConfig.LoggerConfig.InitialLoggers)
	{
		if (!LoggerObject)
		{
			continue;
		}

		if (!LoggerObject->GetClass()->ImplementsInterface(UPubnubLoggerInterface::StaticClass()))
		{
			PUBNUB_LOG_FUNCTION_WARNING(TEXT("Skipping logger registration because object does not implement IPubnubLoggerInterface."));
			continue;
		}

		TScriptInterface<IPubnubLoggerInterface> LoggerInterface;
		LoggerInterface.SetObject(LoggerObject);
		LoggerInterface.SetInterface(Cast<IPubnubLoggerInterface>(LoggerObject));
		LoggerManager->AddLogger(LoggerInterface);
	}
}

bool UPubnubClient::AddSubscribeListenerToPubnubContext()
{
	if(!pubnub_context)
	{return false;}

	pubnub_subscribe_listener_t Listener = {0};
	Listener.on_status = +[](const pubnub_subscribe_status_event_t* StatusEvent, void* UserData)
	{
		UPubnubClient* ThisClient = static_cast<UPubnubClient*>(UserData);
		if(!ThisClient || !ThisClient->IsInitialized.load(std::memory_order_acquire))
		{return;}

		ThisClient->OnCCoreSubscriptionStatusReceived(StatusEvent);
	};
	Listener.on_message = +[](const pubnub_subscribe_event_t* Event, void* UserData)
	{
		UPubnubClient* ThisClient = static_cast<UPubnubClient*>(UserData);
		if(!ThisClient || !ThisClient->IsInitialized.load(std::memory_order_acquire))
		{return;}
		ThisClient->OnCCoreSubscribeEventReceived(Event);
	};
	Listener.on_signal = +[](const pubnub_subscribe_event_t* Event, void* UserData)
	{
		UPubnubClient* ThisClient = static_cast<UPubnubClient*>(UserData);
		if(!ThisClient || !ThisClient->IsInitialized.load(std::memory_order_acquire))
		{return;}
		ThisClient->OnCCoreSubscribeEventReceived(Event);
	};
	Listener.on_presence = +[](const pubnub_subscribe_event_t* Event, void* UserData)
	{
		UPubnubClient* ThisClient = static_cast<UPubnubClient*>(UserData);
		if(!ThisClient || !ThisClient->IsInitialized.load(std::memory_order_acquire))
		{return;}
		ThisClient->OnCCoreSubscribeEventReceived(Event);
	};
	Listener.on_message_action = +[](const pubnub_subscribe_event_t* Event, void* UserData)
	{
		UPubnubClient* ThisClient = static_cast<UPubnubClient*>(UserData);
		if(!ThisClient || !ThisClient->IsInitialized.load(std::memory_order_acquire))
		{return;}
		ThisClient->OnCCoreSubscribeEventReceived(Event);
	};
	Listener.on_app_context = +[](const pubnub_subscribe_event_t* Event, void* UserData)
	{
		UPubnubClient* ThisClient = static_cast<UPubnubClient*>(UserData);
		if(!ThisClient || !ThisClient->IsInitialized.load(std::memory_order_acquire))
		{return;}
		ThisClient->OnCCoreSubscribeEventReceived(Event);
	};
	Listener.on_file = +[](const pubnub_subscribe_event_t* Event, void* UserData)
	{
		UPubnubClient* ThisClient = static_cast<UPubnubClient*>(UserData);
		if(!ThisClient || !ThisClient->IsInitialized.load(std::memory_order_acquire))
		{return;}
		ThisClient->OnCCoreSubscribeEventReceived(Event);
	};
	Listener.user_data = this;
	SubStatusListenerHandle = pubnub_add_listener(pubnub_context, &Listener);
	if (SubStatusListenerHandle == PUBNUB_LISTENER_HANDLE_INVALID)
	{
		PUBNUB_LOG_FUNCTION_ERROR(TEXT("failed to register subscription listener."));
		return false;
	}

	PUBNUB_LOG_FUNCTION_TRACE(TEXT("subscription listener registered."));
	return true;
}


void UPubnubClient::OnCCoreSubscriptionStatusReceived(const pubnub_subscribe_status_event_t* StatusEvent)
{
	if(!StatusEvent)
	{return;}

	PUBNUB_LOG_FUNCTION_TRACE(FString::Printf(TEXT("called. Status=%d"), static_cast<int>(StatusEvent->status)));

	const pubnub_subscribe_status_t status = StatusEvent->status;
	const bool IsError = status == PUBNUB_SUBSCRIBE_STATUS_CONNECTION_ERROR || status == PUBNUB_SUBSCRIBE_STATUS_DISCONNECTED_UNEXPECTEDLY;
	FString Reason = StatusEvent->reason != PUBNUB_OK ? FString(pubnub_res_str(StatusEvent->reason)) : TEXT("");
	if (StatusEvent->http_status_code != 0)
	{
		Reason = Reason.IsEmpty()
			? FString::Printf(TEXT("HTTP %u"), StatusEvent->http_status_code)
			: FString::Printf(TEXT("%s (HTTP %u)"), *Reason, StatusEvent->http_status_code);
	}

	if (IsError)
	{
		PUBNUB_LOG_FUNCTION_ERROR(FString::Printf(TEXT("subscription status processed. Status=%d, Reason=%s"), static_cast<int>(status), *Reason));
	}
	else
	{
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("subscription status processed. Status=%d"), static_cast<int>(status)));
	}

	FPubnubSubscriptionStatusData SubscriptionStatusData;
	SubscriptionStatusData.Reason = Reason;

	if (status == PUBNUB_SUBSCRIBE_STATUS_CONNECTED || status == PUBNUB_SUBSCRIBE_STATUS_SUBSCRIPTION_CHANGED)
	{
		const FString ChannelsCsv = UPubnubInternalUtilities::PubnubStringViewToString(StatusEvent->channels);
		const FString GroupsCsv = UPubnubInternalUtilities::PubnubStringViewToString(StatusEvent->groups);
		if (!ChannelsCsv.IsEmpty())
		{
			ChannelsCsv.ParseIntoArray(SubscriptionStatusData.Channels, TEXT(","), true);
		}
		if (!GroupsCsv.IsEmpty())
		{
			GroupsCsv.ParseIntoArray(SubscriptionStatusData.ChannelGroups, TEXT(","), true);
		}
	}

	const EPubnubSubscriptionStatus FinalStatus = UPubnubInternalUtilities::SubscriptionStatusFromPubnubSubscribeStatus(status);

	//Dispatch SubscriptionStatusChanged delegates on the game thread.
	TWeakObjectPtr<UPubnubClient> ThisClientWeak = MakeWeakObjectPtr<UPubnubClient>(this);
	AsyncTask(ENamedThreads::GameThread, [ThisClientWeak, FinalStatus, SubscriptionStatusData]()
	{
		if(!ThisClientWeak.IsValid())
		{return;}

		UPubnubClient* ThisClient = ThisClientWeak.Get();
		if(ThisClient->OnSubscriptionStatusChanged.IsBound())
		{
			ThisClient->OnSubscriptionStatusChanged.Broadcast(FinalStatus, SubscriptionStatusData);
		}
		if(ThisClient->OnSubscriptionStatusChangedNative.IsBound())
		{
			ThisClient->OnSubscriptionStatusChangedNative.Broadcast(FinalStatus, SubscriptionStatusData);
		}
	});
}

void UPubnubClient::OnCCoreSubscribeEventReceived(const pubnub_subscribe_event_t* Event)
{
	if(!Event || !pubnub_context)
	{return;}

	const FPubnubMessageData MessageData = UPubnubInternalUtilities::UEMessageFromSubscribeEvent(pubnub_context, Event);
	TWeakObjectPtr<UPubnubClient> ThisClientWeak = MakeWeakObjectPtr<UPubnubClient>(this);
	AsyncTask(ENamedThreads::GameThread, [ThisClientWeak, MessageData]()
	{
		if(!ThisClientWeak.IsValid())
		{return;}

		UPubnubClient* ThisClient = ThisClientWeak.Get();
		if(ThisClient->OnMessageReceived.IsBound())
		{
			ThisClient->OnMessageReceived.Broadcast(MessageData);
		}
		if(ThisClient->OnMessageReceivedNative.IsBound())
		{
			ThisClient->OnMessageReceivedNative.Broadcast(MessageData);
		}
	});
}

void UPubnubClient::CleanUpSubscriptions()
{
	FScopeLock SubscriptionsLock(&SubscriptionsMutex);
	if(ChannelSubscriptions.IsEmpty() && ChannelGroupSubscriptions.IsEmpty())
	{
		return;
	}

	if(pubnub_context)
	{
		const pubnub_res_t UnsubscribeAllResult = pubnub_subscribe_unsubscribe_all(pubnub_context);
		if(UnsubscribeAllResult != PUBNUB_OK)
		{
			PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("unsubscribe all during cleanup returned: %s"), UTF8_TO_TCHAR(pubnub_res_str(UnsubscribeAllResult))));
		}
	}

	DestroySubscriptionMap(ChannelSubscriptions);
	DestroySubscriptionMap(ChannelGroupSubscriptions);
}

void UPubnubClient::DeinitializeClient()
{
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	if(!IsInitialized.load(std::memory_order_acquire))
	{return;}

	PUBNUB_LOG_FUNCTION_INFO(TEXT("deinitializing pubnub client."));
	OnClientDeinitializeStart.Broadcast();

	//Mark deinitializing FIRST so new public API calls short-circuit via PUBNUB_RETURN_*_IF_NOT_INITIALIZED before reaching the C-Core contexts.
	IsInitialized.store(false, std::memory_order_release);

	//Stop all current Pubnub calls on the queuing thread.
	if(PubnubCallsThread)
	{
		PubnubCallsThread->Stop();
		
		//Cancel if there is any ongoing Pubnub operation
		if(InFlightFuture && InFlightFuture->ctx)
		{
			pubnub_future_cancel(*InFlightFuture);
		}
		if (PubnubCallsThread->Thread)
		{
			PubnubCallsThread->Thread->WaitForCompletion();
		}
		PUBNUB_LOG_FUNCTION_TRACE(TEXT("pubnub calls thread stopped."));
	}

	PubnubSubsystem = nullptr;
	
	if (pubnub_context)
	{
		CleanUpSubscriptions();

		if (SubStatusListenerHandle != PUBNUB_LISTENER_HANDLE_INVALID)
		{
			pubnub_remove_listener(pubnub_context, SubStatusListenerHandle);
			SubStatusListenerHandle = PUBNUB_LISTENER_HANDLE_INVALID;
		}

		//Destroy the C-Core context.
		pubnub_destroy(pubnub_context);
		pubnub_context = nullptr;
	}
	
	PUBNUB_LOG_FUNCTION_DEBUG_TEXT(TEXT("C-Core context destroyed."));

	delete PubnubCallsThread;
	PubnubCallsThread = nullptr;

	delete InFlightFuture;
	InFlightFuture = nullptr;

	//Notify that Deinitialization is finished
	OnClientDeinitialized.Broadcast();
	PUBNUB_LOG_FUNCTION_INFO(TEXT("client deinitialization finished."));

	DefaultLogger = nullptr;
	LoggerManager = nullptr;
}



FPubnubOperationResult UPubnubClient::SetUserID_priv(const FString &UserID)
{
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(UserID);
	
	FPubnubOperationResult FinalResult;

	FUTF8StringHolder UserIDHolder(UserID);
	pubnub_res_t response = pubnub_set_user_id(pubnub_context, UserIDHolder.Get());
	if (response != PUBNUB_OK)
	{
		FinalResult.Error = true;
		FinalResult.ErrorMessage = pubnub_res_str(response);
	}
	
	return FinalResult;
}

FString UPubnubClient::GetUserID_priv()
{
	if(const char* UserIDChar = pubnub_get_user_id(pubnub_context))
	{
		FString UserIDString(UserIDChar);
		return UserIDString;
	}

	return "";
}

FPubnubPublishMessageResult UPubnubClient::PublishMessage_priv(FString Channel, FString Message, FPubnubPublishSettings PublishSettings)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(Message),
		PUBNUB_LOG_INPUT(PublishSettings)
	);
	
	FPubnubPublishMessageResult FinalResult;
	
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Message, FinalResult);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	FString FinalMessage = Message;

	//If provided string is not a valid Json object or array, we treat it as literal string and serialize it
	if(!UPubnubJsonUtilities::IsCorrectJsonString(Message, false))
	{
		FinalMessage = UPubnubJsonUtilities::SerializeString(FinalMessage);
		PUBNUB_LOG_FUNCTION_TRACE(FString::Printf(TEXT("serialized non-JSON message payload. Final serialized message: %s"), *FinalMessage));
	}
	
	//Convert all UE PublishSettings to Pubnub PublishOptions
	pubnub_publish_opts_t publish_options = PUBNUB_PUBLISH_OPTS_INIT;
	
	//Converted char needs to live in function scope, so we need to create it here
	FUTF8StringHolder MessageHolder(FinalMessage);
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder MetaHolder(PublishSettings.MetaData);
	FUTF8StringHolder CustomMessageTypeHolder(PublishSettings.CustomMessageType);
	
	publish_options.message = MessageHolder.Get();
	publish_options.channel = ChannelHolder.Get();
	publish_options.meta = MetaHolder.GetOrNull();
	publish_options.custom_message_type = CustomMessageTypeHolder.GetOrNull();
	
	UPubnubInternalUtilities::PublishUESettingsToPubnubPublishOptions(PublishSettings, publish_options);
	
	pubnub_future_t operation_future = pubnub_publish(pubnub_context, &publish_options);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);
	
	if (!FinalResult.Result.Error)
	{
		FinalResult.PublishedMessage.Message = Message;
		FinalResult.PublishedMessage.Channel = Channel;
		FinalResult.PublishedMessage.UserID = GetUserID_priv();
		FinalResult.PublishedMessage.Timetoken = UPubnubInternalUtilities::PubnubStringViewToString(pubnub_publish_result_timetoken(operation_future));
		FinalResult.PublishedMessage.Metadata = PublishSettings.MetaData;
		FinalResult.PublishedMessage.MessageType = EPubnubMessageType::PMT_Published;
		FinalResult.PublishedMessage.CustomMessageType = PublishSettings.CustomMessageType;
		PUBNUB_LOG_FUNCTION_DEBUG(
			TEXT("published message: "),
			PUBNUB_LOG_VALUE(FinalResult.PublishedMessage)
		);
	}
	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	
	return FinalResult;
}

FPubnubSignalResult UPubnubClient::Signal_priv(FString Channel, FString Message, FPubnubSignalSettings SignalSettings)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(Message),
		PUBNUB_LOG_INPUT(SignalSettings)
	);
	
	FPubnubSignalResult FinalResult;
	
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Message, FinalResult);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	FString FinalMessage = Message;

	//If provided string is not a valid Json object or array, we treat it as literal string and serialize it
	if(!UPubnubJsonUtilities::IsCorrectJsonString(Message, false))
	{
		FinalMessage = UPubnubJsonUtilities::SerializeString(FinalMessage);
		PUBNUB_LOG_FUNCTION_TRACE(FString::Printf(TEXT("serialized non-JSON signal payload. Final serialized message: %s"), *FinalMessage));
	}
	
	pubnub_signal_opts_t signal_options = PUBNUB_SIGNAL_OPTS_INIT;
	
	//Converted char needs to live in function scope, so we need to create it here
	FUTF8StringHolder MessageHolder(FinalMessage);
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder CustomMessageTypeHolder(SignalSettings.CustomMessageType);
	
	signal_options.message = MessageHolder.Get();
	signal_options.channel = ChannelHolder.Get();
	signal_options.custom_message_type = CustomMessageTypeHolder.GetOrNull();
	
	pubnub_future_t operation_future = pubnub_signal(pubnub_context, &signal_options);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);
	
	if (!FinalResult.Result.Error)
	{
		FinalResult.SignalMessage.Message = Message;
		FinalResult.SignalMessage.Channel = Channel;
		FinalResult.SignalMessage.UserID = GetUserID_priv();
		FinalResult.SignalMessage.Timetoken = UPubnubInternalUtilities::PubnubStringViewToString(pubnub_signal_result_timetoken(operation_future));
		FinalResult.SignalMessage.MessageType = EPubnubMessageType::PMT_Signal;
		FinalResult.SignalMessage.CustomMessageType = SignalSettings.CustomMessageType;
		PUBNUB_LOG_FUNCTION_DEBUG(
			TEXT("signaled message: "),
			PUBNUB_LOG_VALUE(FinalResult.SignalMessage)
		);
	}
	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	
	return FinalResult;
}

FPubnubOperationResult UPubnubClient::SubscribeToChannel_priv(FString Channel, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_VALUE(Channel),
		PUBNUB_LOG_VALUE(SubscribeSettings)
	);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);

	FScopeLock SubscriptionsLock(&SubscriptionsMutex);

	if(!pubnub_context)
	{
		FPubnubOperationResult Result({0, true, TEXT("PubnubClient was deinitialized before the subscribe operation could run.")});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	if(ChannelSubscriptions.Contains(Channel))
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("subscription for channel '%s' already exists. Aborting operation."), *Channel));
		FPubnubOperationResult Result({0, true, TEXT("Already subscribed to this channel. Aborting operation.")});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	pubnub_subscription_t Subscription = UPubnubInternalUtilities::CreateCCoreSubscription(pubnub_context, Channel, EPubnubEntityType::PEnT_Channel, SubscribeSettings);
	if(!Subscription)
	{
		PUBNUB_LOG_FUNCTION_ERROR(FString::Printf(TEXT("Failed to subscribe to channel '%s'. C-Core subscription was not created."), *Channel));
		FPubnubOperationResult Result({0, true, TEXT("Failed to subscribe to channel. C-Core subscription was not created.")});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	const pubnub_res_t SubscribeResultCode = pubnub_subscription_subscribe(Subscription);
	if(SubscribeResultCode != PUBNUB_OK)
	{
		pubnub_subscription_destroy(Subscription);
		FPubnubOperationResult Result({0, true, FString::Printf(TEXT("Failed to subscribe to channel. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(SubscribeResultCode)))});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	ChannelSubscriptions.Add(Channel, Subscription);
	PUBNUB_LOG_FUNCTION_TRACE(FString::Printf(TEXT("channel subscription stored.\n\t-%s"), *PUBNUB_LOG_VALUE(Channel)));

	FPubnubOperationResult Result({200, false, TEXT("")});
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

FPubnubOperationResult UPubnubClient::SubscribeToGroup_priv(FString ChannelGroup, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_VALUE(ChannelGroup),
		PUBNUB_LOG_VALUE(SubscribeSettings)
	);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(ChannelGroup);

	FScopeLock SubscriptionsLock(&SubscriptionsMutex);

	if(!pubnub_context)
	{
		FPubnubOperationResult Result({0, true, TEXT("PubnubClient was deinitialized before the subscribe operation could run.")});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	if(ChannelGroupSubscriptions.Contains(ChannelGroup))
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("subscription for channel group '%s' already exists. Aborting operation."), *ChannelGroup));
		FPubnubOperationResult Result({0, true, TEXT("Already subscribed to this channel group. Aborting operation.")});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	pubnub_subscription_t Subscription = UPubnubInternalUtilities::CreateCCoreSubscription(pubnub_context, ChannelGroup, EPubnubEntityType::PEnT_ChannelGroup, SubscribeSettings);
	if(!Subscription)
	{
		PUBNUB_LOG_FUNCTION_ERROR(FString::Printf(TEXT("Failed to subscribe to channel group '%s'. C-Core subscription was not created."), *ChannelGroup));
		FPubnubOperationResult Result({0, true, TEXT("Failed to subscribe to channel group. C-Core subscription was not created.")});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	const pubnub_res_t SubscribeResultCode = pubnub_subscription_subscribe(Subscription);
	if(SubscribeResultCode != PUBNUB_OK)
	{
		pubnub_subscription_destroy(Subscription);
		FPubnubOperationResult Result({0, true, FString::Printf(TEXT("Failed to subscribe to channel group. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(SubscribeResultCode)))});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	ChannelGroupSubscriptions.Add(ChannelGroup, Subscription);
	PUBNUB_LOG_FUNCTION_TRACE(FString::Printf(TEXT("channel group subscription stored.\n\t-%s"), *PUBNUB_LOG_VALUE(ChannelGroup)));

	FPubnubOperationResult Result({200, false, TEXT("")});
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

FPubnubOperationResult UPubnubClient::UnsubscribeFromChannel_priv(FString Channel)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_VALUE(Channel)
	);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);

	FScopeLock SubscriptionsLock(&SubscriptionsMutex);

	pubnub_subscription_t* SubscriptionPtr = ChannelSubscriptions.Find(Channel);
	const bool bHasSubscription = SubscriptionPtr != nullptr && *SubscriptionPtr != nullptr;
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(bHasSubscription, TEXT("There is no such subscription. Aborting operation."));

	pubnub_subscription_t Subscription = *SubscriptionPtr;
	const pubnub_res_t UnsubscribeResultCode = pubnub_subscription_unsubscribe(Subscription);
	if(UnsubscribeResultCode != PUBNUB_OK)
	{
		PUBNUB_LOG_FUNCTION_ERROR(FString::Printf(TEXT("failed to unsubscribe channel '%s'."), *Channel));
		FPubnubOperationResult Result({0, true, FString::Printf(TEXT("Failed to unsubscribe. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(UnsubscribeResultCode)))});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	pubnub_subscription_destroy(Subscription);
	ChannelSubscriptions.Remove(Channel);
	PUBNUB_LOG_FUNCTION_DEBUG(
		TEXT("channel subscription removed."),
		PUBNUB_LOG_VALUE(Channel)
	);

	FPubnubOperationResult Result({200, false, TEXT("")});
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

FPubnubOperationResult UPubnubClient::UnsubscribeFromGroup_priv(FString ChannelGroup)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_VALUE(ChannelGroup)
	);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(ChannelGroup);

	FScopeLock SubscriptionsLock(&SubscriptionsMutex);

	pubnub_subscription_t* SubscriptionPtr = ChannelGroupSubscriptions.Find(ChannelGroup);
	const bool bHasSubscription = SubscriptionPtr != nullptr && *SubscriptionPtr != nullptr;
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(bHasSubscription, TEXT("There is no such subscription. Aborting operation."));

	pubnub_subscription_t Subscription = *SubscriptionPtr;
	const pubnub_res_t UnsubscribeResultCode = pubnub_subscription_unsubscribe(Subscription);
	if(UnsubscribeResultCode != PUBNUB_OK)
	{
		PUBNUB_LOG_FUNCTION_ERROR(FString::Printf(TEXT("failed to unsubscribe channel group '%s'."), *ChannelGroup));
		FPubnubOperationResult Result({0, true, FString::Printf(TEXT("Failed to unsubscribe. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(UnsubscribeResultCode)))});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	pubnub_subscription_destroy(Subscription);
	ChannelGroupSubscriptions.Remove(ChannelGroup);
	PUBNUB_LOG_FUNCTION_DEBUG(
		TEXT("channel group subscription removed."),
		PUBNUB_LOG_VALUE(ChannelGroup)
	);

	FPubnubOperationResult Result({200, false, TEXT("")});
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

FPubnubOperationResult UPubnubClient::UnsubscribeFromAll_priv()
{
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	FScopeLock SubscriptionsLock(&SubscriptionsMutex);

	if(ChannelSubscriptions.IsEmpty() && ChannelGroupSubscriptions.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(TEXT("unsubscribe all requested but there are no active subscriptions."));
		FPubnubOperationResult Result({200, false, TEXT("")});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	if(!pubnub_context)
	{
		FPubnubOperationResult Result({0, true, TEXT("PubnubClient was deinitialized before the unsubscribe operation could run.")});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	const pubnub_res_t UnsubscribeAllResult = pubnub_subscribe_unsubscribe_all(pubnub_context);
	if(UnsubscribeAllResult != PUBNUB_OK)
	{
		FPubnubOperationResult Result({0, true, FString::Printf(TEXT("Failed to unsubscribe all. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(UnsubscribeAllResult)))});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	DestroySubscriptionMap(ChannelSubscriptions);
	DestroySubscriptionMap(ChannelGroupSubscriptions);

	FPubnubOperationResult Result({200, false, TEXT("")});
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

FPubnubOperationResult UPubnubClient::ReconnectSubscriptions_priv(FString Timetoken)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Timetoken)
	);

	FScopeLock SubscriptionsLock(&SubscriptionsMutex);

	if(!pubnub_context)
	{
		FPubnubOperationResult Result({0, true, TEXT("PubnubClient was deinitialized before the reconnect operation could run.")});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	// A non-empty timetoken is stored as the resume cursor first. Reconnect then
	// restarts the subscribe loop from that cursor. An empty timetoken reuses the
	// cursor already held by the subscribe manager.
	if(!Timetoken.IsEmpty())
	{
		FUTF8StringHolder TimetokenHolder(Timetoken);
		pubnub_timetoken_t Cursor;
		Cursor.ptr = TimetokenHolder.Get();
		Cursor.len = static_cast<size_t>(TimetokenHolder.Converter.Length());

		const pubnub_res_t RestoreResult = pubnub_subscribe_restore(pubnub_context, Cursor);
		if(RestoreResult != PUBNUB_OK)
		{
			PUBNUB_LOG_FUNCTION_ERROR(TEXT("failed to restore subscription cursor."));
			FPubnubOperationResult Result({0, true, FString::Printf(TEXT("Failed to reconnect subscriptions. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(RestoreResult)))});
			PUBNUB_LOG_OPERATION_RESULT(Result);
			return Result;
		}
	}

	const pubnub_res_t ReconnectResult = pubnub_subscribe_reconnect(pubnub_context);
	if(ReconnectResult != PUBNUB_OK)
	{
		PUBNUB_LOG_FUNCTION_ERROR(TEXT("failed to reconnect subscriptions."));
		FPubnubOperationResult Result({0, true, FString::Printf(TEXT("Failed to reconnect subscriptions. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(ReconnectResult)))});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	PUBNUB_LOG_FUNCTION_INFO(TEXT("reconnect completed successfully."));
	FPubnubOperationResult Result({200, false, TEXT("")});
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

FPubnubOperationResult UPubnubClient::DisconnectSubscriptions_priv()
{
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	FScopeLock SubscriptionsLock(&SubscriptionsMutex);

	if(!pubnub_context)
	{
		FPubnubOperationResult Result({0, true, TEXT("PubnubClient was deinitialized before the disconnect operation could run.")});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	const pubnub_res_t DisconnectResult = pubnub_subscribe_disconnect(pubnub_context);
	if(DisconnectResult != PUBNUB_OK)
	{
		PUBNUB_LOG_FUNCTION_ERROR(TEXT("failed to disconnect subscriptions."));
		FPubnubOperationResult Result({0, true, FString::Printf(TEXT("Failed to disconnect subscriptions. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(DisconnectResult)))});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	PUBNUB_LOG_FUNCTION_INFO(TEXT("disconnect completed successfully."));
	FPubnubOperationResult Result({200, false, TEXT("")});
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

FPubnubOperationResult UPubnubClient::AddChannelToGroup_priv(FString Channel, FString ChannelGroup)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(ChannelGroup)
	);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(ChannelGroup);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	pubnub_channel_group_add_opts_t opts = PUBNUB_CHANNEL_GROUP_ADD_OPTS_INIT;

	//Converted char needs to live in function scope, so we need to create it here
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);

	opts.channel_group = ChannelGroupHolder.Get();
	opts.channels = ChannelHolder.Get();

	pubnub_future_t operation_future = pubnub_channel_group_add_channels(pubnub_context, &opts);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FUTURE_NOT_IN_PROGRESS();
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("add channel to group request sent."));

	FPubnubOperationResult Result;
	PUBNUB_OPERATION_RESULT_AWAIT_FOR_FUTURE(Result);

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

FPubnubOperationResult UPubnubClient::RemoveChannelFromGroup_priv(FString Channel, FString ChannelGroup)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(ChannelGroup)
	);

	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(ChannelGroup);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	//Converted char needs to live in function scope, so we need to create it here
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);

	pubnub_channel_group_remove_opts_t opts = PUBNUB_CHANNEL_GROUP_REMOVE_OPTS_INIT;
	opts.channel_group = ChannelGroupHolder.Get();
	opts.channels = ChannelHolder.Get();

	pubnub_future_t operation_future = pubnub_channel_group_remove_channels(pubnub_context, &opts);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FUTURE_NOT_IN_PROGRESS();
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("remove channel from group request sent."));

	FPubnubOperationResult Result;
	PUBNUB_OPERATION_RESULT_AWAIT_FOR_FUTURE(Result);

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(Result);

	return Result;
}

FPubnubListChannelsFromGroupResult UPubnubClient::ListChannelsFromGroup_priv(FString ChannelGroup)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(ChannelGroup)
	);
	
	FPubnubListChannelsFromGroupResult FinalResult;
	
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(ChannelGroup, FinalResult);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	pubnub_channel_group_list_opts_t opts = PUBNUB_CHANNEL_GROUP_LIST_OPTS_INIT;

	//Converted char needs to live in function scope, so we need to create it here
	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);
	opts.channel_group = ChannelGroupHolder.Get();

	pubnub_future_t operation_future = pubnub_channel_group_list_channels(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("list channels from group request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);
	
	if (!FinalResult.Result.Error)
	{
		//Parse the future into the result
		UPubnubInternalUtilities::ListChannelsFromGroupFromFuture(operation_future, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("listed channels. Count=%d"), FinalResult.Channels.Num()));
	}
	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	
	return FinalResult;
}

FPubnubOperationResult UPubnubClient::RemoveChannelGroup_priv(FString ChannelGroup)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(ChannelGroup)
	);
	
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(ChannelGroup);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	pubnub_channel_group_remove_group_opts_t opts = PUBNUB_CHANNEL_GROUP_REMOVE_GROUP_OPTS_INIT;
	
	//Converted char needs to live in function scope, so we need to create it here
	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);
	
	opts.channel_group = ChannelGroupHolder.Get();
	
	pubnub_future_t operation_future = pubnub_channel_group_remove(pubnub_context, &opts);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FUTURE_NOT_IN_PROGRESS();

	PUBNUB_LOG_FUNCTION_TRACE(TEXT("remove channel group request sent."));

	FPubnubOperationResult OperationResult;
	PUBNUB_OPERATION_RESULT_AWAIT_FOR_FUTURE(OperationResult);
	
	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(OperationResult);
	
	return OperationResult;
}

FPubnubListUsersFromChannelResult UPubnubClient::ListUsersFromChannel_priv(FString Channel, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(ListUsersFromChannelSettings)
	);
	
	FPubnubListUsersFromChannelResult FinalResult;
	
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS((ListUsersFromChannelSettings.Limit >= 0), TEXT("Limit can't be below 0."), FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS((ListUsersFromChannelSettings.Offset >= 0), TEXT("Offset can't be below 0."), FinalResult);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	pubnub_here_now_opts_t opts = PUBNUB_HERE_NOW_OPTS_INIT;

	//Converted char needs to live in function scope, so we need to create it here
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder ChannelGroupHolder(ListUsersFromChannelSettings.ChannelGroup);

	opts.channels = ChannelHolder.Get();
	opts.channel_groups = ChannelGroupHolder.GetOrNull();
	// DisableUserID true omits user ids. PUBNUB_HERE_NOW_OPTS_INIT defaults include_uuids to 1.
	opts.include_uuids = ListUsersFromChannelSettings.DisableUserID ? 0 : 1;
	opts.include_state = ListUsersFromChannelSettings.State ? 1 : 0;
	opts.limit = static_cast<uint32_t>(ListUsersFromChannelSettings.Limit);
	opts.offset = static_cast<uint32_t>(ListUsersFromChannelSettings.Offset);

	pubnub_future_t operation_future = pubnub_here_now(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("list users from channel request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);
	
	if (!FinalResult.Result.Error)
	{
		//Parse the future into the result
		UPubnubInternalUtilities::ListUsersFromChannelFromFuture(operation_future, FinalResult);
		// A single-channel here-now body has no channel name. Keep the requested channel so callers can match it.
		if (FinalResult.Channels.Num() == 1 && FinalResult.Channels[0].Channel.IsEmpty())
		{
			FinalResult.Channels[0].Channel = Channel;
		}
	}
	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	
	return FinalResult;
}

FPubnubListUsersSubscribedChannelsResult UPubnubClient::ListUserSubscribedChannels_priv(FString UserID)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(UserID)
	);
	
	FPubnubListUsersSubscribedChannelsResult FinalResult;
	
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(UserID, FinalResult);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	pubnub_where_now_opts_t opts = PUBNUB_WHERE_NOW_OPTS_INIT;

	//Converted char needs to live in function scope, so we need to create it here
	FUTF8StringHolder UserIDHolder(UserID);
	opts.uuid = UserIDHolder.Get();

	pubnub_future_t operation_future = pubnub_where_now(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("list user subscribed channels request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);
	
	if (!FinalResult.Result.Error)
	{
		//Parse the future into the result
		UPubnubInternalUtilities::ListUserSubscribedChannelsFromFuture(operation_future, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("listed user subscribed channels. Count=%d"), FinalResult.Channels.Num()));
	}
	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	
	return FinalResult;
}

FPubnubOperationResult UPubnubClient::SetState_priv(FString Channel, FString StateJson, FPubnubSetStateSettings SetStateSettings)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(StateJson),
		PUBNUB_LOG_INPUT(SetStateSettings)
	);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(StateJson);

	if(!UPubnubJsonUtilities::IsCorrectJsonString(StateJson, false))
	{
		PUBNUB_LOG_FUNCTION_WARNING(TEXT("[SetState]: StateJson has to be a correct Json Object. Aborting operation."));
		FPubnubOperationResult Result;
		Result.Error = true;
		Result.ErrorMessage = TEXT("[SetState]: StateJson has to be a correct Json Object. Operation aborted.");
		return Result;
	}

	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	pubnub_set_state_opts_t opts = PUBNUB_SET_STATE_OPTS_INIT;

	//Converted char needs to live in function scope, so we need to create it here
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder StateJsonHolder(StateJson);
	FUTF8StringHolder ChannelGroupHolder(SetStateSettings.ChannelGroup);

	opts.channels = ChannelHolder.Get();
	opts.channel_groups = ChannelGroupHolder.GetOrNull();
	opts.state = StateJsonHolder.Get();
	opts.state_len = 0;

	pubnub_future_t operation_future = pubnub_set_state(pubnub_context, &opts);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FUTURE_NOT_IN_PROGRESS();
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("set state request sent."));

	FPubnubOperationResult Result;
	PUBNUB_OPERATION_RESULT_AWAIT_FOR_FUTURE(Result);

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

FPubnubGetStateResult UPubnubClient::GetState_priv(FString Channel, FString ChannelGroup, FString UserID)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(ChannelGroup),
		PUBNUB_LOG_INPUT(UserID)
	);

	FPubnubGetStateResult FinalResult;

	// Channel stays required even when ChannelGroup is set. ChannelGroup and UserID may be empty.
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FinalResult);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	pubnub_get_state_opts_t opts = PUBNUB_GET_STATE_OPTS_INIT;

	// Holders must stay alive until pubnub_future_release. timeout_ms stays 0.
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder ChannelGroupHolder(ChannelGroup);
	FUTF8StringHolder UserIDHolder(UserID);

	opts.channels = ChannelHolder.Get();
	opts.channel_groups = ChannelGroupHolder.GetOrNull();
	opts.uuid = UserIDHolder.GetOrNull(); // NULL means the context user id

	pubnub_future_t operation_future = pubnub_get_state(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("get state request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		//Parse the future into the result
		UPubnubInternalUtilities::GetStateFromFuture(operation_future, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("got state. Count=%d"), FinalResult.States.Num()));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubGrantTokenResult UPubnubClient::GrantToken_priv(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FString Meta)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Ttl),
		PUBNUB_LOG_INPUT(AuthorizedUser),
		PUBNUB_LOG_INPUT(Meta)
	);

	FPubnubGrantTokenResult FinalResult;

	// Same local rejects the old permission-object builder used before calling C-Core.
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS((Ttl > 0), TEXT("Ttl must be greater than 0."), FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(AuthorizedUser, FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS((!Permissions.ArePermissionsEmpty()), TEXT("Permissions can't be empty."), FinalResult);
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	// Storage must stay alive until pubnub_future_release. C-Core borrows the name pointers.
	const TUniquePtr<FPubnubGrantTokenResourceStorage, FPubnubGrantTokenResourceStorageDeleter> ResourceStorage =
		UPubnubTokenUtilities::BuildGrantTokenResourceStorage(Permissions);

	FUTF8StringHolder AuthorizedUserHolder(AuthorizedUser);
	FUTF8StringHolder MetaHolder(Meta);

	pubnub_grant_token_opts_t opts = PUBNUB_GRANT_TOKEN_OPTS_INIT;
	opts.ttl = static_cast<uint32_t>(Ttl);
	opts.authorized_uuid = AuthorizedUserHolder.Get();
	// Old builder embedded meta only when it was valid JSON. Invalid meta is omitted.
	opts.meta = (!Meta.IsEmpty() && UPubnubJsonUtilities::IsCorrectJsonString(Meta)) ? MetaHolder.Get() : nullptr;
	UPubnubTokenUtilities::ApplyGrantTokenResourceStorage(*ResourceStorage, opts);

	pubnub_future_t operation_future = pubnub_grant_token(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("grant token request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		FinalResult.Token = UPubnubInternalUtilities::PubnubStringViewToString(pubnub_grant_token_result(operation_future).token);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("grant token response parsed. TokenLength=%d"), FinalResult.Token.Len()));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubOperationResult UPubnubClient::RevokeToken_priv(FString Token)
{
	PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("revoke token called. TokenLength=%d"), Token.Len()));
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Token);
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	FUTF8StringHolder TokenHolder(Token);

	pubnub_revoke_token_opts_t opts = PUBNUB_REVOKE_TOKEN_OPTS_INIT;
	opts.token = TokenHolder.Get();

	pubnub_future_t operation_future = pubnub_revoke_token(pubnub_context, &opts);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FUTURE_NOT_IN_PROGRESS();
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("revoke token request sent."));

	FPubnubOperationResult Result;
	PUBNUB_OPERATION_RESULT_AWAIT_FOR_FUTURE(Result);

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

FString UPubnubClient::ParseToken_priv(FString Token)
{
	PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("parse token called. TokenLength=%d"), Token.Len()));
	PUBNUB_RETURN_IF_FIELD_EMPTY(Token, "");

	FUTF8StringHolder TokenHolder(Token);

	pubnub_parse_token_opts_t opts = PUBNUB_PARSE_TOKEN_OPTS_INIT;
	opts.token = TokenHolder.Get();

	pubnub_parsed_token_t ParsedToken = {};
	const pubnub_res_t ParseResult = pubnub_parse_token(pubnub_context, &opts, &ParsedToken);
	if (ParseResult != PUBNUB_OK)
	{
		PUBNUB_LOG_FUNCTION_ERROR(FString::Printf(TEXT("pubnub_parse_token failed. ResultCode=%s"), UTF8_TO_TCHAR(pubnub_res_str(ParseResult))));
		return FString();
	}

	// Copy resource views out before another parse replaces the cached token.
	const FString ReworkedToken = UPubnubTokenUtilities::BuildReworkedParsedToken(pubnub_context, ParsedToken);

	PUBNUB_LOG_FUNCTION_TRACE(FString::Printf(TEXT("token parsed successfully: %s"), *ReworkedToken));
	return ReworkedToken;
}

void UPubnubClient::SetAuthToken_priv(FString Token)
{
	PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("set auth token called. TokenLength=%d"), Token.Len()));

	// NULL clears the token. An empty string is not a clear.
	FUTF8StringHolder TokenHolder(Token);
	const pubnub_res_t Response = pubnub_set_auth_token(pubnub_context, TokenHolder.GetOrNull());
	if (Response != PUBNUB_OK)
	{
		PUBNUB_LOG_FUNCTION_ERROR(FString::Printf(TEXT("pubnub_set_auth_token failed. ResultCode=%s"), UTF8_TO_TCHAR(pubnub_res_str(Response))));
		return;
	}

	PUBNUB_LOG_FUNCTION_TRACE(TEXT("auth token applied."));
}

FPubnubOperationResult UPubnubClient::SetOrigin_priv(FString Origin)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Origin)
	);

	FPubnubOperationResult FinalResult;
	FUTF8StringHolder OriginHolder(Origin);

	// NULL or empty resets to the compile-time default origin.
	const pubnub_res_t Response = pubnub_set_origin(pubnub_context, OriginHolder.GetOrNull());
	if (Response != PUBNUB_OK)
	{
		FinalResult.Error = true;
		FinalResult.ErrorMessage = pubnub_res_str(Response);
		PUBNUB_LOG_OPERATION_RESULT(FinalResult);
		return FinalResult;
	}

	FinalResult.Status = 200;
	FinalResult.Error = false;
	PUBNUB_LOG_OPERATION_RESULT(FinalResult);
	return FinalResult;
}

FString UPubnubClient::GetOrigin_priv() const
{
	if (const char* Origin = pubnub_get_origin(pubnub_context))
	{
		return FString(UTF8_TO_TCHAR(Origin));
	}

	return TEXT("");
}

FPubnubFetchHistoryResult UPubnubClient::FetchHistory_priv(FString Channel, FPubnubFetchHistorySettings FetchHistorySettings)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(FetchHistorySettings)
	);

	FPubnubFetchHistoryResult FinalResult;

	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS((FetchHistorySettings.MaxPerChannel >= 0), TEXT("MaxPerChannel can't be below 0."), FinalResult);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	pubnub_fetch_messages_opts_t opts = PUBNUB_FETCH_MESSAGES_OPTS_INIT;

	// Holders must stay alive until pubnub_future_release. INIT turns include_uuid and
	// include_message_type on, so every flag is set from the UE settings afterwards.
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder StartHolder(FetchHistorySettings.Start);
	FUTF8StringHolder EndHolder(FetchHistorySettings.End);

	opts.channels = ChannelHolder.Get();
	opts.start = StartHolder.GetOrNull();
	opts.end = EndHolder.GetOrNull();
	opts.count = static_cast<uint16>(FMath::Min(FetchHistorySettings.MaxPerChannel, 65535));
	opts.reverse = FetchHistorySettings.Reverse ? 1 : 0;
	opts.include_meta = FetchHistorySettings.IncludeMeta ? 1 : 0;
	opts.include_uuid = FetchHistorySettings.IncludeUserID ? 1 : 0;
	opts.include_message_type = FetchHistorySettings.IncludeMessageType ? 1 : 0;
	opts.include_custom_message_type = FetchHistorySettings.IncludeCustomMessageType ? 1 : 0;
	opts.include_message_actions = FetchHistorySettings.IncludeMessageActions ? 1 : 0;

	pubnub_future_t operation_future = pubnub_fetch_messages(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("fetch history request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		//Parse the future into the result
		UPubnubInternalUtilities::FetchHistoryFromFuture(operation_future, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("history parsed. MessagesCount=%d"), FinalResult.Messages.Num()));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubOperationResult UPubnubClient::DeleteMessages_priv(FString Channel, FPubnubDeleteMessagesSettings DeleteMessagesSettings)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(DeleteMessagesSettings)
	);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	pubnub_delete_messages_opts_t opts = PUBNUB_DELETE_MESSAGES_OPTS_INIT;

	// Omitted bounds must be NULL. An empty string is not a timetoken and crashes C-Core.
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder StartHolder(DeleteMessagesSettings.Start);
	FUTF8StringHolder EndHolder(DeleteMessagesSettings.End);

	opts.channel = ChannelHolder.Get();
	opts.start = StartHolder.GetOrNull();
	opts.end = EndHolder.GetOrNull();

	pubnub_future_t operation_future = pubnub_delete_messages(pubnub_context, &opts);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FUTURE_NOT_IN_PROGRESS();
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("delete messages request sent."));

	FPubnubOperationResult Result;
	PUBNUB_OPERATION_RESULT_AWAIT_FOR_FUTURE(Result);

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

FPubnubMessageCountsResult UPubnubClient::MessageCounts_priv(FString Channel, FString Timetoken)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(Timetoken)
	);

	FPubnubMessageCountsResult FinalResult;

	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Timetoken, FinalResult);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	pubnub_message_counts_opts_t opts = PUBNUB_MESSAGE_COUNTS_OPTS_INIT;

	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder TimetokenHolder(Timetoken);

	opts.channels = ChannelHolder.Get();
	opts.timetoken = TimetokenHolder.Get();

	pubnub_future_t operation_future = pubnub_message_counts(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("message counts request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		UPubnubInternalUtilities::MessageCountsFromFuture(operation_future, Channel, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("message counts parsed. Count=%d"), FinalResult.MessageCounts));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubMessageCountsMultipleResult UPubnubClient::MessageCountsMultiple_priv(TArray<FString> Channels, TArray<FString> Timetokens)
{
	const FString ChannelsCsv = FString::Join(Channels, TEXT(","));
	const FString TimetokensCsv = FString::Join(Timetokens, TEXT(","));
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(ChannelsCsv),
		PUBNUB_LOG_INPUT(TimetokensCsv)
	);

	FPubnubMessageCountsMultipleResult FinalResult;

	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS((!Channels.IsEmpty()), TEXT("Channels array cannot be empty."), FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS((Channels.Num() == Timetokens.Num()), TEXT("Number of channels must match number of timetokens."), FinalResult);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	pubnub_message_counts_opts_t opts = PUBNUB_MESSAGE_COUNTS_OPTS_INIT;

	FUTF8StringHolder ChannelsHolder(ChannelsCsv);
	FUTF8StringHolder TimetokensHolder(TimetokensCsv);

	opts.channels = ChannelsHolder.Get();
	opts.channels_timetokens = TimetokensHolder.Get();

	pubnub_future_t operation_future = pubnub_message_counts(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("message counts multiple request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		UPubnubInternalUtilities::MessageCountsMultipleFromFuture(operation_future, Channels, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("message counts multiple parsed. ChannelsCount=%d"), FinalResult.MessageCountsPerChannel.Num()));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}


FPubnubGetAllUserMetadataResult UPubnubClient::GetAllUserMetadata_priv(FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Include),
		PUBNUB_LOG_INPUT(Limit),
		PUBNUB_LOG_INPUT(Filter),
		PUBNUB_LOG_INPUT(Sort),
		PUBNUB_LOG_INPUT(Page),
		PUBNUB_LOG_INPUT(Count)
	);
	FPubnubGetAllUserMetadataResult FinalResult;
	
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	FString UnknownInclude;
	pubnub_get_all_uuid_metadata_opts_t opts = PUBNUB_GET_ALL_UUID_METADATA_OPTS_INIT;
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);

	opts.include = UPubnubInternalUtilities::MetadataListIncludeMask(Include, Count, UnknownInclude);
	opts.limit = static_cast<uint32_t>(FMath::Clamp(Limit, 0, PUBNUB_MAX_LIMIT));
	opts.filter = FilterHolder.GetOrNull();
	opts.sort = SortHolder.GetOrNull();
	// If both Next and Prev are provided, Next takes precedence.
	opts.start = PageNextHolder.GetOrNull();
	opts.end = Page.Next.IsEmpty() ? PagePrevHolder.GetOrNull() : nullptr;

	if (!UnknownInclude.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring unsupported include tokens: %s"), *UnknownInclude));
	}

	pubnub_future_t operation_future = pubnub_get_all_uuid_metadata(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("get all user metadata request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		UPubnubInternalUtilities::GetAllUserMetadataFromFuture(operation_future, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("user metadata parsed. UsersCount=%d, TotalCount=%d"), FinalResult.UsersData.Num(), FinalResult.TotalCount));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubUserMetadataResult UPubnubClient::SetUserMetadata_priv(FString User, FString UserMetadataObj, FString Include)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(User),
		PUBNUB_LOG_INPUT(UserMetadataObj),
		PUBNUB_LOG_INPUT(Include)
	);
	FPubnubUserMetadataResult FinalResult;
	
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(User, FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(UserMetadataObj, FinalResult);

	FParsedUserMetadataObject Parsed;
	if (!UPubnubInternalUtilities::ParseUserMetadataObject(UserMetadataObj, Parsed))
	{
		PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(false, *Parsed.Error, FinalResult);
	}
	if (!Parsed.UnknownFields.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring metadata JSON fields the new C-Core set API does not send: %s"), *FString::Join(Parsed.UnknownFields, TEXT(", "))));
	}
	if (!Parsed.NullStringFields.IsEmpty())
	{
		PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(false, *UPubnubInternalUtilities::MetadataNullFieldError(Parsed.NullStringFields), FinalResult);
	}

	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	FString UnknownInclude;
	pubnub_set_uuid_metadata_opts_t opts = {};
	FUTF8StringHolder UserHolder(User);
	FUTF8StringHolder NameHolder(Parsed.Name.Value);
	FUTF8StringHolder ExternalIdHolder(Parsed.ExternalId.Value);
	FUTF8StringHolder ProfileUrlHolder(Parsed.ProfileUrl.Value);
	FUTF8StringHolder EmailHolder(Parsed.Email.Value);
	FUTF8StringHolder StatusHolder(Parsed.Status.Value);
	FUTF8StringHolder TypeHolder(Parsed.Type.Value);
	FUTF8StringHolder CustomHolder(Parsed.CustomJson);

	// Zero-init keeps include empty. The C-Core single-entity default would otherwise request custom.
	opts.uuid = UserHolder.Get();
	opts.include = UPubnubInternalUtilities::AppContextIncludeMaskFromString(Include, UnknownInclude);
	opts.name = UPubnubInternalUtilities::OptionalMetadataString(Parsed.Name, NameHolder);
	opts.external_id = UPubnubInternalUtilities::OptionalMetadataString(Parsed.ExternalId, ExternalIdHolder);
	opts.profile_url = UPubnubInternalUtilities::OptionalMetadataString(Parsed.ProfileUrl, ProfileUrlHolder);
	opts.email = UPubnubInternalUtilities::OptionalMetadataString(Parsed.Email, EmailHolder);
	opts.status = UPubnubInternalUtilities::OptionalMetadataString(Parsed.Status, StatusHolder);
	opts.type = UPubnubInternalUtilities::OptionalMetadataString(Parsed.Type, TypeHolder);
	if (Parsed.bHasCustom)
	{
		opts.custom = CustomHolder.Get();
	}

	if (!UnknownInclude.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring unsupported include tokens: %s"), *UnknownInclude));
	}

	pubnub_future_t operation_future = pubnub_set_uuid_metadata(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("set user metadata request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		FinalResult.UserData = UPubnubInternalUtilities::UserDataFromSetUuidMetadataFuture(operation_future);
		PUBNUB_LOG_FUNCTION_DEBUG(TEXT("set user metadata parsed."), PUBNUB_LOG_VALUE(FinalResult.UserData));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubUserMetadataResult UPubnubClient::GetUserMetadata_priv(FString User, FString Include)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(User),
		PUBNUB_LOG_INPUT(Include)
	);
	FPubnubUserMetadataResult FinalResult;
	
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(User, FinalResult);
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	FString UnknownInclude;
	pubnub_get_uuid_metadata_opts_t opts = {};
	FUTF8StringHolder UserHolder(User);

	// Zero-init keeps include empty. The C-Core single-entity default would otherwise request custom.
	opts.uuid = UserHolder.Get();
	opts.include = UPubnubInternalUtilities::AppContextIncludeMaskFromString(Include, UnknownInclude);

	if (!UnknownInclude.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring unsupported include tokens: %s"), *UnknownInclude));
	}

	pubnub_future_t operation_future = pubnub_get_uuid_metadata(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("get user metadata request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		FinalResult.UserData = UPubnubInternalUtilities::UserDataFromGetUuidMetadataFuture(operation_future);
		PUBNUB_LOG_FUNCTION_DEBUG(TEXT("get user metadata parsed."), PUBNUB_LOG_VALUE(FinalResult.UserData));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubOperationResult UPubnubClient::RemoveUserMetadata_priv(FString User)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(User)
	);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(User);
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	pubnub_remove_uuid_metadata_opts_t opts = PUBNUB_REMOVE_UUID_METADATA_OPTS_INIT;
	FUTF8StringHolder UserHolder(User);
	opts.uuid = UserHolder.Get();

	pubnub_future_t operation_future = pubnub_remove_uuid_metadata(pubnub_context, &opts);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FUTURE_NOT_IN_PROGRESS();
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("remove user metadata request sent."));

	FPubnubOperationResult Result;
	PUBNUB_OPERATION_RESULT_AWAIT_FOR_FUTURE(Result);

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

FPubnubGetAllChannelMetadataResult UPubnubClient::GetAllChannelMetadata_priv(FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Include),
		PUBNUB_LOG_INPUT(Limit),
		PUBNUB_LOG_INPUT(Filter),
		PUBNUB_LOG_INPUT(Sort),
		PUBNUB_LOG_INPUT(Page),
		PUBNUB_LOG_INPUT(Count)
	);
	FPubnubGetAllChannelMetadataResult FinalResult;
	
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	FString UnknownInclude;
	pubnub_get_all_channel_metadata_opts_t opts = PUBNUB_GET_ALL_CHANNEL_METADATA_OPTS_INIT;
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);

	opts.include = UPubnubInternalUtilities::MetadataListIncludeMask(Include, Count, UnknownInclude);
	opts.limit = static_cast<uint32_t>(FMath::Clamp(Limit, 0, PUBNUB_MAX_LIMIT));
	opts.filter = FilterHolder.GetOrNull();
	opts.sort = SortHolder.GetOrNull();
	// If both Next and Prev are provided, Next takes precedence.
	opts.start = PageNextHolder.GetOrNull();
	opts.end = Page.Next.IsEmpty() ? PagePrevHolder.GetOrNull() : nullptr;

	if (!UnknownInclude.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring unsupported include tokens: %s"), *UnknownInclude));
	}

	pubnub_future_t operation_future = pubnub_get_all_channel_metadata(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("get all channel metadata request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		UPubnubInternalUtilities::GetAllChannelMetadataFromFuture(operation_future, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("channel metadata parsed. ChannelsCount=%d, TotalCount=%d"), FinalResult.ChannelsData.Num(), FinalResult.TotalCount));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubChannelMetadataResult UPubnubClient::SetChannelMetadata_priv(FString Channel, FString ChannelMetadataObj, FString Include)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(ChannelMetadataObj),
		PUBNUB_LOG_INPUT(Include)
	);
	FPubnubChannelMetadataResult FinalResult;
	
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(ChannelMetadataObj, FinalResult);

	FParsedChannelMetadataObject Parsed;
	if (!UPubnubInternalUtilities::ParseChannelMetadataObject(ChannelMetadataObj, Parsed))
	{
		PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(false, *Parsed.Error, FinalResult);
	}
	if (!Parsed.UnknownFields.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring metadata JSON fields the new C-Core set API does not send: %s"), *FString::Join(Parsed.UnknownFields, TEXT(", "))));
	}
	if (!Parsed.NullStringFields.IsEmpty())
	{
		PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(false, *UPubnubInternalUtilities::MetadataNullFieldError(Parsed.NullStringFields), FinalResult);
	}

	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	FString UnknownInclude;
	pubnub_set_channel_metadata_opts_t opts = {};
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder NameHolder(Parsed.Name.Value);
	FUTF8StringHolder DescriptionHolder(Parsed.Description.Value);
	FUTF8StringHolder StatusHolder(Parsed.Status.Value);
	FUTF8StringHolder TypeHolder(Parsed.Type.Value);
	FUTF8StringHolder CustomHolder(Parsed.CustomJson);

	// Zero-init keeps include empty. The C-Core single-entity default would otherwise request custom.
	opts.channel = ChannelHolder.Get();
	opts.include = UPubnubInternalUtilities::AppContextIncludeMaskFromString(Include, UnknownInclude);
	opts.name = UPubnubInternalUtilities::OptionalMetadataString(Parsed.Name, NameHolder);
	opts.description = UPubnubInternalUtilities::OptionalMetadataString(Parsed.Description, DescriptionHolder);
	opts.status = UPubnubInternalUtilities::OptionalMetadataString(Parsed.Status, StatusHolder);
	opts.type = UPubnubInternalUtilities::OptionalMetadataString(Parsed.Type, TypeHolder);
	if (Parsed.bHasCustom)
	{
		opts.custom = CustomHolder.Get();
	}

	if (!UnknownInclude.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring unsupported include tokens: %s"), *UnknownInclude));
	}

	pubnub_future_t operation_future = pubnub_set_channel_metadata(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("set channel metadata request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		FinalResult.ChannelData = UPubnubInternalUtilities::ChannelDataFromSetChannelMetadataFuture(operation_future);
		PUBNUB_LOG_FUNCTION_DEBUG(TEXT("set channel metadata parsed."), PUBNUB_LOG_VALUE(FinalResult.ChannelData));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubChannelMetadataResult UPubnubClient::GetChannelMetadata_priv(FString Channel, FString Include)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(Include)
	);
	FPubnubChannelMetadataResult FinalResult;
	
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FinalResult);
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	FString UnknownInclude;
	pubnub_get_channel_metadata_opts_t opts = {};
	FUTF8StringHolder ChannelHolder(Channel);

	// Zero-init keeps include empty. The C-Core single-entity default would otherwise request custom.
	opts.channel = ChannelHolder.Get();
	opts.include = UPubnubInternalUtilities::AppContextIncludeMaskFromString(Include, UnknownInclude);

	if (!UnknownInclude.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring unsupported include tokens: %s"), *UnknownInclude));
	}

	pubnub_future_t operation_future = pubnub_get_channel_metadata(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("get channel metadata request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		FinalResult.ChannelData = UPubnubInternalUtilities::ChannelDataFromGetChannelMetadataFuture(operation_future);
		PUBNUB_LOG_FUNCTION_DEBUG(TEXT("get channel metadata parsed."), PUBNUB_LOG_VALUE(FinalResult.ChannelData));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubOperationResult UPubnubClient::RemoveChannelMetadata_priv(FString Channel)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel)
	);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	pubnub_remove_channel_metadata_opts_t opts = PUBNUB_REMOVE_CHANNEL_METADATA_OPTS_INIT;
	FUTF8StringHolder ChannelHolder(Channel);
	opts.channel = ChannelHolder.Get();

	pubnub_future_t operation_future = pubnub_remove_channel_metadata(pubnub_context, &opts);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FUTURE_NOT_IN_PROGRESS();
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("remove channel metadata request sent."));

	FPubnubOperationResult Result;
	PUBNUB_OPERATION_RESULT_AWAIT_FOR_FUTURE(Result);

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

FPubnubMembershipsResult UPubnubClient::GetMemberships_priv(FString User, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(User),
		PUBNUB_LOG_INPUT(Include),
		PUBNUB_LOG_INPUT(Limit),
		PUBNUB_LOG_INPUT(Filter),
		PUBNUB_LOG_INPUT(Sort),
		PUBNUB_LOG_INPUT(Page),
		PUBNUB_LOG_INPUT(Count)
	);
	FPubnubMembershipsResult FinalResult;
	
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(User, FinalResult);
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	FString UnknownInclude;
	pubnub_get_memberships_opts_t opts = PUBNUB_GET_MEMBERSHIPS_OPTS_INIT;
	FUTF8StringHolder UserHolder(User);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);

	opts.uuid = UserHolder.Get();
	opts.include = UPubnubInternalUtilities::MetadataListIncludeMask(Include, Count, UnknownInclude);
	opts.limit = static_cast<uint32_t>(FMath::Clamp(Limit, 0, PUBNUB_MAX_LIMIT));
	opts.filter = FilterHolder.GetOrNull();
	opts.sort = SortHolder.GetOrNull();
	// If both Next and Prev are provided, Next takes precedence.
	opts.start = PageNextHolder.GetOrNull();
	opts.end = Page.Next.IsEmpty() ? PagePrevHolder.GetOrNull() : nullptr;

	if (!UnknownInclude.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring unsupported include tokens: %s"), *UnknownInclude));
	}

	pubnub_future_t operation_future = pubnub_get_memberships(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("get memberships request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		UPubnubInternalUtilities::GetMembershipsFromFuture(operation_future, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("memberships parsed. MembershipsCount=%d, TotalCount=%d"), FinalResult.MembershipsData.Num(), FinalResult.TotalCount));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubMembershipsResult UPubnubClient::SetMemberships_priv(FString User, FString SetObj, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(User),
		PUBNUB_LOG_INPUT(SetObj),
		PUBNUB_LOG_INPUT(Include),
		PUBNUB_LOG_INPUT(Limit),
		PUBNUB_LOG_INPUT(Filter),
		PUBNUB_LOG_INPUT(Sort),
		PUBNUB_LOG_INPUT(Page),
		PUBNUB_LOG_INPUT(Count)
	);
	FPubnubMembershipsResult FinalResult;
	
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(User, FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(SetObj, FinalResult);

	FParsedRelationList Parsed;
	if (!UPubnubInternalUtilities::ParseMembershipSetList(SetObj, Parsed))
	{
		PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(false, *Parsed.Error, FinalResult);
	}
	if (!Parsed.UnknownFields.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring membership JSON fields the new C-Core set API does not send: %s"), *FString::Join(Parsed.UnknownFields, TEXT(", "))));
	}
	if (!Parsed.NullStringFields.IsEmpty())
	{
		PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(false, *UPubnubInternalUtilities::MetadataNullFieldError(Parsed.NullStringFields), FinalResult);
	}

	FMembershipInputBatch Inputs;
	UPubnubInternalUtilities::BuildMembershipInputs(Parsed, Inputs);

	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	FString UnknownInclude;
	pubnub_set_memberships_opts_t opts = PUBNUB_SET_MEMBERSHIPS_OPTS_INIT;
	FUTF8StringHolder UserHolder(User);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);

	opts.uuid = UserHolder.Get();
	opts.set = Inputs.Items.GetData();
	opts.set_count = static_cast<size_t>(Inputs.Items.Num());
	opts.include = UPubnubInternalUtilities::MetadataListIncludeMask(Include, Count, UnknownInclude);
	opts.limit = static_cast<uint32_t>(FMath::Clamp(Limit, 0, PUBNUB_MAX_LIMIT));
	opts.filter = FilterHolder.GetOrNull();
	opts.sort = SortHolder.GetOrNull();
	opts.start = PageNextHolder.GetOrNull();
	opts.end = Page.Next.IsEmpty() ? PagePrevHolder.GetOrNull() : nullptr;

	if (!UnknownInclude.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring unsupported include tokens: %s"), *UnknownInclude));
	}

	pubnub_future_t operation_future = pubnub_set_memberships(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("set memberships request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		UPubnubInternalUtilities::SetMembershipsFromFuture(operation_future, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("set memberships parsed. MembershipsCount=%d, TotalCount=%d"), FinalResult.MembershipsData.Num(), FinalResult.TotalCount));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubMembershipsResult UPubnubClient::RemoveMemberships_priv(FString User, FString RemoveObj, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(User),
		PUBNUB_LOG_INPUT(RemoveObj),
		PUBNUB_LOG_INPUT(Include),
		PUBNUB_LOG_INPUT(Limit),
		PUBNUB_LOG_INPUT(Filter),
		PUBNUB_LOG_INPUT(Sort),
		PUBNUB_LOG_INPUT(Page),
		PUBNUB_LOG_INPUT(Count)
	);
	FPubnubMembershipsResult FinalResult;
	
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(User, FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(RemoveObj, FinalResult);

	FParsedRelationList Parsed;
	if (!UPubnubInternalUtilities::ParseMembershipRemoveList(RemoveObj, Parsed))
	{
		PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(false, *Parsed.Error, FinalResult);
	}
	if (!Parsed.UnknownFields.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring membership JSON fields the new C-Core remove API does not send: %s"), *FString::Join(Parsed.UnknownFields, TEXT(", "))));
	}

	FMembershipInputBatch Inputs;
	UPubnubInternalUtilities::BuildMembershipInputs(Parsed, Inputs);

	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	FString UnknownInclude;
	pubnub_set_memberships_opts_t opts = PUBNUB_SET_MEMBERSHIPS_OPTS_INIT;
	FUTF8StringHolder UserHolder(User);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);

	opts.uuid = UserHolder.Get();
	opts.remove = Inputs.Items.GetData();
	opts.remove_count = static_cast<size_t>(Inputs.Items.Num());
	opts.include = UPubnubInternalUtilities::MetadataListIncludeMask(Include, Count, UnknownInclude);
	opts.limit = static_cast<uint32_t>(FMath::Clamp(Limit, 0, PUBNUB_MAX_LIMIT));
	opts.filter = FilterHolder.GetOrNull();
	opts.sort = SortHolder.GetOrNull();
	opts.start = PageNextHolder.GetOrNull();
	opts.end = Page.Next.IsEmpty() ? PagePrevHolder.GetOrNull() : nullptr;

	if (!UnknownInclude.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring unsupported include tokens: %s"), *UnknownInclude));
	}

	pubnub_future_t operation_future = pubnub_set_memberships(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("remove memberships request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		UPubnubInternalUtilities::SetMembershipsFromFuture(operation_future, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("remove memberships parsed. MembershipsCount=%d, TotalCount=%d"), FinalResult.MembershipsData.Num(), FinalResult.TotalCount));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubChannelMembersResult UPubnubClient::GetChannelMembers_priv(FString Channel, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(Include),
		PUBNUB_LOG_INPUT(Limit),
		PUBNUB_LOG_INPUT(Filter),
		PUBNUB_LOG_INPUT(Sort),
		PUBNUB_LOG_INPUT(Page),
		PUBNUB_LOG_INPUT(Count)
	);
	FPubnubChannelMembersResult FinalResult;
	
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FinalResult);
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	FString UnknownInclude;
	pubnub_get_channel_members_opts_t opts = PUBNUB_GET_CHANNEL_MEMBERS_OPTS_INIT;
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);

	opts.channel = ChannelHolder.Get();
	opts.include = UPubnubInternalUtilities::MetadataListIncludeMask(Include, Count, UnknownInclude);
	opts.limit = static_cast<uint32_t>(FMath::Clamp(Limit, 0, PUBNUB_MAX_LIMIT));
	opts.filter = FilterHolder.GetOrNull();
	opts.sort = SortHolder.GetOrNull();
	// If both Next and Prev are provided, Next takes precedence.
	opts.start = PageNextHolder.GetOrNull();
	opts.end = Page.Next.IsEmpty() ? PagePrevHolder.GetOrNull() : nullptr;

	if (!UnknownInclude.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring unsupported include tokens: %s"), *UnknownInclude));
	}

	pubnub_future_t operation_future = pubnub_get_channel_members(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("get channel members request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		UPubnubInternalUtilities::GetChannelMembersFromFuture(operation_future, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("channel members parsed. MembersCount=%d, TotalCount=%d"), FinalResult.MembersData.Num(), FinalResult.TotalCount));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubChannelMembersResult UPubnubClient::SetChannelMembers_priv(FString Channel, FString SetObj, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(SetObj),
		PUBNUB_LOG_INPUT(Include),
		PUBNUB_LOG_INPUT(Limit),
		PUBNUB_LOG_INPUT(Filter),
		PUBNUB_LOG_INPUT(Sort),
		PUBNUB_LOG_INPUT(Page),
		PUBNUB_LOG_INPUT(Count)
	);
	FPubnubChannelMembersResult FinalResult;
	
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(SetObj, FinalResult);

	FParsedRelationList Parsed;
	if (!UPubnubInternalUtilities::ParseMemberSetList(SetObj, Parsed))
	{
		PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(false, *Parsed.Error, FinalResult);
	}
	if (!Parsed.UnknownFields.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring member JSON fields the new C-Core set API does not send: %s"), *FString::Join(Parsed.UnknownFields, TEXT(", "))));
	}
	if (!Parsed.NullStringFields.IsEmpty())
	{
		PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(false, *UPubnubInternalUtilities::MetadataNullFieldError(Parsed.NullStringFields), FinalResult);
	}

	FMemberInputBatch Inputs;
	UPubnubInternalUtilities::BuildMemberInputs(Parsed, Inputs);

	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	FString UnknownInclude;
	pubnub_set_channel_members_opts_t opts = PUBNUB_SET_CHANNEL_MEMBERS_OPTS_INIT;
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);

	opts.channel = ChannelHolder.Get();
	opts.set = Inputs.Items.GetData();
	opts.set_count = static_cast<size_t>(Inputs.Items.Num());
	opts.include = UPubnubInternalUtilities::MetadataListIncludeMask(Include, Count, UnknownInclude);
	opts.limit = static_cast<uint32_t>(FMath::Clamp(Limit, 0, PUBNUB_MAX_LIMIT));
	opts.filter = FilterHolder.GetOrNull();
	opts.sort = SortHolder.GetOrNull();
	opts.start = PageNextHolder.GetOrNull();
	opts.end = Page.Next.IsEmpty() ? PagePrevHolder.GetOrNull() : nullptr;

	if (!UnknownInclude.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring unsupported include tokens: %s"), *UnknownInclude));
	}

	pubnub_future_t operation_future = pubnub_set_channel_members(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("set channel members request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		UPubnubInternalUtilities::SetChannelMembersFromFuture(operation_future, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("set channel members parsed. MembersCount=%d, TotalCount=%d"), FinalResult.MembersData.Num(), FinalResult.TotalCount));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubChannelMembersResult UPubnubClient::RemoveChannelMembers_priv(FString Channel, FString RemoveObj, FString Include, int Limit, FString Filter, FString Sort, FPubnubPage Page, EPubnubTribool Count)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(RemoveObj),
		PUBNUB_LOG_INPUT(Include),
		PUBNUB_LOG_INPUT(Limit),
		PUBNUB_LOG_INPUT(Filter),
		PUBNUB_LOG_INPUT(Sort),
		PUBNUB_LOG_INPUT(Page),
		PUBNUB_LOG_INPUT(Count)
	);
	FPubnubChannelMembersResult FinalResult;
	
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(RemoveObj, FinalResult);

	FParsedRelationList Parsed;
	if (!UPubnubInternalUtilities::ParseMemberRemoveList(RemoveObj, Parsed))
	{
		PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(false, *Parsed.Error, FinalResult);
	}
	if (!Parsed.UnknownFields.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring member JSON fields the new C-Core remove API does not send: %s"), *FString::Join(Parsed.UnknownFields, TEXT(", "))));
	}

	FMemberInputBatch Inputs;
	UPubnubInternalUtilities::BuildMemberInputs(Parsed, Inputs);

	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	FString UnknownInclude;
	pubnub_set_channel_members_opts_t opts = PUBNUB_SET_CHANNEL_MEMBERS_OPTS_INIT;
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder FilterHolder(Filter);
	FUTF8StringHolder SortHolder(Sort);
	FUTF8StringHolder PageNextHolder(Page.Next);
	FUTF8StringHolder PagePrevHolder(Page.Prev);

	opts.channel = ChannelHolder.Get();
	opts.remove = Inputs.Items.GetData();
	opts.remove_count = static_cast<size_t>(Inputs.Items.Num());
	opts.include = UPubnubInternalUtilities::MetadataListIncludeMask(Include, Count, UnknownInclude);
	opts.limit = static_cast<uint32_t>(FMath::Clamp(Limit, 0, PUBNUB_MAX_LIMIT));
	opts.filter = FilterHolder.GetOrNull();
	opts.sort = SortHolder.GetOrNull();
	opts.start = PageNextHolder.GetOrNull();
	opts.end = Page.Next.IsEmpty() ? PagePrevHolder.GetOrNull() : nullptr;

	if (!UnknownInclude.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(FString::Printf(TEXT("Ignoring unsupported include tokens: %s"), *UnknownInclude));
	}

	pubnub_future_t operation_future = pubnub_set_channel_members(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("remove channel members request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		UPubnubInternalUtilities::SetChannelMembersFromFuture(operation_future, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("remove channel members parsed. MembersCount=%d, TotalCount=%d"), FinalResult.MembersData.Num(), FinalResult.TotalCount));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubAddMessageActionResult UPubnubClient::AddMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionType, FString Value)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(MessageTimetoken),
		PUBNUB_LOG_INPUT(ActionType),
		PUBNUB_LOG_INPUT(Value)
	);
	FPubnubAddMessageActionResult FinalResult;

	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(MessageTimetoken, FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(ActionType, FinalResult);
	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Value, FinalResult);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	// New C-Core JSON-encodes type and value. Do not quote them the way the old C-Core required.
	pubnub_add_message_action_opts_t opts = PUBNUB_ADD_MESSAGE_ACTION_OPTS_INIT;
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder MessageTimetokenHolder(MessageTimetoken);
	FUTF8StringHolder ActionTypeHolder(ActionType);
	FUTF8StringHolder ValueHolder(Value);

	opts.channel = ChannelHolder.Get();
	opts.message_timetoken = MessageTimetokenHolder.Get();
	opts.type = ActionTypeHolder.Get();
	opts.value = ValueHolder.Get();

	pubnub_future_t operation_future = pubnub_add_message_action(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("add message action request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		UPubnubInternalUtilities::AddMessageActionFromFuture(operation_future, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("message action parsed. Type=%s, ActionTimetoken=%s"), *FinalResult.MessageActionData.Type, *FinalResult.MessageActionData.ActionTimetoken));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

FPubnubOperationResult UPubnubClient::RemoveMessageAction_priv(FString Channel, FString MessageTimetoken, FString ActionTimetoken)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(MessageTimetoken),
		PUBNUB_LOG_INPUT(ActionTimetoken)
	);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Channel);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(MessageTimetoken);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(ActionTimetoken);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED();

	// New C-Core takes raw timetokens. Do not quote them the way the old C-Core required.
	pubnub_remove_message_action_opts_t opts = PUBNUB_REMOVE_MESSAGE_ACTION_OPTS_INIT;
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder MessageTimetokenHolder(MessageTimetoken);
	FUTF8StringHolder ActionTimetokenHolder(ActionTimetoken);

	opts.channel = ChannelHolder.Get();
	opts.message_timetoken = MessageTimetokenHolder.Get();
	opts.action_timetoken = ActionTimetokenHolder.Get();

	pubnub_future_t operation_future = pubnub_remove_message_action(pubnub_context, &opts);
	PUBNUB_RETURN_OPERATION_RESULT_IF_FUTURE_NOT_IN_PROGRESS();
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("remove message action request sent."));

	FPubnubOperationResult Result;
	PUBNUB_OPERATION_RESULT_AWAIT_FOR_FUTURE(Result);

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

FPubnubGetMessageActionsResult UPubnubClient::GetMessageActions_priv(FString Channel, FString Start, FString End, int Limit)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_INPUT(Channel),
		PUBNUB_LOG_INPUT(Start),
		PUBNUB_LOG_INPUT(End),
		PUBNUB_LOG_INPUT(Limit)
	);
	FPubnubGetMessageActionsResult FinalResult;

	PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Channel, FinalResult);
	// Try to acquire lock - fail fast if another operation is in progress
	PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(FinalResult);

	pubnub_get_message_actions_opts_t opts = PUBNUB_GET_MESSAGE_ACTIONS_OPTS_INIT;
	FUTF8StringHolder ChannelHolder(Channel);
	FUTF8StringHolder StartHolder(Start);
	FUTF8StringHolder EndHolder(End);

	opts.channel = ChannelHolder.Get();
	opts.start = StartHolder.GetOrNull();
	opts.end = EndHolder.GetOrNull();
	opts.limit = static_cast<uint32_t>(FMath::Clamp(Limit, 0, PUBNUB_MAX_LIMIT));

	pubnub_future_t operation_future = pubnub_get_message_actions(pubnub_context, &opts);
	PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS(FinalResult);
	PUBNUB_LOG_FUNCTION_TRACE(TEXT("get message actions request sent."));

	PUBNUB_WRAPPER_AWAIT_FOR_FUTURE(FinalResult);

	if (!FinalResult.Result.Error)
	{
		UPubnubInternalUtilities::GetMessageActionsFromFuture(operation_future, FinalResult);
		PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("message actions parsed. Count=%d"), FinalResult.MessageActions.Num()));
	}

	pubnub_future_release(operation_future);
	*InFlightFuture = MakeInvalidFuture();
	PUBNUB_LOG_OPERATION_RESULT(FinalResult.Result);
	return FinalResult;
}

UPubnubChannelEntity* UPubnubClient::CreateChannelEntity(FString Channel)
{
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	PUBNUB_RETURN_IF_FIELD_EMPTY(Channel, nullptr);

	UPubnubChannelEntity* ChannelEntity = UPubnubInternalUtilities::SafeNewObject<UPubnubChannelEntity>(this);
	ChannelEntity->InitEntity(this);
	ChannelEntity->EntityID = Channel;
	PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("channel entity created for '%s'."), *Channel));
	return ChannelEntity;
}

UPubnubChannelGroupEntity* UPubnubClient::CreateChannelGroupEntity(FString ChannelGroup)
{
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	PUBNUB_RETURN_IF_FIELD_EMPTY(ChannelGroup, nullptr);

	UPubnubChannelGroupEntity* ChannelGroupEntity = UPubnubInternalUtilities::SafeNewObject<UPubnubChannelGroupEntity>(this);
	ChannelGroupEntity->InitEntity(this);
	ChannelGroupEntity->EntityID = ChannelGroup;
	PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("channel group entity created for '%s'."), *ChannelGroup));
	return ChannelGroupEntity;
}

UPubnubChannelMetadataEntity* UPubnubClient::CreateChannelMetadataEntity(FString Channel)
{
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	PUBNUB_RETURN_IF_FIELD_EMPTY(Channel, nullptr);

	UPubnubChannelMetadataEntity* ChannelMetadataEntity = UPubnubInternalUtilities::SafeNewObject<UPubnubChannelMetadataEntity>(this);
	ChannelMetadataEntity->InitEntity(this);
	ChannelMetadataEntity->EntityID = Channel;
	PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("channel metadata entity created for '%s'."), *Channel));
	return ChannelMetadataEntity;
}

UPubnubUserMetadataEntity* UPubnubClient::CreateUserMetadataEntity(FString User)
{
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	PUBNUB_RETURN_IF_FIELD_EMPTY(User, nullptr);

	UPubnubUserMetadataEntity* UserMetadataEntity = UPubnubInternalUtilities::SafeNewObject<UPubnubUserMetadataEntity>(this);
	UserMetadataEntity->InitEntity(this);
	UserMetadataEntity->EntityID = User;
	PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("user metadata entity created for '%s'."), *User));
	return UserMetadataEntity;
}

UPubnubSubscriptionSet* UPubnubClient::CreateSubscriptionSet(TArray<FString> Channels, TArray<FString> ChannelGroups, FPubnubSubscribeSettings SubscriptionSettings)
{
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("CreateSubscriptionSet inputs: ChannelsCount=%d, ChannelGroupsCount=%d"), Channels.Num(), ChannelGroups.Num()));
	if (Channels.IsEmpty() && ChannelGroups.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(TEXT("[CreateSubscriptionSet]: at least one Channel or ChannelGroup is needed to create SubscriptionSet."));
	}

	UPubnubSubscriptionSet* SubscriptionSet = UPubnubInternalUtilities::SafeNewObject<UPubnubSubscriptionSet>(this);
	SubscriptionSet->InitSubscriptionSet(this, Channels, ChannelGroups, SubscriptionSettings);
	PUBNUB_LOG_FUNCTION_DEBUG_TEXT(TEXT("subscription set created."));
	return SubscriptionSet;
}

UPubnubSubscriptionSet* UPubnubClient::CreateSubscriptionSetFromEntities(TArray<UPubnubBaseEntity*> Entities, FPubnubSubscribeSettings SubscriptionSettings)
{
	PUBNUB_LOG_FUNCTION_CALLED_TRACE();
	PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("CreateSubscriptionSetFromEntities inputs: EntitiesCount=%d"), Entities.Num()));

	TArray<FString> Channels;
	TArray<FString> ChannelGroups;
	TArray<FString> ChannelMetadataIds;
	TArray<FString> UserMetadataIds;
	for (UPubnubBaseEntity* Entity : Entities)
	{
		if (!Entity || Entity->EntityID.IsEmpty())
		{
			continue;
		}

		switch (Entity->EntityType)
		{
		case EPubnubEntityType::PEnT_ChannelGroup:
			ChannelGroups.Add(Entity->EntityID);
			break;
		case EPubnubEntityType::PEnT_ChannelMetadata:
			ChannelMetadataIds.Add(Entity->EntityID);
			break;
		case EPubnubEntityType::PEnT_UserMetadata:
			UserMetadataIds.Add(Entity->EntityID);
			break;
		case EPubnubEntityType::PEnT_Channel:
		default:
			Channels.Add(Entity->EntityID);
			break;
		}
	}

	if (Channels.IsEmpty() && ChannelGroups.IsEmpty() && ChannelMetadataIds.IsEmpty() && UserMetadataIds.IsEmpty())
	{
		PUBNUB_LOG_FUNCTION_WARNING(TEXT("[CreateSubscriptionSetFromEntities]: at least one Entity is needed to create SubscriptionSet."));
	}

	UPubnubSubscriptionSet* SubscriptionSet = UPubnubInternalUtilities::SafeNewObject<UPubnubSubscriptionSet>(this);
	SubscriptionSet->InitSubscriptionSet(this, Channels, ChannelGroups, ChannelMetadataIds, UserMetadataIds, SubscriptionSettings);
	PUBNUB_LOG_FUNCTION_DEBUG_TEXT(FString::Printf(TEXT("subscription set created from entities. Channels=%d, Groups=%d, ChannelMetadata=%d, UserMetadata=%d"), Channels.Num(), ChannelGroups.Num(), ChannelMetadataIds.Num(), UserMetadataIds.Num()));
	return SubscriptionSet;
}

TArray<UPubnubSubscription*> UPubnubClient::GetActiveSubscriptions()
{
	TArray<pubnub_subscription_t> CCoreSubscriptions;
	UPubnubInternalUtilities::ListActiveCCoreSubscriptions(pubnub_context, CCoreSubscriptions);

	TArray<UPubnubSubscription*> Subscriptions;
	Subscriptions.Reserve(CCoreSubscriptions.Num());
	for (pubnub_subscription_t CCoreSubscription : CCoreSubscriptions)
	{
		if (UPubnubSubscription* Existing = FindManagedSubscription(CCoreSubscription))
		{
			Subscriptions.Add(Existing);
		}
	}
	return Subscriptions;
}

TArray<UPubnubSubscriptionSet*> UPubnubClient::GetActiveSubscriptionSets()
{
	TArray<pubnub_subscription_set_t> CCoreSubscriptionSets;
	UPubnubInternalUtilities::ListActiveCCoreSubscriptionSets(pubnub_context, CCoreSubscriptionSets);

	TArray<UPubnubSubscriptionSet*> SubscriptionSets;
	SubscriptionSets.Reserve(CCoreSubscriptionSets.Num());
	for (pubnub_subscription_set_t CCoreSubscriptionSet : CCoreSubscriptionSets)
	{
		UPubnubSubscriptionSet* SubscriptionSet = FindManagedSubscriptionSet(CCoreSubscriptionSet);
		if (!SubscriptionSet)
		{
			continue;
		}

		SubscriptionSets.Add(SubscriptionSet);
		if (!SubscriptionSet->Subscriptions.IsEmpty())
		{
			continue;
		}

		TArray<pubnub_subscription_t> Members;
		UPubnubInternalUtilities::ListCCoreSetSubscriptions(CCoreSubscriptionSet, Members);
		for (pubnub_subscription_t Member : Members)
		{
			if (UPubnubSubscription* Existing = FindManagedSubscription(Member))
			{
				SubscriptionSet->Subscriptions.Add(Existing);
			}
		}
	}
	return SubscriptionSets;
}

void UPubnubClient::RegisterManagedSubscription(pubnub_subscription_t CCorePtr, UPubnubSubscription* Wrapper)
{
	if (!CCorePtr || !Wrapper)
	{
		return;
	}

	if (const TWeakObjectPtr<UPubnubSubscription>* Existing = ManagedSubscriptions.Find(CCorePtr))
	{
		if (UPubnubSubscription* AlivePrev = Existing->Get(); IsValid(AlivePrev) && AlivePrev != Wrapper)
		{
			return;
		}
	}

	ManagedSubscriptions.Add(CCorePtr, Wrapper);
}

void UPubnubClient::RegisterManagedSubscriptionSet(pubnub_subscription_set_t CCorePtr, UPubnubSubscriptionSet* Wrapper)
{
	if (!CCorePtr || !Wrapper)
	{
		return;
	}

	if (const TWeakObjectPtr<UPubnubSubscriptionSet>* Existing = ManagedSubscriptionSets.Find(CCorePtr))
	{
		if (UPubnubSubscriptionSet* AlivePrev = Existing->Get(); IsValid(AlivePrev) && AlivePrev != Wrapper)
		{
			return;
		}
	}

	ManagedSubscriptionSets.Add(CCorePtr, Wrapper);
}

void UPubnubClient::UnregisterManagedSubscription(pubnub_subscription_t CCorePtr)
{
	if (!CCorePtr)
	{
		return;
	}
	ManagedSubscriptions.Remove(CCorePtr);
}

void UPubnubClient::UnregisterManagedSubscriptionSet(pubnub_subscription_set_t CCorePtr)
{
	if (!CCorePtr)
	{
		return;
	}
	ManagedSubscriptionSets.Remove(CCorePtr);
}

UPubnubSubscription* UPubnubClient::FindManagedSubscription(pubnub_subscription_t CCorePtr)
{
	if (!CCorePtr)
	{
		return nullptr;
	}

	const TWeakObjectPtr<UPubnubSubscription>* Found = ManagedSubscriptions.Find(CCorePtr);
	if (!Found)
	{
		return nullptr;
	}

	UPubnubSubscription* Wrapper = Found->Get();
	if (!IsValid(Wrapper))
	{
		ManagedSubscriptions.Remove(CCorePtr);
		return nullptr;
	}
	return Wrapper;
}

UPubnubSubscriptionSet* UPubnubClient::FindManagedSubscriptionSet(pubnub_subscription_set_t CCorePtr)
{
	if (!CCorePtr)
	{
		return nullptr;
	}

	const TWeakObjectPtr<UPubnubSubscriptionSet>* Found = ManagedSubscriptionSets.Find(CCorePtr);
	if (!Found)
	{
		return nullptr;
	}

	UPubnubSubscriptionSet* Wrapper = Found->Get();
	if (!IsValid(Wrapper))
	{
		ManagedSubscriptionSets.Remove(CCorePtr);
		return nullptr;
	}
	return Wrapper;
}

void UPubnubClient::SubscribeWithSubscriptionAsync(UPubnubSubscription* Subscription, FPubnubSubscriptionCursor Cursor, FOnPubnubSubscribeOperationResponseNative OnSubscribeResponse)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(OnSubscribeResponse);

	TWeakObjectPtr<UPubnubClient> WeakThis(this);
	PubnubCallsThread->AddFunctionToQueue([WeakThis, Subscription, Cursor, OnSubscribeResponse]()
	{
		if (!WeakThis.IsValid())
		{
			return;
		}

		const FPubnubOperationResult SubscribeResult = WeakThis->SubscribeWithSubscription(Subscription, Cursor);
		UPubnubUtilities::CallPubnubDelegate(OnSubscribeResponse, SubscribeResult);
	});
}

FPubnubOperationResult UPubnubClient::SubscribeWithSubscription(UPubnubSubscription* Subscription, FPubnubSubscriptionCursor Cursor)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_VALUE(Subscription),
		PUBNUB_LOG_VALUE(Cursor)
	);
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(Subscription, TEXT("Subscription is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(Subscription->CCoreSubscription, TEXT("CCoreSubscription is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(!Subscription->bIsSubscribed, TEXT("Subscription is already subscribed."));

	FScopeLock SubscriptionsLock(&SubscriptionsMutex);
	if (!pubnub_context)
	{
		FPubnubOperationResult Result({0, true, TEXT("PubnubClient was deinitialized before the subscribe operation could run.")});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	const int32 SubscribeResultCode = UPubnubInternalUtilities::ActivateCCoreSubscription(pubnub_context, Subscription->CCoreSubscription, Cursor);
	if (SubscribeResultCode != PUBNUB_OK)
	{
		FPubnubOperationResult Result({0, true, FString::Printf(TEXT("Failed to subscribe with Subscription. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(static_cast<pubnub_res_t>(SubscribeResultCode))))});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	Subscription->bIsSubscribed = true;
	FPubnubOperationResult Result({200, false, TEXT("")});
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

void UPubnubClient::SubscribeWithSubscriptionSetAsync(UPubnubSubscriptionSet* SubscriptionSet, FPubnubSubscriptionCursor Cursor, FOnPubnubSubscribeOperationResponseNative OnSubscribeResponse)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(OnSubscribeResponse);

	TWeakObjectPtr<UPubnubClient> WeakThis(this);
	PubnubCallsThread->AddFunctionToQueue([WeakThis, SubscriptionSet, Cursor, OnSubscribeResponse]()
	{
		if (!WeakThis.IsValid())
		{
			return;
		}

		const FPubnubOperationResult SubscribeResult = WeakThis->SubscribeWithSubscriptionSet(SubscriptionSet, Cursor);
		UPubnubUtilities::CallPubnubDelegate(OnSubscribeResponse, SubscribeResult);
	});
}

FPubnubOperationResult UPubnubClient::SubscribeWithSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet, FPubnubSubscriptionCursor Cursor)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_VALUE(SubscriptionSet),
		PUBNUB_LOG_VALUE(Cursor)
	);
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(SubscriptionSet, TEXT("SubscriptionSet is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(SubscriptionSet->CCoreSubscriptionSet, TEXT("CCoreSubscriptionSet is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(!SubscriptionSet->bIsSubscribed, TEXT("SubscriptionSet is already subscribed."));

	FScopeLock SubscriptionsLock(&SubscriptionsMutex);
	if (!pubnub_context)
	{
		FPubnubOperationResult Result({0, true, TEXT("PubnubClient was deinitialized before the subscribe operation could run.")});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	const int32 SubscribeResultCode = UPubnubInternalUtilities::ActivateCCoreSubscriptionSet(pubnub_context, SubscriptionSet->CCoreSubscriptionSet, Cursor);
	if (SubscribeResultCode != PUBNUB_OK)
	{
		FPubnubOperationResult Result({0, true, FString::Printf(TEXT("Failed to subscribe with SubscriptionSet. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(static_cast<pubnub_res_t>(SubscribeResultCode))))});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	SubscriptionSet->bIsSubscribed = true;
	FPubnubOperationResult Result({200, false, TEXT("")});
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

void UPubnubClient::UnsubscribeWithSubscriptionAsync(UPubnubSubscription* Subscription, FOnPubnubSubscribeOperationResponseNative OnUnsubscribeResponse)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(OnUnsubscribeResponse);

	TWeakObjectPtr<UPubnubClient> WeakThis(this);
	PubnubCallsThread->AddFunctionToQueue([WeakThis, Subscription, OnUnsubscribeResponse]()
	{
		if (!WeakThis.IsValid())
		{
			return;
		}

		const FPubnubOperationResult UnsubscribeResult = WeakThis->UnsubscribeWithSubscription(Subscription);
		UPubnubUtilities::CallPubnubDelegate(OnUnsubscribeResponse, UnsubscribeResult);
	});
}

FPubnubOperationResult UPubnubClient::UnsubscribeWithSubscription(UPubnubSubscription* Subscription)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_VALUE(Subscription)
	);
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(Subscription, TEXT("Subscription is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(Subscription->CCoreSubscription, TEXT("Subscription CCoreSubscription is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(Subscription->bIsSubscribed, TEXT("Subscription is not subscribed."));

	FScopeLock SubscriptionsLock(&SubscriptionsMutex);
	const pubnub_res_t UnsubscribeResultCode = pubnub_subscription_unsubscribe(Subscription->CCoreSubscription);
	if (UnsubscribeResultCode != PUBNUB_OK)
	{
		FPubnubOperationResult Result({0, true, FString::Printf(TEXT("Failed to unsubscribe with Subscription. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(UnsubscribeResultCode)))});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	Subscription->bIsSubscribed = false;
	FPubnubOperationResult Result({200, false, TEXT("")});
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}

void UPubnubClient::UnsubscribeWithSubscriptionSetAsync(UPubnubSubscriptionSet* SubscriptionSet, FOnPubnubSubscribeOperationResponseNative OnUnsubscribeResponse)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(OnUnsubscribeResponse);

	TWeakObjectPtr<UPubnubClient> WeakThis(this);
	PubnubCallsThread->AddFunctionToQueue([WeakThis, SubscriptionSet, OnUnsubscribeResponse]()
	{
		if (!WeakThis.IsValid())
		{
			return;
		}

		const FPubnubOperationResult UnsubscribeResult = WeakThis->UnsubscribeWithSubscriptionSet(SubscriptionSet);
		UPubnubUtilities::CallPubnubDelegate(OnUnsubscribeResponse, UnsubscribeResult);
	});
}

FPubnubOperationResult UPubnubClient::UnsubscribeWithSubscriptionSet(UPubnubSubscriptionSet* SubscriptionSet)
{
	PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(
		PUBNUB_LOG_VALUE(SubscriptionSet)
	);
	PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED();
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(SubscriptionSet, TEXT("SubscriptionSet is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(SubscriptionSet->CCoreSubscriptionSet, TEXT("SubscriptionSet CCoreSubscriptionSet is invalid."));
	PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(SubscriptionSet->bIsSubscribed, TEXT("SubscriptionSet is not subscribed."));

	FScopeLock SubscriptionsLock(&SubscriptionsMutex);
	const pubnub_res_t UnsubscribeResultCode = pubnub_subscription_set_unsubscribe(SubscriptionSet->CCoreSubscriptionSet);
	if (UnsubscribeResultCode != PUBNUB_OK)
	{
		FPubnubOperationResult Result({0, true, FString::Printf(TEXT("Failed to unsubscribe with SubscriptionSet. Error: %s"), UTF8_TO_TCHAR(pubnub_res_str(UnsubscribeResultCode)))});
		PUBNUB_LOG_OPERATION_RESULT(Result);
		return Result;
	}

	SubscriptionSet->bIsSubscribed = false;
	FPubnubOperationResult Result({200, false, TEXT("")});
	PUBNUB_LOG_OPERATION_RESULT(Result);
	return Result;
}
