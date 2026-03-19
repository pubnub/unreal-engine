// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubClient.h"

// snippet.end

#include "CoreMinimal.h"
#include "PubnubSampleBase.h"
#include "Sample_AccessManager.generated.h"


UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_AccessManager : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_AccessManager();
	

	
	/* SAMPLE FUNCTIONS */
	
	
	// snippet.grant_token
	// blueprint.loxab2ag
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Access Manager")
	void GrantTokenSample();

	UFUNCTION()
	void OnGrantTokenResponse_Simple(FPubnubOperationResult Result, FString Token);

	// snippet.grant_token_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Access Manager")
	void GrantTokenWithLambdaSample();

	// snippet.grant_token_various_resources
	// blueprint.wd_k_rfh
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Access Manager")
	void GrantTokenVariousResourcesSample();

	UFUNCTION()
	void OnGrantTokenResponse_VariousResources(FPubnubOperationResult Result, FString Token);

	// snippet.grant_token_regex
	// blueprint.n98qjkbo
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Access Manager")
	void GrantTokenRegexSample();

	UFUNCTION()
	void OnGrantTokenResponse_Regex(FPubnubOperationResult Result, FString Token);
	
	// snippet.grant_token_complex
	// blueprint.ii4i6u3z
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Access Manager")
	void GrantTokenComplexSample();

	UFUNCTION()
	void OnGrantTokenResponse_Complex(FPubnubOperationResult Result, FString Token);
	
	// snippet.revoke_token
	// blueprint.pkctxqyv
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Access Manager")
	void RevokeTokenSample();

	// snippet.revoke_token_with_result
	// blueprint.430fo6h-
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Access Manager")
	void RevokeTokenWithResultSample();

	UFUNCTION()
	void OnRevokeTokenResponse(FPubnubOperationResult Result);

	// snippet.revoke_token_with_result_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Access Manager")
	void RevokeTokenWithResultLambdaSample();

	// snippet.parse_token
	// blueprint.3y5pxgqf
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Access Manager")
	void ParseTokenSample();

	// snippet.set_auth_token
	// blueprint.4gkh9q7x
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|Access Manager")
	void SetAuthTokenSample();
	
	// snippet.end
	
private:
	UPubnubClient* GetPubnubClient();
};


