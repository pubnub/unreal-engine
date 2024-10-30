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
	JsonObject->TryGetBoolField(ANSI_TO_TCHAR("message"), Error);

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
