// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include <pubnub_memory_block.h>
#include "CoreMinimal.h"
#include "PubnubStructLibrary.h"
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
	static FString PubnubCharMemBlockToString(const pubnub_char_mem_block PnChar);
	static FString MembershipIncludeToString(const FPubnubMembershipInclude& MembershipInclude);
	static FString MemberIncludeToString(const FPubnubMemberInclude& MemberInclude);
};
