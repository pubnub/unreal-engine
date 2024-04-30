// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatSystem/PubnubChatChannel.h"
#include "ChatSystem/PubnubChatSystem.h"
#include "ChatSystem/PubnubChatUser.h"
#include "PubnubSubsystem.h"
#include "FunctionLibraries/PubnubUtilities.h"


void UPubnubChatChannel::Initialize(UPubnubChatSystem* InChatSystem, FString InChannelID, FPubnubChatChannelData InAdditionalChannelData)
{
	ChatSystem = InChatSystem;
	PubnubSubsystem = InChatSystem->GetPubnubSubsystem();
	ChannelID = InChannelID;
	Update(InAdditionalChannelData);
	IsInitialized = true;
}

void UPubnubChatChannel::InitializeWithJsonData(UPubnubChatSystem* InChatSystem, FString InChannelID, FString JsonData)
{
	ChatSystem = InChatSystem;
	PubnubSubsystem = InChatSystem->GetPubnubSubsystem();
	ChannelID = InChannelID;
	Update(ChannelDataFromJson(JsonData));
	IsInitialized = true;
}

void UPubnubChatChannel::Connect()
{
	ChatSystem->SubscribeToChannel(ChannelID);
}

void UPubnubChatChannel::Disconnect()
{
	ChatSystem->UnsubscribeFromChannel(ChannelID);
}

void UPubnubChatChannel::Join(FString AdditionalParams)
{
	//Format all data into the correct Response Json
	FString IncludeString = "totalCount,customFields,channelFields,customChannelFields";
	FString CustomParameterString;
	AdditionalParams.IsEmpty() ? CustomParameterString="{}" : CustomParameterString = AdditionalParams;
	FString SetObjectString = FString::Printf(TEXT("[{\"channel\": {\"id\": \"%s\"}, \"custom\": %s}]"), *ChannelID, *AdditionalParams);

	FString UserID = PubnubSubsystem->GetUserID();
	PubnubSubsystem->SetMemberships(UserID, IncludeString, SetObjectString);
	Connect();
}

void UPubnubChatChannel::Leave()
{
	FString RemoveObjectString = FString::Printf(TEXT("[{\"channel\": {\"id\": \"%s\"}}]"), *ChannelID);
	FString UserID = PubnubSubsystem->GetUserID();
	PubnubSubsystem->RemoveMemberships(UserID, "", RemoveObjectString);
	Disconnect();
}

void UPubnubChatChannel::Delete()
{
	ChatSystem->DeleteChannel(ChannelID);
}

void UPubnubChatChannel::Update(FPubnubChatChannelData InAdditionalChannelData)
{
	ChannelData = InAdditionalChannelData;
	
	TSharedPtr<FJsonObject> MetadataJsonObject = MakeShareable(new FJsonObject);
	AddChannelDataToJson(MetadataJsonObject, ChannelID, InAdditionalChannelData);
	PubnubSubsystem->SetChannelMetadata(ChannelID, "", UPubnubUtilities::JsonObjectToString(MetadataJsonObject));
}

void UPubnubChatChannel::AddChannelDataToJson(TSharedPtr<FJsonObject> &MetadataJsonObject, FString InChannelID, FPubnubChatChannelData InAdditionalChannelData)
{
	MetadataJsonObject->SetStringField("id", InChannelID);
	if(!InAdditionalChannelData.ChannelName.IsEmpty())
	{
		MetadataJsonObject->SetStringField("name", InAdditionalChannelData.ChannelName);
	}
	if(!InAdditionalChannelData.Description.IsEmpty())
	{
		MetadataJsonObject->SetStringField("description", InAdditionalChannelData.Description);
	}
	if(!InAdditionalChannelData.CustomDataJson.IsEmpty())
	{
		TSharedPtr<FJsonObject> CustomDataObject = MakeShareable(new FJsonObject);
		if(UPubnubUtilities::StringToJsonObject(InAdditionalChannelData.CustomDataJson, CustomDataObject))
		{
			MetadataJsonObject->SetObjectField("custom", CustomDataObject);
		}
	}
	if(!InAdditionalChannelData.Updated.IsEmpty())
	{
		MetadataJsonObject->SetStringField("updated", InAdditionalChannelData.Updated);
	}
	if(!InAdditionalChannelData.Status.IsEmpty())
	{
		MetadataJsonObject->SetStringField("status", InAdditionalChannelData.Status);
	}
	if(!InAdditionalChannelData.Type.IsEmpty())
	{
		MetadataJsonObject->SetStringField("type", InAdditionalChannelData.Type);
	}
}

FPubnubChatChannelData UPubnubChatChannel::ChannelDataFromJson(FString JsonData)
{
	FPubnubChatChannelData ChannelDataFromJson;
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	if(!UPubnubUtilities::StringToJsonObject(JsonData, JsonObject))
	{
		return ChannelDataFromJson;
	}

	JsonObject->TryGetStringField("name", ChannelDataFromJson.ChannelName);
	JsonObject->TryGetStringField("description", ChannelDataFromJson.Description);
	JsonObject->TryGetStringField("updated", ChannelDataFromJson.Updated);
	JsonObject->TryGetStringField("status", ChannelDataFromJson.Status);
	JsonObject->TryGetStringField("type", ChannelDataFromJson.Type);

	const TSharedPtr<FJsonObject> *CustomJsonObjectPtr;
	if(JsonObject->TryGetObjectField("custom", CustomJsonObjectPtr))
	{
		ChannelDataFromJson.CustomDataJson = UPubnubUtilities::JsonObjectToString(*CustomJsonObjectPtr);
	}

	return ChannelDataFromJson;
}
