// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PubnubStructLibrary.h"
#include "PubnubJsonUtilities.generated.h"

class FJsonObject;
class FJsonValue;

/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubJsonUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	static FString JsonObjectToString(TSharedPtr<FJsonObject> JsonObject);
	static FString JsonArrayToString(TArray<TSharedPtr<FJsonValue>> JsonArray);
	//Convert FString to JsonObject. Returns true if conversion was successful
	static bool StringToJsonObject(FString JsonString, TSharedPtr<FJsonObject> &JsonObject);

	//Convert FString to JsonArray. Returns true if conversion was successful
	static bool StringToJsonArray(FString JsonString, TArray<TSharedPtr<FJsonValue>>& OutArray);

	//Converts literal string into serialized form adding all needed escapes
	static FString SerializeString(const FString& InString);
	//Converts serialized string into it's normal, literal form
	static FString DeserializeString(const FString InString);

	/**
	 * Checks if gives string can be converted to a json
	 * @param InString - String to check
	 * @param AllowSimpleTypes - If json root can be of type string, number or int
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static bool IsCorrectJsonString(const FString InString, bool AllowSimpleTypes = true);

	/**
	 * When comparing 2 Json Strings, sometimes they may have the same fields, but in different order
	 * or with different syntax (no spaces, tabs, etc), so comparing strings may give false results.
	 * This function compares if given 2 Json Strings have exactly the same fields with the same values.
	 * Returns false if any Json conversion failed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static bool AreJsonObjectStringsEqual(const FString JsonString1, const FString JsonString2);

	/* JSON CONVERTERS*/
	
	/**
	 * Converter from ListChannelsFromGroup_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void ListChannelsFromGroupJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FString>& Channels);

	/**
	 * Converter from ListUserSubscribedChannels_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void ListUserSubscribedChannelsJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FString>& Channels);

	/**
	 * Converter from ListUsersFromChannel_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void ListUsersFromChannelJsonToData(FString ResponseJson, FPubnubOperationResult& Result, FPubnubListUsersFromChannelWrapper &Data);

	/**
	 * Converter from FetchHistory_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void FetchHistoryJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubHistoryMessageData> &Messages);
	
	/**
	 * Converter from GetAllUserMetadata_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void GetAllUserMetadataJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubUserData> &UsersData, FString &PageNext, FString &PagePrev);

	/**
	 * Converter from GetUserMetadata_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void GetUserMetadataJsonToData(FString ResponseJson, FPubnubOperationResult& Result, FPubnubUserData &UserData);

	/**
	 * Converter from GetAllChannelMetadata_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void GetAllChannelMetadataJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubChannelData> &ChannelsData, FString &PageNext, FString &PagePrev);

	/**
	 * Converter from GetChannelMetadata_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void GetChannelMetadataJsonToData(FString ResponseJson, FPubnubOperationResult& Result, FPubnubChannelData &ChannelData);

	/**
	 * Converter from GetMessageActions_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void GetMessageActionsJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubMessageActionData> &MessageActions);

	/**
	 * Converter from AddMessageActions_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void AddMessageActionJsonToData(FString ResponseJson, FPubnubOperationResult& Result, FPubnubMessageActionData& MessageAction);

	/**
	 * Converter from GetMemberships_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void GetMembershipsJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubMembershipData> &MembershipsData, FString &PageNext, FString &PagePrev);

	/**
	 * Converter from GetChannelMembers_Json response to actual types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static void GetChannelMembersJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubChannelMemberData> &MembershipsData, FString &PageNext, FString &PagePrev);

	/**
	 * Converter from Json string containing User data to FPubnubUserData
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FPubnubUserData GetUserDataFromJson(FString ResponseJson);

	/**
	 * Converter from FPubnubUserData to Json string containing User data
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FString GetJsonFromUserData(const FPubnubUserData& UserData);

	/**
	 * Converter from Json string containing Channel data to FPubnubChannelData
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FPubnubChannelData GetChannelDataFromJson(FString ResponseJson);

	/**
	 * Converter from FPubnubChannelData to Json string containing Channel data
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FString GetJsonFromChannelData(const FPubnubChannelData& ChannelData);

	/**
	 * Converter from Json string containing Membership data to FPubnubMembershipData
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FPubnubMembershipData GetMembershipDataFromJson(FString ResponseJson);
	static FPubnubMembershipData GetMembershipDataFromJson(TSharedPtr<FJsonObject> JsonObject);

	/**
	 * Converter from FPubnubMembershipData to Json string containing Membership data
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FString GetJsonFromMembershipData(const FPubnubMembershipData& MembershipData);

	/**
	 * Converter from Json string containing Memberships data to FPubnubMembershipData Array
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static TArray<FPubnubMembershipData> GetMembershipsDataArrayFromJson(FString ResponseJson);
	static TArray<FPubnubMembershipData> GetMembershipsDataArrayFromJson(TSharedPtr<FJsonObject> JsonObject);

	/**
	 * Converter from FPubnubMembershipData Array to Json string containing Memberships data
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FString GetJsonFromMembershipsDataArray(const TArray<FPubnubMembershipData>& MembershipsData);

	/**
	 * Converter from Json string containing Channel Member data to FPubnubChannelMemberData
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FPubnubChannelMemberData GetChannelMemberDataFromJson(FString ResponseJson);
	static FPubnubChannelMemberData GetChannelMemberDataFromJson(TSharedPtr<FJsonObject> JsonObject);

	/**
	 * Converter from FPubnubChannelMemberData to Json string containing Channel Member data
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FString GetJsonFromChannelMemberData(const FPubnubChannelMemberData& ChannelMemberData);

	/**
	 * Converter from Json string containing Channel Members data to FPubnubChannelMemberData Array
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static TArray<FPubnubChannelMemberData> GetChannelMembersDataArrayFromJson(FString ResponseJson);
	static TArray<FPubnubChannelMemberData> GetChannelMembersDataArrayFromJson(TSharedPtr<FJsonObject> JsonObject);

	/**
	 * Converter from FPubnubChannelMemberData Array to Json string containing Channel Members data
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FString GetJsonFromChannelMembersDataArray(const TArray<FPubnubChannelMemberData>& ChannelMembersData);

	/**
	 * Convert list of Memberships (Channel IDs) to RemoveObject accepted by RemoveMemberships function
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FString GetJsonFromMembershipsToRemove(TArray<FString> Memberships);

	/**
	 * Convert list of ChannelMembers (User IDs) to RemoveObject accepted by RemoveChannelMembers function
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FString GetJsonFromChannelMembersToRemove(TArray<FString> ChannelMembers);
	
	/**
	 * Get Operation result from Json Object
	 */
	static FPubnubOperationResult GetOperationResultFromJson(TSharedPtr<FJsonObject> JsonObject);
	static FPubnubOperationResult GetOperationResultFromJson(FString ResponseJson);
	

	/**
	 * Get error message from App Context operation result Json Object. Works also for Access Manager operations.
	 */
	static FPubnubOperationResult GetOperationResultFromJson_AppContext(TSharedPtr<FJsonObject> JsonObject);
	static FPubnubOperationResult GetOperationResultFromJson_AppContext(FString ResponseJson);

};


