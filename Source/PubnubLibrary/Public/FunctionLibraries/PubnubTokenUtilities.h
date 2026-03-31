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
	
private:
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
