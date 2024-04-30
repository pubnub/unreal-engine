// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatSystem/PubnubChatUser.h"
#include "ChatSystem/PubnubChatSystem.h"
#include "PubnubSubsystem.h"
#include "FunctionLibraries/PubnubUtilities.h"

void UPubnubChatUser::Initialize(UPubnubChatSystem* InChatSystem, FString InUserID, FPubnubChatUserData InAdditionalUserData)
{
	ChatSystem = InChatSystem;
	PubnubSubsystem = InChatSystem->GetPubnubSubsystem();
	UserID = InUserID;
	Update(InAdditionalUserData);
	IsInitialized = true;
}

void UPubnubChatUser::InitializeWithJsonData(UPubnubChatSystem* InChatSystem, FString InUserID, FString JsonData)
{
	ChatSystem = InChatSystem;
	PubnubSubsystem = InChatSystem->GetPubnubSubsystem();
	UserID = InUserID;
	Update(UserDataFromJson(JsonData));
	IsInitialized = true;
}

void UPubnubChatUser::Update(FPubnubChatUserData InAdditionalUserData)
{
	if(!IsInitialized)
	{return;}
	
	UserData = InAdditionalUserData;
	
	TSharedPtr<FJsonObject> MetadataJsonObject = MakeShareable(new FJsonObject);
	AddUserDataToJson(MetadataJsonObject, UserID, InAdditionalUserData);
	PubnubSubsystem->SetUUIDMetadata(UserID, "", UPubnubUtilities::JsonObjectToString(MetadataJsonObject));
}

void UPubnubChatUser::Delete()
{
	if(!IsInitialized)
	{return;}

	ChatSystem->DeleteUser(UserID);
	IsInitialized = false;
}

void UPubnubChatUser::AddUserDataToJson(TSharedPtr<FJsonObject>& MetadataJsonObject, FString InUserID, FPubnubChatUserData InAdditionalUserData)
{
	MetadataJsonObject->SetStringField("id", InUserID);
	if(!InAdditionalUserData.UserName.IsEmpty())
	{
		MetadataJsonObject->SetStringField("name", InAdditionalUserData.UserName);
	}
	if(!InAdditionalUserData.ExternalID.IsEmpty())
	{
		MetadataJsonObject->SetStringField("externalId", InAdditionalUserData.ExternalID);
	}
	if(!InAdditionalUserData.ProfileUrl.IsEmpty())
	{
		MetadataJsonObject->SetStringField("profileUrl", InAdditionalUserData.ProfileUrl);
	}
	if(!InAdditionalUserData.Email.IsEmpty())
	{
		MetadataJsonObject->SetStringField("email", InAdditionalUserData.Email);
	}
	if(!InAdditionalUserData.CustomDataJson.IsEmpty())
	{
		TSharedPtr<FJsonObject> CustomDataObject = MakeShareable(new FJsonObject);
		if(UPubnubUtilities::StringToJsonObject(InAdditionalUserData.CustomDataJson, CustomDataObject))
		{
			MetadataJsonObject->SetObjectField("custom", CustomDataObject);
		}
	}
	if(!InAdditionalUserData.Status.IsEmpty())
	{
		MetadataJsonObject->SetStringField("status", InAdditionalUserData.Status);
	}
	if(!InAdditionalUserData.Type.IsEmpty())
	{
		MetadataJsonObject->SetStringField("type", InAdditionalUserData.Type);
	}
}

FPubnubChatUserData UPubnubChatUser::UserDataFromJson(FString JsonData)
{
	FPubnubChatUserData UserDataFromJson;
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	if(!UPubnubUtilities::StringToJsonObject(JsonData, JsonObject))
	{
		return UserDataFromJson;
	}

	JsonObject->TryGetStringField("name", UserDataFromJson.UserName);
	JsonObject->TryGetStringField("externalId", UserDataFromJson.ExternalID);
	JsonObject->TryGetStringField("profileUrl", UserDataFromJson.ProfileUrl);
	JsonObject->TryGetStringField("email", UserDataFromJson.Email);
	JsonObject->TryGetStringField("status", UserDataFromJson.Status);
	JsonObject->TryGetStringField("type", UserDataFromJson.Type);
	
	const TSharedPtr<FJsonObject> *CustomJsonObjectPtr;
	if(JsonObject->TryGetObjectField("custom", CustomJsonObjectPtr))
	{
		UserDataFromJson.CustomDataJson = UPubnubUtilities::JsonObjectToString(*CustomJsonObjectPtr);
	}


	return UserDataFromJson;
}