// Fill out your copyright notice in the Description page of Project Settings.


#include "PubnubClient.h"
#include "PubNub.h"
#include "PubnubSubsystem.h"
#include "Threads/PubnubFunctionThread.h"
#include "FunctionLibraries/PubnubUtilities.h"


struct CCoreSubscriptionCallback
{
	pubnub_subscribe_message_callback_t Callback;
	pubnub_subscription_t* Subscription;
};

//TODO:: Move this to logger
static void PubnubSDKLogConverter(enum pubnub_log_level log_level, const char* message)
{
	
}


void UPubnubClient::DestroyClient()
{
	if(!PubnubSubsystem)
	{return;}

	PubnubSubsystem->DestroyPubnubClient(this);
}

void UPubnubClient::SetUserID(FString UserID)
{
}

FString UPubnubClient::GetUserID()
{
	return "";
}

void UPubnubClient::SetSecretKey()
{
}

void UPubnubClient::PublishMessage(FString Channel, FString Message,
	FPubnubOnPublishMessageResponse OnPublishMessageResponse, FPubnubPublishSettings PublishSettings)
{
}

void UPubnubClient::PublishMessage(FString Channel, FString Message,
	FPubnubOnPublishMessageResponseNative NativeCallback, FPubnubPublishSettings PublishSettings)
{
}

void UPubnubClient::PublishMessage(FString Channel, FString Message, FPubnubPublishSettings PublishSettings)
{
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
	IsInitialized = false;
	PubnubSubsystem = nullptr;
}

void UPubnubClient::SavePubnubConfig(const FPubnubConfig& InConfig)
{
	PubnubConfig = InConfig;
	
	// Safely copy all keys using the utility function
	UPubnubUtilities::SafeCopyFStringToCharBuffer(PublishKey, PublishKeySize + 1, InConfig.PublishKey, TEXT("PublishKey"));
	UPubnubUtilities::SafeCopyFStringToCharBuffer(SubscribeKey, PublishKeySize + 1, InConfig.SubscribeKey, TEXT("SubscribeKey"));
	UPubnubUtilities::SafeCopyFStringToCharBuffer(SecretKey, SecretKeySize + 1, InConfig.SecretKey, TEXT("SecretKey"));
}

void UPubnubClient::PubnubError(FString ErrorMessage, EPubnubErrorType ErrorType)
{
}

void UPubnubClient::PubnubResponseError(pubnub_res PubnubResponse, FString ErrorMessage)
{
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
