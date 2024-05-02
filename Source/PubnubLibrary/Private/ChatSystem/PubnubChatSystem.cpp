// Fill out your copyright notice in the Description page of Project Settings.

#include "ChatSystem/PubnubChatSystem.h"
#include "FunctionLibraries/PubnubUtilities.h"


UPubnubChatChannel* UPubnubChatSystem::CreatePublicConversation(FString ChannelID, FPubnubChatChannelData AdditionalChannelData)
{
	if(!CheckIsChatInitialized())
	{return nullptr;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(ChannelID, "ChannelName", "CreateChannel"))
	{return nullptr;}

	UPubnubChatChannel* ChannelObject = NewObject<UPubnubChatChannel>(this);
	ChannelObject->Initialize(this, ChannelID, AdditionalChannelData);
	
	return ChannelObject;
}

void UPubnubChatSystem::GetChannel(FString ChannelID, FOnGetChannelResponse OnGetChannelResponse)
{
	if(!CheckIsChatInitialized())
	{return;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(ChannelID, "ChannelID", "GetChannel"))
	{return;}

	GetChannelResponse = OnGetChannelResponse;
	
	FOnPubnubResponse OnGetChannelMetadataResponse;
	OnGetChannelMetadataResponse.BindDynamic(this, &UPubnubChatSystem::OnGetChannelResponseReceived);
	PubnubSubsystem->GetChannelMetadata("custom", ChannelID, OnGetChannelMetadataResponse);
}

UPubnubChatChannel* UPubnubChatSystem::UpdateChannel(FString ChannelID, FPubnubChatChannelData AdditionalChannelData)
{
	if(!CheckIsChatInitialized())
	{return nullptr;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(ChannelID, "ChannelID", "UpdateChannel"))
	{return nullptr;}

	UPubnubChatChannel* ChannelObject = NewObject<UPubnubChatChannel>(this);
	ChannelObject->Initialize(this, ChannelID, AdditionalChannelData);
	
	return ChannelObject;
}

void UPubnubChatSystem::DeleteChannel(FString ChannelID)
{
	if(!CheckIsChatInitialized())
	{return;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(ChannelID, "ChannelID", "DeleteChannel"))
	{return;}

	PubnubSubsystem->RemoveChannelMetadata(ChannelID);
}

UPubnubChatUser* UPubnubChatSystem::CreateUser(FString UserID, FPubnubChatUserData AdditionalUserData)
{
	if(!CheckIsChatInitialized())
	{return nullptr;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(UserID, "UserID", "CreateUser"))
	{return nullptr;}

	//If this user already exists, just return it
	if(ChatUser)
	{
		return ChatUser;
	}
	
	//Create User object and initialize it with all data
	ChatUser = NewObject<UPubnubChatUser>(this);
	ChatUser->Initialize(this, UserID, AdditionalUserData);
	
	return ChatUser;
}

void UPubnubChatSystem::GetUser(FString UserID, FOnGetUserResponse OnGetUserResponse)
{
	if(!CheckIsChatInitialized())
	{return;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(UserID, "UserID", "GetUser"))
	{return;}

	GetUserResponse = OnGetUserResponse;
	
	FOnPubnubResponse OnGetUserMetadataResponse;
	OnGetUserMetadataResponse.BindDynamic(this, &UPubnubChatSystem::OnGetUserResponseReceived);
	PubnubSubsystem->GetUUIDMetadata("custom", UserID, OnGetUserMetadataResponse);
}

UPubnubChatUser* UPubnubChatSystem::UpdateUser(FString UserID, FPubnubChatUserData AdditionalUserData)
{
	if(!CheckIsChatInitialized())
	{return nullptr;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(UserID, "UserID", "UpdateUser"))
	{return nullptr;}

	//If this user already exists and UserID matches just Update data and return it
	if(ChatUser)
	{
		if(UserID == ChatUser->UserID)
		{
			ChatUser->Update(AdditionalUserData);
			return ChatUser;
		}
	}

	//If such user doesn't exists create it with new data
	ChatUser = CreateUser(UserID, AdditionalUserData);
	return ChatUser;
}

void UPubnubChatSystem::DeleteUser(FString UserID)
{
	if(!CheckIsChatInitialized())
	{return;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(UserID, "ChannelID", "DeleteUser"))
	{return;}

	PubnubSubsystem->RemoveUUIDMetadata(UserID);
}

void UPubnubChatSystem::SetRestrictions(FString UserID, FString ChannelID, bool BanUser, bool MuteUser, FString Reason)
{
	if(!CheckIsChatInitialized())
	{return;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(UserID, "UserID", "SetRestrictions") || PubnubSubsystem->CheckIsFieldEmpty(ChannelID, "ChannelID", "SetRestrictions"))
	{return;}

	//Restrictions are held in new channel with ID: PUBNUB_INTERNAL_MODERATION_{ChannelName}
	FString RestrictionsChannel = InternalModerationPrefix;
	RestrictionsChannel.Append(ChannelID);

	//Lift restrictions
	if(!BanUser && !MuteUser)
	{
		FString RemoveMemberString = FString::Printf(TEXT("[{\"uuid\": {\"id\": \"%s\"}}]"), *UserID);
		PubnubSubsystem->RemoveChannelMembers(RestrictionsChannel, "", RemoveMemberString);
		FString EventPayloadString = FString::Printf(TEXT("{\"channelId\": \"%s\", \"restriction\": \"lifted\", \"reason\": \"%s\"}"), *RestrictionsChannel, *Reason);
		EmitChatEvent(EPubnubChatEventType::PCET_Moderation, UserID, EventPayloadString);
		return;
	}

	//Ban or mute the user
	FString ParamsString = FString::Printf(TEXT("{\"ban\": %s, \"mute\": %s, \"reason\": \"%s\"}"), *BoolToPrintString(BanUser), *BoolToPrintString(MuteUser), *Reason);
	FString SetMembersString = FString::Printf(TEXT("[{\"uuid\": {\"id\": \"%s\"}, \"custom\": %s}]"), *UserID, *ParamsString);
	UE_LOG(PubnubLog, Warning, TEXT("Set members object: %s"), *SetMembersString);
	PubnubSubsystem->SetChannelMembers(RestrictionsChannel, "", SetMembersString);
	FString RestrictionText;
	BanUser ? RestrictionText = "banned" : "muted";
	FString EventPayloadString = FString::Printf(TEXT("{\"channelId\": \"%s\", \"restriction\": \"%s\", \"reason\": \"%s\"}"), *RestrictionsChannel, *RestrictionText, *Reason);
	EmitChatEvent( EPubnubChatEventType::PCET_Moderation, UserID, EventPayloadString);
	
}

void UPubnubChatSystem::EditMessage(FString ChannelName, FString MessageTimeToken, FString EditedMessage)
{
	if(!CheckIsChatInitialized())
	{return;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(MessageTimeToken, "MessageTimeToken", "EditMessage") || PubnubSubsystem->CheckIsFieldEmpty(EditedMessage, "EditedMessage", "EditMessage")
		|| PubnubSubsystem->CheckIsFieldEmpty(ChannelName, "ChannelName", "EditMessage"))
	{return;}

	PubnubSubsystem->AddMessageAction(ChannelName, MessageTimeToken, EPubnubActionType::pbactypEdited, EditedMessage);
}

void UPubnubChatSystem::DeleteMessage(FString ChannelName, FString MessageTimeToken)
{
	if(!CheckIsChatInitialized())
	{return;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(MessageTimeToken, "MessageTimeToken", "DeleteMessage") || PubnubSubsystem->CheckIsFieldEmpty(ChannelName, "ChannelName", "DeleteMessage"))
	{return;}

	FString DeletedValue = "\"deleted\"";
	
	PubnubSubsystem->AddMessageAction(ChannelName, MessageTimeToken, EPubnubActionType::pbactypDeleted, DeletedValue);
}

FString UPubnubChatSystem::GetStringFromChatEventType(EPubnubChatEventType ChatEventType)
{
	switch(ChatEventType)
	{
	case EPubnubChatEventType::PCET_Typing:
		return "typing";
	case EPubnubChatEventType::PCET_Report:
		return "report";
	case EPubnubChatEventType::PCET_Receipt:
		return "receipt";
	case EPubnubChatEventType::PCET_Mention:
		return "mention";
	case EPubnubChatEventType::PCET_Invite:
		return "invite";
	case EPubnubChatEventType::PCET_Custom:
		return "custom";
	case EPubnubChatEventType::PCET_Moderation:
		return "moderation";
	}
	return "custom";
}

void UPubnubChatSystem::SubscribeToChannel(FString ChannelID)
{
	if(!CheckIsChatInitialized())
	{return;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(ChannelID, "ChannelID", "SubscribeToChannel"))
	{return;}

	PubnubSubsystem->SubscribeToChannel(ChannelID);
}

void UPubnubChatSystem::UnsubscribeFromChannel(FString ChannelID)
{
	if(!CheckIsChatInitialized())
	{return;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(ChannelID, "ChannelID", "UnsubscribeFromChannel"))
	{return;}

	PubnubSubsystem->UnsubscribeFromChannel(ChannelID);
}

UPubnubSubsystem* UPubnubChatSystem::GetPubnubSubsystem()
{
	return PubnubSubsystem;
}

void UPubnubChatSystem::EmitChatEvent(EPubnubChatEventType EventType, FString ChannelName, FString Payload)
{
	//Payload is in form of Json: {"param1": "param1value", "param2": "param2value" ... }. So in order to get just parameters, we remove first and last curl bracket
	FString PayloadParameters = Payload;
	PayloadParameters.LeftChopInline(1);
	PayloadParameters.RightChopInline(1);
	FString EventMessage = FString::Printf(TEXT("{%s, \"type\": \"%s\"}"), *PayloadParameters, *GetStringFromChatEventType(EventType));
	PubnubSubsystem->PublishMessage(ChannelName, EventMessage);
}


void UPubnubChatSystem::InitChatSystem(UPubnubSubsystem* PubnubSubsystemRef)
{
	PubnubSubsystem = PubnubSubsystemRef;
	IsInitialized = true;
}

void UPubnubChatSystem::DeinitChatSystem()
{
	IsInitialized = false;
}

//This functions is a wrapper to IsInitialized bool, so it can print error if user is trying to do anything before initializing Pubnub correctly
bool UPubnubChatSystem::CheckIsChatInitialized()
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("Pubnub Chat System in not initialized. Aborting operation."));
	}
	
	return IsInitialized;
}

void UPubnubChatSystem::OnGetChannelResponseReceived(FString JsonResponse)
{
	if(JsonResponse.IsEmpty())
	{
		GetChannelResponse.ExecuteIfBound(nullptr);
	}
	
	//Convert response to Json
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	if(!UPubnubUtilities::StringToJsonObject(JsonResponse, JsonObject))
	{
		UE_LOG(PubnubLog, Error, TEXT("OnGetChannelResponseReceived - String to Json convertion failed"));
		GetChannelResponse.ExecuteIfBound(nullptr);
	}

	//Create new Json object just with "data" field.
	TSharedPtr<FJsonObject> JsonDataField = JsonObject->GetObjectField("data");

	//Conver "data" field to string
	FString DataString = UPubnubUtilities::JsonObjectToString(JsonDataField);
	
	UPubnubChatChannel* ChannelObject = NewObject<UPubnubChatChannel>(this);
	ChannelObject->InitializeWithJsonData(this, JsonDataField->GetStringField("id"), DataString);
	
	GetChannelResponse.ExecuteIfBound(ChannelObject);
}

void UPubnubChatSystem::OnGetUserResponseReceived(FString JsonResponse)
{
	if(JsonResponse.IsEmpty())
	{
		GetUserResponse.ExecuteIfBound(nullptr);
	}
	
	//Convert response to Json
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	if(!UPubnubUtilities::StringToJsonObject(JsonResponse, JsonObject))
	{
		UE_LOG(PubnubLog, Error, TEXT("OnGetUserResponseReceived - String to Json convertion failed"));
		GetUserResponse.ExecuteIfBound(nullptr);
	}

	//Create new Json object just with "data" field.
	TSharedPtr<FJsonObject> JsonDataField = JsonObject->GetObjectField("data");

	//Conver "data" field to string
	FString DataString = UPubnubUtilities::JsonObjectToString(JsonDataField);

	UPubnubChatUser* UserObject = NewObject<UPubnubChatUser>(this);
	UserObject->InitializeWithJsonData(this, JsonDataField->GetStringField("id"), DataString);
	
	GetUserResponse.ExecuteIfBound(UserObject);
}
