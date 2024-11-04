// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PubnubUtilities.generated.h"

/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	static FString AddQuotesToString(const FString InString, bool SkipIfHasQuotes = true);
};
