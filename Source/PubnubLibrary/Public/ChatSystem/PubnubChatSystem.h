// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PubnubStructLibrary.h"
#include "PubnubEnumLibrary.h"
#include "PubnubSubsystem.h"
#include "ChatSystem/PubnubChatUser.h"
#include "ChatSystem/PubnubChatChannel.h"
#include "PubnubChatSystem.generated.h"


UENUM(BlueprintType)
enum class EPubnubChatEventType : uint8
{
	PCET_Typing				UMETA(DisplayName="Typing"),
	PCET_Report				UMETA(DisplayName="Report"),
	PCET_Receipt			UMETA(DisplayName="Receipt"),
	PCET_Mention			UMETA(DisplayName="Mention"),
	PCET_Invite				UMETA(DisplayName="Invite"),
	PCET_Custom				UMETA(DisplayName="Custom"),
	PCET_Moderation			UMETA(DisplayName="Moderation")
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetChannelResponse, UPubnubChatChannel*, Channel);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetUserResponse, UPubnubChatUser*, User);

UCLASS()
class PUBNUBLIBRARY_API UPubnubChatSystem : public UObject
{
public:
	GENERATED_BODY()
	
	//Initialize Chat System object and save PubnubSystem reference.
	void InitChatSystem(UPubnubSubsystem* PubnubSubsystemRef);
	void DeinitChatSystem();

	/* CHANNELS */
	
	UFUNCTION(BlueprintCallable, Category = "ChatSystem|Channels")
	UPubnubChatChannel* CreatePublicConversation(FString ChannelID, FPubnubChatChannelData AdditionalChannelData);

	UFUNCTION(BlueprintCallable, Category = "ChatSystem|Channels")
	void GetChannel(FString ChannelID, FOnGetChannelResponse OnGetChannelResponse);

	UFUNCTION(BlueprintCallable, Category = "ChatSystem|Channels")
	UPubnubChatChannel* UpdateChannel(FString ChannelID, FPubnubChatChannelData AdditionalChannelData);
	
	UFUNCTION(BlueprintCallable, Category = "ChatSystem|Channels")
	void DeleteChannel(FString ChannelID);

	/* USERS */

	UFUNCTION(BlueprintCallable, Category = "ChatSystem|Users")
	UPubnubChatUser* CreateUser(FString UserID, FPubnubChatUserData AdditionalUserData);

	UFUNCTION(BlueprintCallable, Category = "ChatSystem|Channels")
	void GetUser(FString UserID, FOnGetUserResponse OnGetUserResponse);

	UFUNCTION(BlueprintCallable, Category = "ChatSystem|Users")
	UPubnubChatUser* UpdateUser(FString UserID, FPubnubChatUserData AdditionalUserData);

	UFUNCTION(BlueprintCallable, Category = "ChatSystem|Users")
	void DeleteUser(FString UserID);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|User")
	void SetRestrictions(FString UserID, FString ChannelID, bool BanUser, bool MuteUser, FString Reason = "");
	

	/* Messages */
	
	/**
	 * Send a chat message to given channel.
	 * @param ChannelName Name of the channel to send message to
	 * @param Message Content of the message (doesn't need to be in JSON format)
	 * @param MessageType Type of the message - currently only 1 type is supported
	 * @param MetaData Additional message Data - has to be in JSON format
	 */
	UFUNCTION(BlueprintCallable, Category = "Pubnub|ChatSystem")
	void SendChatMessage(FString ChannelName, FString Message, EPubnubChatMessageType MessageType, FString MetaData);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|ChatSystem")
	void EditMessage(FString ChannelName, FString MessageTimeToken, FString EditedMessage);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|ChatSystem")
	void DeleteMessage(FString ChannelName, FString MessageTimeToken);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pubnub|ChatSystem|Helpers")
	FString GetStringFromChatEventType(EPubnubChatEventType ChatEventType);
	
	void SubscribeToChannel(FString ChannelID);
	void UnsubscribeFromChannel(FString ChannelID);
	UPubnubSubsystem* GetPubnubSubsystem();
	
private:
	UPROPERTY()
	UPubnubSubsystem* PubnubSubsystem;

	void EmitChatEvent(EPubnubChatEventType EventType, FString ChannelName, FString Payload);

	bool IsInitialized;
	bool CheckIsChatInitialized();

	UPubnubChatUser* ChatUser = nullptr;
	
	UPROPERTY()
	FOnGetChannelResponse GetChannelResponse;
	
	UPROPERTY()
	FOnGetUserResponse GetUserResponse;
	
	UFUNCTION()
	void OnGetChannelResponseReceived(FString JsonResponse);

	UFUNCTION()
	void OnGetUserResponseReceived(FString JsonResponse);
	
	FString ChatMessageToPublishString(FString Message, EPubnubChatMessageType MessageType);
	
	inline static const FString InternalModerationPrefix = "PUBNUB_INTERNAL_MODERATION_";

	static FString BoolToPrintString(bool Value)
	{
		if(Value)
		{
			return "true";
		}
		return "false";
	}
};
