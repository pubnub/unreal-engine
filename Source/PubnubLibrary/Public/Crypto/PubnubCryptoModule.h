// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PubnubCryptoModule.generated.h"


struct pubnub_crypto_provider_t;

/**
 * 
 */
UCLASS(Blueprintable)
class PUBNUBLIBRARY_API UPubnubCryptoModule : public UObject
{
	GENERATED_BODY()

	//Allow PubnubSubsystem to use private members from the subsystem
	friend class UPubnubSubsystem;

public:
	UFUNCTION(BlueprintCallable, Category="Pubnub|Crypto")
	virtual void Initialize(FString InCipherKey);
	
	UFUNCTION(BlueprintCallable, Category="Pubnub|Crypto")
	virtual FString Encrypt(FString Data, FString CustomCipherKey = "");
	
	UFUNCTION(BlueprintCallable, Category="Pubnub|Crypto")
	virtual FString Decrypt(FString Data, FString CustomCipherKey = "");

	UFUNCTION(BlueprintCallable, Category="Pubnub|Crypto")
	virtual void SetCipherKey(FString InCipherKey);

protected:
	FString CipherKey = "";
	bool IsInitialized = false;

private:
	//These variables are used for C-Core (Pubnub) implementation of Crypto Module.
	//Should not be used when providing custom encryption functionality.
	int32 KeyLength = 0;
	uint8_t* PersistentKey = nullptr;
	pubnub_crypto_provider_t *crypto_module = nullptr;
	
};
