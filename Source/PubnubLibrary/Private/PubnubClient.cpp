// Fill out your copyright notice in the Description page of Project Settings.


#include "PubnubClient.h"
#include "PubNub.h"
#include "PubnubInternalMacros.h"
#include "PubnubSubsystem.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "Threads/PubnubFunctionThread.h"
#include "FunctionLibraries/PubnubUtilities.h"
#include "FunctionLibraries/PubnubInternalUtilities.h"


struct CCoreSubscriptionCallback
{
	pubnub_subscribe_message_callback_t Callback;
	pubnub_subscription_t* Subscription;
};

//TODO:: Move this to logger
static void PubnubSDKLogConverter(enum pubnub_log_level log_level, const char* message)
{
	
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
		WeakThis.Get()->OnError.Broadcast(ErrorMessage, ErrorType);
		WeakThis.Get()->OnErrorNative.Broadcast(ErrorMessage, ErrorType);
	});
}

//TODO:: Move this to logger
void UPubnubClient::PubnubResponseError(pubnub_res PubnubResponse, FString ErrorMessage)
{
	//Convert all error data into single string
	FString ResponseString(pubnub_res_2_string(PubnubResponse));
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

void UPubnubClient::PublishMessage(FString Channel, FString Message, FPubnubOnPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings)
{
	FPubnubOnPublishMessageResponseNative NativeCallback;
	NativeCallback.BindLambda([OnPublishMessageResponse](FPubnubOperationResult Result, FPubnubMessageData PublishedMessage)
	{
		OnPublishMessageResponse.ExecuteIfBound(Result, PublishedMessage);
	});

	PublishMessage(Channel, Message, NativeCallback, PublishSettings);
}

void UPubnubClient::PublishMessage(FString Channel, FString Message, FPubnubOnPublishMessageResponseNative NativeCallback, FPubnubPublishSettings PublishSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubMessageData());
	
	PubnubCallsThread->AddFunctionToQueue( [this, Channel, Message, NativeCallback, PublishSettings]
	{
		PublishMessage_priv(Channel, Message, NativeCallback, PublishSettings);
	});
}

void UPubnubClient::PublishMessage(FString Channel, FString Message, FPubnubPublishSettings PublishSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	PubnubCallsThread->AddFunctionToQueue( [this, Channel, Message, PublishSettings]
	{
		PublishMessage_priv(Channel, Message, nullptr, PublishSettings);
	});
}

void UPubnubClient::Signal(FString Channel, FString Message, FPubnubOnSignalResponse OnSignalResponse, FPubnubSignalSettings SignalSettings)
{
	FPubnubOnSignalResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSignalResponse](const FPubnubOperationResult& Result, const FPubnubMessageData& SignalMessage)
	{
		OnSignalResponse.ExecuteIfBound(Result, SignalMessage);
	});

	Signal(Channel, Message, NativeCallback, SignalSettings);
}

void UPubnubClient::Signal(FString Channel, FString Message, FPubnubOnSignalResponseNative NativeCallback, FPubnubSignalSettings SignalSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback, FPubnubMessageData());
	
	PubnubCallsThread->AddFunctionToQueue( [this, Channel, Message, NativeCallback, SignalSettings]
	{
		Signal_priv(Channel, Message, NativeCallback, SignalSettings);
	});
}

void UPubnubClient::Signal(FString Channel, FString Message, FPubnubSignalSettings SignalSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	PubnubCallsThread->AddFunctionToQueue( [this, Channel, Message, SignalSettings]
	{
		Signal_priv(Channel, Message, nullptr, SignalSettings);
	});
}


void UPubnubClient::SubscribeToChannel(FString Channel, FPubnubOnSubscribeOperationResponse OnSubscribeToChannelResponse, FPubnubSubscribeSettings SubscribeSettings)
{
	FPubnubOnSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSubscribeToChannelResponse](FPubnubOperationResult Result)
	{
		OnSubscribeToChannelResponse.ExecuteIfBound(Result);
	});

	SubscribeToChannel(Channel, NativeCallback, SubscribeSettings);
}

void UPubnubClient::SubscribeToChannel(FString Channel, FPubnubOnSubscribeOperationResponseNative NativeCallback, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(NativeCallback);
	
	PubnubCallsThread->AddFunctionToQueue( [this, Channel, NativeCallback, SubscribeSettings]
	{
		SubscribeToChannel_priv(Channel, NativeCallback, SubscribeSettings);
	});
}

void UPubnubClient::SubscribeToChannel(FString Channel, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	SubscribeToChannel(Channel, nullptr, SubscribeSettings);
}

void UPubnubClient::SubscribeToGroup(FString ChannelGroup, FPubnubOnSubscribeOperationResponse OnSubscribeToGroupResponse, FPubnubSubscribeSettings SubscribeSettings)
{
	FPubnubOnSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnSubscribeToGroupResponse](FPubnubOperationResult Result)
	{
		OnSubscribeToGroupResponse.ExecuteIfBound(Result);
	});

	SubscribeToGroup(ChannelGroup, NativeCallback, SubscribeSettings);
}

void UPubnubClient::SubscribeToGroup(FString ChannelGroup, FPubnubOnSubscribeOperationResponseNative NativeCallback, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(NativeCallback);
	
	PubnubCallsThread->AddFunctionToQueue( [this, ChannelGroup, NativeCallback, SubscribeSettings]
	{
		SubscribeToGroup_priv(ChannelGroup, NativeCallback, SubscribeSettings);
	});
}

void UPubnubClient::SubscribeToGroup(FString ChannelGroup, FPubnubSubscribeSettings SubscribeSettings)
{
	PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED();
	
	SubscribeToGroup(ChannelGroup, nullptr, SubscribeSettings);
}

void UPubnubClient::UnsubscribeFromChannel(FString Channel, FPubnubOnSubscribeOperationResponse OnUnsubscribeFromChannelResponse)
{
	FPubnubOnSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeFromChannelResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromChannelResponse.ExecuteIfBound(Result);
	});

	UnsubscribeFromChannel(Channel, NativeCallback);
}

void UPubnubClient::UnsubscribeFromChannel(FString Channel, FPubnubOnSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(NativeCallback);
	
	PubnubCallsThread->AddFunctionToQueue( [this, Channel, NativeCallback]
	{
		UnsubscribeFromChannel_priv(Channel, NativeCallback);
	});
}

void UPubnubClient::UnsubscribeFromGroup(FString ChannelGroup, FPubnubOnSubscribeOperationResponse OnUnsubscribeFromGroupResponse)
{
	FPubnubOnSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeFromGroupResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromGroupResponse.ExecuteIfBound(Result);
	});

	UnsubscribeFromGroup(ChannelGroup, NativeCallback);
}

void UPubnubClient::UnsubscribeFromGroup(FString ChannelGroup, FPubnubOnSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(NativeCallback);
	
	PubnubCallsThread->AddFunctionToQueue( [this, ChannelGroup, NativeCallback]
	{
		UnsubscribeFromGroup_priv(ChannelGroup, NativeCallback);
	});
}

void UPubnubClient::UnsubscribeFromAll(FPubnubOnSubscribeOperationResponse OnUnsubscribeFromAllResponse)
{
	FPubnubOnSubscribeOperationResponseNative NativeCallback;
	NativeCallback.BindLambda([OnUnsubscribeFromAllResponse](FPubnubOperationResult Result)
	{
		OnUnsubscribeFromAllResponse.ExecuteIfBound(Result);
	});

	UnsubscribeFromAll(NativeCallback);
}

void UPubnubClient::UnsubscribeFromAll(FPubnubOnSubscribeOperationResponseNative NativeCallback)
{
	PUBNUB_ENSURE_CLIENT_INITIALIZED(NativeCallback);

	//Save this delegate, so it can be called when Subscription Status is changed
	SubscriptionResultDelegates.Add(NativeCallback);
	
	PubnubCallsThread->AddFunctionToQueue( [this, NativeCallback]
	{
		UnsubscribeFromAll_priv(NativeCallback);
	});
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
		delete PubnubCallsThread;
		PubnubCallsThread = nullptr;
	}
	
	//Unsubscribe from all channels and groups so this user will not be visible for others anymore
	UnsubscribeFromAll_priv();
	
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

	SubscriptionResultDelegates.Empty();

	//Notify that Deinitialization is finished
	OnClientDeinitialized.Broadcast(ClientID);
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

	//Unlock the thread to proceed with queue
	PubnubCallsThread->UnlockAfterSubscriptionOperationFinished();
	
	//Don't waste resources to translate data if there is no delegate bound to it
	if(!OnSubscriptionStatusChanged.IsBound() && !OnSubscriptionStatusChangedNative.IsBound())
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
	OnSubscriptionStatusChanged.Broadcast((EPubnubSubscriptionStatus)status, SubscriptionStatusData);
	OnSubscriptionStatusChangedNative.Broadcast((EPubnubSubscriptionStatus)status, SubscriptionStatusData);
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


void UPubnubClient::PublishMessage_priv(FString Channel, FString Message, FPubnubOnPublishMessageResponseNative OnPublishMessageResponse, FPubnubPublishSettings PublishSettings)
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

void UPubnubClient::Signal_priv(FString Channel, FString Message, FPubnubOnSignalResponseNative OnSignalResponse, FPubnubSignalSettings SignalSettings)
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


void UPubnubClient::SubscribeToChannel_priv(FString Channel, FPubnubOnSubscribeOperationResponseNative OnSubscribeToChannelResponse, FPubnubSubscribeSettings SubscribeSettings)
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
				ThisClientWeak.Get()->OnMessageReceived.Broadcast(MessageData);
				ThisClientWeak.Get()->OnMessageReceivedNative.Broadcast(MessageData);
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


void UPubnubClient::SubscribeToGroup_priv(FString ChannelGroup, FPubnubOnSubscribeOperationResponseNative OnSubscribeToGroupResponse, FPubnubSubscribeSettings SubscribeSettings)
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
				ThisClientWeak.Get()->OnMessageReceived.Broadcast(MessageData);
				ThisClientWeak.Get()->OnMessageReceivedNative.Broadcast(MessageData);
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

void UPubnubClient::UnsubscribeFromChannel_priv(FString Channel, FPubnubOnSubscribeOperationResponseNative OnUnsubscribeFromChannelResponse)
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

void UPubnubClient::UnsubscribeFromGroup_priv(FString ChannelGroup, FPubnubOnSubscribeOperationResponseNative OnUnsubscribeFromGroupResponse)
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

void UPubnubClient::UnsubscribeFromAll_priv(FPubnubOnSubscribeOperationResponseNative OnUnsubscribeFromAllResponse)
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

    //Clean up all channel subscriptions
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
