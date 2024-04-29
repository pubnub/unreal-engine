// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PubnubChatChannel.generated.h"

class UPubnubChatSystem;
class UPubnubSubsystem;
class UPubnubChatUser;

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

	//Only listen for chat messages, don't join as a user
	UFUNCTION(BlueprintCallable, Category = "Channel")
	void Connect();

	UFUNCTION(BlueprintCallable, Category = "Channel")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "Channel")
	void Join(FString AdditionalParams);

	UFUNCTION(BlueprintCallable, Category = "Channel")
	void Leave();

	UFUNCTION(BlueprintCallable, Category = "Channel")
	void Delete();

	

private:
	bool IsInitialized = false;
	UPubnubChatSystem* ChatSystem = nullptr;
	UPubnubSubsystem* PubnubSubsystem = nullptr;
};
