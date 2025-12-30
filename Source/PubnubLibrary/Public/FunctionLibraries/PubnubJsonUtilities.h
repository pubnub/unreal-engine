// Copyright 2025 PubNub Inc. All Rights Reserved.

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
	
	//Adds Provided string as ObjectField to JsonObject. If AddNullFieldIfEmpty is set to true, null field will be added in case if empty JsonObjectString
	static void AddObjectFieldToJson(const FString& FieldName, const FString& JsonObjectString, TSharedPtr<FJsonObject> &JsonObject, bool AddNullFieldIfEmpty = false);
	//Adds Provided string as StringField to JsonObject. If AddNullFieldIfEmpty is set to true, null field will be added in case if empty FieldValue
	static void AddStringFieldToJson(const FString& FieldName, const FString& FieldValue, TSharedPtr<FJsonObject> &JsonObject, bool AddNullFieldIfEmpty = false);

	
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
	static void ListChannelsFromGroupJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FString>& Channels);

	/**
	 * Converter from ListUserSubscribedChannels_Json response to actual types
	 */
	static void ListUserSubscribedChannelsJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FString>& Channels);

	/**
	 * Converter from ListUsersFromChannel_Json response to actual types
	 */
	static void ListUsersFromChannelJsonToData(FString ResponseJson, FPubnubOperationResult& Result, FPubnubListUsersFromChannelWrapper &Data);

	/**
	 * Converter from FetchHistory_Json response to actual types
	 */
	static void FetchHistoryJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubHistoryMessageData> &Messages);
	
	/**
	 * Converter from GetAllUserMetadata_Json response to actual types
	 */
	static void GetAllUserMetadataJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubUserData> &UsersData, FPubnubPage &Page, int &TotalCount);

	/**
	 * Converter from GetUserMetadata_Json response to actual types
	 */
	static void GetUserMetadataJsonToData(FString ResponseJson, FPubnubOperationResult& Result, FPubnubUserData &UserData);

	/**
	 * Converter from GetAllChannelMetadata_Json response to actual types
	 */
	static void GetAllChannelMetadataJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubChannelData> &ChannelsData, FPubnubPage &Page, int &TotalCount);

	/**
	 * Converter from GetChannelMetadata_Json response to actual types
	 */
	static void GetChannelMetadataJsonToData(FString ResponseJson, FPubnubOperationResult& Result, FPubnubChannelData &ChannelData);

	/**
	 * Converter from GetMessageActions_Json response to actual types
	 */
	static void GetMessageActionsJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubMessageActionData> &MessageActions);

	/**
	 * Converter from AddMessageActions_Json response to actual types
	 */
	static void AddMessageActionJsonToData(FString ResponseJson, FPubnubOperationResult& Result, FPubnubMessageActionData& MessageAction);

	/**
	 * Converter from GetMemberships_Json response to actual types
	 */
	static void GetMembershipsJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubMembershipData> &MembershipsData, FPubnubPage &Page, int &TotalCount);

	/**
	 * Converter from GetChannelMembers_Json response to actual types
	 */
	static void GetChannelMembersJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubChannelMemberData> &MembershipsData, FPubnubPage &Page, int &TotalCount);

	/**
	 * Converter from Json string containing User data to FPubnubUserData
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FPubnubUserData GetUserDataFromJson(FString ResponseJson);

	/**
	 * Converter from FPubnubUserData to Json string containing User data.
	 * UserID is provided separately, because during Set operations ID from the struct is ignored.
	 */
	static FString GetJsonFromUserData(const FString UserID, const FPubnubUserInputData& UserData);

	/**
	 * Converter from Json string containing Channel data to FPubnubChannelData
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FPubnubChannelData GetChannelDataFromJson(FString ResponseJson);

	/**
	 * Converter from FPubnubChannelData to Json string containing Channel data.
	 * ChannelID is provided separately, because during Set operations ID from the struct is ignored.
	 */
	static FString GetJsonFromChannelData(const FString ChannelID, const FPubnubChannelInputData& ChannelData);

	/**
	 * Converter from Json string containing Membership data to FPubnubMembershipData
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FPubnubMembershipData GetMembershipDataFromJson(FString ResponseJson);
	static FPubnubMembershipData GetMembershipDataFromJson(TSharedPtr<FJsonObject> JsonObject);

	/**
	 * Converter from FPubnubMembershipData to Json string containing Membership data
	 */
	static FString GetJsonFromMembershipInputData(const FPubnubMembershipInputData& MembershipInputData);

	/**
	 * Converter from Json string containing Memberships data to FPubnubMembershipData Array
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static TArray<FPubnubMembershipData> GetMembershipsDataArrayFromJson(FString ResponseJson);
	static TArray<FPubnubMembershipData> GetMembershipsDataArrayFromJson(TSharedPtr<FJsonObject> JsonObject);

	/**
	 * Converter from FPubnubMembershipData Array to Json string containing Memberships data
	 */
	static FString GetJsonFromMembershipsDataArray(const TArray<FPubnubMembershipInputData>& MembershipsInputData);

	/**
	 * Converter from Json string containing Channel Member data to FPubnubChannelMemberData
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static FPubnubChannelMemberData GetChannelMemberDataFromJson(FString ResponseJson);
	static FPubnubChannelMemberData GetChannelMemberDataFromJson(TSharedPtr<FJsonObject> JsonObject);

	/**
	 * Converter from FPubnubChannelMemberData to Json string containing Channel Member data
	 */
	static FString GetJsonFromChannelMemberData(const FPubnubChannelMemberInputData& ChannelMemberInputData);

	/**
	 * Converter from Json string containing Channel Members data to FPubnubChannelMemberData Array
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pubnub|Json Utilities")
	static TArray<FPubnubChannelMemberData> GetChannelMembersDataArrayFromJson(FString ResponseJson);
	static TArray<FPubnubChannelMemberData> GetChannelMembersDataArrayFromJson(TSharedPtr<FJsonObject> JsonObject);

	/**
	 * Converter from FPubnubChannelMemberData Array to Json string containing Channel Members data
	 */
	static FString GetJsonFromChannelMembersDataArray(const TArray<FPubnubChannelMemberInputData>& ChannelMembersInputData);

	/**
	 * Convert list of Memberships (Channel IDs) to RemoveObject accepted by RemoveMemberships function
	 */
	static FString GetJsonFromMembershipsToRemove(TArray<FString> Memberships);

	/**
	 * Convert list of ChannelMembers (User IDs) to RemoveObject accepted by RemoveChannelMembers function
	 */
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


