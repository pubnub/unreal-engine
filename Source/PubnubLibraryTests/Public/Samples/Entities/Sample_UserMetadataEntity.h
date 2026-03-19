// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubClient.h"
#include "Entities/PubnubUserMetadataEntity.h"
#include "Entities/PubnubSubscription.h"

// snippet.end

#include "CoreMinimal.h"
#include "Samples/PubnubSampleBase.h"
#include "Sample_UserMetadataEntity.generated.h"


UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_UserMetadataEntity : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_UserMetadataEntity();

	
	/* USER METADATA ENTITY SAMPLE FUNCTIONS */
	
    // snippet.create_user_metadata_entity
    // blueprint.4kgvihsx
    UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|UserMetadataEntity")
    void CreateUserMetadataEntitySample();

	// snippet.subscribe_with_user_metadata_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|UserMetadataEntity")
	void SubscribeWithUserMetadataEntitySample();

	UFUNCTION()
	void OnObjectEvent_UserMetadataEntitySample(FPubnubMessageData Message);

	// snippet.user_metadata_entity_set_metadata
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|UserMetadataEntity")
	void UserMetadataEntitySetMetadataSample();

	UFUNCTION()
	void OnSetUserMetadataResult_Sample(FPubnubOperationResult Result, FPubnubUserData UserData);

	// snippet.set_user_metadata_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|UserMetadataEntity")
	void SetUserMetadataSample();

	// snippet.set_user_metadata_with_result_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|UserMetadataEntity")
	void SetUserMetadataWithResultSample();

	UFUNCTION()
	void OnSetUserMetadataResponse(FPubnubOperationResult Result, FPubnubUserData UserData);

	// snippet.set_user_metadata_with_lambda_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|UserMetadataEntity")
	void SetUserMetadataWithLambdaSample();

	// snippet.get_user_metadata_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|UserMetadataEntity")
	void GetUserMetadataSample();

	UFUNCTION()
	void OnGetUserMetadataResponse_Simple(FPubnubOperationResult Result, FPubnubUserData UserData);

	// snippet.get_user_metadata_with_lambda_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|UserMetadataEntity")
	void GetUserMetadataWithLambdaSample();

	// snippet.remove_user_metadata_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|UserMetadataEntity")
	void RemoveUserMetadataSample();

	// snippet.remove_user_metadata_with_result_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|UserMetadataEntity")
	void RemoveUserMetadataWithResultSample();

	UFUNCTION()
	void OnRemoveUserMetadataResponse(FPubnubOperationResult Result);

	// snippet.remove_user_metadata_with_result_lambda_entity
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|UserMetadataEntity")
	void RemoveUserMetadataWithResultLambdaSample();

	
	// snippet.end
	
private:
	UPubnubClient* GetPubnubClient();
};
