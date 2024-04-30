// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PubnubChatChannel.generated.h"

class UPubnubChatSystem;
class UPubnubSubsystem;
class UPubnubChatUser;


USTRUCT(BlueprintType)
struct FPubnubChatChannelData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString ChannelName;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString Description;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString CustomDataJson;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString Updated;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString Status;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString Type;
};

/**
 * 
 */
UCLASS(BlueprintType)
class PUBNUBLIBRARY_API UPubnubChatChannel : public UObject
{
	GENERATED_BODY()
	
public:
	
	void Initialize(UPubnubChatSystem* InChatSystem, FString InChannelID, FPubnubChatChannelData InAdditionalChannelData);
	void InitializeWithJsonData(UPubnubChatSystem* InChatSystem, FString InChannelID, FString JsonData);
	
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Channel")
	FString ChannelID;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Channel")
	FPubnubChatChannelData ChannelData;

	//Only listen for chat messages, don't join as a user
	UFUNCTION(BlueprintCallable, Category = "Channel")
	void Connect();

	UFUNCTION(BlueprintCallable, Category = "Channel")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "Channel")
	void Join(FString AdditionalParams);

	UFUNCTION(BlueprintCallable, Category = "Channel")
	void Leave();

	UFUNCTION(BlueprintCallable, Category = "Channel")
	void Delete();

	UFUNCTION(BlueprintCallable, Category = "Channel")
	void Update(FPubnubChatChannelData InAdditionalChannelData);

private:
	bool IsInitialized = false;
	UPubnubChatSystem* ChatSystem = nullptr;
	UPubnubSubsystem* PubnubSubsystem = nullptr;

	void AddChannelDataToJson(TSharedPtr<FJsonObject> &MetadataJsonObject, FString InChannelID, FPubnubChatChannelData InAdditionalChannelData);
	FPubnubChatChannelData ChannelDataFromJson(FString JsonData);
};
