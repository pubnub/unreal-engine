// Copyright 2025 PubNub Inc. All Rights Reserved.


#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

FString UPubnubJsonUtilities::JsonObjectToString(TSharedPtr<FJsonObject> JsonObject)
{
	if(!JsonObject)
	{
		return "";
	}
	
	FString JsonString;
	TSharedRef< TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>> > JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString);

	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);
	return JsonString;
}

FString UPubnubJsonUtilities::JsonArrayToString(TArray<TSharedPtr<FJsonValue>> JsonArray)
{
	if(JsonArray.IsEmpty())
	{
		return "[]";
	}
	
	FString JsonString;
	TSharedRef< TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>> > JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString);

	FJsonSerializer::Serialize(JsonArray, JsonWriter);
	return JsonString;
}

bool UPubnubJsonUtilities::StringToJsonObject(FString JsonString, TSharedPtr<FJsonObject>& JsonObject)
{
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonString);
	return FJsonSerializer::Deserialize(JsonReader, JsonObject);
}

bool UPubnubJsonUtilities::StringToJsonArray(FString JsonString, TArray<TSharedPtr<FJsonValue>>& OutArray)
{
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonString);
	return FJsonSerializer::Deserialize(JsonReader, OutArray);
}

FString UPubnubJsonUtilities::SerializeString(const FString& InString)
{
	FString Out = TEXT("\"");

	for (const TCHAR& Ch : InString)
	{
		switch (Ch)
		{
		case '\"': Out += TEXT("\\\""); break;
		case '\\': Out += TEXT("\\\\"); break;
		case '\b': Out += TEXT("\\b"); break;
		case '\f': Out += TEXT("\\f"); break;
		case '\n': Out += TEXT("\\n"); break;
		case '\r': Out += TEXT("\\r"); break;
		case '\t': Out += TEXT("\\t"); break;
		default:
			if (Ch < 0x20)
			{
				Out += FString::Printf(TEXT("\\u%04x"), Ch);
			}
			else
			{
				Out += Ch;
			}
		}
	}

	Out += TEXT("\"");
	return Out;
}

FString UPubnubJsonUtilities::DeserializeString(const FString InString)
{
	//Unreal Engine JsonSerializer doesn't work well with string JSON, but wrapping it into an array works as expected
	FString JsonArray = "[" + InString + "]";

	TArray<TSharedPtr<FJsonValue>> ParsedArray;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonArray);

	FString Parsed = InString;
	if (FJsonSerializer::Deserialize(Reader, ParsedArray) && ParsedArray.Num() > 0)
	{
		Parsed = ParsedArray[0]->AsString();
	}

	return Parsed;
}

bool UPubnubJsonUtilities::IsCorrectJsonString(const FString InString, bool AllowSimpleTypes)
{
	//A String is correct Json if it's a valid Json Object
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	if(StringToJsonObject(InString, JsonObject))
	{
		return true;
	}

	//or a Json Array
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if(StringToJsonArray(InString, JsonArray))
	{
		return true;
	}
	
	if(!AllowSimpleTypes)
	{
		return false;
	}
	
	//Or it's a text in quotes (string type)
	if(InString.Left(1) == "\"" && InString.Right(1) == "\"")
	{
		return true;
	}
	//Or a numeric field
	if(InString.IsNumeric())
	{
		return true;
	}
	//Or a bool field
	if(InString == "true" || InString == "false")
	{
		return true;
	}

	return false;
}

bool UPubnubJsonUtilities::AreJsonObjectStringsEqual(const FString JsonString1, const FString JsonString2)
{
	if(JsonString1 == JsonString2)
	{return true;}

	//Try to parse both strings into Json objects
	TSharedPtr<FJsonObject> JsonObject1;
	if(!StringToJsonObject(JsonString1, JsonObject1))
	{return false;}
	TSharedPtr<FJsonObject> JsonObject2;
	if(!StringToJsonObject(JsonString2, JsonObject2))
	{return false;}

	//Sort keys from both Json objects and compare if there is the same amount of them
	TArray<FString> Keys1;
	JsonObject1->Values.GetKeys(Keys1);
	Keys1.Sort();

	TArray<FString> Keys2;
	JsonObject2->Values.GetKeys(Keys2);
	Keys2.Sort();

	if(Keys1.Num() != Keys2.Num())
	{return false;}

	//Compare if all keys and all values are the same
	for(int i = 0; i < Keys1.Num(); i++)
	{
		if(Keys1[i] != Keys2[i])
		{return false;}
		if(*JsonObject1->Values[Keys1[i]] != *JsonObject2->Values[Keys2[i]])
		{return false;}
	}

	return true;
}

void UPubnubJsonUtilities::ListChannelsFromGroupJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FString>& Channels)
{
	Channels.Empty();
	
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		Result.Error = true;
		Result.ErrorMessage = "Failed to parse Response";
		return;
	}

	Result = GetOperationResultFromJson(JsonObject);

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

void UPubnubJsonUtilities::ListUserSubscribedChannelsJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FString>& Channels)
{
	Channels.Empty();
	
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		Result.Error = true;
		Result.ErrorMessage = "Failed to parse Response";
		return;
	}
	Result = GetOperationResultFromJson(JsonObject);
	Result.Error = Result.Status != 200;

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

void UPubnubJsonUtilities::ListUsersFromChannelJsonToData(FString ResponseJson, FPubnubOperationResult& Result, FPubnubListUsersFromChannelWrapper &Data)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		Result.Error = true;
		Result.ErrorMessage = "Failed to parse Response";
		return;
	}
	FPubnubOperationResult ResultFromJson = GetOperationResultFromJson(JsonObject);
	Result.ErrorMessage = ResultFromJson.ErrorMessage;
	//Override status only if it was 0. Status could be set before in case of server error.
	Result.Status = Result.Status == 0 ? ResultFromJson.Status : Result.Status;
	Result.Error = Result.Status != 200;
	
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
			Data.UsersState.Add(Uuid, State);
		}
	}
}

void UPubnubJsonUtilities::FetchHistoryJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubHistoryMessageData> &Messages)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		Result.Error = true;
		Result.ErrorMessage = "Failed to parse Response";
		return;
	}

	Result = GetOperationResultFromJson(JsonObject);

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
			FPubnubHistoryMessageData CurrentMessage;
			if(!MessageValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("message"), CurrentMessage.Message))
			{
				const TSharedPtr<FJsonObject>* MetaJsonObject = nullptr;
				if(MessageValue->AsObject()->TryGetObjectField(ANSI_TO_TCHAR("message"), MetaJsonObject))
				{
					CurrentMessage.Message = JsonObjectToString(*MetaJsonObject);
				}
			}
			MessageValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("uuid"), CurrentMessage.UserID);
			MessageValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("timetoken"), CurrentMessage.Timetoken);
			MessageValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("message_type"), CurrentMessage.MessageType);
			MessageValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("custom_message_type"), CurrentMessage.CustomMessageType);
			if(!MessageValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("meta"), CurrentMessage.Meta))
			{
				const TSharedPtr<FJsonObject>* MetaJsonObject = nullptr;
				if(MessageValue->AsObject()->TryGetObjectField(ANSI_TO_TCHAR("meta"), MetaJsonObject))
				{
					CurrentMessage.Meta = JsonObjectToString(*MetaJsonObject);
				}
			}

			//Iterate through all message actions from the message
			const TSharedPtr<FJsonObject>* MessageActionsJsonObject = nullptr;
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

void UPubnubJsonUtilities::GetAllUserMetadataJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubUserData>& UsersData, FString& PageNext, FString& PagePrev)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		Result.Error = true;
		Result.ErrorMessage = "Failed to parse Response";
		return;
	}
	
	Result = GetOperationResultFromJson_AppContext(JsonObject);
	
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

void UPubnubJsonUtilities::GetUserMetadataJsonToData(FString ResponseJson, FPubnubOperationResult& Result, FPubnubUserData& UserData)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		Result.Error = true;
		Result.ErrorMessage = "Failed to parse Response";
		return;
	}

	Result = GetOperationResultFromJson_AppContext(JsonObject);

	if(JsonObject->HasField(ANSI_TO_TCHAR("data")))
	{
		const TSharedPtr<FJsonObject> *DataJsonObject = nullptr;
		if(JsonObject->TryGetObjectField(ANSI_TO_TCHAR("data"), DataJsonObject))
		{
			UserData = GetUserDataFromJson(JsonObjectToString(*DataJsonObject));
		}
	}
}

void UPubnubJsonUtilities::GetAllChannelMetadataJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubChannelData>& ChannelsData, FString& PageNext, FString& PagePrev)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		Result.Error = true;
		Result.ErrorMessage = "Failed to parse Response";
		return;
	}
	
	Result = GetOperationResultFromJson_AppContext(JsonObject);
	
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

void UPubnubJsonUtilities::GetChannelMetadataJsonToData(FString ResponseJson, FPubnubOperationResult& Result, FPubnubChannelData& ChannelData)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		Result.Error = true;
		Result.ErrorMessage = "Failed to parse Response";
		return;
	}
	
	Result = GetOperationResultFromJson_AppContext(JsonObject);

	if(JsonObject->HasField(ANSI_TO_TCHAR("data")))
	{
		const TSharedPtr<FJsonObject> *DataJsonObject = nullptr;
		if(JsonObject->TryGetObjectField(ANSI_TO_TCHAR("data"), DataJsonObject))
		{
			ChannelData = GetChannelDataFromJson(JsonObjectToString(*DataJsonObject));
		}
	}
}

void UPubnubJsonUtilities::GetMessageActionsJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubMessageActionData>& MessageActions)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		Result.Error = true;
		Result.ErrorMessage = "Failed to parse Response";
		return;
	}
	
	Result = GetOperationResultFromJson_AppContext(JsonObject);

	if(JsonObject->HasField(ANSI_TO_TCHAR("data")))
	{
		TArray<TSharedPtr<FJsonValue>> ActionsJsonValue = JsonObject->GetArrayField(ANSI_TO_TCHAR("data"));
		
		for(auto ActionJsonValue : ActionsJsonValue)
		{
			FPubnubMessageActionData CurrentMessageAction;
			ActionJsonValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("messageTimetoken"), CurrentMessageAction.MessageTimetoken);
			ActionJsonValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("type"), CurrentMessageAction.Type);
			ActionJsonValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("uuid"), CurrentMessageAction.UserID);
			ActionJsonValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("value"), CurrentMessageAction.Value);
			ActionJsonValue->AsObject()->TryGetStringField(ANSI_TO_TCHAR("actionTimetoken"), CurrentMessageAction.ActionTimetoken);

			MessageActions.Add(CurrentMessageAction);
		}
	}
}

void UPubnubJsonUtilities::AddMessageActionJsonToData(FString ResponseJson, FPubnubOperationResult& Result, FPubnubMessageActionData& MessageAction)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		Result.Error = true;
		Result.ErrorMessage = "Failed to parse Response";
		return;
	}
	
	Result = GetOperationResultFromJson_AppContext(JsonObject);

	if(JsonObject->HasField(ANSI_TO_TCHAR("data")))
	{
		TSharedPtr<FJsonObject> MessageActionJson = JsonObject->GetObjectField(ANSI_TO_TCHAR("data"));
		
		MessageActionJson->TryGetStringField(ANSI_TO_TCHAR("messageTimetoken"), MessageAction.MessageTimetoken);
		MessageActionJson->TryGetStringField(ANSI_TO_TCHAR("type"), MessageAction.Type);
		MessageActionJson->TryGetStringField(ANSI_TO_TCHAR("uuid"), MessageAction.UserID);
		MessageActionJson->TryGetStringField(ANSI_TO_TCHAR("value"), MessageAction.Value);
		MessageActionJson->TryGetStringField(ANSI_TO_TCHAR("actionTimetoken"), MessageAction.ActionTimetoken);
	}
}

void UPubnubJsonUtilities::GetMembershipsJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubMembershipData>& MembershipsData, FString& PageNext, FString& PagePrev)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		Result.Error = true;
		Result.ErrorMessage = "Failed to parse Response";
		return;
	}

	Result = GetOperationResultFromJson_AppContext(JsonObject);
	
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("next"), PageNext);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("prev"), PagePrev);

	if(JsonObject->HasField(ANSI_TO_TCHAR("data")))
	{
		MembershipsData = GetMembershipsDataArrayFromJson(JsonObject);
	}
}

void UPubnubJsonUtilities::GetChannelMembersJsonToData(FString ResponseJson, FPubnubOperationResult& Result, TArray<FPubnubChannelMemberData>& MembershipsData, FString& PageNext, FString& PagePrev)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		Result.Error = true;
		Result.ErrorMessage = "Failed to parse Response";
		return;
	}
	
	Result = GetOperationResultFromJson_AppContext(JsonObject);
	
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("next"), PageNext);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("prev"), PagePrev);

	if(JsonObject->HasField(ANSI_TO_TCHAR("data")))
	{
		MembershipsData = GetChannelMembersDataArrayFromJson(JsonObject);
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

	const TSharedPtr<FJsonObject> *CustomJsonObject = nullptr;
	if(JsonObject->TryGetObjectField(ANSI_TO_TCHAR("custom"), CustomJsonObject))
	{
		UserData.Custom = JsonObjectToString(*CustomJsonObject);
	}

	return UserData;
}

FString UPubnubJsonUtilities::GetJsonFromUserData(const FPubnubUserData& UserData)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	if (!UserData.UserID.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("id"), UserData.UserID);
	}
	if (!UserData.UserName.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("name"), UserData.UserName);
	}
	if (!UserData.ExternalID.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("externalId"), UserData.ExternalID);
	}
	if (!UserData.ProfileUrl.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("profileUrl"), UserData.ProfileUrl);
	}
	if (!UserData.Email.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("email"), UserData.Email);
	}
	if (!UserData.Status.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("status"), UserData.Status);
	}
	if (!UserData.Type.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("type"), UserData.Type);
	}
	if (!UserData.Updated.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("updated"), UserData.Updated);
	}
	if (!UserData.ETag.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("eTag"), UserData.ETag);
	}

	if (!UserData.Custom.IsEmpty())
	{
		TSharedPtr<FJsonObject> CustomJsonObject;
		if (StringToJsonObject(UserData.Custom, CustomJsonObject))
		{
			JsonObject->SetObjectField(ANSI_TO_TCHAR("custom"), CustomJsonObject);
		}
	}

	return JsonObjectToString(JsonObject);
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

	const TSharedPtr<FJsonObject> *CustomJsonObject = nullptr;
	if(JsonObject->TryGetObjectField(ANSI_TO_TCHAR("custom"), CustomJsonObject))
	{
		ChannelData.Custom = JsonObjectToString(*CustomJsonObject);
	}

	return ChannelData;
}

FString UPubnubJsonUtilities::GetJsonFromChannelData(const FPubnubChannelData& ChannelData)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	if (!ChannelData.ChannelID.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("id"), ChannelData.ChannelID);
	}
	if (!ChannelData.ChannelName.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("name"), ChannelData.ChannelName);
	}
	if (!ChannelData.Description.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("description"), ChannelData.Description);
	}
	if (!ChannelData.Status.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("status"), ChannelData.Status);
	}
	if (!ChannelData.Type.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("type"), ChannelData.Type);
	}
	if (!ChannelData.Updated.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("updated"), ChannelData.Updated);
	}
	if (!ChannelData.ETag.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("eTag"), ChannelData.ETag);
	}

	if (!ChannelData.Custom.IsEmpty())
	{
		TSharedPtr<FJsonObject> CustomJsonObject;
		if (StringToJsonObject(ChannelData.Custom, CustomJsonObject))
		{
			JsonObject->SetObjectField(ANSI_TO_TCHAR("custom"), CustomJsonObject);
		}
	}

	return JsonObjectToString(JsonObject);
}

FPubnubMembershipData UPubnubJsonUtilities::GetMembershipDataFromJson(FString ResponseJson)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		return FPubnubMembershipData();
	}

	return GetMembershipDataFromJson(JsonObject);
}

FPubnubMembershipData UPubnubJsonUtilities::GetMembershipDataFromJson(TSharedPtr<FJsonObject> JsonObject)
{
	if(!JsonObject)
	{
		return FPubnubMembershipData();
	}

	FPubnubMembershipData MembershipData;

	const TSharedPtr<FJsonObject>* ChannelJsonObject = nullptr;
	if(JsonObject->TryGetObjectField(ANSI_TO_TCHAR("channel"), ChannelJsonObject))
	{
		MembershipData.Channel = GetChannelDataFromJson(JsonObjectToString(*ChannelJsonObject));
	}
			
	const TSharedPtr<FJsonObject>* CustomJsonObject = nullptr;
	if(JsonObject->TryGetObjectField(ANSI_TO_TCHAR("custom"), CustomJsonObject))
	{
		MembershipData.Custom = JsonObjectToString(*CustomJsonObject);
	}
			
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("status"), MembershipData.Status);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("type"), MembershipData.Type);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("updated"), MembershipData.Updated);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("eTag"), MembershipData.ETag);

	return MembershipData;
}

FString UPubnubJsonUtilities::GetJsonFromMembershipInputData(const FPubnubMembershipInputData& MembershipInputData)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	if (!MembershipInputData.Channel.IsEmpty())
	{
		TSharedPtr<FJsonObject> ChannelJsonObject = MakeShareable(new FJsonObject());
		ChannelJsonObject->SetStringField(ANSI_TO_TCHAR("id"), MembershipInputData.Channel);
		JsonObject->SetObjectField(ANSI_TO_TCHAR("channel"), ChannelJsonObject);
	}

	if (!MembershipInputData.Custom.IsEmpty())
	{
		TSharedPtr<FJsonObject> CustomJsonObject;
		if (StringToJsonObject(MembershipInputData.Custom, CustomJsonObject))
		{
			JsonObject->SetObjectField(ANSI_TO_TCHAR("custom"), CustomJsonObject);
		}
	}
	
	if (!MembershipInputData.Status.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("status"), MembershipInputData.Status);
	}
	if (!MembershipInputData.Type.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("type"), MembershipInputData.Type);
	}

	return JsonObjectToString(JsonObject);
}

TArray<FPubnubMembershipData> UPubnubJsonUtilities::GetMembershipsDataArrayFromJson(FString ResponseJson)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		return {};
	}

	return GetMembershipsDataArrayFromJson(JsonObject);
}

TArray<FPubnubMembershipData> UPubnubJsonUtilities::GetMembershipsDataArrayFromJson(TSharedPtr<FJsonObject> JsonObject)
{
	if(!JsonObject)
	{
		return {};
	}
	
	TArray<FPubnubMembershipData> MembershipsDataArray;

	TArray<TSharedPtr<FJsonValue>> MembershipsJsonValue = JsonObject->GetArrayField(ANSI_TO_TCHAR("data"));
		
	for(auto MembershipJsonValue : MembershipsJsonValue)
	{
		MembershipsDataArray.Add(GetMembershipDataFromJson(MembershipJsonValue->AsObject()));
	}

	return MembershipsDataArray;
}

FString UPubnubJsonUtilities::GetJsonFromMembershipsDataArray(const TArray<FPubnubMembershipInputData>& MembershipsInputData)
{
	if(MembershipsInputData.IsEmpty())
	{
		return "[]";
	}
	
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	for (const auto& MembershipData : MembershipsInputData)
	{
		TSharedPtr<FJsonObject> MembershipJsonObject;
		if (StringToJsonObject(GetJsonFromMembershipInputData(MembershipData), MembershipJsonObject))
		{
			JsonArray.Add(MakeShareable(new FJsonValueObject(MembershipJsonObject)));
		}
	}

return JsonArrayToString(JsonArray);
}

FPubnubOperationResult UPubnubJsonUtilities::GetOperationResultFromJson(FString ResponseJson)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		return FPubnubOperationResult();
	}
	
	return GetOperationResultFromJson(JsonObject);
}

FPubnubChannelMemberData UPubnubJsonUtilities::GetChannelMemberDataFromJson(FString ResponseJson)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		return FPubnubChannelMemberData();
	}

	return GetChannelMemberDataFromJson(JsonObject);
}

FPubnubChannelMemberData UPubnubJsonUtilities::GetChannelMemberDataFromJson(TSharedPtr<FJsonObject> JsonObject)
{
	if(!JsonObject)
	{
		return FPubnubChannelMemberData();
	}

	FPubnubChannelMemberData ChannelMemberData;

	const TSharedPtr<FJsonObject>* UserJsonObject = nullptr;
	if(JsonObject->TryGetObjectField(ANSI_TO_TCHAR("uuid"), UserJsonObject))
	{
		ChannelMemberData.User = GetUserDataFromJson(JsonObjectToString(*UserJsonObject));
	}
			
	const TSharedPtr<FJsonObject>* CustomJsonObject = nullptr;
	if(JsonObject->TryGetObjectField(ANSI_TO_TCHAR("custom"), CustomJsonObject))
	{
		ChannelMemberData.Custom = JsonObjectToString(*CustomJsonObject);
	}
			
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("status"), ChannelMemberData.Status);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("type"), ChannelMemberData.Type);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("updated"), ChannelMemberData.Updated);
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("eTag"), ChannelMemberData.ETag);

	return ChannelMemberData;
}

FString UPubnubJsonUtilities::GetJsonFromChannelMemberData(const FPubnubChannelMemberInputData& ChannelMemberInputData)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	if (!ChannelMemberInputData.User.IsEmpty())
	{
		TSharedPtr<FJsonObject> UserJsonObject = MakeShareable(new FJsonObject());
		UserJsonObject->SetStringField(ANSI_TO_TCHAR("id"), ChannelMemberInputData.User);
		JsonObject->SetObjectField(ANSI_TO_TCHAR("uuid"), UserJsonObject);
	}

	if (!ChannelMemberInputData.Custom.IsEmpty())
	{
		TSharedPtr<FJsonObject> CustomJsonObject;
		if (StringToJsonObject(ChannelMemberInputData.Custom, CustomJsonObject))
		{
			JsonObject->SetObjectField(ANSI_TO_TCHAR("custom"), CustomJsonObject);
		}
	}
	
	if (!ChannelMemberInputData.Status.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("status"), ChannelMemberInputData.Status);
	}
	if (!ChannelMemberInputData.Type.IsEmpty())
	{
		JsonObject->SetStringField(ANSI_TO_TCHAR("type"), ChannelMemberInputData.Type);
	}

	return JsonObjectToString(JsonObject);
}

TArray<FPubnubChannelMemberData> UPubnubJsonUtilities::GetChannelMembersDataArrayFromJson(FString ResponseJson)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		return {};
	}

	return GetChannelMembersDataArrayFromJson(JsonObject);
}

TArray<FPubnubChannelMemberData> UPubnubJsonUtilities::GetChannelMembersDataArrayFromJson(TSharedPtr<FJsonObject> JsonObject)
{
	TArray<FPubnubChannelMemberData> ChannelMembersDataArray;

	if (JsonObject->HasField(ANSI_TO_TCHAR("data")))
	{
		TArray<TSharedPtr<FJsonValue>> MembersJsonValue = JsonObject->GetArrayField(ANSI_TO_TCHAR("data"));
		
		for(auto MemberJsonValue : MembersJsonValue)
		{
			ChannelMembersDataArray.Add(GetChannelMemberDataFromJson(MemberJsonValue->AsObject()));
		}
	}

	return ChannelMembersDataArray;
}

FString UPubnubJsonUtilities::GetJsonFromChannelMembersDataArray(const TArray<FPubnubChannelMemberInputData>& ChannelMembersInputData)
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	for (const auto& ChannelMemberData : ChannelMembersInputData)
	{
		TSharedPtr<FJsonObject> MemberJsonObject;
		if (StringToJsonObject(GetJsonFromChannelMemberData(ChannelMemberData), MemberJsonObject))
		{
			JsonArray.Add(MakeShareable(new FJsonValueObject(MemberJsonObject)));
		}
	}

return JsonArrayToString(JsonArray);
}

FString UPubnubJsonUtilities::GetJsonFromMembershipsToRemove(TArray<FString> Memberships)
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;

	for (const FString& ChannelId : Memberships)
	{
		TSharedPtr<FJsonObject> IdObject = MakeShareable(new FJsonObject());
		IdObject->SetStringField(ANSI_TO_TCHAR("id"), ChannelId);
		
		TSharedPtr<FJsonObject> ChannelObject = MakeShareable(new FJsonObject());
		ChannelObject->SetObjectField(ANSI_TO_TCHAR("channel"), IdObject);
		
		JsonArray.Add(MakeShareable(new FJsonValueObject(ChannelObject)));
	}

	return JsonArrayToString(JsonArray);
}

FString UPubnubJsonUtilities::GetJsonFromChannelMembersToRemove(TArray<FString> ChannelMembers)
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;

	for (const FString& UserId : ChannelMembers)
	{
		TSharedPtr<FJsonObject> IdObject = MakeShareable(new FJsonObject());
		IdObject->SetStringField(ANSI_TO_TCHAR("id"), UserId);

		TSharedPtr<FJsonObject> UuidObject = MakeShareable(new FJsonObject());
		UuidObject->SetObjectField(ANSI_TO_TCHAR("uuid"), IdObject);

		JsonArray.Add(MakeShareable(new FJsonValueObject(UuidObject)));
	}

	return JsonArrayToString(JsonArray);
}

FPubnubOperationResult UPubnubJsonUtilities::GetOperationResultFromJson(TSharedPtr<FJsonObject> JsonObject)
{
	if(!JsonObject)
	{
		return FPubnubOperationResult();
	}

	FPubnubOperationResult OperationResult;

	JsonObject->TryGetNumberField(ANSI_TO_TCHAR("status"), OperationResult.Status);
	JsonObject->TryGetBoolField(ANSI_TO_TCHAR("error"), OperationResult.Error);
	

	//In some endpoints this field is called "error_message" and in some "message"
	JsonObject->TryGetStringField(ANSI_TO_TCHAR("error_message"), OperationResult.ErrorMessage);
	if(OperationResult.ErrorMessage.IsEmpty())
	{
		JsonObject->TryGetStringField(ANSI_TO_TCHAR("message"), OperationResult.ErrorMessage);
	}

	return OperationResult;
}

FPubnubOperationResult UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(TSharedPtr<FJsonObject> JsonObject)
{
	if(!JsonObject)
	{
		return FPubnubOperationResult();
	}
	
	FPubnubOperationResult Result = GetOperationResultFromJson(JsonObject);
	if(Result.Status != 200)
	{
		Result.Error = true;
		//Don't override message when it's not empty. This could be the case in Access Denied from PAM, as then error message is in "message" field 
		if(Result.ErrorMessage.IsEmpty())
		{
			//App Context error message is returned as Json Object in "error" field
			if(JsonObject->HasField(ANSI_TO_TCHAR("error")))
			{
				const TSharedPtr<FJsonObject> *DataJsonObject = nullptr;
				if(JsonObject->TryGetObjectField(ANSI_TO_TCHAR("error"), DataJsonObject))
				{
					Result.ErrorMessage =  JsonObjectToString(*DataJsonObject);
				}
			}
		}
	}

	return Result;
}

FPubnubOperationResult UPubnubJsonUtilities::GetOperationResultFromJson_AppContext(FString ResponseJson)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	if(!StringToJsonObject(ResponseJson, JsonObject))
	{
		return FPubnubOperationResult();
	}
	
	return GetOperationResultFromJson_AppContext(JsonObject);
}