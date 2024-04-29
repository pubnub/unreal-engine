// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatSystem/PubnubChatUser.h"
#include "ChatSystem/PubnubChatSystem.h"
#include "PubnubSubsystem.h"
#include "FunctionLibraries/PubnubUtilities.h"

void UPubnubChatUser::Initialize(UPubnubChatSystem* InChatSystem, FString InUserID, FPubnubChatUserData InAdditionalUserData)
{
	ChatSystem = InChatSystem;
	PubnubSubsystem = InChatSystem->GetPubnubSubsystem();
	UpdateUser(InUserID,InAdditionalUserData);
	IsInitialized = true;
}

void UPubnubChatUser::UpdateUser(FString InUserID, FPubnubChatUserData InAdditionalUserData)
{
	UserID = InUserID;
	UserData = InAdditionalUserData;
	
	TSharedPtr<FJsonObject> MetadataJsonObject = MakeShareable(new FJsonObject);
	AddUserDataToJson(MetadataJsonObject, InUserID, InAdditionalUserData);
	PubnubSubsystem->SetUUIDMetadata(InUserID, "", UPubnubUtilities::JsonObjectToString(MetadataJsonObject));
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
		MetadataJsonObject->SetStringField("custom", InAdditionalUserData.CustomDataJson);
	}
}