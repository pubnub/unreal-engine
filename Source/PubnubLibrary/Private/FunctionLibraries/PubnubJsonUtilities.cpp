// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "Json.h"

FString UPubnubJsonUtilities::JsonObjectToString(TSharedPtr<FJsonObject> JsonObject)
{
	FString JsonString;
	TSharedRef< TJsonWriter<> > JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);
	return JsonString;
}

bool UPubnubJsonUtilities::StringToJsonObject(FString JsonString, TSharedPtr<FJsonObject>& JsonObject)
{
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonString);
	bool Result =  FJsonSerializer::Deserialize(JsonReader, JsonObject);
	return Result;
}

void UPubnubJsonUtilities::ListChannelsFromGroupJsonToData(FString ResponseJson, bool& Error, int& Status, TArray<FString>& Channels)
{
	Channels.Empty();
	
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		Error = true;
		return;
	}
	
	JsonObject->TryGetNumberField(ANSI_TO_TCHAR("status"), Status);
	JsonObject->TryGetBoolField(ANSI_TO_TCHAR("error"), Error);

	if(!JsonObject->HasField(ANSI_TO_TCHAR("payload")))
	{
		return;
	}
	
	TSharedPtr<FJsonObject> PayloadJsonObject = JsonObject->GetObjectField(ANSI_TO_TCHAR("payload"));
	
	if(PayloadJsonObject->HasField(ANSI_TO_TCHAR("channels")))
	{
		TArray<TSharedPtr<FJsonValue>> ChannelsJsonValue = PayloadJsonObject->GetArrayField(ANSI_TO_TCHAR("channels"));
		
		for(auto ChannelJsonValue : ChannelsJsonValue)
		{
			Channels.Add(ChannelJsonValue->AsString());
		}
	}
}

void UPubnubJsonUtilities::ListUserSubscribedChannelsJsonToData(FString ResponseJson, int& Status, FString& Message, TArray<FString>& Channels)
{
	Channels.Empty();
	
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		return;
	}
	
	JsonObject->TryGetNumberField(ANSI_TO_TCHAR("status"), Status);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("message"), Message);

	if(!JsonObject->HasField(ANSI_TO_TCHAR("payload")))
	{
		return;
	}
	
	TSharedPtr<FJsonObject> PayloadJsonObject = JsonObject->GetObjectField(ANSI_TO_TCHAR("payload"));
	
	if(PayloadJsonObject->HasField(ANSI_TO_TCHAR("channels")))
	{
		TArray<TSharedPtr<FJsonValue>> ChannelsJsonValue = PayloadJsonObject->GetArrayField(ANSI_TO_TCHAR("channels"));
		
		for(auto ChannelJsonValue : ChannelsJsonValue)
		{
			Channels.Add(ChannelJsonValue->AsString());
		}
	}
}

void UPubnubJsonUtilities::ListUsersFromChannelJsonToData(FString ResponseJson, int& Status, FString& Message, FPubnubListUsersFromChannelWrapper &Data)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		return;
	}

	JsonObject->TryGetNumberField(ANSI_TO_TCHAR("status"), Status);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("message"), Message);
	JsonObject->TryGetNumberField(ANSI_TO_TCHAR("occupancy"), Data.Occupancy);

	if(JsonObject->HasField(ANSI_TO_TCHAR("uuids")))
	{
		TArray<TSharedPtr<FJsonValue>> UuidsJsonValue = JsonObject->GetArrayField(ANSI_TO_TCHAR("uuids"));
		
		for(auto UuidJsonValue : UuidsJsonValue)
		{
			FString Uuid;
			FString State;
			//Depending on if response was set to include uuids state this will be a string field or an object field
			if(!UuidJsonValue->TryGetString(Uuid))
			{
				Uuid = UuidJsonValue->AsObject()->GetStringField(ANSI_TO_TCHAR("uuid"));
				State = UuidJsonValue->AsObject()->HasField(ANSI_TO_TCHAR("state")) ?
					JsonObjectToString(UuidJsonValue->AsObject()->GetObjectField(ANSI_TO_TCHAR("state"))) : "";
			}
			Data.UuidsState.Add(Uuid, State);
		}
	}
}

void UPubnubJsonUtilities::FetchHistoryJsonToData(FString ResponseJson, bool& Error, int& Status, FString& ErrorMessage, TArray<FPubnubMessageData> &Messages)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		return;
	}

	JsonObject->TryGetBoolField(ANSI_TO_TCHAR("error"), Error);
	JsonObject->TryGetNumberField(ANSI_TO_TCHAR("status"), Status);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("error_message"), ErrorMessage);

	if(!JsonObject->HasField(ANSI_TO_TCHAR("channels")))
	{
		return;
	}

	TSharedPtr<FJsonObject> ChannelsJsonObject = JsonObject->GetObjectField(ANSI_TO_TCHAR("channels"));
	
	for(auto Iterator = ChannelsJsonObject->Values.CreateIterator(); Iterator; ++Iterator)
	{
		TSharedPtr<FJsonValue> ChannelJsonValue = Iterator.Value();
		
		//Iterate through all messages from the response
		for(auto MessageValue : ChannelJsonValue->AsArray())
		{
			FPubnubMessageData CurrentMessage;
			MessageValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("message"), CurrentMessage.Message);
			MessageValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("uuid"), CurrentMessage.UserID);
			MessageValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("timetoken"), CurrentMessage.Timetoken);
			MessageValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("message_type"), CurrentMessage.MessageType);
			if(!MessageValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("meta"), CurrentMessage.Meta))
			{
				UE_LOG(LogTemp, Warning, TEXT("Reading Meta as Json Object"));
				const TSharedPtr<FJsonObject>* MetaJsonObject;
				if(MessageValue->AsObject()->TryGetObjectField(ANSI_TO_TCHAR("meta"), MetaJsonObject))
				{
					CurrentMessage.Meta = JsonObjectToString(*MetaJsonObject);
				}
			}

			//Iterate through all message actions from the message
			const TSharedPtr<FJsonObject>* MessageActionsJsonObject;
			if(MessageValue->AsObject()->TryGetObjectField(ANSI_TO_TCHAR("actions"), MessageActionsJsonObject))
			{
				FPubnubMessageActionData CurrentMessageAction;
				CurrentMessageAction.MessageTimetoken = CurrentMessage.Timetoken;
				for(auto ActionsTypeIterator = MessageActionsJsonObject->Get()->Values.CreateIterator(); ActionsTypeIterator; ++ActionsTypeIterator)
				{
					CurrentMessageAction.Type = ActionsTypeIterator.Key();
					for(auto ActionsValueIterator = ActionsTypeIterator.Value()->AsObject()->Values.CreateIterator(); ActionsValueIterator; ++ActionsValueIterator)
					{
						CurrentMessageAction.Value = ActionsValueIterator.Key();
						for(auto MessageActionValue : ActionsValueIterator.Value()->AsArray())
						{
							MessageActionValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("uuid"), CurrentMessageAction.UserID);
							MessageActionValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("actionTimetoken"), CurrentMessageAction.ActionTimetoken);
							CurrentMessage.MessageActions.Add(CurrentMessageAction);
						}
					}
				}
			}

			Messages.Add(CurrentMessage);
		}
		//At the moment C-Core and UE SDK support Fetching history from one channel. 
		break;
	}
}

void UPubnubJsonUtilities::FOnGetAllUUIDMetadataJsonToData(FString ResponseJson, int& Status, TArray<FPubnubUserData>& UsersData, FString& PageNext, FString& PagePrev)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		return;
	}
	
	JsonObject->TryGetNumberField(ANSI_TO_TCHAR("status"), Status);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("next"), PageNext);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("prev"), PagePrev);

	if(JsonObject->HasField(ANSI_TO_TCHAR("data")))
	{
		TArray<TSharedPtr<FJsonValue>> UuidsJsonValue = JsonObject->GetArrayField(ANSI_TO_TCHAR("data"));
		
		for(auto UuidJsonValue : UuidsJsonValue)
		{
			FPubnubUserData CurrentUuidData = GetUserDataFromJson(JsonObjectToString(UuidJsonValue->AsObject()));
			UsersData.Add(CurrentUuidData);
		}
	}
}

void UPubnubJsonUtilities::FOnGetUUIDMetadataJsonToData(FString ResponseJson, int& Status, FPubnubUserData& UserData)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		return;
	}
	
	JsonObject->TryGetNumberField(ANSI_TO_TCHAR("status"), Status);

	if(JsonObject->HasField(ANSI_TO_TCHAR("data")))
	{
		const TSharedPtr<FJsonObject> *DataJsonObject;
		if(JsonObject->TryGetObjectField(ANSI_TO_TCHAR("data"), DataJsonObject))
		{
			UserData = GetUserDataFromJson(JsonObjectToString(*DataJsonObject));
		}
	}
}

void UPubnubJsonUtilities::FOnGetAllChannelMetadataJsonToData(FString ResponseJson, int& Status, TArray<FPubnubChannelData>& ChannelsData, FString& PageNext, FString& PagePrev)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		return;
	}
	
	JsonObject->TryGetNumberField(ANSI_TO_TCHAR("status"), Status);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("next"), PageNext);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("prev"), PagePrev);

	if(JsonObject->HasField(ANSI_TO_TCHAR("data")))
	{
		TArray<TSharedPtr<FJsonValue>> ChannelsJsonValue = JsonObject->GetArrayField(ANSI_TO_TCHAR("data"));
		
		for(auto ChannelJsonValue : ChannelsJsonValue)
		{
			FPubnubChannelData CurrentChannelData = GetChannelDataFromJson(JsonObjectToString(ChannelJsonValue->AsObject()));
			ChannelsData.Add(CurrentChannelData);
		}
	}
}

void UPubnubJsonUtilities::FOnGetChannelMetadataJsonToData(FString ResponseJson, int& Status, FPubnubChannelData& ChannelData)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		return;
	}
	
	JsonObject->TryGetNumberField(ANSI_TO_TCHAR("status"), Status);

	if(JsonObject->HasField(ANSI_TO_TCHAR("data")))
	{
		const TSharedPtr<FJsonObject> *DataJsonObject;
		if(JsonObject->TryGetObjectField(ANSI_TO_TCHAR("data"), DataJsonObject))
		{
			ChannelData = GetChannelDataFromJson(JsonObjectToString(*DataJsonObject));
		}
	}
}

FPubnubUserData UPubnubJsonUtilities::GetUserDataFromJson(FString ResponseJson)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		return FPubnubUserData();
	}

	FPubnubUserData UserData;

	JsonObject->TryGetStringField(ANSI_TO_TCHAR("id"), UserData.UserID);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("name"), UserData.UserName);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("externalId"), UserData.ExternalID);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("profileUrl"), UserData.ProfileUrl);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("email"), UserData.Email);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("status"), UserData.Status);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("type"), UserData.Type);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("updated"), UserData.Updated);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("eTag"), UserData.ETag);

	const TSharedPtr<FJsonObject> *CustomJsonObject;
	if(JsonObject->TryGetObjectField(ANSI_TO_TCHAR("custom"), CustomJsonObject))
	{
		UserData.Custom = JsonObjectToString(*CustomJsonObject);
	}

	return UserData;
}

FPubnubChannelData UPubnubJsonUtilities::GetChannelDataFromJson(FString ResponseJson)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		return FPubnubChannelData();
	}

	FPubnubChannelData ChannelData;

	JsonObject->TryGetStringField(ANSI_TO_TCHAR("id"), ChannelData.ChannelID);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("name"), ChannelData.ChannelName);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("description"), ChannelData.Description);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("status"), ChannelData.Status);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("type"), ChannelData.Type);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("updated"), ChannelData.Updated);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("eTag"), ChannelData.ETag);

	const TSharedPtr<FJsonObject> *CustomJsonObject;
	if(JsonObject->TryGetObjectField(ANSI_TO_TCHAR("custom"), CustomJsonObject))
	{
		ChannelData.Custom = JsonObjectToString(*CustomJsonObject);
	}

	return ChannelData;
}

