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

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void FetchHistoryJsonToData(FString ResponseJson, bool &Error, int &Status, FString &ErrorMessage, TArray<FPubnubMessageData> &Messages);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void FOnGetAllUUIDMetadataJsonToData(FString ResponseJson, int &Status, TArray<FPubnubUserData> &UsersData, FString &PageNext, FString &PagePrev);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void FOnGetUUIDMetadataJsonToData(FString ResponseJson, int &Status, FPubnubUserData &UserData);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void FOnGetAllChannelMetadataJsonToData(FString ResponseJson, int &Status, TArray<FPubnubChannelData> &ChannelsData, FString &PageNext, FString &PagePrev);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void FOnGetChannelMetadataJsonToData(FString ResponseJson, int &Status, FPubnubChannelData &ChannelData);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void FOnGetMessageActionsJsonToData(FString ResponseJson, int &Status, TArray<FPubnubMessageActionData> &MessageActions);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FPubnubUserData GetUserDataFromJson(FString ResponseJson);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FPubnubChannelData GetChannelDataFromJson(FString ResponseJson);
};
