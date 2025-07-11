// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "FunctionLibraries/PubnubTokenUtilities.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "PubnubSubsystem.h"
#include "PubnubStructLibrary.h"


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
		//Create bit mask value from all permissions
		struct pam_permission ChPerm;
		ChPerm.read = Channel.Permissions.Read;
		ChPerm.write = Channel.Permissions.Write;
		ChPerm.del = Channel.Permissions.Delete;
		ChPerm.get = Channel.Permissions.Get;
		ChPerm.update = Channel.Permissions.Update;
		ChPerm.manage = Channel.Permissions.Manage;
		ChPerm.join = Channel.Permissions.Join;
		ChPerm.create = false;
		int PermBitMask = pubnub_get_grant_bit_mask_value(ChPerm);

		ChannelsObject->SetNumberField(Channel.Channel, PermBitMask);
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
		//Create bit mask value from all permissions
		struct pam_permission ChPerm;
		ChPerm.read = ChannelGroup.Permissions.Read;
		ChPerm.manage = ChannelGroup.Permissions.Manage;
		ChPerm.write = false;
		ChPerm.del = false;
		ChPerm.get = false;
		ChPerm.update = false;
		ChPerm.join = false;
		ChPerm.create = false;
		int PermBitMask = pubnub_get_grant_bit_mask_value(ChPerm);

		ChannelGroupsObject->SetNumberField(ChannelGroup.ChannelGroup, PermBitMask);
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
		//Create bit mask value from all permissions
		struct pam_permission ChPerm;
		ChPerm.del = User.Permissions.Delete;
		ChPerm.get = User.Permissions.Get;
		ChPerm.update = User.Permissions.Update;
		ChPerm.read = false;
		ChPerm.write = false;
		ChPerm.manage = false;
		ChPerm.join = false;
		ChPerm.create = false;
		int PermBitMask = pubnub_get_grant_bit_mask_value(ChPerm);

		UsersObject->SetNumberField(User.User, PermBitMask);
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

TSharedPtr<FJsonObject> UPubnubTokenUtilities::ConvertChannelPermissionsFromBitmask(const TSharedPtr<FJsonObject>& SourceObject)
{
	TSharedPtr<FJsonObject> ConvertedObject = MakeShareable(new FJsonObject);
	
	for (const auto& Pair : SourceObject->Values)
	{
		if (Pair.Value.IsValid() && Pair.Value->Type == EJson::Number)
		{
			int Bitmask = static_cast<int>(Pair.Value->AsNumber());
			TSharedPtr<FJsonObject> PermissionsObject = CreateChannelPermissionsObject(Bitmask);
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
			TSharedPtr<FJsonObject> PermissionsObject = CreateChannelGroupPermissionsObject(Bitmask);
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
			TSharedPtr<FJsonObject> PermissionsObject = CreateUserPermissionsObject(Bitmask);
			ConvertedObject->SetObjectField(Pair.Key, PermissionsObject);
		}
	}
	
	return ConvertedObject;
}

TSharedPtr<FJsonObject> UPubnubTokenUtilities::CreateChannelPermissionsObject(int Bitmask)
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

TSharedPtr<FJsonObject> UPubnubTokenUtilities::CreateChannelGroupPermissionsObject(int Bitmask)
{
	TSharedPtr<FJsonObject> PermissionsObject = MakeShareable(new FJsonObject);
	
	// Channel Group permissions: Read, Manage
	// Based on bit values: READ=1, MANAGE=4
	
	// Always include all relevant channel group permissions
	PermissionsObject->SetBoolField(TEXT("Read"), (Bitmask & 1) != 0);
	PermissionsObject->SetBoolField(TEXT("Manage"), (Bitmask & 4) != 0);
	
	return PermissionsObject;
}

TSharedPtr<FJsonObject> UPubnubTokenUtilities::CreateUserPermissionsObject(int Bitmask)
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