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
	
	if(JsonObject->HasField(ANSI_TO_TCHAR("error")))
	{
		Error = JsonObject->GetBoolField(ANSI_TO_TCHAR("error"));
	}

	if(JsonObject->HasField(ANSI_TO_TCHAR("status")))
	{
		Status = JsonObject->GetIntegerField(ANSI_TO_TCHAR("status"));
	}

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

	if(JsonObject->HasField(ANSI_TO_TCHAR("status")))
	{
		Status = JsonObject->GetIntegerField(ANSI_TO_TCHAR("status"));
	}
	
	if(JsonObject->HasField(ANSI_TO_TCHAR("message")))
	{
		Message = JsonObject->GetStringField(ANSI_TO_TCHAR("message"));
	}

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
