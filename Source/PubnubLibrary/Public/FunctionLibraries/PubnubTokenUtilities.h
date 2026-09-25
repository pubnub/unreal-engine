// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PubnubTokenUtilities.generated.h"

class FJsonObject;
class FJsonValue;

struct FPubnubGrantTokenPermissions;
struct FChannelGrant;
struct FChannelGroupGrant;
struct FUserGrant;
struct FPubnubChannelPermissions;
struct FPubnubChannelGroupPermissions;
struct FPubnubUserPermissions;

struct pubnub_grant_token_opts;
typedef struct pubnub_grant_token_opts pubnub_grant_token_opts_t;
struct pubnub_access_resource_permission;
typedef struct pubnub_access_resource_permission pubnub_access_resource_permission_t;
struct pubnub_context;
typedef struct pubnub_context pubnub_context_t;
struct pubnub_parsed_token;
typedef struct pubnub_parsed_token pubnub_parsed_token_t;
struct pubnub_parsed_token_resource;
typedef struct pubnub_parsed_token_resource pubnub_parsed_token_resource_t;

enum class EParsedTokenResourceKind : uint8
{
	Channel,
	Group,
	Uuid,
	ChannelPattern,
	GroupPattern,
	UuidPattern
};

/** One resource list (channels, groups, or users) prepared for pubnub_grant_token. Defined in the utilities cpp. */
struct FGrantResourcePermissions;

/**
 * All six grant-token resource lists. Name strings stay alive for as long as this object does.
 * Defined in the utilities cpp so the reflected header does not include the C-Core access types.
 */
struct FPubnubGrantTokenResourceStorage;

struct FPubnubGrantTokenResourceStorageDeleter
{
	void operator()(FPubnubGrantTokenResourceStorage* Storage) const;
};

/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubTokenUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, Category="Pubnub|Token Utilities")
	static FString CreateGrantTokenPermissionObjectString(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FString Meta = "");

	UFUNCTION(BlueprintCallable, Category="Pubnub|Token Utilities")
	static FString ReworkParsedToken(const FString& ParsedToken);

	// Helper function to calculate expected bitmask for Channel permissions
	UFUNCTION(BlueprintCallable, Category="Pubnub|Token Utilities")
	static int CalculateChannelPermissionsBitmask(const FPubnubChannelPermissions& Perms);

	// Helper function to calculate expected bitmask for Channel Group permissions
	UFUNCTION(BlueprintCallable, Category="Pubnub|Token Utilities")
	static int CalculateChannelGroupPermissionsBitmask(const FPubnubChannelGroupPermissions& Perms);

	// Helper function to calculate expected bitmask for User permissions
	UFUNCTION(BlueprintCallable, Category="Pubnub|Token Utilities")
	static int CalculateUserPermissionsBitmask(const FPubnubUserPermissions& Perms);

	/**
	 * Builds the same human-readable JSON that ReworkParsedToken produces, from
	 * already-decoded resource names and permission bitmasks.
	 * Empty resource lists are omitted. An empty AuthorizedUuid is omitted.
	 */
	static FString BuildReworkedParsedToken(
		int32 Version,
		uint64 Timestamp,
		uint32 Ttl,
		const FString& AuthorizedUuid,
		const TArray<TPair<FString, int32>>& Channels,
		const TArray<TPair<FString, int32>>& ChannelGroups,
		const TArray<TPair<FString, int32>>& Users,
		const TArray<TPair<FString, int32>>& ChannelPatterns,
		const TArray<TPair<FString, int32>>& ChannelGroupPatterns,
		const TArray<TPair<FString, int32>>& UserPatterns);

	/** Builds C-Core resource lists for every grant field. The returned storage must outlive the grant call. */
	static TUniquePtr<FPubnubGrantTokenResourceStorage, FPubnubGrantTokenResourceStorageDeleter> BuildGrantTokenResourceStorage(const FPubnubGrantTokenPermissions& Permissions);

	/** Writes borrowed resource pointers from Storage into Opts. Does not set ttl, authorized_uuid, or meta. */
	static void ApplyGrantTokenResourceStorage(const FPubnubGrantTokenResourceStorage& Storage, pubnub_grant_token_opts_t& Opts);

	/**
	 * Copies resources off the context's last parse and returns the same JSON as BuildReworkedParsedToken.
	 * Resource views are only valid until the next pubnub_parse_token on this context.
	 */
	static FString BuildReworkedParsedToken(pubnub_context_t* Context, const pubnub_parsed_token_t& ParsedToken);
	
private:
	static void ParsedTokenResourceAt(pubnub_context_t* Context, EParsedTokenResourceKind Kind, size_t Index, pubnub_parsed_token_resource_t& OutResource);
	static void AppendParsedTokenResources(pubnub_context_t* Context, uint32 Count, EParsedTokenResourceKind Kind, TArray<TPair<FString, int32>>& Out);

	static void FillChannelGrantPermissions(const TArray<FChannelGrant>& Grants, FGrantResourcePermissions& Out);
	static void FillChannelGroupGrantPermissions(const TArray<FChannelGroupGrant>& Grants, FGrantResourcePermissions& Out);
	static void FillUserGrantPermissions(const TArray<FUserGrant>& Grants, FGrantResourcePermissions& Out);
	static void ApplyGrantResourceList(const FGrantResourcePermissions& List, const pubnub_access_resource_permission_t*& OutPermissions, size_t& OutCount);

	static void AddChannelPermissionsToJson(TArray<FChannelGrant> Channels, TSharedPtr<FJsonObject> JsonObject);
	static void AddChannelGroupPermissionsToJson(TArray<FChannelGroupGrant> ChannelGroups, TSharedPtr<FJsonObject> JsonObject);
	static void AddUserPermissionsToJson(TArray<FUserGrant> ChannelGroups, TSharedPtr<FJsonObject> JsonObject);

	static TSharedPtr<FJsonObject> ConvertChannelPermissionsFromBitmask(const TSharedPtr<FJsonObject>& SourceObject);
	static TSharedPtr<FJsonObject> ConvertChannelGroupPermissionsFromBitmask(const TSharedPtr<FJsonObject>& SourceObject);
	static TSharedPtr<FJsonObject> ConvertUserPermissionsFromBitmask(const TSharedPtr<FJsonObject>& SourceObject);
	static TSharedPtr<FJsonObject> CreateChannelPermissionsFromBitmask(int Bitmask);
	static TSharedPtr<FJsonObject> CreateChannelGroupPermissionsFromBitmask(int Bitmask);
	static TSharedPtr<FJsonObject> CreateUserPermissionsFromBitmask(int Bitmask);

};
