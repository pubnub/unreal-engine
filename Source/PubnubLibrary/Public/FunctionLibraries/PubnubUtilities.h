// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PubnubUtilities.generated.h"

class FJsonObject;

/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	static FString JsonObjectToString(TSharedPtr<FJsonObject> JsonObject);
	//Convert FString to JsonObject. Returns true if conversion was successful
	static bool StringToJsonObject(FString JsonString, TSharedPtr<FJsonObject> &JsonObject);
};
