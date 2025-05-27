// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PubnubTimetokenUtilities.generated.h"


/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubTimetokenUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/**
	 * Returns current Unix Timestap in form of 17-digit string, so the form required for most Pubnub operations,
	 * like MessageCounts, AddMessageAction, etc.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub | TimetokenUtilities")
	static FString GetCurrentUnixTimetoken();

};
