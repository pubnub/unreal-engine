// Fill out your copyright notice in the Description page of Project Settings.

#include "PubnubChatSystem.h"

#include "PubnubSubsystem.h"


void UPubnubChatSystem::CreateChannel(FString ChannelName, FString ChannelData)
{
	if(!CheckIsChatInitialized())
	{return;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(ChannelName, "ChannelName", "CreateChannel"))
	{return;}
	
	PubnubSubsystem->SetChannelMetadata(ChannelName, "", ChannelData.IsEmpty() ? "{}" : ChannelData);
}

void UPubnubChatSystem::JoinChannel(FString ChannelName)
{
	if(!CheckIsChatInitialized())
	{return;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(ChannelName, "ChannelName", "JoinChannel"))
	{return;}

	PubnubSubsystem->SubscribeToChannel(ChannelName);
}

void UPubnubChatSystem::SendChatMessage(FString ChannelName, FString Message, EPubnubChatMessageType MessageType, FString MetaData)
{
	if(!CheckIsChatInitialized())
	{return;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(ChannelName, "ChannelName", "SendChatMessage") || PubnubSubsystem->CheckIsFieldEmpty(Message, "Message", "SendChatMessage"))
	{return;}

	FPubnubPublishSettings PublishSettings;
	PublishSettings.MetaData = MetaData;
	
	PubnubSubsystem->PublishMessage(ChannelName, ChatMessageToPublishString(Message, MessageType), PublishSettings);
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

void UPubnubChatSystem::SetRestrictions(FString UserID, FString ChannelName, bool BanUser, bool MuteUser, FString Reason)
{
	if(!CheckIsChatInitialized())
	{return;}
	
	if(PubnubSubsystem->CheckIsFieldEmpty(UserID, "UserID", "SetRestrictions") || PubnubSubsystem->CheckIsFieldEmpty(ChannelName, "ChannelName", "SetRestrictions"))
	{return;}

	//Restrictions are held in new channel with ID: PUBNUB_INTERNAL_MODERATION_{ChannelName}
	FString RestrictionsChannel = InternalModerationPrefix;
	RestrictionsChannel.Append(ChannelName);

	//Lift restrictions
	if(!BanUser && !MuteUser)
	{
		FString RemoveMemberString = FString::Printf(TEXT("[{\"uuid\": {\"id\": \"%s\"}}]"), *UserID);
		PubnubSubsystem->RemoveMembers(RestrictionsChannel, "", RemoveMemberString);
		FString EventPayloadString = FString::Printf(TEXT("{\"channelId\": \"%s\", \"restriction\": \"lifted\", \"reason\": \"%s\"}"), *RestrictionsChannel, *Reason);
		EmitChatEvent(EPubnubChatEventType::PCET_Moderation, UserID, EventPayloadString);
		return;
	}

	//Ban or mute the user
	FString ParamsString = FString::Printf(TEXT("{\"ban\": %s, \"mute\": %s, \"reason\": \"%s\"}"), *BoolToPrintString(BanUser), *BoolToPrintString(MuteUser), *Reason);
	FString SetMembersString = FString::Printf(TEXT("[{\"uuid\": {\"id\": \"%s\"}, \"custom\": %s}]"), *UserID, *ParamsString);
	UE_LOG(PubnubLog, Warning, TEXT("Set members object: %s"), *SetMembersString);
	PubnubSubsystem->SetMembers(RestrictionsChannel, "", SetMembersString);
	FString RestrictionText;
	BanUser ? RestrictionText = "banned" : "muted";
	FString EventPayloadString = FString::Printf(TEXT("{\"channelId\": \"%s\", \"restriction\": \"%s\", \"reason\": \"%s\"}"), *RestrictionsChannel, *RestrictionText, *Reason);
	EmitChatEvent( EPubnubChatEventType::PCET_Moderation, UserID, EventPayloadString);
	
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

FString UPubnubChatSystem::ChatMessageToPublishString(FString Message, EPubnubChatMessageType MessageType)
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
	FString FinalJsonString;
	TSharedRef< TJsonWriter<> > JsonWriter = TJsonWriterFactory<>::Create(&FinalJsonString);
	FJsonSerializer::Serialize(MessageJsonObject.ToSharedRef(), JsonWriter);

	return FinalJsonString;
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