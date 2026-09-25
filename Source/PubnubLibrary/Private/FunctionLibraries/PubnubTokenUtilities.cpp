// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "FunctionLibraries/PubnubTokenUtilities.h"
#include "FunctionLibraries/PubnubInternalUtilities.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "FunctionLibraries/PubnubUtilities.h"
#include "PubnubStructLibrary.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "pubnub/features/access.h"


FString UPubnubTokenUtilities::CreateGrantTokenPermissionObjectString(int Ttl, FString AuthorizedUser, const FPubnubGrantTokenPermissions& Permissions, FString Meta)
{
	if(AuthorizedUser.IsEmpty()) {return "";}
	if(Permissions.ArePermissionsEmpty()) {return "";}

	TSharedPtr<FJsonObject> ResourcesJsonObject = MakeShareable(new FJsonObject);
	TSharedPtr<FJsonObject> PatternsJsonObject = MakeShareable(new FJsonObject);
	
	//Create Json objects with channels, groups, users permissions and their patterns
	AddChannelPermissionsToJson(Permissions.Channels, ResourcesJsonObject);
	AddChannelGroupPermissionsToJson(Permissions.ChannelGroups, ResourcesJsonObject);
	AddUserPermissionsToJson(Permissions.Users, ResourcesJsonObject);
	AddChannelPermissionsToJson(Permissions.ChannelPatterns, PatternsJsonObject);
	AddChannelGroupPermissionsToJson(Permissions.ChannelGroupPatterns, PatternsJsonObject);
	AddUserPermissionsToJson(Permissions.UserPatterns, PatternsJsonObject);

	TSharedPtr<FJsonObject> TokenStructureJsonObject = MakeShareable(new FJsonObject);
	TokenStructureJsonObject->SetObjectField(ANSI_TO_TCHAR("resources"), ResourcesJsonObject);
	TokenStructureJsonObject->SetObjectField(ANSI_TO_TCHAR("patterns"), PatternsJsonObject);
	if(UPubnubJsonUtilities::IsCorrectJsonString(Meta))
	{
		TSharedPtr<FJsonObject> MetaJsonObject = MakeShareable(new FJsonObject);
		UPubnubJsonUtilities::StringToJsonObject(Meta, MetaJsonObject);
		TokenStructureJsonObject->SetObjectField(ANSI_TO_TCHAR("meta"), MetaJsonObject);
	}


	TSharedPtr<FJsonObject> PermissionsJsonObject = MakeShareable(new FJsonObject);
	PermissionsJsonObject->SetNumberField(ANSI_TO_TCHAR("ttl"), Ttl);
	PermissionsJsonObject->SetStringField(ANSI_TO_TCHAR("authorized_uuid"), AuthorizedUser);
	PermissionsJsonObject->SetObjectField(ANSI_TO_TCHAR("permissions"), TokenStructureJsonObject);
	
	//Convert created Json object to string
	return UPubnubJsonUtilities::JsonObjectToString(PermissionsJsonObject);
}

void UPubnubTokenUtilities::AddChannelPermissionsToJson(TArray<FChannelGrant> Channels, TSharedPtr<FJsonObject> JsonObject)
{
	if(!JsonObject) {return;}
	if(Channels.IsEmpty()) {return;}

	TSharedPtr<FJsonObject> ChannelsObject = MakeShareable(new FJsonObject);
	
	for(auto Channel : Channels)
	{
		ChannelsObject->SetNumberField(Channel.Channel, CalculateChannelPermissionsBitmask(Channel.Permissions));
	}

	JsonObject->SetObjectField(ANSI_TO_TCHAR("channels"), ChannelsObject);
}

void UPubnubTokenUtilities::AddChannelGroupPermissionsToJson(TArray<FChannelGroupGrant> ChannelGroups, TSharedPtr<FJsonObject> JsonObject)
{
	if(!JsonObject) {return;}
	if(ChannelGroups.IsEmpty()) {return;}

	TSharedPtr<FJsonObject> ChannelGroupsObject = MakeShareable(new FJsonObject);
	
	for(auto ChannelGroup : ChannelGroups)
	{
		ChannelGroupsObject->SetNumberField(ChannelGroup.ChannelGroup, CalculateChannelGroupPermissionsBitmask(ChannelGroup.Permissions));
	}
	
	JsonObject->SetObjectField(ANSI_TO_TCHAR("groups"), ChannelGroupsObject);
}

void UPubnubTokenUtilities::AddUserPermissionsToJson(TArray<FUserGrant> Users, TSharedPtr<FJsonObject> JsonObject)
{
	if(!JsonObject) {return;}
	if(Users.IsEmpty()) {return;}

	TSharedPtr<FJsonObject> UsersObject = MakeShareable(new FJsonObject);
	
	for(auto User : Users)
	{
		UsersObject->SetNumberField(User.User, CalculateUserPermissionsBitmask(User.Permissions));
	}

	JsonObject->SetObjectField(ANSI_TO_TCHAR("uuids"), UsersObject);
}

FString UPubnubTokenUtilities::ReworkParsedToken(const FString& ParsedToken)
{
	if (ParsedToken.IsEmpty())
	{
		return "";
	}

	TSharedPtr<FJsonObject> ParsedTokenObject;
	if (!UPubnubJsonUtilities::StringToJsonObject(ParsedToken, ParsedTokenObject) || !ParsedTokenObject.IsValid())
	{
		return "";
	}

	// Create the reworked token object
	TSharedPtr<FJsonObject> ReworkedTokenObject = MakeShareable(new FJsonObject);

	// Convert main fields
	if (ParsedTokenObject->HasField(TEXT("v")))
	{
		ReworkedTokenObject->SetNumberField(TEXT("Version"), ParsedTokenObject->GetNumberField(TEXT("v")));
	}
	if (ParsedTokenObject->HasField(TEXT("t")))
	{
		ReworkedTokenObject->SetNumberField(TEXT("Timestamp"), ParsedTokenObject->GetNumberField(TEXT("t")));
	}
	if (ParsedTokenObject->HasField(TEXT("ttl")))
	{
		ReworkedTokenObject->SetNumberField(TEXT("TTL"), ParsedTokenObject->GetNumberField(TEXT("ttl")));
	}
	if (ParsedTokenObject->HasField(TEXT("aud")))
	{
		ReworkedTokenObject->SetStringField(TEXT("AuthorizedUuid"), ParsedTokenObject->GetStringField(TEXT("aud")));
	}

	// Convert Resources
	const TSharedPtr<FJsonObject>* ResourcesObjectPtr = nullptr;
	if (ParsedTokenObject->TryGetObjectField(TEXT("res"), ResourcesObjectPtr) && ResourcesObjectPtr && (*ResourcesObjectPtr).IsValid())
	{
		TSharedPtr<FJsonObject> ResourcesObject = MakeShareable(new FJsonObject);
		const TSharedPtr<FJsonObject>& SourceResources = *ResourcesObjectPtr;

		// Convert Channels
		const TSharedPtr<FJsonObject>* ChannelsObjectPtr = nullptr;
		if (SourceResources->TryGetObjectField(TEXT("chan"), ChannelsObjectPtr) && ChannelsObjectPtr && (*ChannelsObjectPtr).IsValid())
		{
			TSharedPtr<FJsonObject> ConvertedChannels = ConvertChannelPermissionsFromBitmask(*ChannelsObjectPtr);
			if (ConvertedChannels.IsValid() && ConvertedChannels->Values.Num() > 0)
			{
				ResourcesObject->SetObjectField(TEXT("Channels"), ConvertedChannels);
			}
		}

		// Convert Channel Groups
		const TSharedPtr<FJsonObject>* GroupsObjectPtr = nullptr;
		if (SourceResources->TryGetObjectField(TEXT("grp"), GroupsObjectPtr) && GroupsObjectPtr && (*GroupsObjectPtr).IsValid())
		{
			TSharedPtr<FJsonObject> ConvertedGroups = ConvertChannelGroupPermissionsFromBitmask(*GroupsObjectPtr);
			if (ConvertedGroups.IsValid() && ConvertedGroups->Values.Num() > 0)
			{
				ResourcesObject->SetObjectField(TEXT("ChannelGroups"), ConvertedGroups);
			}
		}

		// Convert UUIDs
		const TSharedPtr<FJsonObject>* UuidsObjectPtr = nullptr;
		if (SourceResources->TryGetObjectField(TEXT("uuid"), UuidsObjectPtr) && UuidsObjectPtr && (*UuidsObjectPtr).IsValid())
		{
			TSharedPtr<FJsonObject> ConvertedUuids = ConvertUserPermissionsFromBitmask(*UuidsObjectPtr);
			if (ConvertedUuids.IsValid() && ConvertedUuids->Values.Num() > 0)
			{
				ResourcesObject->SetObjectField(TEXT("Uuids"), ConvertedUuids);
			}
		}

		if (ResourcesObject->Values.Num() > 0)
		{
			ReworkedTokenObject->SetObjectField(TEXT("Resources"), ResourcesObject);
		}
	}

	// Convert Patterns
	const TSharedPtr<FJsonObject>* PatternsObjectPtr = nullptr;
	if (ParsedTokenObject->TryGetObjectField(TEXT("pat"), PatternsObjectPtr) && PatternsObjectPtr && (*PatternsObjectPtr).IsValid())
	{
		TSharedPtr<FJsonObject> PatternsObject = MakeShareable(new FJsonObject);
		const TSharedPtr<FJsonObject>& SourcePatterns = *PatternsObjectPtr;

		// Convert Channel Patterns
		const TSharedPtr<FJsonObject>* ChannelPatternsObjectPtr = nullptr;
		if (SourcePatterns->TryGetObjectField(TEXT("chan"), ChannelPatternsObjectPtr) && ChannelPatternsObjectPtr && (*ChannelPatternsObjectPtr).IsValid())
		{
			TSharedPtr<FJsonObject> ConvertedChannelPatterns = ConvertChannelPermissionsFromBitmask(*ChannelPatternsObjectPtr);
			if (ConvertedChannelPatterns.IsValid() && ConvertedChannelPatterns->Values.Num() > 0)
			{
				PatternsObject->SetObjectField(TEXT("Channels"), ConvertedChannelPatterns);
			}
		}

		// Convert Channel Group Patterns
		const TSharedPtr<FJsonObject>* GroupPatternsObjectPtr = nullptr;
		if (SourcePatterns->TryGetObjectField(TEXT("grp"), GroupPatternsObjectPtr) && GroupPatternsObjectPtr && (*GroupPatternsObjectPtr).IsValid())
		{
			TSharedPtr<FJsonObject> ConvertedGroupPatterns = ConvertChannelGroupPermissionsFromBitmask(*GroupPatternsObjectPtr);
			if (ConvertedGroupPatterns.IsValid() && ConvertedGroupPatterns->Values.Num() > 0)
			{
				PatternsObject->SetObjectField(TEXT("ChannelGroups"), ConvertedGroupPatterns);
			}
		}

		// Convert UUID Patterns
		const TSharedPtr<FJsonObject>* UuidPatternsObjectPtr = nullptr;
		if (SourcePatterns->TryGetObjectField(TEXT("uuid"), UuidPatternsObjectPtr) && UuidPatternsObjectPtr && (*UuidPatternsObjectPtr).IsValid())
		{
			TSharedPtr<FJsonObject> ConvertedUuidPatterns = ConvertUserPermissionsFromBitmask(*UuidPatternsObjectPtr);
			if (ConvertedUuidPatterns.IsValid() && ConvertedUuidPatterns->Values.Num() > 0)
			{
				PatternsObject->SetObjectField(TEXT("Uuids"), ConvertedUuidPatterns);
			}
		}

		if (PatternsObject->Values.Num() > 0)
		{
			ReworkedTokenObject->SetObjectField(TEXT("Patterns"), PatternsObject);
		}
	}

	return UPubnubJsonUtilities::JsonObjectToString(ReworkedTokenObject);
}

void UPubnubTokenUtilities::ParsedTokenResourceAt(pubnub_context_t* Context, EParsedTokenResourceKind Kind, size_t Index, pubnub_parsed_token_resource_t& OutResource)
{
	switch (Kind)
	{
	case EParsedTokenResourceKind::Channel:
		OutResource = pubnub_parsed_token_channel_at(Context, Index);
		return;
	case EParsedTokenResourceKind::Group:
		OutResource = pubnub_parsed_token_group_at(Context, Index);
		return;
	case EParsedTokenResourceKind::Uuid:
		OutResource = pubnub_parsed_token_uuid_at(Context, Index);
		return;
	case EParsedTokenResourceKind::ChannelPattern:
		OutResource = pubnub_parsed_token_channel_pattern_at(Context, Index);
		return;
	case EParsedTokenResourceKind::GroupPattern:
		OutResource = pubnub_parsed_token_group_pattern_at(Context, Index);
		return;
	case EParsedTokenResourceKind::UuidPattern:
		OutResource = pubnub_parsed_token_uuid_pattern_at(Context, Index);
		return;
	default:
		OutResource = pubnub_parsed_token_resource_t{};
		return;
	}
}

void UPubnubTokenUtilities::AppendParsedTokenResources(pubnub_context_t* Context, uint32 Count, EParsedTokenResourceKind Kind, TArray<TPair<FString, int32>>& Out)
{
	Out.Reserve(static_cast<int32>(Count));
	for (uint32 Index = 0; Index < Count; ++Index)
	{
		pubnub_parsed_token_resource_t Resource = {};
		ParsedTokenResourceAt(Context, Kind, static_cast<size_t>(Index), Resource);
		Out.Emplace(
			UPubnubInternalUtilities::PubnubStringViewToString(Resource.name),
			static_cast<int32>(Resource.permissions));
	}
}

FString UPubnubTokenUtilities::BuildReworkedParsedToken(pubnub_context_t* Context, const pubnub_parsed_token_t& ParsedToken)
{
	TArray<TPair<FString, int32>> Channels;
	TArray<TPair<FString, int32>> ChannelGroups;
	TArray<TPair<FString, int32>> Users;
	TArray<TPair<FString, int32>> ChannelPatterns;
	TArray<TPair<FString, int32>> ChannelGroupPatterns;
	TArray<TPair<FString, int32>> UserPatterns;
	AppendParsedTokenResources(Context, ParsedToken.channel_count, EParsedTokenResourceKind::Channel, Channels);
	AppendParsedTokenResources(Context, ParsedToken.group_count, EParsedTokenResourceKind::Group, ChannelGroups);
	AppendParsedTokenResources(Context, ParsedToken.uuid_count, EParsedTokenResourceKind::Uuid, Users);
	AppendParsedTokenResources(Context, ParsedToken.channel_pattern_count, EParsedTokenResourceKind::ChannelPattern, ChannelPatterns);
	AppendParsedTokenResources(Context, ParsedToken.group_pattern_count, EParsedTokenResourceKind::GroupPattern, ChannelGroupPatterns);
	AppendParsedTokenResources(Context, ParsedToken.uuid_pattern_count, EParsedTokenResourceKind::UuidPattern, UserPatterns);

	return BuildReworkedParsedToken(
		ParsedToken.version,
		ParsedToken.timestamp,
		ParsedToken.ttl,
		UPubnubInternalUtilities::PubnubStringViewToString(ParsedToken.authorized_uuid),
		Channels,
		ChannelGroups,
		Users,
		ChannelPatterns,
		ChannelGroupPatterns,
		UserPatterns);
}

template<typename MakePermissionsFunc>
static TSharedPtr<FJsonObject> BitmaskEntriesToPermissionsObject(const TArray<TPair<FString, int32>>& Entries, MakePermissionsFunc MakePermissions)
{
	TSharedPtr<FJsonObject> Object = MakeShareable(new FJsonObject);
	for (const TPair<FString, int32>& Entry : Entries)
	{
		Object->SetObjectField(Entry.Key, MakePermissions(Entry.Value));
	}
	return Object;
}

FString UPubnubTokenUtilities::BuildReworkedParsedToken(
	int32 Version,
	uint64 Timestamp,
	uint32 Ttl,
	const FString& AuthorizedUuid,
	const TArray<TPair<FString, int32>>& Channels,
	const TArray<TPair<FString, int32>>& ChannelGroups,
	const TArray<TPair<FString, int32>>& Users,
	const TArray<TPair<FString, int32>>& ChannelPatterns,
	const TArray<TPair<FString, int32>>& ChannelGroupPatterns,
	const TArray<TPair<FString, int32>>& UserPatterns)
{
	TSharedPtr<FJsonObject> ReworkedTokenObject = MakeShareable(new FJsonObject);
	ReworkedTokenObject->SetNumberField(TEXT("Version"), Version);
	ReworkedTokenObject->SetNumberField(TEXT("Timestamp"), static_cast<double>(Timestamp));
	ReworkedTokenObject->SetNumberField(TEXT("TTL"), Ttl);
	if (!AuthorizedUuid.IsEmpty())
	{
		ReworkedTokenObject->SetStringField(TEXT("AuthorizedUuid"), AuthorizedUuid);
	}

	TSharedPtr<FJsonObject> ResourcesObject = MakeShareable(new FJsonObject);
	if (Channels.Num() > 0)
	{
		ResourcesObject->SetObjectField(TEXT("Channels"), BitmaskEntriesToPermissionsObject(Channels, [](int Bitmask)
		{
			return CreateChannelPermissionsFromBitmask(Bitmask);
		}));
	}
	if (ChannelGroups.Num() > 0)
	{
		ResourcesObject->SetObjectField(TEXT("ChannelGroups"), BitmaskEntriesToPermissionsObject(ChannelGroups, [](int Bitmask)
		{
			return CreateChannelGroupPermissionsFromBitmask(Bitmask);
		}));
	}
	if (Users.Num() > 0)
	{
		ResourcesObject->SetObjectField(TEXT("Uuids"), BitmaskEntriesToPermissionsObject(Users, [](int Bitmask)
		{
			return CreateUserPermissionsFromBitmask(Bitmask);
		}));
	}
	if (ResourcesObject->Values.Num() > 0)
	{
		ReworkedTokenObject->SetObjectField(TEXT("Resources"), ResourcesObject);
	}

	TSharedPtr<FJsonObject> PatternsObject = MakeShareable(new FJsonObject);
	if (ChannelPatterns.Num() > 0)
	{
		PatternsObject->SetObjectField(TEXT("Channels"), BitmaskEntriesToPermissionsObject(ChannelPatterns, [](int Bitmask)
		{
			return CreateChannelPermissionsFromBitmask(Bitmask);
		}));
	}
	if (ChannelGroupPatterns.Num() > 0)
	{
		PatternsObject->SetObjectField(TEXT("ChannelGroups"), BitmaskEntriesToPermissionsObject(ChannelGroupPatterns, [](int Bitmask)
		{
			return CreateChannelGroupPermissionsFromBitmask(Bitmask);
		}));
	}
	if (UserPatterns.Num() > 0)
	{
		PatternsObject->SetObjectField(TEXT("Uuids"), BitmaskEntriesToPermissionsObject(UserPatterns, [](int Bitmask)
		{
			return CreateUserPermissionsFromBitmask(Bitmask);
		}));
	}
	if (PatternsObject->Values.Num() > 0)
	{
		ReworkedTokenObject->SetObjectField(TEXT("Patterns"), PatternsObject);
	}

	return UPubnubJsonUtilities::JsonObjectToString(ReworkedTokenObject);
}

int UPubnubTokenUtilities::CalculateChannelPermissionsBitmask(const FPubnubChannelPermissions& Perms)
{
	// Calculate bitmask for channel permissions based on individual permission flags
	// Bit values: READ=1, WRITE=2, MANAGE=4, DELETE=8, GET=32, UPDATE=64, JOIN=128
	int Bitmask = 0;
	if (Perms.Read) Bitmask |= 1;
	if (Perms.Write) Bitmask |= 2;
	if (Perms.Manage) Bitmask |= 4;
	if (Perms.Delete) Bitmask |= 8;
	if (Perms.Get) Bitmask |= 32;
	if (Perms.Update) Bitmask |= 64;
	if (Perms.Join) Bitmask |= 128;
	return Bitmask;
}

int UPubnubTokenUtilities::CalculateChannelGroupPermissionsBitmask(const FPubnubChannelGroupPermissions& Perms)
{
	// Calculate bitmask for channel group permissions based on individual permission flags
	// Bit values: READ=1, MANAGE=4
	int Bitmask = 0;
	if (Perms.Read) Bitmask |= 1;
	if (Perms.Manage) Bitmask |= 4;
	return Bitmask;
}

int UPubnubTokenUtilities::CalculateUserPermissionsBitmask(const FPubnubUserPermissions& Perms)
{
	// Calculate bitmask for user permissions based on individual permission flags
	// Bit values: DELETE=8, GET=32, UPDATE=64
	int Bitmask = 0;
	if (Perms.Delete) Bitmask |= 8;
	if (Perms.Get) Bitmask |= 32;
	if (Perms.Update) Bitmask |= 64;
	return Bitmask;
}

TSharedPtr<FJsonObject> UPubnubTokenUtilities::ConvertChannelPermissionsFromBitmask(const TSharedPtr<FJsonObject>& SourceObject)
{
	TSharedPtr<FJsonObject> ConvertedObject = MakeShareable(new FJsonObject);
	
	for (const auto& Pair : SourceObject->Values)
	{
		if (Pair.Value.IsValid() && Pair.Value->Type == EJson::Number)
		{
			int Bitmask = static_cast<int>(Pair.Value->AsNumber());
			TSharedPtr<FJsonObject> PermissionsObject = CreateChannelPermissionsFromBitmask(Bitmask);
			ConvertedObject->SetObjectField(Pair.Key, PermissionsObject);
		}
	}
	
	return ConvertedObject;
}

TSharedPtr<FJsonObject> UPubnubTokenUtilities::ConvertChannelGroupPermissionsFromBitmask(const TSharedPtr<FJsonObject>& SourceObject)
{
	TSharedPtr<FJsonObject> ConvertedObject = MakeShareable(new FJsonObject);
	
	for (const auto& Pair : SourceObject->Values)
	{
		if (Pair.Value.IsValid() && Pair.Value->Type == EJson::Number)
		{
			int Bitmask = static_cast<int>(Pair.Value->AsNumber());
			TSharedPtr<FJsonObject> PermissionsObject = CreateChannelGroupPermissionsFromBitmask(Bitmask);
			ConvertedObject->SetObjectField(Pair.Key, PermissionsObject);
		}
	}
	
	return ConvertedObject;
}

TSharedPtr<FJsonObject> UPubnubTokenUtilities::ConvertUserPermissionsFromBitmask(const TSharedPtr<FJsonObject>& SourceObject)
{
	TSharedPtr<FJsonObject> ConvertedObject = MakeShareable(new FJsonObject);
	
	for (const auto& Pair : SourceObject->Values)
	{
		if (Pair.Value.IsValid() && Pair.Value->Type == EJson::Number)
		{
			int Bitmask = static_cast<int>(Pair.Value->AsNumber());
			TSharedPtr<FJsonObject> PermissionsObject = CreateUserPermissionsFromBitmask(Bitmask);
			ConvertedObject->SetObjectField(Pair.Key, PermissionsObject);
		}
	}
	
	return ConvertedObject;
}

TSharedPtr<FJsonObject> UPubnubTokenUtilities::CreateChannelPermissionsFromBitmask(int Bitmask)
{
	TSharedPtr<FJsonObject> PermissionsObject = MakeShareable(new FJsonObject);
	
	// Channel permissions: Read, Write, Delete, Get, Update, Manage, Join
	// Based on bit values: READ=1, WRITE=2, MANAGE=4, DELETE=8, GET=32, UPDATE=64, JOIN=128
	
	// Always include all relevant channel permissions
	PermissionsObject->SetBoolField(TEXT("Read"), (Bitmask & 1) != 0);
	PermissionsObject->SetBoolField(TEXT("Write"), (Bitmask & 2) != 0);
	PermissionsObject->SetBoolField(TEXT("Manage"), (Bitmask & 4) != 0);
	PermissionsObject->SetBoolField(TEXT("Delete"), (Bitmask & 8) != 0);
	PermissionsObject->SetBoolField(TEXT("Get"), (Bitmask & 32) != 0);
	PermissionsObject->SetBoolField(TEXT("Update"), (Bitmask & 64) != 0);
	PermissionsObject->SetBoolField(TEXT("Join"), (Bitmask & 128) != 0);
	
	return PermissionsObject;
}

TSharedPtr<FJsonObject> UPubnubTokenUtilities::CreateChannelGroupPermissionsFromBitmask(int Bitmask)
{
	TSharedPtr<FJsonObject> PermissionsObject = MakeShareable(new FJsonObject);
	
	// Channel Group permissions: Read, Manage
	// Based on bit values: READ=1, MANAGE=4
	
	// Always include all relevant channel group permissions
	PermissionsObject->SetBoolField(TEXT("Read"), (Bitmask & 1) != 0);
	PermissionsObject->SetBoolField(TEXT("Manage"), (Bitmask & 4) != 0);
	
	return PermissionsObject;
}

TSharedPtr<FJsonObject> UPubnubTokenUtilities::CreateUserPermissionsFromBitmask(int Bitmask)
{
	TSharedPtr<FJsonObject> PermissionsObject = MakeShareable(new FJsonObject);
	
	// User permissions: Delete, Get, Update
	// Based on bit values: DELETE=8, GET=32, UPDATE=64
	
	// Always include all relevant user permissions
	PermissionsObject->SetBoolField(TEXT("Delete"), (Bitmask & 8) != 0);
	PermissionsObject->SetBoolField(TEXT("Get"), (Bitmask & 32) != 0);
	PermissionsObject->SetBoolField(TEXT("Update"), (Bitmask & 64) != 0);
	
	return PermissionsObject;
}

/** Names stay alive for the C-Core call. Permission name pointers alias those holders. */
struct FGrantResourcePermissions
{
	TArray<TUniquePtr<FUTF8StringHolder>> Names;
	TArray<pubnub_access_resource_permission_t> Permissions;
};

struct FPubnubGrantTokenResourceStorage
{
	FGrantResourcePermissions Channels;
	FGrantResourcePermissions ChannelGroups;
	FGrantResourcePermissions Users;
	FGrantResourcePermissions ChannelPatterns;
	FGrantResourcePermissions ChannelGroupPatterns;
	FGrantResourcePermissions UserPatterns;
};

void FPubnubGrantTokenResourceStorageDeleter::operator()(FPubnubGrantTokenResourceStorage* Storage) const
{
	delete Storage;
}

void UPubnubTokenUtilities::FillChannelGrantPermissions(const TArray<FChannelGrant>& Grants, FGrantResourcePermissions& Out)
{
	Out.Names.Reserve(Grants.Num());
	Out.Permissions.Reserve(Grants.Num());
	for (const FChannelGrant& Grant : Grants)
	{
		TUniquePtr<FUTF8StringHolder> Name = MakeUnique<FUTF8StringHolder>(Grant.Channel);
		pubnub_access_resource_permission_t Permission = {};
		Permission.name = Name->Get();
		Permission.permissions = static_cast<uint32_t>(CalculateChannelPermissionsBitmask(Grant.Permissions));
		Out.Names.Add(MoveTemp(Name));
		Out.Permissions.Add(Permission);
	}
}

void UPubnubTokenUtilities::FillChannelGroupGrantPermissions(const TArray<FChannelGroupGrant>& Grants, FGrantResourcePermissions& Out)
{
	Out.Names.Reserve(Grants.Num());
	Out.Permissions.Reserve(Grants.Num());
	for (const FChannelGroupGrant& Grant : Grants)
	{
		TUniquePtr<FUTF8StringHolder> Name = MakeUnique<FUTF8StringHolder>(Grant.ChannelGroup);
		pubnub_access_resource_permission_t Permission = {};
		Permission.name = Name->Get();
		Permission.permissions = static_cast<uint32_t>(CalculateChannelGroupPermissionsBitmask(Grant.Permissions));
		Out.Names.Add(MoveTemp(Name));
		Out.Permissions.Add(Permission);
	}
}

void UPubnubTokenUtilities::FillUserGrantPermissions(const TArray<FUserGrant>& Grants, FGrantResourcePermissions& Out)
{
	Out.Names.Reserve(Grants.Num());
	Out.Permissions.Reserve(Grants.Num());
	for (const FUserGrant& Grant : Grants)
	{
		TUniquePtr<FUTF8StringHolder> Name = MakeUnique<FUTF8StringHolder>(Grant.User);
		pubnub_access_resource_permission_t Permission = {};
		Permission.name = Name->Get();
		Permission.permissions = static_cast<uint32_t>(CalculateUserPermissionsBitmask(Grant.Permissions));
		Out.Names.Add(MoveTemp(Name));
		Out.Permissions.Add(Permission);
	}
}

void UPubnubTokenUtilities::ApplyGrantResourceList(const FGrantResourcePermissions& List, const pubnub_access_resource_permission_t*& OutPermissions, size_t& OutCount)
{
	if (List.Permissions.IsEmpty())
	{
		OutPermissions = nullptr;
		OutCount = 0;
		return;
	}

	OutPermissions = List.Permissions.GetData();
	OutCount = static_cast<size_t>(List.Permissions.Num());
}

TUniquePtr<FPubnubGrantTokenResourceStorage, FPubnubGrantTokenResourceStorageDeleter> UPubnubTokenUtilities::BuildGrantTokenResourceStorage(const FPubnubGrantTokenPermissions& Permissions)
{
	TUniquePtr<FPubnubGrantTokenResourceStorage, FPubnubGrantTokenResourceStorageDeleter> Storage(new FPubnubGrantTokenResourceStorage());
	FillChannelGrantPermissions(Permissions.Channels, Storage->Channels);
	FillChannelGroupGrantPermissions(Permissions.ChannelGroups, Storage->ChannelGroups);
	FillUserGrantPermissions(Permissions.Users, Storage->Users);
	FillChannelGrantPermissions(Permissions.ChannelPatterns, Storage->ChannelPatterns);
	FillChannelGroupGrantPermissions(Permissions.ChannelGroupPatterns, Storage->ChannelGroupPatterns);
	FillUserGrantPermissions(Permissions.UserPatterns, Storage->UserPatterns);
	return Storage;
}

void UPubnubTokenUtilities::ApplyGrantTokenResourceStorage(const FPubnubGrantTokenResourceStorage& Storage, pubnub_grant_token_opts_t& Opts)
{
	ApplyGrantResourceList(Storage.Channels, Opts.channels, Opts.channel_count);
	ApplyGrantResourceList(Storage.ChannelGroups, Opts.groups, Opts.group_count);
	ApplyGrantResourceList(Storage.Users, Opts.uuids, Opts.uuid_count);
	ApplyGrantResourceList(Storage.ChannelPatterns, Opts.channel_patterns, Opts.channel_pattern_count);
	ApplyGrantResourceList(Storage.ChannelGroupPatterns, Opts.group_patterns, Opts.group_pattern_count);
	ApplyGrantResourceList(Storage.UserPatterns, Opts.uuid_patterns, Opts.uuid_pattern_count);
}