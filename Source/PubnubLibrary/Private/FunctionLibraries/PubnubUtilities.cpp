// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "FunctionLibraries/PubnubUtilities.h"
#include "Json.h"

FString UPubnubUtilities::JsonObjectToString(TSharedPtr<FJsonObject> JsonObject)
{
	FString JsonString;
	TSharedRef< TJsonWriter<> > JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);
	return JsonString;
}

bool UPubnubUtilities::StringToJsonObject(FString JsonString, TSharedPtr<FJsonObject>& JsonObject)
{
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonString);
	bool Result =  FJsonSerializer::Deserialize(JsonReader, JsonObject);
	return Result;
}
