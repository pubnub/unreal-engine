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

	/**
	 * Checks if gives string can be converted to a json
	 * @param InString - String to check
	 * @param AllowSimpleTypes - If json root can be of type string, number or int
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static bool IsCorrectJsonString(const FString InString, bool AllowSimpleTypes = true);

	/* JSON CONVERTERS*/
	
	/**
	 * Converter from ListChannelsFromGroup_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void ListChannelsFromGroupJsonToData(FString ResponseJson, bool &Error, int &Status, TArray<FString>& Channels);

	/**
	 * Converter from ListUserSubscribedChannels_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void ListUserSubscribedChannelsJsonToData(FString ResponseJson, int &Status, FString &Message, TArray<FString>& Channels);

	/**
	 * Converter from ListUsersFromChannel_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void ListUsersFromChannelJsonToData(FString ResponseJson, int &Status, FString &Message, FPubnubListUsersFromChannelWrapper &Data);

	/**
	 * Converter from FetchHistory_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void FetchHistoryJsonToData(FString ResponseJson, bool &Error, int &Status, FString &ErrorMessage, TArray<FPubnubHistoryMessageData> &Messages);

	/**
	 * Converter from GetAllUserMetadata_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void GetAllUserMetadataJsonToData(FString ResponseJson, int &Status, TArray<FPubnubUserData> &UsersData, FString &PageNext, FString &PagePrev);

	/**
	 * Converter from GetUserMetadata_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void GetUserMetadataJsonToData(FString ResponseJson, int &Status, FPubnubUserData &UserData);

	/**
	 * Converter from GetAllChannelMetadata_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void GetAllChannelMetadataJsonToData(FString ResponseJson, int &Status, TArray<FPubnubChannelData> &ChannelsData, FString &PageNext, FString &PagePrev);

	/**
	 * Converter from GetChannelMetadata_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void GetChannelMetadataJsonToData(FString ResponseJson, int &Status, FPubnubChannelData &ChannelData);

	/**
	 * Converter from GetMessageActions_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void GetMessageActionsJsonToData(FString ResponseJson, int &Status, TArray<FPubnubMessageActionData> &MessageActions);

	/**
	 * Converter from GetMemberships_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void GetMembershipsJsonToData(FString ResponseJson, int &Status, TArray<FPubnubGetMembershipsWrapper> &MembershipsData, FString &PageNext, FString &PagePrev);

	/**
	 * Converter from GetChannelMembers_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void GetChannelMembersJsonToData(FString ResponseJson, int &Status, TArray<FPubnubGetChannelMembersWrapper> &MembershipsData, FString &PageNext, FString &PagePrev);

	/**
	 * Converter from Json string containing User data to FPubnubUserData
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FPubnubUserData GetUserDataFromJson(FString ResponseJson);

	/**
	 * Converter from Json string containing Channel data to FPubnubChannelData
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FPubnubChannelData GetChannelDataFromJson(FString ResponseJson);
};
