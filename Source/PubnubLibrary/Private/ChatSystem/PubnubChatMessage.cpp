// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatSystem/PubnubChatMessage.h"
#include "FunctionLibraries/PubnubUtilities.h"

void UPubnubChatMessage::InitializeFromJson(FString MessageJson)
{

	TSharedPtr<FJsonObject> MessageJsonObject = MakeShareable(new FJsonObject);
	UPubnubUtilities::StringToJsonObject(MessageJson, MessageJsonObject);
	IsInitialized = true;
}
