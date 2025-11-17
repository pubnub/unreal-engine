// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PubnubStructLibrary.h"
#include "PubnubClient.generated.h"

class UPubnubSubsystem;

/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubClient : public UObject
{
	GENERATED_BODY()

	friend class UPubnubSubsystem;

public:
	

	UFUNCTION(BlueprintCallable, Category="PubnubClient")
	int GetClientID() {return ClientID;};

	UFUNCTION(BlueprintCallable, Category="PubnubClient")
	void DestroyClient();

private:

	void InitWithConfig(UPubnubSubsystem* InPubnubSubsystem, FPubnubConfig InConfig, int InClientID, FString InDebugName = "");
	void DeinitializeClient();

	UPROPERTY()
	TObjectPtr<UPubnubSubsystem> PubnubSubsystem = nullptr;
	int ClientID = -1;
	bool IsInitialized = false;
	FString DebugName = "";
};
