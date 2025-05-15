#include "Misc/AutomationTest.h"
#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/PubnubTestsUtils.h"

using namespace PubnubTests;

//This is an Unit test, but it still requires getting Pubnub subsystem, that's why it's here, not with other Unit tests
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGrantTokenStructureToJsonStringUnitTest, FPubnubAutomationTestBase, "Pubnub.Unit.AccessManager.GrantTokenStructureToJsonString", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGrantAndParseTokenTest, FPubnubAutomationTestBase, "Pubnub.E2E.AccessManager.GrantAndParseToken", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

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
        AddError(TEXT("TestInitialization failed for FPubnubGrantTokenStructureToJsonStringUnitTest"));
        return false;
    }

    // --- Test Case 1: Basic Valid Structure (similar to E2E test) ---
    {
        const FString TestDescription = TEXT("Case 1: Basic Valid Structure");
        FPubnubGrantTokenStructure Ts;
        Ts.TTLMinutes = 60;
        Ts.AuthorizedUser = SDK_PREFIX + TEXT("auth_user_unit_1");
        
        FPubnubChannelPermissions ChanPerms; ChanPerms.Read = true; ChanPerms.Write = true; // Mask = 3
        Ts.Channels.Add(SDK_PREFIX + TEXT("chan1"));
        Ts.ChannelPermissions.Add(ChanPerms);

        FPubnubChannelGroupPermissions GroupPerms; GroupPerms.Read = true; GroupPerms.Manage = true; // Mask = 5
        Ts.ChannelGroups.Add(SDK_PREFIX + TEXT("group1"));
        Ts.ChannelGroupPermissions.Add(GroupPerms);

        FPubnubUserPermissions UserResPerms; UserResPerms.Get = true; UserResPerms.Update = true; // Mask = 96
        Ts.Users.Add(SDK_PREFIX + TEXT("uuid_res1"));
        Ts.UserPermissions.Add(UserResPerms);

        bool bSuccess = false;
        FString JsonString = PubnubSubsystem->GrantTokenStructureToJsonString(Ts, bSuccess);
        TestTrue(TestDescription + TEXT(" - Conversion Success"), bSuccess);

        if (bSuccess)
        {
            TSharedPtr<FJsonObject> RootObject;
            TestTrue(TestDescription + TEXT(" - Is Valid JSON"), UPubnubJsonUtilities::StringToJsonObject(JsonString, RootObject) && RootObject.IsValid());
            if (RootObject.IsValid())
            {
                TestEqual(TestDescription + TEXT(" - TTL"), RootObject->GetIntegerField(TEXT("ttl")), Ts.TTLMinutes);
                TestEqual(TestDescription + TEXT(" - Auth User"), RootObject->GetStringField(TEXT("authorized_uuid")), Ts.AuthorizedUser);

                const TSharedPtr<FJsonObject>* PermissionsObjectPtr;
                TestTrue(TestDescription + TEXT(" - Has 'permissions' object"), RootObject->TryGetObjectField(TEXT("permissions"), PermissionsObjectPtr) && PermissionsObjectPtr && (*PermissionsObjectPtr).IsValid());
                if (PermissionsObjectPtr && (*PermissionsObjectPtr).IsValid())
                {
                    const TSharedPtr<FJsonObject>* ResourcesObjectPtr;
                    TestTrue(TestDescription + TEXT(" - Has 'resources' object"), (*PermissionsObjectPtr)->TryGetObjectField(TEXT("resources"), ResourcesObjectPtr) && ResourcesObjectPtr && (*ResourcesObjectPtr).IsValid());
                    if (ResourcesObjectPtr && (*ResourcesObjectPtr).IsValid())
                    {
                        const TSharedPtr<FJsonObject>* ChannelsObjectPtr;
                        TestTrue(TestDescription + TEXT(" - Resources has 'channels'"), (*ResourcesObjectPtr)->TryGetObjectField(TEXT("channels"), ChannelsObjectPtr) && ChannelsObjectPtr && (*ChannelsObjectPtr).IsValid());
                        if (ChannelsObjectPtr && (*ChannelsObjectPtr).IsValid())
                        {
                            TestEqual(TestDescription + TEXT(" - Channel Perms"), (*ChannelsObjectPtr)->GetIntegerField(Ts.Channels[0]), CalculateChannelPermissionsBitmask(ChanPerms));
                        }
                        const TSharedPtr<FJsonObject>* GroupsObjectPtr;
                        TestTrue(TestDescription + TEXT(" - Resources has 'groups'"), (*ResourcesObjectPtr)->TryGetObjectField(TEXT("groups"), GroupsObjectPtr) && GroupsObjectPtr && (*GroupsObjectPtr).IsValid());
                        if (GroupsObjectPtr && (*GroupsObjectPtr).IsValid())
                        {
                             TestEqual(TestDescription + TEXT(" - Group Perms"), (*GroupsObjectPtr)->GetIntegerField(Ts.ChannelGroups[0]), CalculateChannelGroupPermissionsBitmask(GroupPerms));
                        }
                        const TSharedPtr<FJsonObject>* UuidsObjectPtr;
                        TestTrue(TestDescription + TEXT(" - Resources has 'uuids'"), (*ResourcesObjectPtr)->TryGetObjectField(TEXT("uuids"), UuidsObjectPtr) && UuidsObjectPtr && (*UuidsObjectPtr).IsValid());
                        if (UuidsObjectPtr && (*UuidsObjectPtr).IsValid())
                        {
                            TestEqual(TestDescription + TEXT(" - UUID Resource Perms"), (*UuidsObjectPtr)->GetIntegerField(Ts.Users[0]), CalculateUserPermissionsBitmask(UserResPerms));
                        }
                    }
                     const TSharedPtr<FJsonObject>* PatternsObjectPtr;
                     TestTrue(TestDescription + TEXT(" - Has 'patterns' object"), (*PermissionsObjectPtr)->TryGetObjectField(TEXT("patterns"), PatternsObjectPtr) && PatternsObjectPtr && (*PatternsObjectPtr).IsValid());
                }
            }
        }
    }

    // --- Test Case 2: One Channel Permission for Multiple Channels ---
    {
        const FString TestDescription = TEXT("Case 2: One Perm for Multiple Channels");
        FPubnubGrantTokenStructure Ts;
        Ts.TTLMinutes = 10;
        Ts.AuthorizedUser = SDK_PREFIX + TEXT("auth_user_unit_2");
        
        FPubnubChannelPermissions ChanPerm; ChanPerm.Read = true; // Mask = 1
        Ts.Channels.Add(SDK_PREFIX + TEXT("chanA"));
        Ts.Channels.Add(SDK_PREFIX + TEXT("chanB"));
        Ts.ChannelPermissions.Add(ChanPerm); // Single permission object

        bool bSuccess = false;
        FString JsonString = PubnubSubsystem->GrantTokenStructureToJsonString(Ts, bSuccess);
        TestTrue(TestDescription + TEXT(" - Conversion Success"), bSuccess);
        if (bSuccess)
        {
            TSharedPtr<FJsonObject> RootObject;
            TestTrue(TestDescription + TEXT(" - Is Valid JSON"), UPubnubJsonUtilities::StringToJsonObject(JsonString, RootObject) && RootObject.IsValid());
            // ... (basic TTL, AuthUser checks) ...
            const TSharedPtr<FJsonObject>* PermsObj, *ResObj, *ChansObj;
            if (RootObject->TryGetObjectField(TEXT("permissions"), PermsObj) && (*PermsObj)->TryGetObjectField(TEXT("resources"), ResObj) && (*ResObj)->TryGetObjectField(TEXT("channels"), ChansObj))
            {
                TestEqual(TestDescription + TEXT(" - ChanA Perms"), (*ChansObj)->GetIntegerField(Ts.Channels[0]), CalculateChannelPermissionsBitmask(ChanPerm));
                TestEqual(TestDescription + TEXT(" - ChanB Perms"), (*ChansObj)->GetIntegerField(Ts.Channels[1]), CalculateChannelPermissionsBitmask(ChanPerm));
            } else { AddError(TestDescription + TEXT(" - JSON structure error")); }
        }
    }

    // --- Test Case 3: Matching Channel Permissions for Multiple Channels ---
    {
        const FString TestDescription = TEXT("Case 3: Matching Perms for Multiple Channels");
        FPubnubGrantTokenStructure Ts;
        Ts.TTLMinutes = 10;
        Ts.AuthorizedUser = SDK_PREFIX + TEXT("auth_user_unit_3");

        FPubnubChannelPermissions ChanPermC; ChanPermC.Read = true; // Mask = 1
        FPubnubChannelPermissions ChanPermD; ChanPermD.Write = true; // Mask = 2
        Ts.Channels.Add(SDK_PREFIX + TEXT("chanC"));
        Ts.Channels.Add(SDK_PREFIX + TEXT("chanD"));
        Ts.ChannelPermissions.Add(ChanPermC);
        Ts.ChannelPermissions.Add(ChanPermD);

        bool bSuccess = false;
        FString JsonString = PubnubSubsystem->GrantTokenStructureToJsonString(Ts, bSuccess);
        TestTrue(TestDescription + TEXT(" - Conversion Success"), bSuccess);
         if (bSuccess)
        {
            TSharedPtr<FJsonObject> RootObject;
            TestTrue(TestDescription + TEXT(" - Is Valid JSON"), UPubnubJsonUtilities::StringToJsonObject(JsonString, RootObject) && RootObject.IsValid());
            const TSharedPtr<FJsonObject>* PermsObj, *ResObj, *ChansObj;
            if (RootObject->TryGetObjectField(TEXT("permissions"), PermsObj) && (*PermsObj)->TryGetObjectField(TEXT("resources"), ResObj) && (*ResObj)->TryGetObjectField(TEXT("channels"), ChansObj))
            {
                TestEqual(TestDescription + TEXT(" - ChanC Perms"), (*ChansObj)->GetIntegerField(Ts.Channels[0]), CalculateChannelPermissionsBitmask(ChanPermC));
                TestEqual(TestDescription + TEXT(" - ChanD Perms"), (*ChansObj)->GetIntegerField(Ts.Channels[1]), CalculateChannelPermissionsBitmask(ChanPermD));
            } else { AddError(TestDescription + TEXT(" - JSON structure error")); }
        }
    }

    // --- Test Case 4: Mismatched Channel Permissions (Fail Case) ---
    {
        const FString TestDescription = TEXT("Case 4: Mismatched Perms (3 chans, 2 perms)");
        FPubnubGrantTokenStructure Ts;
        Ts.TTLMinutes = 10;
        Ts.AuthorizedUser = SDK_PREFIX + TEXT("auth_user_unit_4");

        FPubnubChannelPermissions ChanPermE; ChanPermE.Read = true;
        FPubnubChannelPermissions ChanPermF; ChanPermF.Write = true;
        Ts.Channels.Add(SDK_PREFIX + TEXT("chanE"));
        Ts.Channels.Add(SDK_PREFIX + TEXT("chanF"));
        Ts.Channels.Add(SDK_PREFIX + TEXT("chanG"));
        Ts.ChannelPermissions.Add(ChanPermE);
        Ts.ChannelPermissions.Add(ChanPermF); // 3 channels, 2 permissions

        bool bSuccess = true; // Expecting this to be set to false by the function
        FString JsonString = PubnubSubsystem->GrantTokenStructureToJsonString(Ts, bSuccess);
        TestFalse(TestDescription + TEXT(" - Conversion should Fail"), bSuccess);
    }
    
    // --- Test Case 5: Empty Resources and Patterns ---
    {
        const FString TestDescription = TEXT("Case 5: Empty Resources and Patterns");
        FPubnubGrantTokenStructure Ts;
        Ts.TTLMinutes = 5;
        Ts.AuthorizedUser = SDK_PREFIX + TEXT("auth_user_unit_5");

        bool bSuccess = false;
        FString JsonString = PubnubSubsystem->GrantTokenStructureToJsonString(Ts, bSuccess);
        TestTrue(TestDescription + TEXT(" - Conversion Success"), bSuccess);
        if(bSuccess)
        {
            TSharedPtr<FJsonObject> RootObject;
            TestTrue(TestDescription + TEXT(" - Is Valid JSON"), UPubnubJsonUtilities::StringToJsonObject(JsonString, RootObject) && RootObject.IsValid());
            if (RootObject.IsValid())
            {
                 TestEqual(TestDescription + TEXT(" - TTL"), RootObject->GetIntegerField(TEXT("ttl")), Ts.TTLMinutes);
                 TestEqual(TestDescription + TEXT(" - Auth User"), RootObject->GetStringField(TEXT("authorized_uuid")), Ts.AuthorizedUser);
                 const TSharedPtr<FJsonObject>* PermsObj;
                 TestTrue(TestDescription + TEXT(" - Has 'permissions' object"), RootObject->TryGetObjectField(TEXT("permissions"), PermsObj) && PermsObj && (*PermsObj).IsValid());
                 if(PermsObj && (*PermsObj).IsValid())
                 {
                    const TSharedPtr<FJsonObject>* ResObj;
                    TestTrue(TestDescription + TEXT(" - Perms has 'resources' object"), (*PermsObj)->TryGetObjectField(TEXT("resources"), ResObj) && ResObj && (*ResObj).IsValid());
                    if(ResObj && (*ResObj).IsValid()) TestEqual(TestDescription + TEXT(" - Resources is empty"), (*ResObj)->Values.Num(), 0);
                    
                    const TSharedPtr<FJsonObject>* PatObj;
                    TestTrue(TestDescription + TEXT(" - Perms has 'patterns' object"), (*PermsObj)->TryGetObjectField(TEXT("patterns"), PatObj) && PatObj && (*PatObj).IsValid());
                    if(PatObj && (*PatObj).IsValid()) TestEqual(TestDescription + TEXT(" - Patterns is empty"), (*PatObj)->Values.Num(), 0);
                 }
            }
        }
    }

    // --- Test Case 6: Only Patterns ---
    {
        const FString TestDescription = TEXT("Case 6: Only Patterns");
        FPubnubGrantTokenStructure Ts;
        Ts.TTLMinutes = 15;
        Ts.AuthorizedUser = SDK_PREFIX + TEXT("auth_user_unit_6");

        FPubnubChannelPermissions ChanPatternPerms; ChanPatternPerms.Manage = true; // Mask = 4
        Ts.ChannelPatterns.Add(SDK_PREFIX + TEXT("chan-pat-*"));
        Ts.ChannelPatternPermissions.Add(ChanPatternPerms);

        bool bSuccess = false;
        FString JsonString = PubnubSubsystem->GrantTokenStructureToJsonString(Ts, bSuccess);
        TestTrue(TestDescription + TEXT(" - Conversion Success"), bSuccess);
        if (bSuccess)
        {
            TSharedPtr<FJsonObject> RootObject;
            TestTrue(TestDescription + TEXT(" - Is Valid JSON"), UPubnubJsonUtilities::StringToJsonObject(JsonString, RootObject) && RootObject.IsValid());
            if (RootObject.IsValid())
            {
                // ... (basic TTL, AuthUser checks) ...
                const TSharedPtr<FJsonObject>* PermsObj, *ResObj, *PatObj, *ChanPatObj;
                if (RootObject->TryGetObjectField(TEXT("permissions"), PermsObj) &&
                    (*PermsObj)->TryGetObjectField(TEXT("resources"), ResObj) &&
                    (*PermsObj)->TryGetObjectField(TEXT("patterns"), PatObj) &&
                    (*PatObj)->TryGetObjectField(TEXT("channels"), ChanPatObj))
                {
                    TestEqual(TestDescription + TEXT(" - Resources is empty"), (*ResObj)->Values.Num(), 0);
                    TestEqual(TestDescription + TEXT(" - Pattern Perms"), (*ChanPatObj)->GetIntegerField(Ts.ChannelPatterns[0]), CalculateChannelPermissionsBitmask(ChanPatternPerms));
                } else { AddError(TestDescription + TEXT(" - JSON structure error")); }
            }
        }
    }

    CleanUp();
    return true;
}



bool FPubnubGrantAndParseTokenTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestAuthUser = SDK_PREFIX + TEXT("auth_user_grant_parse");
    const FString TestChannelName = SDK_PREFIX + TEXT("channel_grant_parse");
    const FString TestGroupName = SDK_PREFIX + TEXT("group_grant_parse");
    const FString TestTargetResourceUID = SDK_PREFIX + TEXT("target_uuid_grant_parse"); // For UUID-specific permissions
    const int TestTTLMinutes = 60;

    TSharedPtr<FString> GrantedToken = MakeShared<FString>();
    TSharedPtr<bool> bGrantTokenSuccess = MakeShared<bool>(false);
    TSharedPtr<bool> bGrantTokenCallbackReceived = MakeShared<bool>(false);

    TSharedPtr<FString> ParsedTokenResponseJson = MakeShared<FString>();
    TSharedPtr<bool> bParseTokenSuccess = MakeShared<bool>(false);
    TSharedPtr<bool> bParseTokenCallbackReceived = MakeShared<bool>(false);

    if (!InitTest())
    {
        AddError(TEXT("TestInitialization failed for FPubnubGrantAndParseTokenTest"));
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
        AddError(TEXT("Failed to convert GrantTokenStructure to JSON. Check logs for details from PubnubSubsystem."));
        CleanUp();
        return false;
    }
    TestTrue(TEXT("GrantTokenStructureToJsonString conversion success"), bJsonConversionSuccess);


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
            AddError(TEXT("GrantToken response was empty. Expected a token string."));
            *bGrantTokenSuccess = false;
        }
    });

    // ParseToken callback
    FOnPubnubResponseNative ParseTokenCallback;
    ParseTokenCallback.BindLambda([this, ParsedTokenResponseJson, bParseTokenSuccess, bParseTokenCallbackReceived](FString JsonResponse)
    {
        *bParseTokenCallbackReceived = true;
        *ParsedTokenResponseJson = JsonResponse;
        // Basic check: if we got any non-empty response, assume the call itself was "successful" at transport layer
        // Deeper validation of content will happen in subsequent latent command.
        if (!JsonResponse.IsEmpty())
        {
            // ParseToken returns the full parsed token structure as JSON string.
            // The PubNub C-Core returns a string that is already a JSON object representing the token.
            // Success here means we got a string. Further validation of the string content is needed.
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
            AddError(TEXT("ParseToken response was empty."));
        }
    });


    // --- Test Execution ---

    // Set UserID (required for some PubNub operations, though Grant is admin-side typically)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestAuthUser]()
    {
        PubnubSubsystem->SetUserID(TestAuthUser); 
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
        TestTrue(TEXT("GrantToken operation was successful (callback received and token extracted)"), *bGrantTokenSuccess);
        if (*bGrantTokenSuccess && !GrantedToken->IsEmpty())
        {
            PubnubSubsystem->ParseToken(*GrantedToken, ParseTokenCallback);
        }
        else if (GrantedToken->IsEmpty())
        {
            AddError(TEXT("Granted token string is empty, cannot proceed to ParseToken."));
        }
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bParseTokenCallbackReceived, bGrantTokenSuccess](){ return *bParseTokenCallbackReceived || !*bGrantTokenSuccess; }, MAX_WAIT_TIME)); // Wait if grant was successful

    // Step 3: Verify Parsed Token
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bParseTokenSuccess, ParsedTokenResponseJson, TestTTLMinutes, TestChannelName, ExpectedChannelBitmask, TestGroupName, ExpectedGroupBitmask, TestTargetResourceUID, ExpectedUserResourceBitmask]()
    {
        TestTrue(TEXT("ParseToken operation was successful (callback received and response is valid JSON)"), *bParseTokenSuccess);
        if (!*bParseTokenSuccess || ParsedTokenResponseJson->IsEmpty())
        {
            AddError(TEXT("Skipping ParsedToken verification due to previous errors or empty response."));
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
             TestEqual(TEXT("Parsed token TTL matches granted TTL"), ParsedTTL, TestTTLMinutes);
        } else { AddError(TEXT("Parsed token JSON does not contain 'ttl' field.")); }
        
        const TSharedPtr<FJsonObject>* ResourcesObjectPtr; // This will now point to the "res" object
        if (!ParsedTokenObject->TryGetObjectField(TEXT("res"), ResourcesObjectPtr) || !ResourcesObjectPtr || !(*ResourcesObjectPtr).IsValid())
        {
            AddError(TEXT("Parsed token JSON does not contain 'res' (resources) object."));
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
