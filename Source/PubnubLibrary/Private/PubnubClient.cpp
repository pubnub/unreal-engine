// Fill out your copyright notice in the Description page of Project Settings.


#include "PubnubClient.h"
#include "PubnubSubsystem.h"

void UPubnubClient::DestroyClient()
{
	if(!PubnubSubsystem)
	{return;}

	PubnubSubsystem->DestroyPubnubClient(this);
}

void UPubnubClient::InitWithConfig(UPubnubSubsystem* InPubnubSubsystem, FPubnubConfig InConfig, int InClientID, FString InDebugName )
{
	PubnubSubsystem = InPubnubSubsystem;
	ClientID = InClientID;
	DebugName = InDebugName;

	IsInitialized = true;
}

void UPubnubClient::DeinitializeClient()
{
	IsInitialized = false;
	PubnubSubsystem = nullptr;
}
