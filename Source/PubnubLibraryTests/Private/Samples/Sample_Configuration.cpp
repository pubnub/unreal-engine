// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_Configuration.h"
// snippet.includes
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

// snippet.end

/**
 * NOTE: Each sample is designed to be fully self-contained and portable. 
 * You can copy-paste any individual sample into a new project, and it should compile and run without errors 
 * — as long as you also include the necessary `#include` statements.
 *
 * To ensure independence, each sample retrieves the PubnubSubsystem and explicitly calls `SetUserID()` 
 * before performing any PubNub operations.
 *
 * In a real project, however, you only need to call `SetUserID()` once — typically during initialization 
 * (e.g., in GameInstance or at login) before making your first PubNub request.
 * 
 * The samples assume that in Pubnub SDK settings sections in ProjectSettings following fields are set:
 * PublishKey and SubscribeKey have correct keys, InitializeAutomatically is true.
 */

// NOTE: Comments marked with `ACTION REQUIRED` indicate lines you must change.


//Internal function, don't copy it with the samples
void ASample_Configuration::RunSamples()
{
	Super::RunSamples();

	MessageListenerSample();
	MessageListenerLambdaSample();
	ErrorListenerSample();
	ErrorListenerLambdaSample();
	SubscriptionStatusListenerSample();
	SubscriptionStatusListenerLambdaSample();
	ListUsersFromChannelLambdaSample();
	
}
//Internal function, don't copy it with the samples
ASample_Configuration::ASample_Configuration()
{
	SamplesName = "Configuration";
}


/* SAMPLE FUNCTIONS */

// snippet.publish_simple
// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::MessageListenerSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Bind delegate to the messages listener
	// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
	PubnubSubsystem->OnMessageReceived.AddDynamic(this, &ASample_Configuration::OnPubnubMessageReceived);
}

// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::OnPubnubMessageReceived(FPubnubMessageData MessageData)
{
	//Do something with the message, for example print it
	UE_LOG(LogTemp, Log, TEXT("Message received on channel: %s. Message: %s"), *MessageData.Channel, *MessageData.Message);
}

// snippet.message_listener_lambda
// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::MessageListenerLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Bind lambda delegate to the messages listener
	PubnubSubsystem->OnMessageReceivedNative.AddLambda([](const FPubnubMessageData& MessageData)
	{
		//Do something with the message, for example print it
		UE_LOG(LogTemp, Log, TEXT("Message received on channel: %s. Message: %s"), *MessageData.Channel, *MessageData.Message);
	});
}

// snippet.message_listener
// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::ErrorListenerSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Bind delegate to the messages listener
	// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
	PubnubSubsystem->OnPubnubError.AddDynamic(this, &ASample_Configuration::OnPubnubErrorReceived);
}

// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::OnPubnubErrorReceived(FString ErrorMessage, EPubnubErrorType ErrorType)
{
	//Do something with the error, for example print it
	UE_LOG(LogTemp, Warning, TEXT("Pubnub error: %s"), *ErrorMessage);
}

// snippet.error_listener_lambda
// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::ErrorListenerLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Bind lambda delegate to the messages listener
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		//Do something with the error, for example print it
		UE_LOG(LogTemp, Warning, TEXT("Pubnub error: %s"), *ErrorMessage);
	});
}

// snippet.subscription_status_listener
// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::SubscriptionStatusListenerSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Bind delegate to the subscription status listener
	// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
	PubnubSubsystem->OnSubscriptionStatusChanged.AddDynamic(this, &ASample_Configuration::OnSubscriptionStatusChanged);
}

// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::OnSubscriptionStatusChanged(EPubnubSubscriptionStatus Status, FPubnubSubscriptionStatusData StatusData)
{
	//Do something with changed status, for example print whenever there is connection error
	if(Status == EPubnubSubscriptionStatus::PSS_ConnectionError || Status == EPubnubSubscriptionStatus::PSS_DisconnectedUnexpectedly)
	{
		UE_LOG(LogTemp, Warning, TEXT("Something went wrong with subscription. Reason: %s"), *StatusData.Reason);
	}
}

// snippet.subscription_status_listener
// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::SubscriptionStatusListenerLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Bind lambda delegate to the messages listener
	PubnubSubsystem->OnSubscriptionStatusChangedNative.AddLambda([](EPubnubSubscriptionStatus Status, FPubnubSubscriptionStatusData StatusData)
	{
		//Do something with changed status, for example print whenever there is connection error
		if(Status == EPubnubSubscriptionStatus::PSS_ConnectionError || Status == EPubnubSubscriptionStatus::PSS_DisconnectedUnexpectedly)
		{
			UE_LOG(LogTemp, Warning, TEXT("Something went wrong with subscription. Reason: %s"), *StatusData.Reason);
		}
	});
}

// snippet.callback_function
// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::ListUsersFromChannelSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
	FOnListUsersFromChannelResponse OnListUsersFromChannelResponse;
	OnListUsersFromChannelResponse.BindDynamic(this, &ASample_Configuration::OnListUsersFromChannelResponse);

	//List users from a channel
	FString Channel = TEXT("guild-channel");
	PubnubSubsystem->ListUsersFromChannel(Channel, OnListUsersFromChannelResponse);
}

// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::OnListUsersFromChannelResponse(FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper Data)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to list users from channel. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Users successfully listed from channel. Occupancy: %d"), Data.Occupancy);
	}
}

// snippet.callback_function_lambda
// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::ListUsersFromChannelLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	// Bind lambda to response delegate
	FOnListUsersFromChannelResponseNative OnListUsersFromChannelResponse;
	OnListUsersFromChannelResponse.BindLambda([](const FPubnubOperationResult& Result, const FPubnubListUsersFromChannelWrapper& Data)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to list users from channel. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Users successfully listed from channel. Occupancy: %d"), Data.Occupancy);
		}
	});
	
	//List users from a channel
	FString Channel = TEXT("guild-channel");
	PubnubSubsystem->ListUsersFromChannel(Channel, OnListUsersFromChannelResponse);
}

// snippet.init_with_config
// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::InitWithConfigSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// InitPubnubWithConfig will only work if in ProjectSettings in Pubnub SDK section "InitializeAutomatically" is disabled
	FPubnubConfig PubnubConfig;
	PubnubConfig.PublishKey = "demo";
	PubnubConfig.SubscribeKey = "demo";
	PubnubConfig.UserID = "Player_001";
	PubnubSubsystem->InitPubnubWithConfig(PubnubConfig);
}

// snippet.end
