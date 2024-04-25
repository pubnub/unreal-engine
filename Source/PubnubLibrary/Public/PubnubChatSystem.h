// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PubnubStructLibrary.h"
#include "PubnubEnumLibrary.h"
#include "PubnubChatSystem.generated.h"

class UPubnubSubsystem;

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

UCLASS()
class PUBNUBLIBRARY_API UPubnubChatSystem : public UObject
{
public:
	GENERATED_BODY()
	
	//Initialize Chat System object and save PubnubSystem reference.
	void InitChatSystem(UPubnubSubsystem* PubnubSubsystemRef);
	void DeinitChatSystem();

	UFUNCTION(BlueprintCallable, Category = "Pubnub|ChatSystem")
	void CreateChannel(FString ChannelName, FString ChannelData);

	UFUNCTION(BlueprintCallable, Category = "Pubnub|ChatSystem")
	void JoinChannel(FString ChannelName);
	
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

	UFUNCTION(BlueprintCallable, Category = "Pubnub|ChatSystem")
	void SetRestrictions(FString UserID, FString ChannelName, bool BanUser, bool MuteUser, FString Reason = "");

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pubnub|ChatSystem|Helpers")
	FString GetStringFromChatEventType(EPubnubChatEventType ChatEventType);
	
private:
	UPROPERTY()
	UPubnubSubsystem* PubnubSubsystem;

	void EmitChatEvent(EPubnubChatEventType EventType, FString ChannelName, FString Payload);

	bool IsInitialized;
	bool CheckIsChatInitialized();

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
