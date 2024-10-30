// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PubnubStructLibrary.h"
#include "PubnubJsonUtilities.generated.h"

class FJsonObject;

/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubJsonUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	static FString JsonObjectToString(TSharedPtr<FJsonObject> JsonObject);
	//Convert FString to JsonObject. Returns true if conversion was successful
	static bool StringToJsonObject(FString JsonString, TSharedPtr<FJsonObject> &JsonObject);

	/* JSON CONVERTERS*/

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void ListChannelsFromGroupJsonToData(FString ResponseJson, bool &Error, int &Status, TArray<FString>& Channels);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void ListUserSubscribedChannelsJsonToData(FString ResponseJson, int &Status, FString &Message, TArray<FString>& Channels);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void ListUsersFromChannelJsonToData(FString ResponseJson, int &Status, FString &Message, FPubnubListUsersFromChannelWrapper &Data);
};
