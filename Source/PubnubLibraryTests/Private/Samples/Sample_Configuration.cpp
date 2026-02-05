// Copyright 2025 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_Configuration.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "PubnubSubsystem.h"

/**
 * NOTE: Each sample is designed to be fully self-contained and portable. 
 * You can copy-paste any individual sample into a new project, and it should compile and run without errors 
 * — as long as you also include the necessary `#include` statements.
 *
 * The samples assume that in Pubnub SDK settings sections in ProjectSettings following fields are set:
 * PublishKey and SubscribeKey have correct keys, InitializeAutomatically is true.
 */

// NOTE: Comments marked with `ACTION REQUIRED` indicate lines you must change/adjust.


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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Bind delegate to the messages listener
	// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
	PubnubClient->OnMessageReceived.AddDynamic(this, &ASample_Configuration::OnPubnubMessageReceived);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Bind lambda delegate to the messages listener
	PubnubClient->OnMessageReceivedNative.AddLambda([](const FPubnubMessageData& MessageData)
	{
		//Do something with the message, for example print it
		UE_LOG(LogTemp, Log, TEXT("Message received on channel: %s. Message: %s"), *MessageData.Channel, *MessageData.Message);
	});
}

// snippet.error_listener
// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::ErrorListenerSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Bind delegate to the error listener
	// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
	PubnubClient->OnPubnubError.AddDynamic(this, &ASample_Configuration::OnPubnubErrorReceived);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Bind lambda delegate to the error listener
	PubnubClient->OnPubnubErrorNative.AddLambda([](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		//Do something with the error, for example print it
		UE_LOG(LogTemp, Warning, TEXT("Pubnub error: %s"), *ErrorMessage);
	});
}

// snippet.subscription_status_listener
// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::SubscriptionStatusListenerSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Bind delegate to the subscription status listener
	// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
	PubnubClient->OnSubscriptionStatusChanged.AddDynamic(this, &ASample_Configuration::OnSubscriptionStatusChanged);
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

// snippet.subscription_status_listener_lambda
// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::SubscriptionStatusListenerLambdaSample()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	//Bind lambda delegate to the subscription status listener
	PubnubClient->OnSubscriptionStatusChangedNative.AddLambda([](EPubnubSubscriptionStatus Status, FPubnubSubscriptionStatusData StatusData)
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
	FOnPubnubListUsersFromChannelResponse OnListUsersFromChannelResponse;
	OnListUsersFromChannelResponse.BindDynamic(this, &ASample_Configuration::OnListUsersFromChannelResponse);

	//List users from a channel
	FString Channel = TEXT("guild-channel");
	PubnubClient->ListUsersFromChannelAsync(Channel, OnListUsersFromChannelResponse);
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
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	//Assumes PubnubClient is created and UserID is set

	// Bind lambda to response delegate
	FOnPubnubListUsersFromChannelResponseNative OnListUsersFromChannelResponse;
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
	PubnubClient->ListUsersFromChannelAsync(Channel, OnListUsersFromChannelResponse);
}

// snippet.create_pubnub_client
// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::CreatePubnubClient()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	// Create Pubnub Client using Pubnub Subsystem
	FPubnubConfig PubnubConfig;
	PubnubConfig.PublishKey = TEXT("demo");   //replace with your Publish Key from Admin Portal
	PubnubConfig.SubscribeKey = TEXT("demo"); //replace with your Subscribe Key from Admin Portal
	PubnubConfig.UserID = TEXT("Player_001");
	UPubnubClient* PubnubClient = PubnubSubsystem->CreatePubnubClient(PubnubConfig);
	
	UE_LOG(LogTemp, Log, TEXT("Pubnub Client created with config"));
}

// snippet.get_pubnub_client
// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
UPubnubClient* ASample_Configuration::GetPubnubClient()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	
	//Get default PubnubClient - created automatically if PluginSettings are set to do so
	UPubnubClient* PubnubClient = PubnubSubsystem->GetPubnubClient(0);
	
	PubnubClient->SetUserID(TEXT("player_001"));
	return PubnubClient;
}

// snippet.destroy_pubnub_client
// ACTION REQUIRED: Replace ASample_Configuration with name of your Actor class
void ASample_Configuration::DestroyPubnubClient()
{
	// snippet.hide
	UPubnubClient* PubnubClient = GetPubnubClient();
	// snippet.show
	
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();
	
	PubnubSubsystem->DestroyPubnubClient(PubnubClient);
}

// snippet.end