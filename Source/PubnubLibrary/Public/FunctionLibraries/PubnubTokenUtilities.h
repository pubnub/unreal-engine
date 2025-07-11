// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PubnubTokenUtilities.generated.h"

struct FPubnubGrantTokenPermissions;
struct FChannelGrant;
struct FChannelGroupGrant;
struct FUserGrant;


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
	
private:
	static void AddChannelPermissionsToJson(TArray<FChannelGrant> Channels, TSharedPtr<FJsonObject> JsonObject);
	static void AddChannelGroupPermissionsToJson(TArray<FChannelGroupGrant> ChannelGroups, TSharedPtr<FJsonObject> JsonObject);
	static void AddUserPermissionsToJson(TArray<FUserGrant> ChannelGroups, TSharedPtr<FJsonObject> JsonObject);

	static TSharedPtr<FJsonObject> ConvertChannelPermissionsFromBitmask(const TSharedPtr<FJsonObject>& SourceObject);
	static TSharedPtr<FJsonObject> ConvertChannelGroupPermissionsFromBitmask(const TSharedPtr<FJsonObject>& SourceObject);
	static TSharedPtr<FJsonObject> ConvertUserPermissionsFromBitmask(const TSharedPtr<FJsonObject>& SourceObject);
	static TSharedPtr<FJsonObject> CreateChannelPermissionsObject(int Bitmask);
	static TSharedPtr<FJsonObject> CreateChannelGroupPermissionsObject(int Bitmask);
	static TSharedPtr<FJsonObject> CreateUserPermissionsObject(int Bitmask);

};
