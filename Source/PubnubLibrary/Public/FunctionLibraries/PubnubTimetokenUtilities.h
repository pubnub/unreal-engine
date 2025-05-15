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

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub | TimetokenUtilities")
	static FString GetCurrentUnixTimetoken();

};
