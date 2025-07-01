// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "Samples/Sample_AccessManager.h"
// snippet.includes
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

// snippet.end

/**
 * NOTE: Each sample is designed to be fully self-contained and portable. 
 * You can copy-paste any individual sample into a new project, and it should compile and run without errors 
 * — as long as you also include the necessary `#include` statements.
 *
 * To ensure independence, each sample retrieves the PubnubSubsystem and explicitly calls `SetUserID()` 
 * before performing any PubNub operations.
 *
 * In a real project, however, you only need to call `SetUserID()` once — typically during initialization 
 * (e.g., in GameInstance or at login) before making your first PubNub request.
 * 
 * The samples assume that in Pubnub SDK settings sections in ProjectSettings following fields are set:
 * PublishKey and SubscribeKey have correct keys, InitializeAutomatically is true.
 */

// NOTE: Comments marked with `ACTION REQUIRED` indicate lines you must change.


//Internal function, don't copy it with the samples
void ASample_AccessManager::RunSamples()
{
	Super::RunSamples();
	
	GrantTokenSample();
	GrantTokenWithLambdaSample();
	GrantTokenComplexSample();
	RevokeTokenSample();
	RevokeTokenWithResultSample();
	RevokeTokenWithResultLambdaSample();
	ParseTokenSample();
	SetAuthTokenSample();
}
//Internal function, don't copy it with the samples
ASample_AccessManager::ASample_AccessManager()
{
	SamplesName = "AccessManager";
}


/* SAMPLE FUNCTIONS */

// snippet.grant_token
// ACTION REQUIRED: Replace ASample_AccessManager with name of your Actor class
void ASample_AccessManager::GrantTokenSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Set secret key to be able to grant token
	//Make sure proper SecretKey is set in PubnubSDK PluginSettings
	//This is not required if "SetSecretKeyAutomatically" is set to true in PubnubSDK PluginSettings
	PubnubSubsystem->SetSecretKey();

	// Create the token structure
	FPubnubGrantTokenStructure TokenStructure;
	TokenStructure.TTLMinutes = 60;
	TokenStructure.AuthorizedUser = UserID;

	//Add Read and Write permissions to "global_chat" channel
	FPubnubChannelPermissions ChannelPermissions;
	ChannelPermissions.Read = true;
	ChannelPermissions.Write = true;
	TokenStructure.Channels.Add("global_chat");
	TokenStructure.ChannelPermissions.Add(ChannelPermissions);

	// Convert structure to JSON string permission object
	bool bSuccess;
	FString PermissionObject = PubnubSubsystem->GrantTokenStructureToJsonString(TokenStructure, bSuccess);

	if (bSuccess)
	{
		// Bind response delegate
		FOnGrantTokenResponse OnGrantTokenResponse;
		OnGrantTokenResponse.BindDynamic(this, &ASample_AccessManager::OnGrantTokenResponse_Simple);
		
		// Request the token from the server
		PubnubSubsystem->GrantToken(PermissionObject, OnGrantTokenResponse);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create token structure JSON."));
	}
}

// ACTION REQUIRED: Replace ASample_AccessManager with name of your Actor class
void ASample_AccessManager::OnGrantTokenResponse_Simple(FPubnubOperationResult Result, FString Token)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to Grant Token. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Grant Token Success. The Token: %s"), *Token);
	}
}

// snippet.grant_token_with_lambda
// ACTION REQUIRED: Replace ASample_AccessManager with name of your Actor class
void ASample_AccessManager::GrantTokenWithLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);
	
	//Set secret key to be able to grant token
	//Make sure proper SecretKey is set in PubnubSDK PluginSettings
	//This is not required if "SetSecretKeyAutomatically" is set to true in PubnubSDK PluginSettings
	PubnubSubsystem->SetSecretKey();

	// Create the token structure
	FPubnubGrantTokenStructure TokenStructure;
	TokenStructure.TTLMinutes = 60;
	TokenStructure.AuthorizedUser = UserID;

	//Add Read and Write permissions to "global_chat" channel
	FPubnubChannelPermissions ChannelPermissions;
	ChannelPermissions.Read = true;
	ChannelPermissions.Write = true;
	TokenStructure.Channels.Add("global_chat");
	TokenStructure.ChannelPermissions.Add(ChannelPermissions);

	// Convert structure to JSON string
	bool bSuccess;
	FString PermissionObject = PubnubSubsystem->GrantTokenStructureToJsonString(TokenStructure, bSuccess);

	if (bSuccess)
	{
		// Bind lambda to response delegate
		FOnGrantTokenResponseNative OnGrantTokenResponse;
		OnGrantTokenResponse.BindLambda([](const FPubnubOperationResult& Result, FString Token)
		{
			if(Result.Error)
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to Grant Token. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Grant Token Success. The Token: %s"), *Token);
			}
		});
		
		// Request the token from the server
		PubnubSubsystem->GrantToken(PermissionObject, OnGrantTokenResponse);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create token structure JSON."));
	}
}

// snippet.grant_token_complex
// ACTION REQUIRED: Replace ASample_AccessManager with name of your Actor class
void ASample_AccessManager::GrantTokenComplexSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Set secret key to be able to grant token
	//Make sure proper SecretKey is set in PubnubSDK PluginSettings
	//This is not required if "SetSecretKeyAutomatically" is set to true in PubnubSDK PluginSettings
	PubnubSubsystem->SetSecretKey();

	// Create the token structure
	FPubnubGrantTokenStructure TokenStructure;
	TokenStructure.TTLMinutes = 1440;
	TokenStructure.AuthorizedUser = UserID;

	//Add Read permission to channel "channel-a"
	TokenStructure.Channels.Add("channel-a");
	FPubnubChannelPermissions ChannelAReadPermission;
	ChannelAReadPermission.Read = true;
	TokenStructure.ChannelPermissions.Add(ChannelAReadPermission);

	//Add Read permission to group "channel-group-b"
	TokenStructure.ChannelGroups.Add("channel-group-b");
	FPubnubChannelGroupPermissions ChannelGroupBReadPermission;
	ChannelGroupBReadPermission.Read = true;
	TokenStructure.ChannelGroupPermissions.Add(ChannelGroupBReadPermission);

	//Add Get permission to user "user-c"
	TokenStructure.Users.Add("user-c");
	FPubnubUserPermissions UserCGetPermission;
	UserCGetPermission.Get = true;
	TokenStructure.UserPermissions.Add(UserCGetPermission);

	//Add Read and Write permissions to 3 additional channels
	TArray<FString> ChannelsWithReadWrite = { "channel-b", "channel-c", "channel-d" };
	for (const FString& Channel : ChannelsWithReadWrite)
	{
		TokenStructure.Channels.Add(Channel);
		FPubnubChannelPermissions ReadWritePermission;
		ReadWritePermission.Read = true;
		ReadWritePermission.Write = true;
		TokenStructure.ChannelPermissions.Add(ReadWritePermission);
	}

	//Add Get and Update permission to user "user-d"
	TokenStructure.Users.Add("user-d");
	FPubnubUserPermissions UserGetUpdatePermission;
	UserGetUpdatePermission.Get = true;
	UserGetUpdatePermission.Update = true;
	TokenStructure.UserPermissions.Add(UserGetUpdatePermission);

	//Add Read permission to the whole channels pattern
	TokenStructure.ChannelPatterns.Add("channel-[A-Za-z0-9]");
	FPubnubChannelPermissions PatternReadPermission;
	PatternReadPermission.Read = true;
	TokenStructure.ChannelPatternPermissions.Add(PatternReadPermission);

	// Convert structure to JSON string permission object
	bool bSuccess;
	FString PermissionObject = PubnubSubsystem->GrantTokenStructureToJsonString(TokenStructure, bSuccess);

	if (bSuccess)
	{
		// Bind response delegate
		FOnGrantTokenResponse OnGrantTokenResponse;
		OnGrantTokenResponse.BindDynamic(this, &ASample_AccessManager::OnGrantTokenResponse_Complex);
		
		// Request the token from the server
		PubnubSubsystem->GrantToken(PermissionObject, OnGrantTokenResponse);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create token structure JSON."));
	}
}

// ACTION REQUIRED: Replace ASample_AccessManager with name of your Actor class
void ASample_AccessManager::OnGrantTokenResponse_Complex(FPubnubOperationResult Result, FString Token)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to Grant Token. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Grant Token Success. The Token: %s"), *Token);
	}
}

// snippet.revoke_token
// ACTION REQUIRED: Replace ASample_AccessManager with name of your Actor class
void ASample_AccessManager::RevokeTokenSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Set secret key to be able to revoke token
	//Make sure proper SecretKey is set in PubnubSDK PluginSettings
	//This is not required if "SetSecretKeyAutomatically" is set to true in PubnubSDK PluginSettings
	PubnubSubsystem->SetSecretKey();
	
	// Revoke the token
	// ACTION REQUIRED: This is an old token, so revoking it will return an error. Replace with a valid token returned from GrantToken method, to get success response
	FString TokenToRevoke = TEXT("p0F2AkF0GmheUpNDdHRsGDxDcmVzpURjaGFuoWtnbG9iYWxfY2hhdANDZ3JwoENzcGOgQ3VzcqBEdXVpZKBDcGF0pURjaGFuoENncnCgQ3NwY6BDdXNyoER1dWlkoERtZXRhoENzaWdYILa9OLrP_dhe31sW_seO2r9KhD6mp9Yi9vZxcX9QY04R");
	PubnubSubsystem->RevokeToken(TokenToRevoke);
}

// snippet.revoke_token_with_result
// ACTION REQUIRED: Replace ASample_AccessManager with name of your Actor class
void ASample_AccessManager::RevokeTokenWithResultSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Set secret key to be able to revoke token
	//Make sure proper SecretKey is set in PubnubSDK PluginSettings
	//This is not required if "SetSecretKeyAutomatically" is set to true in PubnubSDK PluginSettings
	PubnubSubsystem->SetSecretKey();

	// Bind response delegate
	// ACTION REQUIRED: Replace ASample_AccessManager with name of your Actor class
	FOnRevokeTokenResponse OnRevokeTokenResponse;
	OnRevokeTokenResponse.BindDynamic(this, &ASample_AccessManager::OnRevokeTokenResponse);
	
	// Revoke the token
	// ACTION REQUIRED: This is an old token, so revoking it will return an error. Replace with a valid token returned from GrantToken method, to get success response
	FString TokenToRevoke = TEXT("p0F2AkF0GmheUpNDdHRsGDxDcmVzpURjaGFuoWtnbG9iYWxfY2hhdANDZ3JwoENzcGOgQ3VzcqBEdXVpZKBDcGF0pURjaGFuoENncnCgQ3NwY6BDdXNyoER1dWlkoERtZXRhoENzaWdYILa9OLrP_dhe31sW_seO2r9KhD6mp9Yi9vZxcX9QY04R");
	PubnubSubsystem->RevokeToken(TokenToRevoke, OnRevokeTokenResponse);
}

// ACTION REQUIRED: Replace ASample_AccessManager with name of your Actor class
void ASample_AccessManager::OnRevokeTokenResponse(FPubnubOperationResult Result)
{
	if(Result.Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to revoke token. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully revoked token."));
	}
}

// snippet.revoke_token_with_result_lambda
// ACTION REQUIRED: Replace ASample_AccessManager with name of your Actor class
void ASample_AccessManager::RevokeTokenWithResultLambdaSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Set secret key to be able to revoke token
	//Make sure proper SecretKey is set in PubnubSDK PluginSettings
	//This is not required if "SetSecretKeyAutomatically" is set to true in PubnubSDK PluginSettings
	PubnubSubsystem->SetSecretKey();

	// Bind lambda to response delegate
	FOnRevokeTokenResponseNative OnRevokeTokenResponse;
	OnRevokeTokenResponse.BindLambda([](const FPubnubOperationResult& Result)
	{
		if(Result.Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to revoke token. Status: %d, Reason: %s"), Result.Status, *Result.ErrorMessage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully revoked token."));
		}
	});
	
	// Revoke the token
	// ACTION REQUIRED: This is an old token, so revoking it will return an error. Replace with a valid token returned from GrantToken method, to get success response
	FString TokenToRevoke = TEXT("p0F2AkF0GmheUpNDdHRsGDxDcmVzpURjaGFuoWtnbG9iYWxfY2hhdANDZ3JwoENzcGOgQ3VzcqBEdXVpZKBDcGF0pURjaGFuoENncnCgQ3NwY6BDdXNyoER1dWlkoERtZXRhoENzaWdYILa9OLrP_dhe31sW_seO2r9KhD6mp9Yi9vZxcX9QY04R");
	PubnubSubsystem->RevokeToken(TokenToRevoke, OnRevokeTokenResponse);
}

// snippet.parse_token
// ACTION REQUIRED: Replace ASample_AccessManager with name of your Actor class
void ASample_AccessManager::ParseTokenSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Parse the token
	FString TokenToParse = TEXT("p0F2AkF0GmheUpNDdHRsGDxDcmVzpURjaGFuoWtnbG9iYWxfY2hhdANDZ3JwoENzcGOgQ3VzcqBEdXVpZKBDcGF0pURjaGFuoENncnCgQ3NwY6BDdXNyoER1dWlkoERtZXRhoENzaWdYILa9OLrP_dhe31sW_seO2r9KhD6mp9Yi9vZxcX9QY04R");
	FString ParsedToken = PubnubSubsystem->ParseToken(TokenToParse);

	//If parsed token is empty it means that something went wrong. Check Output Log for more details.
	if(ParsedToken.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("Parsing Token failed."));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Parsed Token: %s."), *ParsedToken);
	}
}

// snippet.set_auth_token
// ACTION REQUIRED: Replace ASample_AccessManager with name of your Actor class
void ASample_AccessManager::SetAuthTokenSample()
{
	//Get PubnubSubsystem from GameInstance
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UPubnubSubsystem* PubnubSubsystem = GameInstance->GetSubsystem<UPubnubSubsystem>();

	//Set UserID
	FString UserID = TEXT("Player_001");
	PubnubSubsystem->SetUserID(UserID);

	//Set Auth Token
	FString Token = TEXT("p0F2AkF0GmheUpNDdHRsGDxDcmVzpURjaGFuoWtnbG9iYWxfY2hhdANDZ3JwoENzcGOgQ3VzcqBEdXVpZKBDcGF0pURjaGFuoENncnCgQ3NwY6BDdXNyoER1dWlkoERtZXRhoENzaWdYILa9OLrP_dhe31sW_seO2r9KhD6mp9Yi9vZxcX9QY04R");
	PubnubSubsystem->SetAuthToken(Token);
}

// snippet.end
