// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

// snippet.includes
#include "PubnubClient.h"

// snippet.end

#include "CoreMinimal.h"
#include "PubnubSampleBase.h"
#include "Sample_AppContext.generated.h"


UCLASS()
class PUBNUBLIBRARYTESTS_API ASample_AppContext : public APubnubSampleBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples")
	void RunSamples() override;

	ASample_AppContext();
	
	/* SAMPLE FUNCTIONS */
	
	// snippet.set_user_metadata
	// blueprint.3budd992
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetUserMetadataSample();

	// snippet.set_user_metadata_with_result
	// blueprint.w9k9he3e
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetUserMetadataWithResultSample();

	UFUNCTION()
	void OnSetUserMetadataResponse(FPubnubOperationResult Result, FPubnubUserData UserData);

	// snippet.set_user_metadata_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetUserMetadataWithLambdaSample();

	// snippet.set_user_metadata_raw
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetUserMetadataRawSample();

	UFUNCTION()
	void OnSetUserMetadataRawResponse(FPubnubOperationResult Result, FPubnubUserData UserData);

	//snippet.update_user_metadata_iteratively
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void UpdateUserMetadataIterativelySample();
	
	UFUNCTION()
	void OnInitialSetUserMetadataResponse(FPubnubOperationResult Result, FPubnubUserData UserData);
	
	UFUNCTION()
	void OnUpdateUserMetadataResponse(FPubnubOperationResult Result, FPubnubUserData UserData);

	// snippet.get_all_user_metadata
	// blueprint.sarms73o
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetAllUserMetadataSample();

	UFUNCTION()
	void OnGetAllUserMetadataResponse_Simple(FPubnubOperationResult Result, const TArray<FPubnubUserData>& UsersData, FPubnubPage Page, int TotalCount);

	// snippet.get_all_user_metadata_with_settings
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetAllUserMetadataWithSettingsSample();

	UFUNCTION()
	void OnGetAllUserMetadataResponse_WithSettings(FPubnubOperationResult Result, const TArray<FPubnubUserData>& UsersData, FPubnubPage Page, int TotalCount);

	// snippet.get_all_user_metadata_with_all_includes
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetAllUserMetadataWithAllIncludesSample();

	UFUNCTION()
	void OnGetAllUserMetadataResponse_WithAllIncludes(FPubnubOperationResult Result, const TArray<FPubnubUserData>& UsersData, FPubnubPage Page, int TotalCount);

	// snippet.get_all_user_metadata_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetAllUserMetadataWithLambdaSample();

	// snippet.get_all_user_metadata_raw
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetAllUserMetadataRawSample();

	UFUNCTION()
	void OnGetAllUserMetadataRawResponse(FPubnubOperationResult Result, const TArray<FPubnubUserData>& UsersData, FPubnubPage Page, int TotalCount);

	// snippet.get_user_metadata
	// blueprint.2jr6qjos
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetUserMetadataSample();

	UFUNCTION()
	void OnGetUserMetadataResponse_Simple(FPubnubOperationResult Result, FPubnubUserData UserData);

	// snippet.get_user_metadata_with_all_includes
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetUserMetadataWithAllIncludesSample();

	UFUNCTION()
	void OnGetUserMetadataResponse_WithAllIncludes(FPubnubOperationResult Result, FPubnubUserData UserData);

	// snippet.get_user_metadata_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetUserMetadataWithLambdaSample();

	// snippet.get_user_metadata_raw
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetUserMetadataRawSample();

	UFUNCTION()
	void OnGetUserMetadataRawResponse(FPubnubOperationResult Result, FPubnubUserData UserData);

	// snippet.remove_user_metadata
	// blueprint.ugk9ll3z
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void RemoveUserMetadataSample();

	// snippet.remove_user_metadata_with_result
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void RemoveUserMetadataWithResultSample();

	UFUNCTION()
	void OnRemoveUserMetadataResponse(FPubnubOperationResult Result);

	// snippet.remove_user_metadata_with_result_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void RemoveUserMetadataWithResultLambdaSample();

	// snippet.set_channel_metadata
	// blueprint.3mxkam_d
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetChannelMetadataSample();

	// snippet.set_channel_metadata_with_result
	// blueprint.fr0nokir
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetChannelMetadataWithResultSample();

	UFUNCTION()
	void OnSetChannelMetadataResponse(FPubnubOperationResult Result, FPubnubChannelData ChannelData);

	// snippet.set_channel_metadata_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetChannelMetadataWithLambdaSample();

	// snippet.set_channel_metadata_raw
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void SetChannelMetadataRawSample();
	
    //snippet.update_channel_metadata_iteratively
    UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
    void UpdateChannelMetadataIterativelySample();
    
    UFUNCTION()
    void OnInitialSetChannelMetadataResponse(FPubnubOperationResult Result, FPubnubChannelData ChannelData);
    
    UFUNCTION()
    void OnUpdateChannelMetadataResponse(FPubnubOperationResult Result, FPubnubChannelData ChannelData);
    

	UFUNCTION()
	void OnSetChannelMetadataRawResponse(FPubnubOperationResult Result, FPubnubChannelData ChannelData);

	// snippet.get_all_channel_metadata
	// blueprint.s3xd4eok
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetAllChannelMetadataSample();

	UFUNCTION()
	void OnGetAllChannelMetadataResponse_Simple(FPubnubOperationResult Result, const TArray<FPubnubChannelData>& ChannelsData, FPubnubPage Page, int TotalCount);

	// snippet.get_all_channel_metadata_with_settings
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetAllChannelMetadataWithSettingsSample();

	UFUNCTION()
	void OnGetAllChannelMetadataResponse_WithSettings(FPubnubOperationResult Result, const TArray<FPubnubChannelData>& ChannelsData, FPubnubPage Page, int TotalCount);

	// snippet.get_all_channel_metadata_with_all_includes
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetAllChannelMetadataWithAllIncludesSample();

	UFUNCTION()
	void OnGetAllChannelMetadataResponse_WithAllIncludes(FPubnubOperationResult Result, const TArray<FPubnubChannelData>& ChannelsData, FPubnubPage Page, int TotalCount);

	// snippet.get_all_channel_metadata_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetAllChannelMetadataWithLambdaSample();

	// snippet.get_all_channel_metadata_raw
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetAllChannelMetadataRawSample();

	UFUNCTION()
	void OnGetAllChannelMetadataRawResponse(FPubnubOperationResult Result, const TArray<FPubnubChannelData>& ChannelsData, FPubnubPage Page, int TotalCount);

	// snippet.get_channel_metadata
	// blueprint.rsj2c5zl
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetChannelMetadataSample();

	UFUNCTION()
	void OnGetChannelMetadataResponse_Simple(FPubnubOperationResult Result, FPubnubChannelData ChannelData);

	// snippet.get_channel_metadata_with_all_includes
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetChannelMetadataWithAllIncludesSample();

	UFUNCTION()
	void OnGetChannelMetadataResponse_WithAllIncludes(FPubnubOperationResult Result, FPubnubChannelData ChannelData);

	// snippet.get_channel_metadata_with_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetChannelMetadataWithLambdaSample();

	// snippet.get_channel_metadata_raw
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void GetChannelMetadataRawSample();

	UFUNCTION()
	void OnGetChannelMetadataRawResponse(FPubnubOperationResult Result, FPubnubChannelData ChannelData);

	// snippet.remove_channel_metadata
	// blueprint.k80afln-
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void RemoveChannelMetadataSample();

	// snippet.remove_channel_metadata_with_result
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void RemoveChannelMetadataWithResultSample();

	UFUNCTION()
	void OnRemoveChannelMetadataResponse(FPubnubOperationResult Result);

	// snippet.remove_channel_metadata_with_result_lambda
	UFUNCTION(BlueprintCallable, Category = "Pubnub|Samples|App Context")
	void RemoveChannelMetadataWithResultLambdaSample();

	// snippet.end
	
private:
	UPubnubClient* GetPubnubClient();
};


