// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PubnubStructLibrary.h"
#include "PubnubSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PubnubLog, Log, All);

constexpr int PUBNUB_MAX_LIMIT = 100;

class UPubnubClient;


UCLASS()
class PUBNUBLIBRARY_API UPubnubSubsystem : public UGameInstanceSubsystem
{
	
	GENERATED_BODY()
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Client")
	UPubnubClient* CreatePubnubClient(FPubnubConfig Config, FString DebugName = "");

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Client")
	UPubnubClient* GetPubnubClient(int ClientID);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Client")
	bool DestroyPubnubClient(UPubnubClient* ClientToDestroy);
	
private:

	UPROPERTY()
	TMap<int, TObjectPtr<UPubnubClient>> PubnubClients;
	int NextClientID = 0;
};
