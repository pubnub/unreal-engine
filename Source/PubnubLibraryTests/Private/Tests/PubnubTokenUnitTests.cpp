// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "PubnubStructLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "FunctionLibraries/PubnubTokenUtilities.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "Dom/JsonObject.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPubnubGrantTokenPermissionsStructureUnitTest, "Pubnub.aUnit.TokenUtilities.PubnubGrantTokenPermissionsStructure", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPubnubReworkParsedTokenUnitTest, "Pubnub.aUnit.TokenUtilities.PubnubReworkParsedToken", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCalculateChannelPermissionsBitmaskUnitTest, "Pubnub.aUnit.TokenUtilities.CalculateChannelPermissionsBitmask", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCalculateChannelGroupPermissionsBitmaskUnitTest, "Pubnub.aUnit.TokenUtilities.CalculateChannelGroupPermissionsBitmask", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCalculateUserPermissionsBitmaskUnitTest, "Pubnub.aUnit.TokenUtilities.CalculateUserPermissionsBitmask", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter);


bool FPubnubGrantTokenPermissionsStructureUnitTest::RunTest(const FString& Parameters)
{
    // --- Test Case 1: Basic Valid Structure (similar to E2E test) ---
    {
        const FString TestDescription = "Case 1: Basic Valid Structure";
        FPubnubGrantTokenPermissions Permissions;
        
        // Set up channel permissions
        FChannelGrant ChannelGrant;
        ChannelGrant.Channel = "chan1";
        ChannelGrant.Permissions.Read = true;
        ChannelGrant.Permissions.Write = true; // Mask = 3
        Permissions.Channels.Add(ChannelGrant);

        // Set up channel group permissions
        FChannelGroupGrant GroupGrant;
        GroupGrant.ChannelGroup = "group1";
        GroupGrant.Permissions.Read = true;
        GroupGrant.Permissions.Manage = true; // Mask = 5
        Permissions.ChannelGroups.Add(GroupGrant);

        // Set up user permissions
        FUserGrant UserGrant;
        UserGrant.User = "uuid_res1";
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
        ChannelGrantA.Channel = "chanA";
        ChannelGrantA.Permissions.Read = true; // Mask = 1
        Permissions.Channels.Add(ChannelGrantA);

        // Set up second channel with write permission
        FChannelGrant ChannelGrantB;
        ChannelGrantB.Channel = "chanB";
        ChannelGrantB.Permissions.Write = true; // Mask = 2
        Permissions.Channels.Add(ChannelGrantB);

        // Test structure validity
        TestFalse(TestDescription + " - Permissions should not be empty", Permissions.ArePermissionsEmpty());
        TestEqual(TestDescription + " - Channel count", Permissions.Channels.Num(), 2);
        
        // Test individual channel permissions
        TestTrue(TestDescription + " - ChanA Read permission", Permissions.Channels[0].Permissions.Read);
        TestFalse(TestDescription + " - ChanA Write permission", Permissions.Channels[0].Permissions.Write);
        TestEqual(TestDescription + " - ChanA name", Permissions.Channels[0].Channel, "chanA");
        
        TestFalse(TestDescription + " - ChanB Read permission", Permissions.Channels[1].Permissions.Read);
        TestTrue(TestDescription + " - ChanB Write permission", Permissions.Channels[1].Permissions.Write);
        TestEqual(TestDescription + " - ChanB name", Permissions.Channels[1].Channel, "chanB");
    }

    // --- Test Case 3: Channel Patterns with Permissions ---
    {
        const FString TestDescription = "Case 3: Channel Patterns with Permissions";
        FPubnubGrantTokenPermissions Permissions;

        // Set up channel patterns
        FChannelGrant ChannelPatternC;
        ChannelPatternC.Channel = "chan-pattern-*";
        ChannelPatternC.Permissions.Read = true; // Mask = 1
        Permissions.ChannelPatterns.Add(ChannelPatternC);

        FChannelGrant ChannelPatternD;
        ChannelPatternD.Channel = "write-pattern-*";
        ChannelPatternD.Permissions.Write = true; // Mask = 2
        Permissions.ChannelPatterns.Add(ChannelPatternD);

        // Test structure validity
        TestFalse(TestDescription + " - Permissions should not be empty", Permissions.ArePermissionsEmpty());
        TestEqual(TestDescription + " - Channel pattern count", Permissions.ChannelPatterns.Num(), 2);
        
        // Test individual pattern permissions
        TestTrue(TestDescription + " - PatternC Read permission", Permissions.ChannelPatterns[0].Permissions.Read);
        TestFalse(TestDescription + " - PatternC Write permission", Permissions.ChannelPatterns[0].Permissions.Write);
        TestEqual(TestDescription + " - PatternC name", Permissions.ChannelPatterns[0].Channel, "chan-pattern-*");
        
        TestFalse(TestDescription + " - PatternD Read permission", Permissions.ChannelPatterns[1].Permissions.Read);
        TestTrue(TestDescription + " - PatternD Write permission", Permissions.ChannelPatterns[1].Permissions.Write);
        TestEqual(TestDescription + " - PatternD name", Permissions.ChannelPatterns[1].Channel, "write-pattern-*");
    }

    // --- Test Case 4: Mixed Resource Types ---
    {
        const FString TestDescription = "Case 4: Mixed Resource Types";
        FPubnubGrantTokenPermissions Permissions;

        // Set up channel with manage permission
        FChannelGrant ChannelGrant;
        ChannelGrant.Channel = "chanE";
        ChannelGrant.Permissions.Manage = true;
        Permissions.Channels.Add(ChannelGrant);

        // Set up user with delete permission
        FUserGrant UserGrant;
        UserGrant.User = "userF";
        UserGrant.Permissions.Delete = true;
        Permissions.Users.Add(UserGrant);

        // Set up channel group pattern
        FChannelGroupGrant GroupPatternGrant;
        GroupPatternGrant.ChannelGroup = "group-pattern-*";
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
        ChannelPatternGrant.Channel = "chan-pat-*";
        ChannelPatternGrant.Permissions.Manage = true; // Mask = 4
        Permissions.ChannelPatterns.Add(ChannelPatternGrant);

        // Set up user pattern
        FUserGrant UserPatternGrant;
        UserPatternGrant.User = "user-pat-*";
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
        TestEqual(TestDescription + " - Channel Pattern name", Permissions.ChannelPatterns[0].Channel, "chan-pat-*");
        TestTrue(TestDescription + " - User Pattern Get permission", Permissions.UserPatterns[0].Permissions.Get);
        TestTrue(TestDescription + " - User Pattern Update permission", Permissions.UserPatterns[0].Permissions.Update);
        TestEqual(TestDescription + " - User Pattern name", Permissions.UserPatterns[0].User, "user-pat-*");
    }
    
    return true;
}

bool FPubnubReworkParsedTokenUnitTest::RunTest(const FString& Parameters)
{
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
    
    return true;
}

bool FCalculateChannelPermissionsBitmaskUnitTest::RunTest(const FString& Parameters)
{
    // Test individual permissions
    // Bit values: READ=1, WRITE=2, MANAGE=4, DELETE=8, GET=32, UPDATE=64, JOIN=128
    
    // Test case 1: No permissions (all false)
    {
        FPubnubChannelPermissions Permissions;
        // All permissions are false by default
        int Result = UPubnubTokenUtilities::CalculateChannelPermissionsBitmask(Permissions);
        TestEqual("No permissions should give bitmask 0", Result, 0);
    }
    
    // Test case 2: Individual permissions
    {
        FPubnubChannelPermissions Permissions;
        
        // Test READ permission (bit 1)
        Permissions = FPubnubChannelPermissions(); // Reset
        Permissions.Read = true;
        TestEqual("Read permission should give bitmask 1", UPubnubTokenUtilities::CalculateChannelPermissionsBitmask(Permissions), 1);
        
        // Test WRITE permission (bit 2)
        Permissions = FPubnubChannelPermissions(); // Reset
        Permissions.Write = true;
        TestEqual("Write permission should give bitmask 2", UPubnubTokenUtilities::CalculateChannelPermissionsBitmask(Permissions), 2);
        
        // Test MANAGE permission (bit 4)
        Permissions = FPubnubChannelPermissions(); // Reset
        Permissions.Manage = true;
        TestEqual("Manage permission should give bitmask 4", UPubnubTokenUtilities::CalculateChannelPermissionsBitmask(Permissions), 4);
        
        // Test DELETE permission (bit 8)
        Permissions = FPubnubChannelPermissions(); // Reset
        Permissions.Delete = true;
        TestEqual("Delete permission should give bitmask 8", UPubnubTokenUtilities::CalculateChannelPermissionsBitmask(Permissions), 8);
        
        // Test GET permission (bit 32)
        Permissions = FPubnubChannelPermissions(); // Reset
        Permissions.Get = true;
        TestEqual("Get permission should give bitmask 32", UPubnubTokenUtilities::CalculateChannelPermissionsBitmask(Permissions), 32);
        
        // Test UPDATE permission (bit 64)
        Permissions = FPubnubChannelPermissions(); // Reset
        Permissions.Update = true;
        TestEqual("Update permission should give bitmask 64", UPubnubTokenUtilities::CalculateChannelPermissionsBitmask(Permissions), 64);
        
        // Test JOIN permission (bit 128)
        Permissions = FPubnubChannelPermissions(); // Reset
        Permissions.Join = true;
        TestEqual("Join permission should give bitmask 128", UPubnubTokenUtilities::CalculateChannelPermissionsBitmask(Permissions), 128);
    }
    
    // Test case 3: Multiple permissions combinations
    {
        FPubnubChannelPermissions Permissions;
        
        // Read + Write (1 + 2 = 3)
        Permissions = FPubnubChannelPermissions(); // Reset
        Permissions.Read = true;
        Permissions.Write = true;
        TestEqual("Read + Write should give bitmask 3", UPubnubTokenUtilities::CalculateChannelPermissionsBitmask(Permissions), 3);
        
        // Read + Write + Manage (1 + 2 + 4 = 7)
        Permissions.Manage = true;
        TestEqual("Read + Write + Manage should give bitmask 7", UPubnubTokenUtilities::CalculateChannelPermissionsBitmask(Permissions), 7);
        
        // Get + Update (32 + 64 = 96)
        Permissions = FPubnubChannelPermissions(); // Reset
        Permissions.Get = true;
        Permissions.Update = true;
        TestEqual("Get + Update should give bitmask 96", UPubnubTokenUtilities::CalculateChannelPermissionsBitmask(Permissions), 96);
        
        // Delete + Get + Update (8 + 32 + 64 = 104)
        Permissions.Delete = true;
        TestEqual("Delete + Get + Update should give bitmask 104", UPubnubTokenUtilities::CalculateChannelPermissionsBitmask(Permissions), 104);
    }
    
    // Test case 4: All permissions (1+2+4+8+32+64+128 = 239)
    {
        FPubnubChannelPermissions Permissions;
        Permissions.Read = true;
        Permissions.Write = true;
        Permissions.Manage = true;
        Permissions.Delete = true;
        Permissions.Get = true;
        Permissions.Update = true;
        Permissions.Join = true;
        TestEqual("All permissions should give bitmask 239", UPubnubTokenUtilities::CalculateChannelPermissionsBitmask(Permissions), 239);
    }
    
    return true;
}

bool FCalculateChannelGroupPermissionsBitmaskUnitTest::RunTest(const FString& Parameters)
{
    // Test channel group permissions
    // Bit values: READ=1, MANAGE=4
    
    // Test case 1: No permissions (all false)
    {
        FPubnubChannelGroupPermissions Permissions;
        // All permissions are false by default
        int Result = UPubnubTokenUtilities::CalculateChannelGroupPermissionsBitmask(Permissions);
        TestEqual("No permissions should give bitmask 0", Result, 0);
    }
    
    // Test case 2: Individual permissions
    {
        FPubnubChannelGroupPermissions Permissions;
        
        // Test READ permission (bit 1)
        Permissions = FPubnubChannelGroupPermissions(); // Reset
        Permissions.Read = true;
        TestEqual("Read permission should give bitmask 1", UPubnubTokenUtilities::CalculateChannelGroupPermissionsBitmask(Permissions), 1);
        
        // Test MANAGE permission (bit 4)
        Permissions = FPubnubChannelGroupPermissions(); // Reset
        Permissions.Manage = true;
        TestEqual("Manage permission should give bitmask 4", UPubnubTokenUtilities::CalculateChannelGroupPermissionsBitmask(Permissions), 4);
    }
    
    // Test case 3: Multiple permissions
    {
        FPubnubChannelGroupPermissions Permissions;
        
        // Read + Manage (1 + 4 = 5)
        Permissions.Read = true;
        Permissions.Manage = true;
        TestEqual("Read + Manage should give bitmask 5", UPubnubTokenUtilities::CalculateChannelGroupPermissionsBitmask(Permissions), 5);
    }
    
    // Test case 4: All permissions (1+4 = 5)
    {
        FPubnubChannelGroupPermissions Permissions;
        Permissions.Read = true;
        Permissions.Manage = true;
        TestEqual("All channel group permissions should give bitmask 5", UPubnubTokenUtilities::CalculateChannelGroupPermissionsBitmask(Permissions), 5);
    }
    
    return true;
}

bool FCalculateUserPermissionsBitmaskUnitTest::RunTest(const FString& Parameters)
{
    // Test user permissions
    // Bit values: DELETE=8, GET=32, UPDATE=64
    
    // Test case 1: No permissions (all false)
    {
        FPubnubUserPermissions Permissions;
        // All permissions are false by default
        int Result = UPubnubTokenUtilities::CalculateUserPermissionsBitmask(Permissions);
        TestEqual("No permissions should give bitmask 0", Result, 0);
    }
    
    // Test case 2: Individual permissions
    {
        FPubnubUserPermissions Permissions;
        
        // Test DELETE permission (bit 8)
        Permissions = FPubnubUserPermissions(); // Reset
        Permissions.Delete = true;
        TestEqual("Delete permission should give bitmask 8", UPubnubTokenUtilities::CalculateUserPermissionsBitmask(Permissions), 8);
        
        // Test GET permission (bit 32)
        Permissions = FPubnubUserPermissions(); // Reset
        Permissions.Get = true;
        TestEqual("Get permission should give bitmask 32", UPubnubTokenUtilities::CalculateUserPermissionsBitmask(Permissions), 32);
        
        // Test UPDATE permission (bit 64)
        Permissions = FPubnubUserPermissions(); // Reset
        Permissions.Update = true;
        TestEqual("Update permission should give bitmask 64", UPubnubTokenUtilities::CalculateUserPermissionsBitmask(Permissions), 64);
    }
    
    // Test case 3: Multiple permissions combinations
    {
        FPubnubUserPermissions Permissions;
        
        // Get + Update (32 + 64 = 96)
        Permissions = FPubnubUserPermissions(); // Reset
        Permissions.Get = true;
        Permissions.Update = true;
        TestEqual("Get + Update should give bitmask 96", UPubnubTokenUtilities::CalculateUserPermissionsBitmask(Permissions), 96);
        
        // Delete + Get (8 + 32 = 40)
        Permissions = FPubnubUserPermissions(); // Reset
        Permissions.Delete = true;
        Permissions.Get = true;
        TestEqual("Delete + Get should give bitmask 40", UPubnubTokenUtilities::CalculateUserPermissionsBitmask(Permissions), 40);
        
        // Delete + Update (8 + 64 = 72)
        Permissions = FPubnubUserPermissions(); // Reset
        Permissions.Delete = true;
        Permissions.Update = true;
        TestEqual("Delete + Update should give bitmask 72", UPubnubTokenUtilities::CalculateUserPermissionsBitmask(Permissions), 72);
        
        // Delete + Get + Update (8 + 32 + 64 = 104)
        Permissions.Get = true;
        TestEqual("Delete + Get + Update should give bitmask 104", UPubnubTokenUtilities::CalculateUserPermissionsBitmask(Permissions), 104);
    }
    
    // Test case 4: All permissions (8+32+64 = 104)
    {
        FPubnubUserPermissions Permissions;
        Permissions.Delete = true;
        Permissions.Get = true;
        Permissions.Update = true;
        TestEqual("All user permissions should give bitmask 104", UPubnubTokenUtilities::CalculateUserPermissionsBitmask(Permissions), 104);
    }
    
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS