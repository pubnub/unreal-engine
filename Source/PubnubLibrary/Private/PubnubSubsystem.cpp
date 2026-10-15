// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubClient.h"
#include "FunctionLibraries/PubnubInternalUtilities.h"

DEFINE_LOG_CATEGORY(PubnubLog)

void UPubnubSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPubnubSubsystem::Deinitialize()
{
	// Game instance shutdown does not garbage-collect clients immediately.
	// Destroy them here so a later GC pass does not pubnub_destroy a live context
	// after sibling objects (logger bridge, crypto) have already been freed.
	TArray<TObjectPtr<UPubnubClient>> Clients;
	PubnubClients.GenerateValueArray(Clients);
	for (UPubnubClient* Client : Clients)
	{
		DestroyPubnubClient(Client);
	}
	PubnubClients.Empty();

	Super::Deinitialize();
}

UPubnubClient* UPubnubSubsystem::CreatePubnubClient(FPubnubConfig Config, FString DebugName)
{
	UPubnubClient* PubnubClient = UPubnubInternalUtilities::SafeNewObject<UPubnubClient>(this);

	int NewID = NextClientID++;

	if (!PubnubClient->InitWithConfig(this, Config, NewID, DebugName))
	{
		return nullptr;
	}
	
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