// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PubnubChatUser.generated.h"

class UPubnubChatSystem;
class UPubnubSubsystem;

USTRUCT(BlueprintType)
struct FPubnubChatUserData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString UserName;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString ExternalID;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString ProfileUrl;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString Email;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString CustomDataJson;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString Status;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FString Type;
};


/**
 * 
 */
UCLASS(BlueprintType)
class PUBNUBLIBRARY_API UPubnubChatUser : public UObject
{
	GENERATED_BODY()
	
public:
	void Initialize(UPubnubChatSystem* InChatSystem, FString InUserID, FPubnubChatUserData InAdditionalUserData);
	void InitializeWithJsonData(UPubnubChatSystem* InChatSystem, FString InUserID, FString JsonData);

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "User")
	FString UserID;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "User")
	FPubnubChatUserData UserData;

	UFUNCTION(BlueprintCallable, Category = "User")
	void Update(FPubnubChatUserData InAdditionalUserData);

	UFUNCTION(BlueprintCallable, Category = "User")
	void Delete();

private:
	bool IsInitialized = false;
	UPubnubChatSystem* ChatSystem = nullptr;
	UPubnubSubsystem* PubnubSubsystem = nullptr;

	void AddUserDataToJson(TSharedPtr<FJsonObject> &MetadataJsonObject, FString UserID, FPubnubChatUserData AdditionalUserData);
	FPubnubChatUserData UserDataFromJson(FString JsonData);
};
