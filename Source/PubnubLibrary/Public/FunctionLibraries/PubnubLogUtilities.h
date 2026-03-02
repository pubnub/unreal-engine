#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PubnubLogUtilities.generated.h"

UCLASS()
class PUBNUBLIBRARY_API UPubnubLogUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Formats UTC time into ISO-8601 with milliseconds, e.g. 2026-02-25T12:18:11.797Z */
	UFUNCTION(BlueprintPure, Category = "Pubnub|Logger")
	static FString FormatIso8601UtcWithMilliseconds(const FDateTime& InUtcTime);
};
