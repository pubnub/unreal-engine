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

//This is an Unit test, but it still requires getting Pubnub subsystem, that's why it's here, not with other Unit tests
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGrantTokenPermissionsStructureUnitTest, FPubnubAutomationTestBase, "Pubnub.aUnit.AccessManager.GrantTokenPermissionsStructure", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubReworkParsedTokenUnitTest, FPubnubAutomationTestBase, "Pubnub.aUnit.AccessManager.ReworkParsedTokenUnit", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

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

bool FPubnubGrantTokenPermissionsStructureUnitTest::RunTest(const FString& Parameters)
{
    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubGrantTokenPermissionsStructureUnitTest");
        return false;
    }

    // --- Test Case 1: Basic Valid Structure (similar to E2E test) ---
    {
        const FString TestDescription = "Case 1: Basic Valid Structure";
        FPubnubGrantTokenPermissions Permissions;
        
        // Set up channel permissions
        FChannelGrant ChannelGrant;
        ChannelGrant.Channel = SDK_PREFIX + "chan1";
        ChannelGrant.Permissions.Read = true;
        ChannelGrant.Permissions.Write = true; // Mask = 3
        Permissions.Channels.Add(ChannelGrant);

        // Set up channel group permissions
        FChannelGroupGrant GroupGrant;
        GroupGrant.ChannelGroup = SDK_PREFIX + "group1";
        GroupGrant.Permissions.Read = true;
        GroupGrant.Permissions.Manage = true; // Mask = 5
        Permissions.ChannelGroups.Add(GroupGrant);

        // Set up user permissions
        FUserGrant UserGrant;
        UserGrant.User = SDK_PREFIX + "uuid_res1";
        UserGrant.Permissions.Get = true;
        UserGrant.Permissions.Update = true; // Mask = 96
        Permissions.Users.Add(UserGrant);

        // Test structure validity
        TestFalse(TestDescription + " - Permissions should not be empty", Permissions.ArePermissionsEmpty());
        TestEqual(TestDescription + " - Channel count", Permissions.Channels.Num(), 1);
        TestEqual(TestDescription + " - Channel group count", Permissions.ChannelGroups.Num(), 1);
        TestEqual(TestDescription + " - User count", Permissions.Users.Num(), 1);
        
        // Test individual permission settings
        TestTrue(TestDescription + " - Channel Read permission", Permissions.Channels[0].Permissions.Read);
        TestTrue(TestDescription + " - Channel Write permission", Permissions.Channels[0].Permissions.Write);
        TestTrue(TestDescription + " - Group Read permission", Permissions.ChannelGroups[0].Permissions.Read);
        TestTrue(TestDescription + " - Group Manage permission", Permissions.ChannelGroups[0].Permissions.Manage);
        TestTrue(TestDescription + " - User Get permission", Permissions.Users[0].Permissions.Get);
        TestTrue(TestDescription + " - User Update permission", Permissions.Users[0].Permissions.Update);
    }

    // --- Test Case 2: Multiple Channels with Different Permissions ---
    {
        const FString TestDescription = "Case 2: Multiple Channels with Different Permissions";
        FPubnubGrantTokenPermissions Permissions;
        
        // Set up first channel with read permission
        FChannelGrant ChannelGrantA;
        ChannelGrantA.Channel = SDK_PREFIX + "chanA";
        ChannelGrantA.Permissions.Read = true; // Mask = 1
        Permissions.Channels.Add(ChannelGrantA);

        // Set up second channel with write permission
        FChannelGrant ChannelGrantB;
        ChannelGrantB.Channel = SDK_PREFIX + "chanB";
        ChannelGrantB.Permissions.Write = true; // Mask = 2
        Permissions.Channels.Add(ChannelGrantB);

        // Test structure validity
        TestFalse(TestDescription + " - Permissions should not be empty", Permissions.ArePermissionsEmpty());
        TestEqual(TestDescription + " - Channel count", Permissions.Channels.Num(), 2);
        
        // Test individual channel permissions
        TestTrue(TestDescription + " - ChanA Read permission", Permissions.Channels[0].Permissions.Read);
        TestFalse(TestDescription + " - ChanA Write permission", Permissions.Channels[0].Permissions.Write);
        TestEqual(TestDescription + " - ChanA name", Permissions.Channels[0].Channel, SDK_PREFIX + "chanA");
        
        TestFalse(TestDescription + " - ChanB Read permission", Permissions.Channels[1].Permissions.Read);
        TestTrue(TestDescription + " - ChanB Write permission", Permissions.Channels[1].Permissions.Write);
        TestEqual(TestDescription + " - ChanB name", Permissions.Channels[1].Channel, SDK_PREFIX + "chanB");
    }

    // --- Test Case 3: Channel Patterns with Permissions ---
    {
        const FString TestDescription = "Case 3: Channel Patterns with Permissions";
        FPubnubGrantTokenPermissions Permissions;

        // Set up channel patterns
        FChannelGrant ChannelPatternC;
        ChannelPatternC.Channel = SDK_PREFIX + "chan-pattern-*";
        ChannelPatternC.Permissions.Read = true; // Mask = 1
        Permissions.ChannelPatterns.Add(ChannelPatternC);

        FChannelGrant ChannelPatternD;
        ChannelPatternD.Channel = SDK_PREFIX + "write-pattern-*";
        ChannelPatternD.Permissions.Write = true; // Mask = 2
        Permissions.ChannelPatterns.Add(ChannelPatternD);

        // Test structure validity
        TestFalse(TestDescription + " - Permissions should not be empty", Permissions.ArePermissionsEmpty());
        TestEqual(TestDescription + " - Channel pattern count", Permissions.ChannelPatterns.Num(), 2);
        
        // Test individual pattern permissions
        TestTrue(TestDescription + " - PatternC Read permission", Permissions.ChannelPatterns[0].Permissions.Read);
        TestFalse(TestDescription + " - PatternC Write permission", Permissions.ChannelPatterns[0].Permissions.Write);
        TestEqual(TestDescription + " - PatternC name", Permissions.ChannelPatterns[0].Channel, SDK_PREFIX + "chan-pattern-*");
        
        TestFalse(TestDescription + " - PatternD Read permission", Permissions.ChannelPatterns[1].Permissions.Read);
        TestTrue(TestDescription + " - PatternD Write permission", Permissions.ChannelPatterns[1].Permissions.Write);
        TestEqual(TestDescription + " - PatternD name", Permissions.ChannelPatterns[1].Channel, SDK_PREFIX + "write-pattern-*");
    }

    // --- Test Case 4: Mixed Resource Types ---
    {
        const FString TestDescription = "Case 4: Mixed Resource Types";
        FPubnubGrantTokenPermissions Permissions;

        // Set up channel with manage permission
        FChannelGrant ChannelGrant;
        ChannelGrant.Channel = SDK_PREFIX + "chanE";
        ChannelGrant.Permissions.Manage = true;
        Permissions.Channels.Add(ChannelGrant);

        // Set up user with delete permission
        FUserGrant UserGrant;
        UserGrant.User = SDK_PREFIX + "userF";
        UserGrant.Permissions.Delete = true;
        Permissions.Users.Add(UserGrant);

        // Set up channel group pattern
        FChannelGroupGrant GroupPatternGrant;
        GroupPatternGrant.ChannelGroup = SDK_PREFIX + "group-pattern-*";
        GroupPatternGrant.Permissions.Read = true;
        GroupPatternGrant.Permissions.Manage = true;
        Permissions.ChannelGroupPatterns.Add(GroupPatternGrant);

        // Test structure validity
        TestFalse(TestDescription + " - Permissions should not be empty", Permissions.ArePermissionsEmpty());
        TestEqual(TestDescription + " - Channel count", Permissions.Channels.Num(), 1);
        TestEqual(TestDescription + " - User count", Permissions.Users.Num(), 1);
        TestEqual(TestDescription + " - Channel group pattern count", Permissions.ChannelGroupPatterns.Num(), 1);
        
        // Test individual permissions
        TestTrue(TestDescription + " - Channel Manage permission", Permissions.Channels[0].Permissions.Manage);
        TestTrue(TestDescription + " - User Delete permission", Permissions.Users[0].Permissions.Delete);
        TestTrue(TestDescription + " - Group Pattern Read permission", Permissions.ChannelGroupPatterns[0].Permissions.Read);
        TestTrue(TestDescription + " - Group Pattern Manage permission", Permissions.ChannelGroupPatterns[0].Permissions.Manage);
    }
    
    // --- Test Case 5: Empty Permissions Structure ---
    {
        const FString TestDescription = "Case 5: Empty Permissions Structure";
        FPubnubGrantTokenPermissions Permissions;

        // Test empty structure
        TestTrue(TestDescription + " - Permissions should be empty", Permissions.ArePermissionsEmpty());
        TestEqual(TestDescription + " - Channel count", Permissions.Channels.Num(), 0);
        TestEqual(TestDescription + " - Channel group count", Permissions.ChannelGroups.Num(), 0);
        TestEqual(TestDescription + " - User count", Permissions.Users.Num(), 0);
        TestEqual(TestDescription + " - Channel pattern count", Permissions.ChannelPatterns.Num(), 0);
        TestEqual(TestDescription + " - Channel group pattern count", Permissions.ChannelGroupPatterns.Num(), 0);
        TestEqual(TestDescription + " - User pattern count", Permissions.UserPatterns.Num(), 0);
    }

    // --- Test Case 6: Only Patterns ---
    {
        const FString TestDescription = "Case 6: Only Patterns";
        FPubnubGrantTokenPermissions Permissions;

        // Set up channel pattern
        FChannelGrant ChannelPatternGrant;
        ChannelPatternGrant.Channel = SDK_PREFIX + "chan-pat-*";
        ChannelPatternGrant.Permissions.Manage = true; // Mask = 4
        Permissions.ChannelPatterns.Add(ChannelPatternGrant);

        // Set up user pattern
        FUserGrant UserPatternGrant;
        UserPatternGrant.User = SDK_PREFIX + "user-pat-*";
        UserPatternGrant.Permissions.Get = true;
        UserPatternGrant.Permissions.Update = true;
        Permissions.UserPatterns.Add(UserPatternGrant);

        // Test structure validity
        TestFalse(TestDescription + " - Permissions should not be empty", Permissions.ArePermissionsEmpty());
        TestEqual(TestDescription + " - Channel count", Permissions.Channels.Num(), 0);
        TestEqual(TestDescription + " - Channel group count", Permissions.ChannelGroups.Num(), 0);
        TestEqual(TestDescription + " - User count", Permissions.Users.Num(), 0);
        TestEqual(TestDescription + " - Channel pattern count", Permissions.ChannelPatterns.Num(), 1);
        TestEqual(TestDescription + " - Channel group pattern count", Permissions.ChannelGroupPatterns.Num(), 0);
        TestEqual(TestDescription + " - User pattern count", Permissions.UserPatterns.Num(), 1);
        
        // Test pattern permissions
        TestTrue(TestDescription + " - Channel Pattern Manage permission", Permissions.ChannelPatterns[0].Permissions.Manage);
        TestEqual(TestDescription + " - Channel Pattern name", Permissions.ChannelPatterns[0].Channel, SDK_PREFIX + "chan-pat-*");
        TestTrue(TestDescription + " - User Pattern Get permission", Permissions.UserPatterns[0].Permissions.Get);
        TestTrue(TestDescription + " - User Pattern Update permission", Permissions.UserPatterns[0].Permissions.Update);
        TestEqual(TestDescription + " - User Pattern name", Permissions.UserPatterns[0].User, SDK_PREFIX + "user-pat-*");
    }

    CleanUp();
    return true;
}

bool FPubnubReworkParsedTokenUnitTest::RunTest(const FString& Parameters)
{
    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubReworkParsedTokenUnitTest");
        return false;
    }

    // Test with a sample parsed token JSON (compact format)
    FString SampleParsedToken = TEXT(R"({"v":2,"t":1752219810,"ttl":30,"res":{"chan":{"my_channel":239},"grp":{"my_group":5},"uuid":{"User1":104}},"pat":{"chan":{"chan_pattern*":7},"grp":{},"uuid":{"user_pattern*":96}}})");
    
    // Call the ReworkParsedToken function
    FString ReworkedToken = UPubnubTokenUtilities::ReworkParsedToken(SampleParsedToken);
    
    // Verify the function returned a non-empty result
    TestFalse("ReworkParsedToken should return non-empty result", ReworkedToken.IsEmpty());
    
    if (!ReworkedToken.IsEmpty())
    {
        // Parse the reworked token to verify its structure
        TSharedPtr<FJsonObject> ReworkedTokenObject;
        TestTrue("ReworkedToken should be valid JSON", UPubnubJsonUtilities::StringToJsonObject(ReworkedToken, ReworkedTokenObject) && ReworkedTokenObject.IsValid());
        
        if (ReworkedTokenObject.IsValid())
        {
            // Verify main fields
            TestEqual("Version should be 2", static_cast<int>(ReworkedTokenObject->GetNumberField(TEXT("Version"))), 2);
            TestEqual("Timestamp should match", static_cast<int64>(ReworkedTokenObject->GetNumberField(TEXT("Timestamp"))), 1752219810);
            TestEqual("TTL should be 30", static_cast<int>(ReworkedTokenObject->GetNumberField(TEXT("TTL"))), 30);
            
            // Verify Resources structure
            const TSharedPtr<FJsonObject>* ResourcesPtr = nullptr;
            TestTrue("Should have Resources object", ReworkedTokenObject->TryGetObjectField(TEXT("Resources"), ResourcesPtr) && ResourcesPtr && (*ResourcesPtr).IsValid());
            
            if (ResourcesPtr && (*ResourcesPtr).IsValid())
            {
                const TSharedPtr<FJsonObject>& Resources = *ResourcesPtr;
                
                // Test channel permissions (bitmask 239 = 1+2+4+8+32+64+128 = Read+Write+Manage+Delete+Get+Update+Join)
                const TSharedPtr<FJsonObject>* ChannelsPtr = nullptr;
                TestTrue("Should have Channels in Resources", Resources->TryGetObjectField(TEXT("Channels"), ChannelsPtr) && ChannelsPtr && (*ChannelsPtr).IsValid());
                
                if (ChannelsPtr && (*ChannelsPtr).IsValid())
                {
                    const TSharedPtr<FJsonObject>* ChannelPermsPtr = nullptr;
                    TestTrue("Should have my_channel permissions", (*ChannelsPtr)->TryGetObjectField(TEXT("my_channel"), ChannelPermsPtr) && ChannelPermsPtr && (*ChannelPermsPtr).IsValid());
                    
                    if (ChannelPermsPtr && (*ChannelPermsPtr).IsValid())
                    {
                        TestTrue("Channel should have Read permission", (*ChannelPermsPtr)->GetBoolField(TEXT("Read")));
                        TestTrue("Channel should have Write permission", (*ChannelPermsPtr)->GetBoolField(TEXT("Write")));
                        TestTrue("Channel should have Manage permission", (*ChannelPermsPtr)->GetBoolField(TEXT("Manage")));
                        TestTrue("Channel should have Delete permission", (*ChannelPermsPtr)->GetBoolField(TEXT("Delete")));
                        TestTrue("Channel should have Get permission", (*ChannelPermsPtr)->GetBoolField(TEXT("Get")));
                        TestTrue("Channel should have Update permission", (*ChannelPermsPtr)->GetBoolField(TEXT("Update")));
                        TestTrue("Channel should have Join permission", (*ChannelPermsPtr)->GetBoolField(TEXT("Join")));
                    }
                }
                
                // Test channel group permissions (bitmask 5 = 1+4 = Read+Manage)
                const TSharedPtr<FJsonObject>* ChannelGroupsPtr = nullptr;
                TestTrue("Should have ChannelGroups in Resources", Resources->TryGetObjectField(TEXT("ChannelGroups"), ChannelGroupsPtr) && ChannelGroupsPtr && (*ChannelGroupsPtr).IsValid());
                
                if (ChannelGroupsPtr && (*ChannelGroupsPtr).IsValid())
                {
                    const TSharedPtr<FJsonObject>* GroupPermsPtr = nullptr;
                    TestTrue("Should have my_group permissions", (*ChannelGroupsPtr)->TryGetObjectField(TEXT("my_group"), GroupPermsPtr) && GroupPermsPtr && (*GroupPermsPtr).IsValid());
                    
                    if (GroupPermsPtr && (*GroupPermsPtr).IsValid())
                    {
                        TestTrue("Group should have Read permission", (*GroupPermsPtr)->GetBoolField(TEXT("Read")));
                        TestTrue("Group should have Manage permission", (*GroupPermsPtr)->GetBoolField(TEXT("Manage")));
                    }
                }
                
                // Test user permissions (bitmask 104 = 8+32+64 = Delete+Get+Update)
                const TSharedPtr<FJsonObject>* UuidsPtr = nullptr;
                TestTrue("Should have Uuids in Resources", Resources->TryGetObjectField(TEXT("Uuids"), UuidsPtr) && UuidsPtr && (*UuidsPtr).IsValid());
                
                if (UuidsPtr && (*UuidsPtr).IsValid())
                {
                    const TSharedPtr<FJsonObject>* UserPermsPtr = nullptr;
                    TestTrue("Should have User1 permissions", (*UuidsPtr)->TryGetObjectField(TEXT("User1"), UserPermsPtr) && UserPermsPtr && (*UserPermsPtr).IsValid());
                    
                    if (UserPermsPtr && (*UserPermsPtr).IsValid())
                    {
                        TestTrue("User should have Delete permission", (*UserPermsPtr)->GetBoolField(TEXT("Delete")));
                        TestTrue("User should have Get permission", (*UserPermsPtr)->GetBoolField(TEXT("Get")));
                        TestTrue("User should have Update permission", (*UserPermsPtr)->GetBoolField(TEXT("Update")));
                    }
                }
            }
            
            // Verify Patterns structure
            const TSharedPtr<FJsonObject>* PatternsPtr = nullptr;
            TestTrue("Should have Patterns object", ReworkedTokenObject->TryGetObjectField(TEXT("Patterns"), PatternsPtr) && PatternsPtr && (*PatternsPtr).IsValid());
            
            if (PatternsPtr && (*PatternsPtr).IsValid())
            {
                const TSharedPtr<FJsonObject>& Patterns = *PatternsPtr;
                
                // Test channel pattern permissions (bitmask 7 = 1+2+4 = Read+Write+Manage)
                const TSharedPtr<FJsonObject>* ChannelPatternsPtr = nullptr;
                TestTrue("Should have Channels in Patterns", Patterns->TryGetObjectField(TEXT("Channels"), ChannelPatternsPtr) && ChannelPatternsPtr && (*ChannelPatternsPtr).IsValid());
                
                if (ChannelPatternsPtr && (*ChannelPatternsPtr).IsValid())
                {
                    const TSharedPtr<FJsonObject>* ChannelPatternPermsPtr = nullptr;
                    TestTrue("Should have chan_pattern* permissions", (*ChannelPatternsPtr)->TryGetObjectField(TEXT("chan_pattern*"), ChannelPatternPermsPtr) && ChannelPatternPermsPtr && (*ChannelPatternPermsPtr).IsValid());
                    
                    if (ChannelPatternPermsPtr && (*ChannelPatternPermsPtr).IsValid())
                    {
                        TestTrue("Channel pattern should have Read permission", (*ChannelPatternPermsPtr)->GetBoolField(TEXT("Read")));
                        TestTrue("Channel pattern should have Write permission", (*ChannelPatternPermsPtr)->GetBoolField(TEXT("Write")));
                        TestTrue("Channel pattern should have Manage permission", (*ChannelPatternPermsPtr)->GetBoolField(TEXT("Manage")));
                        TestFalse("Channel pattern should not have Delete permission", (*ChannelPatternPermsPtr)->GetBoolField(TEXT("Delete")));
                        TestFalse("Channel pattern should not have Get permission", (*ChannelPatternPermsPtr)->GetBoolField(TEXT("Get")));
                        TestFalse("Channel pattern should not have Update permission", (*ChannelPatternPermsPtr)->GetBoolField(TEXT("Update")));
                        TestFalse("Channel pattern should not have Join permission", (*ChannelPatternPermsPtr)->GetBoolField(TEXT("Join")));
                    }
                }
                
                // Test user pattern permissions (bitmask 96 = 32+64 = Get+Update)
                const TSharedPtr<FJsonObject>* UuidPatternsPtr = nullptr;
                TestTrue("Should have Uuids in Patterns", Patterns->TryGetObjectField(TEXT("Uuids"), UuidPatternsPtr) && UuidPatternsPtr && (*UuidPatternsPtr).IsValid());
                
                if (UuidPatternsPtr && (*UuidPatternsPtr).IsValid())
                {
                    const TSharedPtr<FJsonObject>* UserPatternPermsPtr = nullptr;
                    TestTrue("Should have user_pattern* permissions", (*UuidPatternsPtr)->TryGetObjectField(TEXT("user_pattern*"), UserPatternPermsPtr) && UserPatternPermsPtr && (*UserPatternPermsPtr).IsValid());
                    
                    if (UserPatternPermsPtr && (*UserPatternPermsPtr).IsValid())
                    {
                        TestFalse("User pattern should not have Delete permission", (*UserPatternPermsPtr)->GetBoolField(TEXT("Delete")));
                        TestTrue("User pattern should have Get permission", (*UserPatternPermsPtr)->GetBoolField(TEXT("Get")));
                        TestTrue("User pattern should have Update permission", (*UserPatternPermsPtr)->GetBoolField(TEXT("Update")));
                    }
                }
            }
        }
    }
    
    // Test with empty input
    FString EmptyResult = UPubnubTokenUtilities::ReworkParsedToken(TEXT(""));
    TestTrue("Empty input should return empty result", EmptyResult.IsEmpty());
    
    // Test with invalid JSON
    FString InvalidResult = UPubnubTokenUtilities::ReworkParsedToken(TEXT("invalid json"));
    TestTrue("Invalid JSON should return empty result", InvalidResult.IsEmpty());

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
    const int ExpectedChannelBitmask = CalculateChannelPermissionsBitmask(ChannelGrant.Permissions);

    // Set up channel group permissions
    FChannelGroupGrant GroupGrant;
    GroupGrant.ChannelGroup = TestGroupName;
    GroupGrant.Permissions.Read = true;
    GroupGrant.Permissions.Manage = true;
    TokenPermissions.ChannelGroups.Add(GroupGrant);
    const int ExpectedGroupBitmask = CalculateChannelGroupPermissionsBitmask(GroupGrant.Permissions);

    // Set up user permissions
    FUserGrant UserGrant;
    UserGrant.User = TestTargetResourceUID;
    UserGrant.Permissions.Get = true;
    UserGrant.Permissions.Update = true;
    TokenPermissions.Users.Add(UserGrant);
    const int ExpectedUserResourceBitmask = CalculateUserPermissionsBitmask(UserGrant.Permissions);
    
    TestFalse("TokenPermissions should not be empty", TokenPermissions.ArePermissionsEmpty());


    // GrantToken callback
    FOnGrantTokenResponseNative GrantTokenCallback;
    GrantTokenCallback.BindLambda([this, GrantedToken, bGrantTokenSuccess, bGrantTokenCallbackReceived](const FPubnubOperationResult& Result, FString Token)
    {
        *bGrantTokenCallbackReceived = true;
        if (!Token.IsEmpty())
        {
            *GrantedToken = Token; // The response is the token itself
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
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestAuthUser]()
    {
        PubnubSubsystem->SetSecretKey(); 
    }, 0.1f));

    // Step 1: Grant Token
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestTTLMinutes, TestAuthUser, TokenPermissions, GrantTokenCallback]()
    {
        PubnubSubsystem->GrantToken(TestTTLMinutes, TestAuthUser, TokenPermissions, GrantTokenCallback);
    }, 0.2f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGrantTokenCallbackReceived](){ return *bGrantTokenCallbackReceived; }, MAX_WAIT_TIME));

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
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGrantTokenSuccess, bParseTokenSuccess](){ return *bParseTokenSuccess || !*bGrantTokenSuccess; }, MAX_WAIT_TIME)); // Wait if grant was successful

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

    // Prepare GrantToken permissions for a temporary token
    FPubnubGrantTokenPermissions TokenPermissions;
    FChannelGrant ChannelGrant;
    ChannelGrant.Channel = TestChannelForGrant;
    ChannelGrant.Permissions.Read = true;
    TokenPermissions.Channels.Add(ChannelGrant);

    TestFalse("TokenPermissions should not be empty", TokenPermissions.ArePermissionsEmpty());

    // GrantToken callback (copied from GrantAndParse, simplified as we just need the token string)
    FOnGrantTokenResponseNative GrantTokenCallback;
    GrantTokenCallback.BindLambda([this, GrantedToken, bGrantTokenSuccess, bGrantTokenCallbackReceived](const FPubnubOperationResult& Result, FString Token)
    {
        *bGrantTokenCallbackReceived = true;
        if (!Token.IsEmpty())
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