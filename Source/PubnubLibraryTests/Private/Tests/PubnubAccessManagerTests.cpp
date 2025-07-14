// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "FunctionLibraries/PubnubTokenUtilities.h"
#include "Dom/JsonObject.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGrantAndParseTokenTest, FPubnubAutomationTestBase, "Pubnub.Integration.AccessManager.GrantAndParseToken", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRevokeTokenTest, FPubnubAutomationTestBase, "Pubnub.Integration.AccessManager.RevokeToken", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetAuthTokenSimpleTest, FPubnubAutomationTestBase, "Pubnub.Integration.AccessManager.SetAuthTokenSimple", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);


bool FPubnubGrantAndParseTokenTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestAuthUser = SDK_PREFIX + "auth_user_grant_parse";
    const FString TestChannelName = SDK_PREFIX + "channel_grant_parse";
    const FString TestGroupName = SDK_PREFIX + "group_grant_parse";
    const FString TestTargetResourceUID = SDK_PREFIX + "target_uuid_grant_parse"; // For UUID-specific permissions
    const int TestTTLMinutes = 60;

    TSharedPtr<FString> GrantedToken = MakeShared<FString>();
    TSharedPtr<bool> bGrantTokenSuccess = MakeShared<bool>(false);
    TSharedPtr<bool> bGrantTokenDone = MakeShared<bool>(false);

    TSharedPtr<FString> ParsedTokenResponseJson = MakeShared<FString>();
    TSharedPtr<bool> bParseTokenSuccess = MakeShared<bool>(false);

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubGrantAndParseTokenTest");
        return false;
    }

    // General error handler
    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("Pubnub Error in FPubnubGrantAndParseTokenTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    // Prepare GrantToken permissions
    FPubnubGrantTokenPermissions TokenPermissions;

    // Set up channel permissions
    FChannelGrant ChannelGrant;
    ChannelGrant.Channel = TestChannelName;
    ChannelGrant.Permissions.Read = true;
    ChannelGrant.Permissions.Write = true;
    TokenPermissions.Channels.Add(ChannelGrant);
    const int ExpectedChannelBitmask = UPubnubTokenUtilities::CalculateChannelPermissionsBitmask(ChannelGrant.Permissions);

    // Set up channel group permissions
    FChannelGroupGrant GroupGrant;
    GroupGrant.ChannelGroup = TestGroupName;
    GroupGrant.Permissions.Read = true;
    GroupGrant.Permissions.Manage = true;
    TokenPermissions.ChannelGroups.Add(GroupGrant);
    const int ExpectedGroupBitmask = UPubnubTokenUtilities::CalculateChannelGroupPermissionsBitmask(GroupGrant.Permissions);

    // Set up user permissions
    FUserGrant UserGrant;
    UserGrant.User = TestTargetResourceUID;
    UserGrant.Permissions.Get = true;
    UserGrant.Permissions.Update = true;
    TokenPermissions.Users.Add(UserGrant);
    const int ExpectedUserResourceBitmask = UPubnubTokenUtilities::CalculateUserPermissionsBitmask(UserGrant.Permissions);
    
    TestFalse("TokenPermissions should not be empty", TokenPermissions.ArePermissionsEmpty());

    // GrantToken callback with proper error checking
    FOnGrantTokenResponseNative GrantTokenCallback;
    GrantTokenCallback.BindLambda([this, GrantedToken, bGrantTokenSuccess, bGrantTokenDone](const FPubnubOperationResult& Result, FString Token)
    {
        *bGrantTokenDone = true;
        if (Result.Error || Result.Status != 200)
        {
            AddError(FString::Printf(TEXT("GrantToken failed. Status: %d, ErrorMessage: %s"), Result.Status, *Result.ErrorMessage));
            *bGrantTokenSuccess = false;
        }
        else if (!Token.IsEmpty())
        {
            *GrantedToken = Token;
            *bGrantTokenSuccess = true;
        }
        else
        {
            AddError("GrantToken response was empty. Expected a token string.");
            *bGrantTokenSuccess = false;
        }
    });
    
    // --- Test Execution ---

    // Set UserID
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestAuthUser]()
    {
        PubnubSubsystem->SetUserID(TestAuthUser); 
    }, 0.1f));

    // Set secret key is needed for Token operations
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this]()
    {
        PubnubSubsystem->SetSecretKey(); 
    }, 0.1f));

    // Step 1: Grant Token
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestTTLMinutes, TestAuthUser, TokenPermissions, GrantTokenCallback]()
    {
        PubnubSubsystem->GrantToken(TestTTLMinutes, TestAuthUser, TokenPermissions, GrantTokenCallback);
    }, 0.2f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGrantTokenDone](){ return *bGrantTokenDone; }, MAX_WAIT_TIME));

    // Step 2: Verify Grant Token Success and Proceed to Parse
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bGrantTokenSuccess, GrantedToken, bParseTokenSuccess, ParsedTokenResponseJson]()
    {
        TestTrue("GrantToken operation was successful (callback received and token extracted)", *bGrantTokenSuccess);
        if (*bGrantTokenSuccess && !GrantedToken->IsEmpty())
        {
            FString ParsedToken = PubnubSubsystem->ParseToken(*GrantedToken);
            *ParsedTokenResponseJson = ParsedToken;
            if (!ParsedToken.IsEmpty())
             {
                  TSharedPtr<FJsonObject> ParsedTokenObject;
                  if (UPubnubJsonUtilities::StringToJsonObject(ParsedToken, ParsedTokenObject) && ParsedTokenObject.IsValid())
                  {
                     *bParseTokenSuccess = true; 
                  }
                  else
                  {
                     AddError(FString::Printf(TEXT("ParseToken response is not a valid JSON: %s"), *ParsedToken));
                  }
             }
             else
             {
                 AddError("ParseToken response was empty.");
             }
        }
        else if (GrantedToken->IsEmpty())
        {
            AddError("Granted token string is empty, cannot proceed to ParseToken.");
        }
    }, 0.1f));

    // Step 3: Verify Parsed Token
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bParseTokenSuccess, ParsedTokenResponseJson, TestTTLMinutes, TestChannelName, TestGroupName, TestTargetResourceUID]()
    {
        TestTrue("ParseToken operation was successful (callback received and response is valid JSON)", *bParseTokenSuccess);
        if (!*bParseTokenSuccess || ParsedTokenResponseJson->IsEmpty())
        {
            AddError("Skipping ParsedToken verification due to previous errors or empty response.");
            return;
        }

        TSharedPtr<FJsonObject> ParsedTokenObject;
        if (!UPubnubJsonUtilities::StringToJsonObject(*ParsedTokenResponseJson, ParsedTokenObject) || !ParsedTokenObject.IsValid())
        {
            AddError(FString::Printf(TEXT("Could not parse ParsedTokenResponseJson: %s"), **ParsedTokenResponseJson));
            return;
        }

        // Verify TTL (now in enhanced format)
        if (ParsedTokenObject->HasField(TEXT("TTL")))
        {
            TestEqual("Parsed token TTL matches granted TTL", static_cast<int>(ParsedTokenObject->GetNumberField(TEXT("TTL"))), TestTTLMinutes);
        } else { AddError("Parsed token JSON does not contain 'TTL' field."); }

        // Verify Version
        if (ParsedTokenObject->HasField(TEXT("Version")))
        {
            TestEqual("Parsed token Version is 2", static_cast<int>(ParsedTokenObject->GetNumberField(TEXT("Version"))), 2);
        } else { AddError("Parsed token JSON does not contain 'Version' field."); }
        
        // Verify Resources (now in enhanced format)
        const TSharedPtr<FJsonObject>* ResourcesObjectPtr = nullptr;
        if (!ParsedTokenObject->TryGetObjectField(TEXT("Resources"), ResourcesObjectPtr) || !ResourcesObjectPtr || !(*ResourcesObjectPtr).IsValid())
        {
            AddError("Parsed token JSON does not contain 'Resources' object.");
            return;
        }
        const TSharedPtr<FJsonObject>& ResourcesObject = *ResourcesObjectPtr;

        // Channel Permissions (enhanced human-readable format)
        const TSharedPtr<FJsonObject>* ChannelsObjectPtr = nullptr;
        if (ResourcesObject->TryGetObjectField(TEXT("Channels"), ChannelsObjectPtr) && ChannelsObjectPtr && (*ChannelsObjectPtr).IsValid())
        {
            const TSharedPtr<FJsonObject>* ChannelPermissionsPtr = nullptr;
            if ((*ChannelsObjectPtr)->TryGetObjectField(TestChannelName, ChannelPermissionsPtr) && ChannelPermissionsPtr && (*ChannelPermissionsPtr).IsValid())
            {
                TestTrue(FString::Printf(TEXT("Channel '%s' has Read permission"), *TestChannelName), (*ChannelPermissionsPtr)->GetBoolField(TEXT("Read")));
                TestTrue(FString::Printf(TEXT("Channel '%s' has Write permission"), *TestChannelName), (*ChannelPermissionsPtr)->GetBoolField(TEXT("Write")));
                TestFalse(FString::Printf(TEXT("Channel '%s' does not have Manage permission"), *TestChannelName), (*ChannelPermissionsPtr)->GetBoolField(TEXT("Manage")));
                TestFalse(FString::Printf(TEXT("Channel '%s' does not have Delete permission"), *TestChannelName), (*ChannelPermissionsPtr)->GetBoolField(TEXT("Delete")));
                TestFalse(FString::Printf(TEXT("Channel '%s' does not have Get permission"), *TestChannelName), (*ChannelPermissionsPtr)->GetBoolField(TEXT("Get")));
                TestFalse(FString::Printf(TEXT("Channel '%s' does not have Update permission"), *TestChannelName), (*ChannelPermissionsPtr)->GetBoolField(TEXT("Update")));
                TestFalse(FString::Printf(TEXT("Channel '%s' does not have Join permission"), *TestChannelName), (*ChannelPermissionsPtr)->GetBoolField(TEXT("Join")));
            } else { AddError(FString::Printf(TEXT("Channel '%s' not found in parsed token Resources.Channels."), *TestChannelName)); }
        } else { AddWarning(FString::Printf(TEXT("No 'Channels' found in parsed token Resources (expected for %s)."), *TestChannelName)); }

        // Group Permissions (enhanced human-readable format)
        const TSharedPtr<FJsonObject>* ChannelGroupsObjectPtr = nullptr;
        if (ResourcesObject->TryGetObjectField(TEXT("ChannelGroups"), ChannelGroupsObjectPtr) && ChannelGroupsObjectPtr && (*ChannelGroupsObjectPtr).IsValid())
        {
            const TSharedPtr<FJsonObject>* GroupPermissionsPtr = nullptr;
            if ((*ChannelGroupsObjectPtr)->TryGetObjectField(TestGroupName, GroupPermissionsPtr) && GroupPermissionsPtr && (*GroupPermissionsPtr).IsValid())
            {
                TestTrue(FString::Printf(TEXT("Group '%s' has Read permission"), *TestGroupName), (*GroupPermissionsPtr)->GetBoolField(TEXT("Read")));
                TestTrue(FString::Printf(TEXT("Group '%s' has Manage permission"), *TestGroupName), (*GroupPermissionsPtr)->GetBoolField(TEXT("Manage")));
            } else { AddError(FString::Printf(TEXT("Group '%s' not found in parsed token Resources.ChannelGroups."), *TestGroupName)); }
        } else { AddWarning(FString::Printf(TEXT("No 'ChannelGroups' found in parsed token Resources (expected for %s)."), *TestGroupName)); }
        
        // UUID (User Resource) Permissions (enhanced human-readable format)
        const TSharedPtr<FJsonObject>* UuidsObjectPtr = nullptr;
        if (ResourcesObject->TryGetObjectField(TEXT("Uuids"), UuidsObjectPtr) && UuidsObjectPtr && (*UuidsObjectPtr).IsValid())
        {
            const TSharedPtr<FJsonObject>* UserPermissionsPtr = nullptr;
            if ((*UuidsObjectPtr)->TryGetObjectField(TestTargetResourceUID, UserPermissionsPtr) && UserPermissionsPtr && (*UserPermissionsPtr).IsValid())
            {
                TestFalse(FString::Printf(TEXT("User '%s' does not have Delete permission"), *TestTargetResourceUID), (*UserPermissionsPtr)->GetBoolField(TEXT("Delete")));
                TestTrue(FString::Printf(TEXT("User '%s' has Get permission"), *TestTargetResourceUID), (*UserPermissionsPtr)->GetBoolField(TEXT("Get")));
                TestTrue(FString::Printf(TEXT("User '%s' has Update permission"), *TestTargetResourceUID), (*UserPermissionsPtr)->GetBoolField(TEXT("Update")));
            } else { AddError(FString::Printf(TEXT("User resource '%s' not found in parsed token Resources.Uuids."), *TestTargetResourceUID)); }
        } else { AddWarning(FString::Printf(TEXT("No 'Uuids' resource key found in parsed token Resources (expected for %s)."), *TestTargetResourceUID)); }
        
    }, 0.1f));

    CleanUp();
    return true;
}

bool FPubnubRevokeTokenTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestAuthUserForGrant = SDK_PREFIX + "auth_user_revoke_test";
    const FString TestChannelForGrant = SDK_PREFIX + "channel_revoke_test";
    const int TestTTLForGrant = 5; // Short TTL for the token to be revoked

    TSharedPtr<FString> GrantedToken = MakeShared<FString>();
    TSharedPtr<bool> bGrantTokenSuccess = MakeShared<bool>(false);
    TSharedPtr<bool> bGrantTokenDone = MakeShared<bool>(false);
    TSharedPtr<bool> bRevokeTokenSuccess = MakeShared<bool>(false);
    TSharedPtr<bool> bRevokeTokenDone = MakeShared<bool>(false);

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubRevokeTokenTest");
        return false;
    }

    // General error handler
    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("Pubnub Error in FPubnubRevokeTokenTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    // Prepare GrantToken permissions for a temporary token
    FPubnubGrantTokenPermissions TokenPermissions;
    FChannelGrant ChannelGrant;
    ChannelGrant.Channel = TestChannelForGrant;
    ChannelGrant.Permissions.Read = true;
    TokenPermissions.Channels.Add(ChannelGrant);

    TestFalse("TokenPermissions should not be empty", TokenPermissions.ArePermissionsEmpty());

    // GrantToken callback with proper error checking
    FOnGrantTokenResponseNative GrantTokenCallback;
    GrantTokenCallback.BindLambda([this, GrantedToken, bGrantTokenSuccess, bGrantTokenDone](const FPubnubOperationResult& Result, FString Token)
    {
        *bGrantTokenDone = true;
        if (Result.Error || Result.Status != 200)
        {
            AddError(FString::Printf(TEXT("GrantToken failed. Status: %d, ErrorMessage: %s"), Result.Status, *Result.ErrorMessage));
            *bGrantTokenSuccess = false;
        }
        else if (!Token.IsEmpty())
        {
            *GrantedToken = Token;
            *bGrantTokenSuccess = true;
        }
        else
        {
            AddError("GrantToken response was empty in RevokeTest. Expected a token string.");
            *bGrantTokenSuccess = false;
        }
    });

    // RevokeToken callback with proper error checking
    FOnRevokeTokenResponseNative RevokeTokenCallback;
    RevokeTokenCallback.BindLambda([this, bRevokeTokenSuccess, bRevokeTokenDone](const FPubnubOperationResult& Result)
    {
        *bRevokeTokenDone = true;
        if (Result.Error || Result.Status != 200)
        {
            AddError(FString::Printf(TEXT("RevokeToken failed. Status: %d, ErrorMessage: %s"), Result.Status, *Result.ErrorMessage));
            *bRevokeTokenSuccess = false;
        }
        else
        {
            *bRevokeTokenSuccess = true;
        }
    });

    // --- Test Execution ---

    // Set UserID
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestAuthUserForGrant]()
    {
        PubnubSubsystem->SetUserID(TestAuthUserForGrant); 
    }, 0.1f));
    
    // Set secret key is needed for Token operations
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this]()
    {
        PubnubSubsystem->SetSecretKey(); 
    }, 0.1f));

    // Step 1: Grant a temporary token
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestTTLForGrant, TestAuthUserForGrant, TokenPermissions, GrantTokenCallback]()
    {
        PubnubSubsystem->GrantToken(TestTTLForGrant, TestAuthUserForGrant, TokenPermissions, GrantTokenCallback);
    }, 0.2f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGrantTokenDone](){ return *bGrantTokenDone; }, MAX_WAIT_TIME));

    // Step 2: If token granted, Revoke it using callback
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bGrantTokenSuccess, GrantedToken, RevokeTokenCallback]()
    {
        TestTrue("GrantToken operation for RevokeTest was successful", *bGrantTokenSuccess);
        if (*bGrantTokenSuccess && !GrantedToken->IsEmpty())
        {
            PubnubSubsystem->RevokeToken(*GrantedToken, RevokeTokenCallback);
        }
        else
        {
            AddError("Cannot proceed to RevokeToken: GrantToken failed or token is empty.");
        }
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRevokeTokenDone, bGrantTokenSuccess](){ return *bRevokeTokenDone || !*bGrantTokenSuccess; }, MAX_WAIT_TIME));

    // Step 3: Verify RevokeToken Success
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bRevokeTokenSuccess, bGrantTokenSuccess]()
    {
        if(*bGrantTokenSuccess) // Only perform this check if grant was successful
        {
            TestTrue("RevokeToken operation was successful", *bRevokeTokenSuccess);
        }
    }, 0.1f));

    CleanUp();
    return true;
}

bool FPubnubSetAuthTokenSimpleTest::RunTest(const FString& Parameters)
{
    const FString UserID = SDK_PREFIX + "AuthTokenUser";
    const FString DummyToken = "pn-dummy-auth-token-for-simple-set-test";

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubSetAuthTokenSimpleTest");
        return false;
    }
    
    // Error handler for this test
    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("Pubnub Error in FPubnubSetAuthTokenSimpleTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    // Set UserID
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, UserID]()
    {
        PubnubSubsystem->SetUserID(UserID); 
    }, 0.1f));

    // Step 1: Set the Auth Token
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, DummyToken]()
    {
        PubnubSubsystem->SetAuthToken(DummyToken);
        // SetAuthToken is synchronous on the client-side, so no callback needed
        // The test passes if no errors are logged via OnPubnubErrorNative
    }, 0.1f));

    CleanUp();
    return true;
}


#endif // WITH_DEV_AUTOMATION_TESTS