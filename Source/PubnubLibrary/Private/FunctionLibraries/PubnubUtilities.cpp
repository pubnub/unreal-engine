// Fill out your copyright notice in the Description page of Project Settings.


#include "FunctionLibraries/PubnubUtilities.h"

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