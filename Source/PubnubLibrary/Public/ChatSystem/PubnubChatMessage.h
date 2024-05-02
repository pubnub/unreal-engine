// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PubnubChatMessage.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PUBNUBLIBRARY_API UPubnubChatMessage : public UObject
{
	GENERATED_BODY()

public:

	void InitializeFromJson(FString MessageJson);
	
	//Timetoken is also like an id for the message
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Message")
	FString Timetoken;

	//TODO: maybe change this to struct...
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Message")
	FString Content;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Message")
	FString ChannelID;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Message")
	FString UserID;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Message")
	FString Metadata;
	
	//MessageActions

private:
	bool IsInitialized = false;
};
