// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PubnubCryptoUtilities.generated.h"


/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubCryptoUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	static bool Base64Decode(const FString& In, TArray<uint8>& Out);
	static bool Base64Decode(const FString& B64, uint8*& OutData, size_t& OutSize);
	static FString Base64Encode(const TArray<uint8>& In);
	static FString Base64Encode(const uint8* Data, size_t Size);
	static FString ConvertBytesToString(const uint8* Data, size_t Size);
	static FString ConvertBytesToBase64(const uint8* Data, size_t Size);
	static bool ConvertBase64ToBytes(const FString& Base64String, uint8*& OutData, size_t& OutSize);
	static bool ConvertStringToBytes(const FString& String, uint8*& OutData, size_t& OutSize);

};
