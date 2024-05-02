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
	if(!IsInitialized)
	{return;}
	
	ChatSystem->SubscribeToChannel(ChannelID);
}

void UPubnubChatChannel::Disconnect()
{
	if(!IsInitialized)
	{return;}
	
	ChatSystem->UnsubscribeFromChannel(ChannelID);
}

void UPubnubChatChannel::Join(FString AdditionalParams)
{
	if(!IsInitialized)
	{return;}
	
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
	if(!IsInitialized)
	{return;}
	
	FString RemoveObjectString = FString::Printf(TEXT("[{\"channel\": {\"id\": \"%s\"}}]"), *ChannelID);
	FString UserID = PubnubSubsystem->GetUserID();
	PubnubSubsystem->RemoveMemberships(UserID, "", RemoveObjectString);
	Disconnect();
}

void UPubnubChatChannel::Delete()
{
	if(!IsInitialized)
	{return;}
	
	ChatSystem->DeleteChannel(ChannelID);
	IsInitialized = false;
}

void UPubnubChatChannel::Update(FPubnubChatChannelData InAdditionalChannelData)
{
	if(!IsInitialized)
	{return;}
	
	ChannelData = InAdditionalChannelData;
	
	TSharedPtr<FJsonObject> MetadataJsonObject = MakeShareable(new FJsonObject);
	AddChannelDataToJson(MetadataJsonObject, ChannelID, InAdditionalChannelData);
	PubnubSubsystem->SetChannelMetadata(ChannelID, "", UPubnubUtilities::JsonObjectToString(MetadataJsonObject));
}

void UPubnubChatChannel::SetRestrictions(FString UserID, bool BanUser, bool MuteUser, FString Reason)
{
	if(!IsInitialized)
	{return;}

	ChatSystem->SetRestrictions(UserID, ChannelID, BanUser, MuteUser, Reason);
}

void UPubnubChatChannel::SendText(FString Message, EPubnubChatMessageType MessageType,
	FString MetaData)
{
	if(!IsInitialized)
	{return;}

	FPubnubPublishSettings PublishSettings;
	PublishSettings.MetaData = MetaData;
	
	PubnubSubsystem->PublishMessage(ChannelID, ChatMessageToPublishString(Message, MessageType), PublishSettings);
}

FString UPubnubChatChannel::ChatMessageToPublishString(FString Message, EPubnubChatMessageType MessageType)
{
	TSharedPtr<FJsonObject> MessageJsonObject = MakeShareable(new FJsonObject);
	
	//Convert MessageType to FString (currently only 1 type is supported)
	FString MessageTypeString;
	switch (MessageType)
	{
	case EPubnubChatMessageType::PCMT_TEXT:
		MessageTypeString = "text";
		break;
	}
	
	MessageJsonObject->SetStringField("type", MessageTypeString);
	MessageJsonObject->SetStringField("text", Message);

	//Convert constructed Json to FString
	return UPubnubUtilities::JsonObjectToString(MessageJsonObject);
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
