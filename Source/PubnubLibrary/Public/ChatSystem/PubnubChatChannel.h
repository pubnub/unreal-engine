// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PubnubChatChannel.generated.h"

class UPubnubChatSystem;

/**
 * 
 */
UCLASS(BlueprintType)
class PUBNUBLIBRARY_API UPubnubChatChannel : public UObject
{
	GENERATED_BODY()
	
public:
	
	void Initialize(UPubnubChatSystem* InChatSystem, FString InChannelID, FString InChannelData);
	
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Channel")
	FString ChannelID;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Channel")
	FString ChannelData;

	UFUNCTION(BlueprintCallable, Category = "Channel")
	void Connect();

private:
	bool IsInitialized = false;
	UPubnubChatSystem* ChatSystem = nullptr;
};
