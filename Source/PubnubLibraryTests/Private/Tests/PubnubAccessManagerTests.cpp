// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

//This is an Unit test, but it still requires getting Pubnub subsystem, that's why it's here, not with other Unit tests
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGrantTokenStructureToJsonStringUnitTest, FPubnubAutomationTestBase, "Pubnub.aUnit.AccessManager.GrantTokenStructureToJsonString", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGrantAndParseTokenTest, FPubnubAutomationTestBase, "Pubnub.Integration.AccessManager.GrantAndParseToken", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRevokeTokenTest, FPubnubAutomationTestBase, "Pubnub.Integration.AccessManager.RevokeToken", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetAuthTokenSimpleTest, FPubnubAutomationTestBase, "Pubnub.Integration.AccessManager.SetAuthTokenSimple", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// Helper function to calculate expected bitmask for FPubnubChannelPermissions
int CalculateChannelPermissionsBitmask(const FPubnubChannelPermissions& Perms)
{
    int Bitmask = 0;
    if (Perms.Read) Bitmask |= 1;
    if (Perms.Write) Bitmask |= 2;
    if (Perms.Manage) Bitmask |= 4;
    if (Perms.Delete) Bitmask |= 8;
    if (Perms.Get) Bitmask |= 32;
    if (Perms.Update) Bitmask |= 64;
    if (Perms.Join) Bitmask |= 128;
    return Bitmask;
}

// Helper function to calculate expected bitmask for FPubnubChannelGroupPermissions
int CalculateChannelGroupPermissionsBitmask(const FPubnubChannelGroupPermissions& Perms)
{
    int Bitmask = 0;
    if (Perms.Read) Bitmask |= 1;
    if (Perms.Manage) Bitmask |= 4;
    return Bitmask;
}

// Helper function to calculate expected bitmask for FPubnubUserPermissions
int CalculateUserPermissionsBitmask(const FPubnubUserPermissions& Perms)
{
    int Bitmask = 0;
    if (Perms.Delete) Bitmask |= 8;
    if (Perms.Get) Bitmask |= 32;
    if (Perms.Update) Bitmask |= 64;
    return Bitmask;
}

bool FPubnubGrantTokenStructureToJsonStringUnitTest::RunTest(const FString& Parameters)
{
    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubGrantTokenStructureToJsonStringUnitTest");
        return false;
    }

    // --- Test Case 1: Basic Valid Structure (similar to E2E test) ---
    {
        const FString TestDescription = "Case 1: Basic Valid Structure";
        FPubnubGrantTokenStructure Ts;
        Ts.TTLMinutes = 60;
        Ts.AuthorizedUser = SDK_PREFIX + "auth_user_unit_1";
        
        FPubnubChannelPermissions ChanPerms; ChanPerms.Read = true; ChanPerms.Write = true; // Mask = 3
        Ts.Channels.Add(SDK_PREFIX + "chan1");
        Ts.ChannelPermissions.Add(ChanPerms);

        FPubnubChannelGroupPermissions GroupPerms; GroupPerms.Read = true; GroupPerms.Manage = true; // Mask = 5
        Ts.ChannelGroups.Add(SDK_PREFIX + "group1");
        Ts.ChannelGroupPermissions.Add(GroupPerms);

        FPubnubUserPermissions UserResPerms; UserResPerms.Get = true; UserResPerms.Update = true; // Mask = 96
        Ts.Users.Add(SDK_PREFIX + "uuid_res1");
        Ts.UserPermissions.Add(UserResPerms);

        bool bSuccess = false;
        FString JsonString = PubnubSubsystem->GrantTokenStructureToJsonString(Ts, bSuccess);
        TestTrue(TestDescription + " - Conversion Success", bSuccess);

        if (bSuccess)
        {
            TSharedPtr<FJsonObject> RootObject;
            TestTrue(TestDescription + " - Is Valid JSON", UPubnubJsonUtilities::StringToJsonObject(JsonString, RootObject) && RootObject.IsValid());
            if (RootObject.IsValid())
            {
                TestEqual(TestDescription + " - TTL", RootObject->GetIntegerField(TEXT("ttl")), Ts.TTLMinutes);
                TestEqual(TestDescription + " - Auth User", RootObject->GetStringField(TEXT("authorized_uuid")), Ts.AuthorizedUser);

                const TSharedPtr<FJsonObject>* PermissionsObjectPtr;
                TestTrue(TestDescription + " - Has 'permissions' object", RootObject->TryGetObjectField(TEXT("permissions"), PermissionsObjectPtr) && PermissionsObjectPtr && (*PermissionsObjectPtr).IsValid());
                if (PermissionsObjectPtr && (*PermissionsObjectPtr).IsValid())
                {
                    const TSharedPtr<FJsonObject>* ResourcesObjectPtr;
                    TestTrue(TestDescription + " - Has 'resources' object", (*PermissionsObjectPtr)->TryGetObjectField(TEXT("resources"), ResourcesObjectPtr) && ResourcesObjectPtr && (*ResourcesObjectPtr).IsValid());
                    if (ResourcesObjectPtr && (*ResourcesObjectPtr).IsValid())
                    {
                        const TSharedPtr<FJsonObject>* ChannelsObjectPtr;
                        TestTrue(TestDescription + " - Resources has 'channels'", (*ResourcesObjectPtr)->TryGetObjectField(TEXT("channels"), ChannelsObjectPtr) && ChannelsObjectPtr && (*ChannelsObjectPtr).IsValid());
                        if (ChannelsObjectPtr && (*ChannelsObjectPtr).IsValid())
                        {
                            TestEqual(TestDescription + " - Channel Perms", (*ChannelsObjectPtr)->GetIntegerField(Ts.Channels[0]), CalculateChannelPermissionsBitmask(ChanPerms));
                        }
                        const TSharedPtr<FJsonObject>* GroupsObjectPtr;
                        TestTrue(TestDescription + " - Resources has 'groups'", (*ResourcesObjectPtr)->TryGetObjectField(TEXT("groups"), GroupsObjectPtr) && GroupsObjectPtr && (*GroupsObjectPtr).IsValid());
                        if (GroupsObjectPtr && (*GroupsObjectPtr).IsValid())
                        {
                             TestEqual(TestDescription + " - Group Perms", (*GroupsObjectPtr)->GetIntegerField(Ts.ChannelGroups[0]), CalculateChannelGroupPermissionsBitmask(GroupPerms));
                        }
                        const TSharedPtr<FJsonObject>* UuidsObjectPtr;
                        TestTrue(TestDescription + " - Resources has 'uuids'", (*ResourcesObjectPtr)->TryGetObjectField(TEXT("uuids"), UuidsObjectPtr) && UuidsObjectPtr && (*UuidsObjectPtr).IsValid());
                        if (UuidsObjectPtr && (*UuidsObjectPtr).IsValid())
                        {
                            TestEqual(TestDescription + " - UUID Resource Perms", (*UuidsObjectPtr)->GetIntegerField(Ts.Users[0]), CalculateUserPermissionsBitmask(UserResPerms));
                        }
                    }
                     const TSharedPtr<FJsonObject>* PatternsObjectPtr;
                     TestTrue(TestDescription + " - Has 'patterns' object", (*PermissionsObjectPtr)->TryGetObjectField(TEXT("patterns"), PatternsObjectPtr) && PatternsObjectPtr && (*PatternsObjectPtr).IsValid());
                }
            }
        }
    }

    // --- Test Case 2: One Channel Permission for Multiple Channels ---
    {
        const FString TestDescription = "Case 2: One Perm for Multiple Channels";
        FPubnubGrantTokenStructure Ts;
        Ts.TTLMinutes = 10;
        Ts.AuthorizedUser = SDK_PREFIX + "auth_user_unit_2";
        
        FPubnubChannelPermissions ChanPerm; ChanPerm.Read = true; // Mask = 1
        Ts.Channels.Add(SDK_PREFIX + "chanA");
        Ts.Channels.Add(SDK_PREFIX + "chanB");
        Ts.ChannelPermissions.Add(ChanPerm); // Single permission object

        bool bSuccess = false;
        FString JsonString = PubnubSubsystem->GrantTokenStructureToJsonString(Ts, bSuccess);
        TestTrue(TestDescription + " - Conversion Success", bSuccess);
        if (bSuccess)
        {
            TSharedPtr<FJsonObject> RootObject;
            TestTrue(TestDescription + " - Is Valid JSON", UPubnubJsonUtilities::StringToJsonObject(JsonString, RootObject) && RootObject.IsValid());
            // ... (basic TTL, AuthUser checks) ...
            const TSharedPtr<FJsonObject>* PermsObj, *ResObj, *ChansObj;
            if (RootObject->TryGetObjectField(TEXT("permissions"), PermsObj) && (*PermsObj)->TryGetObjectField(TEXT("resources"), ResObj) && (*ResObj)->TryGetObjectField(TEXT("channels"), ChansObj))
            {
                TestEqual(TestDescription + " - ChanA Perms", (*ChansObj)->GetIntegerField(Ts.Channels[0]), CalculateChannelPermissionsBitmask(ChanPerm));
                TestEqual(TestDescription + " - ChanB Perms", (*ChansObj)->GetIntegerField(Ts.Channels[1]), CalculateChannelPermissionsBitmask(ChanPerm));
            } else { AddError(TestDescription + " - JSON structure error"); }
        }
    }

    // --- Test Case 3: Matching Channel Permissions for Multiple Channels ---
    {
        const FString TestDescription = "Case 3: Matching Perms for Multiple Channels";
        FPubnubGrantTokenStructure Ts;
        Ts.TTLMinutes = 10;
        Ts.AuthorizedUser = SDK_PREFIX + "auth_user_unit_3";

        FPubnubChannelPermissions ChanPermC; ChanPermC.Read = true; // Mask = 1
        FPubnubChannelPermissions ChanPermD; ChanPermD.Write = true; // Mask = 2
        Ts.Channels.Add(SDK_PREFIX + "chanC");
        Ts.Channels.Add(SDK_PREFIX + "chanD");
        Ts.ChannelPermissions.Add(ChanPermC);
        Ts.ChannelPermissions.Add(ChanPermD);

        bool bSuccess = false;
        FString JsonString = PubnubSubsystem->GrantTokenStructureToJsonString(Ts, bSuccess);
        TestTrue(TestDescription + " - Conversion Success", bSuccess);
         if (bSuccess)
        {
            TSharedPtr<FJsonObject> RootObject;
            TestTrue(TestDescription + " - Is Valid JSON", UPubnubJsonUtilities::StringToJsonObject(JsonString, RootObject) && RootObject.IsValid());
            const TSharedPtr<FJsonObject>* PermsObj, *ResObj, *ChansObj;
            if (RootObject->TryGetObjectField(TEXT("permissions"), PermsObj) && (*PermsObj)->TryGetObjectField(TEXT("resources"), ResObj) && (*ResObj)->TryGetObjectField(TEXT("channels"), ChansObj))
            {
                TestEqual(TestDescription + " - ChanC Perms", (*ChansObj)->GetIntegerField(Ts.Channels[0]), CalculateChannelPermissionsBitmask(ChanPermC));
                TestEqual(TestDescription + " - ChanD Perms", (*ChansObj)->GetIntegerField(Ts.Channels[1]), CalculateChannelPermissionsBitmask(ChanPermD));
            } else { AddError(TestDescription + " - JSON structure error"); }
        }
    }

    // --- Test Case 4: Mismatched Channel Permissions (Fail Case) ---
    {
        const FString TestDescription = "Case 4: Mismatched Perms (3 chans, 2 perms)";
        FPubnubGrantTokenStructure Ts;
        Ts.TTLMinutes = 10;
        Ts.AuthorizedUser = SDK_PREFIX + "auth_user_unit_4";

        FPubnubChannelPermissions ChanPermE; ChanPermE.Read = true;
        FPubnubChannelPermissions ChanPermF; ChanPermF.Write = true;
        Ts.Channels.Add(SDK_PREFIX + "chanE");
        Ts.Channels.Add(SDK_PREFIX + "chanF");
        Ts.Channels.Add(SDK_PREFIX + "chanG");
        Ts.ChannelPermissions.Add(ChanPermE);
        Ts.ChannelPermissions.Add(ChanPermF); // 3 channels, 2 permissions

        bool bSuccess = true; // Expecting this to be set to false by the function
        FString JsonString = PubnubSubsystem->GrantTokenStructureToJsonString(Ts, bSuccess);
        TestFalse(TestDescription + " - Conversion should Fail", bSuccess);
    }
    
    // --- Test Case 5: Empty Resources and Patterns ---
    {
        const FString TestDescription = "Case 5: Empty Resources and Patterns";
        FPubnubGrantTokenStructure Ts;
        Ts.TTLMinutes = 5;
        Ts.AuthorizedUser = SDK_PREFIX + "auth_user_unit_5";

        bool bSuccess = false;
        FString JsonString = PubnubSubsystem->GrantTokenStructureToJsonString(Ts, bSuccess);
        TestTrue(TestDescription + " - Conversion Success", bSuccess);
        if(bSuccess)
        {
            TSharedPtr<FJsonObject> RootObject;
            TestTrue(TestDescription + " - Is Valid JSON", UPubnubJsonUtilities::StringToJsonObject(JsonString, RootObject) && RootObject.IsValid());
            if (RootObject.IsValid())
            {
                 TestEqual(TestDescription + " - TTL", RootObject->GetIntegerField(TEXT("ttl")), Ts.TTLMinutes);
                 TestEqual(TestDescription + " - Auth User", RootObject->GetStringField(TEXT("authorized_uuid")), Ts.AuthorizedUser);
                 const TSharedPtr<FJsonObject>* PermsObj;
                 TestTrue(TestDescription + " - Has 'permissions' object", RootObject->TryGetObjectField(TEXT("permissions"), PermsObj) && PermsObj && (*PermsObj).IsValid());
                 if(PermsObj && (*PermsObj).IsValid())
                 {
                    const TSharedPtr<FJsonObject>* ResObj;
                    TestTrue(TestDescription + " - Perms has 'resources' object", (*PermsObj)->TryGetObjectField(TEXT("resources"), ResObj) && ResObj && (*ResObj).IsValid());
                    if(ResObj && (*ResObj).IsValid()) TestEqual(TestDescription + " - Resources is empty", (*ResObj)->Values.Num(), 0);
                    
                    const TSharedPtr<FJsonObject>* PatObj;
                    TestTrue(TestDescription + " - Perms has 'patterns' object", (*PermsObj)->TryGetObjectField(TEXT("patterns"), PatObj) && PatObj && (*PatObj).IsValid());
                    if(PatObj && (*PatObj).IsValid()) TestEqual(TestDescription + " - Patterns is empty", (*PatObj)->Values.Num(), 0);
                 }
            }
        }
    }

    // --- Test Case 6: Only Patterns ---
    {
        const FString TestDescription = "Case 6: Only Patterns";
        FPubnubGrantTokenStructure Ts;
        Ts.TTLMinutes = 15;
        Ts.AuthorizedUser = SDK_PREFIX + "auth_user_unit_6";

        FPubnubChannelPermissions ChanPatternPerms; ChanPatternPerms.Manage = true; // Mask = 4
        Ts.ChannelPatterns.Add(SDK_PREFIX + "chan-pat-*");
        Ts.ChannelPatternPermissions.Add(ChanPatternPerms);

        bool bSuccess = false;
        FString JsonString = PubnubSubsystem->GrantTokenStructureToJsonString(Ts, bSuccess);
        TestTrue(TestDescription + " - Conversion Success", bSuccess);
        if (bSuccess)
        {
            TSharedPtr<FJsonObject> RootObject;
            TestTrue(TestDescription + " - Is Valid JSON", UPubnubJsonUtilities::StringToJsonObject(JsonString, RootObject) && RootObject.IsValid());
            if (RootObject.IsValid())
            {
                // ... (basic TTL, AuthUser checks) ...
                const TSharedPtr<FJsonObject>* PermsObj, *ResObj, *PatObj, *ChanPatObj;
                if (RootObject->TryGetObjectField(TEXT("permissions"), PermsObj) &&
                    (*PermsObj)->TryGetObjectField(TEXT("resources"), ResObj) &&
                    (*PermsObj)->TryGetObjectField(TEXT("patterns"), PatObj) &&
                    (*PatObj)->TryGetObjectField(TEXT("channels"), ChanPatObj))
                {
                    TestEqual(TestDescription + " - Resources is empty", (*ResObj)->Values.Num(), 0);
                    TestEqual(TestDescription + " - Pattern Perms", (*ChanPatObj)->GetIntegerField(Ts.ChannelPatterns[0]), CalculateChannelPermissionsBitmask(ChanPatternPerms));
                } else { AddError(TestDescription + " - JSON structure error"); }
            }
        }
    }

    CleanUp();
    return true;
}

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
    TSharedPtr<bool> bGrantTokenCallbackReceived = MakeShared<bool>(false);

    TSharedPtr<FString> ParsedTokenResponseJson = MakeShared<FString>();
    TSharedPtr<bool> bParseTokenSuccess = MakeShared<bool>(false);
    TSharedPtr<bool> bParseTokenCallbackReceived = MakeShared<bool>(false);

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

    // Prepare GrantTokenStructure
    FPubnubGrantTokenStructure TokenStructure;
    TokenStructure.TTLMinutes = TestTTLMinutes;
    TokenStructure.AuthorizedUser = TestAuthUser;

    FPubnubChannelPermissions ChanPerms;
    ChanPerms.Read = true;
    ChanPerms.Write = true;
    TokenStructure.Channels.Add(TestChannelName);
    TokenStructure.ChannelPermissions.Add(ChanPerms);
    const int ExpectedChannelBitmask = CalculateChannelPermissionsBitmask(ChanPerms);

    FPubnubChannelGroupPermissions GroupPerms;
    GroupPerms.Read = true;
    GroupPerms.Manage = true;
    TokenStructure.ChannelGroups.Add(TestGroupName);
    TokenStructure.ChannelGroupPermissions.Add(GroupPerms);
    const int ExpectedGroupBitmask = CalculateChannelGroupPermissionsBitmask(GroupPerms);

    FPubnubUserPermissions UserResourcePerms; // Permissions for a specific UUID resource (App Context)
    UserResourcePerms.Get = true;
    UserResourcePerms.Update = true;
    TokenStructure.Users.Add(TestTargetResourceUID); // Using 'Users' field for UUID resources as per FPubnubGrantTokenStructure
    TokenStructure.UserPermissions.Add(UserResourcePerms);
    const int ExpectedUserResourceBitmask = CalculateUserPermissionsBitmask(UserResourcePerms);
    
    bool bJsonConversionSuccess = false;
    FString PermissionObjectJson = PubnubSubsystem->GrantTokenStructureToJsonString(TokenStructure, bJsonConversionSuccess);
    if (!bJsonConversionSuccess)
    {
        AddError("Failed to convert GrantTokenStructure to JSON. Check logs for details from PubnubSubsystem.");
        CleanUp();
        return false;
    }
    TestTrue("GrantTokenStructureToJsonString conversion success", bJsonConversionSuccess);


    // GrantToken callback
    FOnPubnubResponseNative GrantTokenCallback;
    GrantTokenCallback.BindLambda([this, GrantedToken, bGrantTokenSuccess, bGrantTokenCallbackReceived](FString JsonResponse)
    {
        *bGrantTokenCallbackReceived = true;
        if (!JsonResponse.IsEmpty())
        {
            *GrantedToken = JsonResponse; // The response is the token itself
            *bGrantTokenSuccess = true;
        }
        else
        {
            AddError("GrantToken response was empty. Expected a token string.");
            *bGrantTokenSuccess = false;
        }
    });

    // ParseToken callback
    FOnPubnubResponseNative ParseTokenCallback;
    ParseTokenCallback.BindLambda([this, ParsedTokenResponseJson, bParseTokenSuccess, bParseTokenCallbackReceived](FString JsonResponse)
    {
        *bParseTokenCallbackReceived = true;
        *ParsedTokenResponseJson = JsonResponse;
        if (!JsonResponse.IsEmpty())
        {
             TSharedPtr<FJsonObject> ParsedTokenObject;
             if (UPubnubJsonUtilities::StringToJsonObject(JsonResponse, ParsedTokenObject) && ParsedTokenObject.IsValid())
             {
                *bParseTokenSuccess = true; 
             }
             else
             {
                AddError(FString::Printf(TEXT("ParseToken response is not a valid JSON: %s"), *JsonResponse));
             }
        }
        else
        {
            AddError("ParseToken response was empty.");
        }
    });


    // --- Test Execution ---

    // Set UserID
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestAuthUser]()
    {
        PubnubSubsystem->SetUserID(TestAuthUser); 
    }, 0.1f));

    // Set secret key is needed for Token operations
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestAuthUser]()
    {
        PubnubSubsystem->SetSecretKey(); 
    }, 0.1f));

    // Step 1: Grant Token
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, PermissionObjectJson, GrantTokenCallback]()
    {
        PubnubSubsystem->GrantToken(PermissionObjectJson, GrantTokenCallback);
    }, 0.2f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGrantTokenCallbackReceived](){ return *bGrantTokenCallbackReceived; }, MAX_WAIT_TIME));

    // Step 2: Verify Grant Token Success and Proceed to Parse
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bGrantTokenSuccess, GrantedToken, ParseTokenCallback]()
    {
        TestTrue("GrantToken operation was successful (callback received and token extracted)", *bGrantTokenSuccess);
        if (*bGrantTokenSuccess && !GrantedToken->IsEmpty())
        {
            PubnubSubsystem->ParseToken(*GrantedToken, ParseTokenCallback);
        }
        else if (GrantedToken->IsEmpty())
        {
            AddError("Granted token string is empty, cannot proceed to ParseToken.");
        }
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bParseTokenCallbackReceived, bGrantTokenSuccess](){ return *bParseTokenCallbackReceived || !*bGrantTokenSuccess; }, MAX_WAIT_TIME)); // Wait if grant was successful

    // Step 3: Verify Parsed Token
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bParseTokenSuccess, ParsedTokenResponseJson, TestTTLMinutes, TestChannelName, ExpectedChannelBitmask, TestGroupName, ExpectedGroupBitmask, TestTargetResourceUID, ExpectedUserResourceBitmask]()
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

        // Verify TTL
        int ParsedTTL = 0;
        if (ParsedTokenObject->TryGetNumberField(TEXT("ttl"), ParsedTTL))
        {
             TestEqual("Parsed token TTL matches granted TTL", ParsedTTL, TestTTLMinutes);
        } else { AddError("Parsed token JSON does not contain 'ttl' field."); }
        
        const TSharedPtr<FJsonObject>* ResourcesObjectPtr; // This will now point to the "res" object
        if (!ParsedTokenObject->TryGetObjectField(TEXT("res"), ResourcesObjectPtr) || !ResourcesObjectPtr || !(*ResourcesObjectPtr).IsValid())
        {
            AddError("Parsed token JSON does not contain 'res' (resources) object.");
            return;
        }
        const TSharedPtr<FJsonObject>& ResourcesObject = *ResourcesObjectPtr;

        // Channel Permissions
        const TSharedPtr<FJsonObject>* ChannelsObjectPtr;
        if (ResourcesObject->TryGetObjectField(TEXT("chan"), ChannelsObjectPtr) && ChannelsObjectPtr && (*ChannelsObjectPtr).IsValid())
        {
            int ActualChannelPerms = 0;
            if ((*ChannelsObjectPtr)->TryGetNumberField(TestChannelName, ActualChannelPerms))
            {
                TestEqual(FString::Printf(TEXT("Permissions for channel '%s'"), *TestChannelName), ActualChannelPerms, ExpectedChannelBitmask);
            } else { AddError(FString::Printf(TEXT("Channel '%s' not found in parsed token res.chan."), *TestChannelName)); }
        } else { AddWarning(FString::Printf(TEXT("No 'chan' found in parsed token res (expected for %s)."), *TestChannelName)); }

        // Group Permissions
        const TSharedPtr<FJsonObject>* GroupsObjectPtr;
        if (ResourcesObject->TryGetObjectField(TEXT("grp"), GroupsObjectPtr) && GroupsObjectPtr && (*GroupsObjectPtr).IsValid())
        {
            int ActualGroupPerms = 0;
            if ((*GroupsObjectPtr)->TryGetNumberField(TestGroupName, ActualGroupPerms))
            {
                TestEqual(FString::Printf(TEXT("Permissions for group '%s'"), *TestGroupName), ActualGroupPerms, ExpectedGroupBitmask);
            } else { AddError(FString::Printf(TEXT("Group '%s' not found in parsed token res.grp."), *TestGroupName)); }
        } else { AddWarning(FString::Printf(TEXT("No 'grp' found in parsed token res (expected for %s)."),*TestGroupName)); }
        
        // UUID (User Resource) Permissions
        const TSharedPtr<FJsonObject>* UuidsObjectPtr;
        if (ResourcesObject->TryGetObjectField(TEXT("uuid"), UuidsObjectPtr) && UuidsObjectPtr && (*UuidsObjectPtr).IsValid()) // "uuid" here is the key for uuid-specific permissions inside "res"
        {
            int ActualUserResourcePerms = 0;
            if ((*UuidsObjectPtr)->TryGetNumberField(TestTargetResourceUID, ActualUserResourcePerms))
            {
                TestEqual(FString::Printf(TEXT("Permissions for UUID resource '%s'"), *TestTargetResourceUID), ActualUserResourcePerms, ExpectedUserResourceBitmask);
            } else { AddError(FString::Printf(TEXT("UUID resource '%s' not found in parsed token res.uuid."), *TestTargetResourceUID)); }
        } else { AddWarning(FString::Printf(TEXT("No 'uuid' resource key found in parsed token res (expected for %s)."), *TestTargetResourceUID)); }
        
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
    TSharedPtr<bool> bGrantTokenCallbackReceived = MakeShared<bool>(false);
    TSharedPtr<bool> bRevokeErrorOccurred = MakeShared<bool>(false);

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubRevokeTokenTest");
        return false;
    }

    // General error handler
    // We are primarily interested in errors *after* RevokeToken is called, but this will catch any.
    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this, bRevokeErrorOccurred](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        // Only set the flag if an error occurs. We will check its state later.
        *bRevokeErrorOccurred = true;
        AddError(FString::Printf(TEXT("Pubnub Error in FPubnubRevokeTokenTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    // Prepare GrantTokenStructure for a temporary token
    FPubnubGrantTokenStructure TokenStructure;
    TokenStructure.TTLMinutes = TestTTLForGrant;
    TokenStructure.AuthorizedUser = TestAuthUserForGrant;
    FPubnubChannelPermissions ChanPerms; ChanPerms.Read = true;
    TokenStructure.Channels.Add(TestChannelForGrant);
    TokenStructure.ChannelPermissions.Add(ChanPerms);

    bool bJsonConversionSuccess = false;
    FString PermissionObjectJson = PubnubSubsystem->GrantTokenStructureToJsonString(TokenStructure, bJsonConversionSuccess);
    if (!bJsonConversionSuccess)
    {
        AddError("Failed to convert GrantTokenStructure to JSON for RevokeTest.");
        CleanUp();
        return false;
    }

    // GrantToken callback (copied from GrantAndParse, simplified as we just need the token string)
    FOnPubnubResponseNative GrantTokenCallback;
    GrantTokenCallback.BindLambda([this, GrantedToken, bGrantTokenSuccess, bGrantTokenCallbackReceived](FString RawTokenString)
    {
        *bGrantTokenCallbackReceived = true;
        if (!RawTokenString.IsEmpty())
        {
            *GrantedToken = RawTokenString;
            *bGrantTokenSuccess = true;
        }
        else
        {
            AddError("GrantToken response was empty in RevokeTest. Expected a token string.");
            *bGrantTokenSuccess = false;
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
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, PermissionObjectJson, GrantTokenCallback]()
    {
        PubnubSubsystem->GrantToken(PermissionObjectJson, GrantTokenCallback);
    }, 0.2f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGrantTokenCallbackReceived](){ return *bGrantTokenCallbackReceived; }, MAX_WAIT_TIME));

    // Step 2: If token granted, Revoke it
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bGrantTokenSuccess, GrantedToken, bRevokeErrorOccurred]()
    {
        TestTrue("GrantToken operation for RevokeTest was successful", *bGrantTokenSuccess);
        if (*bGrantTokenSuccess && !GrantedToken->IsEmpty())
        {
            // Reset error flag before calling RevokeToken to isolate errors from this operation
            *bRevokeErrorOccurred = false; 
            PubnubSubsystem->RevokeToken(*GrantedToken);
            // No direct callback for RevokeToken success/failure, we rely on OnPubnubErrorNative
        }
        else
        {
            AddError("Cannot proceed to RevokeToken: GrantToken failed or token is empty.");
            // To prevent waiting indefinitely if grant failed
            *bRevokeErrorOccurred = true; // Mark as error to skip further checks if grant failed
        }
    }, 0.1f));

    // Step 3: Wait a moment for any async errors from RevokeToken to be processed
    // This duration might need adjustment based on typical network/server response times for errors.
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(2.0f)); 

    // Step 4: Verify no errors occurred during/after RevokeToken
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bRevokeErrorOccurred, bGrantTokenSuccess]()
    {
        if(*bGrantTokenSuccess) // Only perform this check if grant was successful
        {
            TestFalse("No PubNub errors should have occurred during or after RevokeToken call.", *bRevokeErrorOccurred);
            if(*bRevokeErrorOccurred)
            {
                AddError("An error was flagged by OnPubnubErrorNative, potentially related to RevokeToken.");
            }
        }
        // If grant failed, an error would have already been added.
    }, 0.1f));

    CleanUp();
    return true;
}

bool FPubnubSetAuthTokenSimpleTest::RunTest(const FString& Parameters)
{
    const FString UserID = SDK_PREFIX + "AuthTokenUser";
    const FString DummyToken = "pn-dummy-auth-token-for-simple-set-test";
    TSharedPtr<bool> bErrorOccurred = MakeShared<bool>(false);

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubSetAuthTokenSimpleTest");
        return false;
    }
    
    // Error handler for this test
    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this, bErrorOccurred](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        *bErrorOccurred = true;
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
        // No direct callback, this function is synchronous on the client-side.
        // We are checking that it doesn't cause an immediate error logged via OnPubnubErrorNative.
    }, 0.1f));

    // Step 2: Wait a brief moment to catch any immediate asynchronous errors if they were to occur (though unlikely for SetAuthToken)
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(MAX_WAIT_TIME/2.0)); 

    CleanUp();
    return true;
}


#endif // WITH_DEV_AUTOMATION_TESTS