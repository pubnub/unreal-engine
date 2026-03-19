// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "Dom/JsonObject.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUsersFromChannelTest, FPubnubAutomationTestBase, "Pubnub.Integration.Presence.ListUsersFromChannel", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUserSubscribedChannelsTest, FPubnubAutomationTestBase, "Pubnub.Integration.Presence.ListUserSubscribedChannels", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubChannelSetGetStateTest, FPubnubAutomationTestBase, "Pubnub.Integration.Presence.SetGetState", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubChannelSetGetStateForMultipleTest, FPubnubAutomationTestBase, "Pubnub.Integration.Presence.SetGetStateMultipleChannels", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUsersFromChannelWithLimitTest, FPubnubAutomationTestBase, "Pubnub.Integration.Presence.ListUsersFromChannelWithLimit", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUsersFromChannelWithOffsetTest, FPubnubAutomationTestBase, "Pubnub.Integration.Presence.ListUsersFromChannelWithOffset", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUsersFromChannelWithLimitAndOffsetTest, FPubnubAutomationTestBase, "Pubnub.Integration.Presence.ListUsersFromChannelWithLimitAndOffset", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FPubnubListUsersFromChannelTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestUserID = SDK_PREFIX + "test_user_list_users";
    const FString TestChannelName = SDK_PREFIX + "test_channel_list_users";

    TSharedPtr<bool> bListUsersOperationDone = MakeShared<bool>(false);
    TSharedPtr<bool> bListUsersOperationSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FString>> CurrentListedUserIDs = MakeShared<TArray<FString>>();
    TSharedPtr<int> CurrentOccupancy = MakeShared<int>(0);

    TSharedPtr<bool> bSubscribeToChannelDone = MakeShared<bool>(false);
    TSharedPtr<bool> bUnsubscribeFromChannelDone = MakeShared<bool>(false);

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubListUsersFromChannelTest");
        return false;
    }

    // General error handler
    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("General Pubnub Error in FPubnubListUsersFromChannelTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    // ListUsersFromChannel callback handler
    FOnListUsersFromChannelResponseNative ListUsersCallback;
    ListUsersCallback.BindLambda([this, bListUsersOperationDone, bListUsersOperationSuccess, CurrentListedUserIDs, CurrentOccupancy](FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper ResponseData)
    {
        *bListUsersOperationDone = true;
        CurrentListedUserIDs->Empty();
        *CurrentOccupancy = ResponseData.Occupancy;
        if (Result.Status == 200) 
        {
            *bListUsersOperationSuccess = true;
            ResponseData.UsersState.GetKeys(*CurrentListedUserIDs);
        }
        else
        {
            *bListUsersOperationSuccess = false;
            AddError(FString::Printf(TEXT("ListUsersFromChannel failed. Status: %d"), Result.Status));
        }
    });

    //Create subscribe result callback
    FOnSubscribeOperationResponseNative SubscribeToChannelCallback;
    SubscribeToChannelCallback.BindLambda([this, bSubscribeToChannelDone](const FPubnubOperationResult& Result)
    {
        *bSubscribeToChannelDone = true;
        TestFalse("SubscribeToChannel operation should not have failed", Result.Error);
        TestEqual("SubscribeToChannel HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("SubscribeToChannel failed with error: %s"), *Result.ErrorMessage));
        }
    });

    //Create unsubscribe result callback
    FOnSubscribeOperationResponseNative UnsubscribeFromChannelCallback;
    UnsubscribeFromChannelCallback.BindLambda([this, bUnsubscribeFromChannelDone](const FPubnubOperationResult& Result)
    {
        *bUnsubscribeFromChannelDone = true;
        TestFalse("UnsubscribeFromChannel operation should not have failed", Result.Error);
        TestEqual("UnsubscribeFromChannel HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("UnsubscribeFromChannel failed with error: %s"), *Result.ErrorMessage));
        }
    });

    // Set UserID first
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID]()
    {
        PubnubSubsystem->SetUserID(TestUserID);
    }, 0.1f));

    // Step 1: Subscribe to channel
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, SubscribeToChannelCallback, bSubscribeToChannelDone]()
    {
        *bSubscribeToChannelDone = false;
        PubnubSubsystem->SubscribeToChannel(TestChannelName, SubscribeToChannelCallback);
    }, 0.1f));

    //Wait until subscribe to channel result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeToChannelDone]() -> bool {
        return *bSubscribeToChannelDone;
    }, MAX_WAIT_TIME));

    //Check whether subscribe to channel result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSubscribeToChannelDone]()
    {
        if(!*bSubscribeToChannelDone)
        {
            AddError("SubscribeToChannel result callback was not received");
        }
    }, 0.1f));

    // Step 2: List users and verify TestUserID is present
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, ListUsersCallback, bListUsersOperationDone, bListUsersOperationSuccess]()
    {
        *bListUsersOperationDone = false;
        *bListUsersOperationSuccess = false;
        FPubnubListUsersFromChannelSettings Settings;
        Settings.DisableUserID = false;
        PubnubSubsystem->ListUsersFromChannel(TestChannelName, ListUsersCallback, Settings);
    }, 0.1f)); 
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListUsersOperationDone]() { return *bListUsersOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, CurrentListedUserIDs, bListUsersOperationSuccess, CurrentOccupancy]()
    {
        TestTrue("ListUsersFromChannel operation (1) was successful", *bListUsersOperationSuccess);
        if (*bListUsersOperationSuccess)
        {
            TestTrue(FString::Printf(TEXT("TestUserID '%s' should be in the list after subscribe"), *TestUserID), CurrentListedUserIDs->Contains(TestUserID));
            TestEqual("Occupancy should be 1 after subscribe", *CurrentOccupancy, 1);
        }
    }, 0.1f));

    // Step 3: Unsubscribe from channel
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, UnsubscribeFromChannelCallback, bUnsubscribeFromChannelDone]()
    {
        *bUnsubscribeFromChannelDone = false;
        PubnubSubsystem->UnsubscribeFromChannel(TestChannelName, UnsubscribeFromChannelCallback);
    }, 0.1f));

    //Wait until unsubscribe from channel result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bUnsubscribeFromChannelDone]() -> bool {
        return *bUnsubscribeFromChannelDone;
    }, MAX_WAIT_TIME));

    //Check whether unsubscribe from channel result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bUnsubscribeFromChannelDone]()
    {
        if(!*bUnsubscribeFromChannelDone)
        {
            AddError("UnsubscribeFromChannel result callback was not received");
        }
    }, 0.1f));

    // Step 4: List users again and verify TestUserID is NOT present
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, ListUsersCallback, bListUsersOperationDone, bListUsersOperationSuccess]()
    {
        *bListUsersOperationDone = false;
        *bListUsersOperationSuccess = false;
        FPubnubListUsersFromChannelSettings Settings;
        Settings.DisableUserID = false;
        PubnubSubsystem->ListUsersFromChannel(TestChannelName, ListUsersCallback, Settings);
    }, 2.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListUsersOperationDone]() { return *bListUsersOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, CurrentListedUserIDs, bListUsersOperationSuccess, CurrentOccupancy]()
    {
        TestTrue("ListUsersFromChannel operation (2) was successful", *bListUsersOperationSuccess);
        if (*bListUsersOperationSuccess)
        {
            TestFalse(FString::Printf(TEXT("TestUserID '%s' should NOT be in the list after unsubscribe"), *TestUserID), CurrentListedUserIDs->Contains(TestUserID));
            TestEqual("Occupancy should be 0 after unsubscribe", *CurrentOccupancy, 0);
        }
    }, 0.1f));

    CleanUp();
    return true;
}

bool FPubnubListUserSubscribedChannelsTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestUserID = SDK_PREFIX + "test_user_list_subs";
    const FString TestChannelA = SDK_PREFIX + "test_channel_A_list_subs";
    const FString TestChannelB = SDK_PREFIX + "test_channel_B_list_subs";

    TSharedPtr<bool> bListChannelsOpDone = MakeShared<bool>(false);
    TSharedPtr<bool> bListChannelsOpSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FString>> ReceivedSubscribedChannels = MakeShared<TArray<FString>>();

    TSharedPtr<bool> bSubscribeToChannelADone = MakeShared<bool>(false);
    TSharedPtr<bool> bSubscribeToChannelBDone = MakeShared<bool>(false);
    TSharedPtr<bool> bUnsubscribeFromChannelADone = MakeShared<bool>(false);
    TSharedPtr<bool> bUnsubscribeFromChannelBDone = MakeShared<bool>(false);

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubListUserSubscribedChannelsTest");
        return false;
    }

    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("General Pubnub Error in FPubnubListUserSubscribedChannelsTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    FOnListUsersSubscribedChannelsResponseNative ListChannelsCallback;
    ListChannelsCallback.BindLambda([this, bListChannelsOpDone, bListChannelsOpSuccess, ReceivedSubscribedChannels](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
    {
        *bListChannelsOpDone = true;
        *bListChannelsOpSuccess = (Result.Status == 200);
        if (*bListChannelsOpSuccess)
        {
            *ReceivedSubscribedChannels = Channels;
        }
        else
        {
            ReceivedSubscribedChannels->Empty();
            AddError(FString::Printf(TEXT("ListUserSubscribedChannels failed. Status: %d"), Result.Status));
        }
    });

    //Create subscribe result callbacks
    FOnSubscribeOperationResponseNative SubscribeToChannelACallback;
    SubscribeToChannelACallback.BindLambda([this, bSubscribeToChannelADone](const FPubnubOperationResult& Result)
    {
        *bSubscribeToChannelADone = true;
        TestFalse("SubscribeToChannelA operation should not have failed", Result.Error);
        TestEqual("SubscribeToChannelA HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("SubscribeToChannelA failed with error: %s"), *Result.ErrorMessage));
        }
    });

    FOnSubscribeOperationResponseNative SubscribeToChannelBCallback;
    SubscribeToChannelBCallback.BindLambda([this, bSubscribeToChannelBDone](const FPubnubOperationResult& Result)
    {
        *bSubscribeToChannelBDone = true;
        TestFalse("SubscribeToChannelB operation should not have failed", Result.Error);
        TestEqual("SubscribeToChannelB HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("SubscribeToChannelB failed with error: %s"), *Result.ErrorMessage));
        }
    });

    //Create unsubscribe result callbacks
    FOnSubscribeOperationResponseNative UnsubscribeFromChannelACallback;
    UnsubscribeFromChannelACallback.BindLambda([this, bUnsubscribeFromChannelADone](const FPubnubOperationResult& Result)
    {
        *bUnsubscribeFromChannelADone = true;
        TestFalse("UnsubscribeFromChannelA operation should not have failed", Result.Error);
        TestEqual("UnsubscribeFromChannelA HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("UnsubscribeFromChannelA failed with error: %s"), *Result.ErrorMessage));
        }
    });

    FOnSubscribeOperationResponseNative UnsubscribeFromChannelBCallback;
    UnsubscribeFromChannelBCallback.BindLambda([this, bUnsubscribeFromChannelBDone](const FPubnubOperationResult& Result)
    {
        *bUnsubscribeFromChannelBDone = true;
        TestFalse("UnsubscribeFromChannelB operation should not have failed", Result.Error);
        TestEqual("UnsubscribeFromChannelB HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("UnsubscribeFromChannelB failed with error: %s"), *Result.ErrorMessage));
        }
    });

    // Set UserID first
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID]()
    {
        PubnubSubsystem->SetUserID(TestUserID);
    }, 0.1f));

    // Step 1: Subscribe to Channel A
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelA, SubscribeToChannelACallback, bSubscribeToChannelADone]()
    {
        *bSubscribeToChannelADone = false;
        PubnubSubsystem->SubscribeToChannel(TestChannelA, SubscribeToChannelACallback);
    }, 0.1f));

    //Wait until subscribe to channel A result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeToChannelADone]() -> bool {
        return *bSubscribeToChannelADone;
    }, MAX_WAIT_TIME));

    //Check whether subscribe to channel A result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSubscribeToChannelADone]()
    {
        if(!*bSubscribeToChannelADone)
        {
            AddError("SubscribeToChannelA result callback was not received");
        }
    }, 0.1f));

    // Step 2: List and Verify Channel A
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, ListChannelsCallback, bListChannelsOpDone, bListChannelsOpSuccess, ReceivedSubscribedChannels]()
    {
        *bListChannelsOpDone = false; *bListChannelsOpSuccess = false; ReceivedSubscribedChannels->Empty();
        PubnubSubsystem->ListUserSubscribedChannels(TestUserID, ListChannelsCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListChannelsOpDone]() { return *bListChannelsOpDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelA, ReceivedSubscribedChannels, bListChannelsOpSuccess]()
    {
        TestTrue("ListUserSubscribedChannels (1) operation successful", *bListChannelsOpSuccess);
        if(*bListChannelsOpSuccess)
        {
            TestEqual("Should be subscribed to 1 channel", ReceivedSubscribedChannels->Num(), 1);
            TestTrue(FString::Printf(TEXT("Should contain '%s'"), *TestChannelA), ReceivedSubscribedChannels->Contains(TestChannelA));
        }
    }, 0.1f));

    // Step 3: Subscribe to Channel B
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelB, SubscribeToChannelBCallback, bSubscribeToChannelBDone]()
    {
        *bSubscribeToChannelBDone = false;
        PubnubSubsystem->SubscribeToChannel(TestChannelB, SubscribeToChannelBCallback);
    }, 0.1f));

    //Wait until subscribe to channel B result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeToChannelBDone]() -> bool {
        return *bSubscribeToChannelBDone;
    }, MAX_WAIT_TIME));

    //Check whether subscribe to channel B result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSubscribeToChannelBDone]()
    {
        if(!*bSubscribeToChannelBDone)
        {
            AddError("SubscribeToChannelB result callback was not received");
        }
    }, 0.1f));

    // Step 4: List and Verify Channel A & B
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, ListChannelsCallback, bListChannelsOpDone, bListChannelsOpSuccess, ReceivedSubscribedChannels]()
    {
        *bListChannelsOpDone = false; *bListChannelsOpSuccess = false; ReceivedSubscribedChannels->Empty();
        PubnubSubsystem->ListUserSubscribedChannels(TestUserID, ListChannelsCallback);
    }, 2.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListChannelsOpDone]() { return *bListChannelsOpDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelA, TestChannelB, ReceivedSubscribedChannels, bListChannelsOpSuccess]()
    {
        TestTrue("ListUserSubscribedChannels (2) operation successful", *bListChannelsOpSuccess);
        if(*bListChannelsOpSuccess)
        {
            TestEqual("Should be subscribed to 2 channels", ReceivedSubscribedChannels->Num(), 2);
            TestTrue(FString::Printf(TEXT("Should contain '%s'"), *TestChannelA), ReceivedSubscribedChannels->Contains(TestChannelA));
            TestTrue(FString::Printf(TEXT("Should contain '%s'"), *TestChannelB), ReceivedSubscribedChannels->Contains(TestChannelB));
        }
    }, 0.1f));

    // Step 5: Unsubscribe from Channel A
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelA, UnsubscribeFromChannelACallback, bUnsubscribeFromChannelADone]()
    {
        *bUnsubscribeFromChannelADone = false;
        PubnubSubsystem->UnsubscribeFromChannel(TestChannelA, UnsubscribeFromChannelACallback);
    }, 0.1f));

    //Wait until unsubscribe from channel A result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bUnsubscribeFromChannelADone]() -> bool {
        return *bUnsubscribeFromChannelADone;
    }, MAX_WAIT_TIME));

    //Check whether unsubscribe from channel A result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bUnsubscribeFromChannelADone]()
    {
        if(!*bUnsubscribeFromChannelADone)
        {
            AddError("UnsubscribeFromChannelA result callback was not received");
        }
    }, 0.1f));

    // Step 6: List and Verify Only Channel B
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, ListChannelsCallback, bListChannelsOpDone, bListChannelsOpSuccess, ReceivedSubscribedChannels]()
    {
        *bListChannelsOpDone = false; *bListChannelsOpSuccess = false; ReceivedSubscribedChannels->Empty();
        PubnubSubsystem->ListUserSubscribedChannels(TestUserID, ListChannelsCallback);
    }, 2.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListChannelsOpDone]() { return *bListChannelsOpDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelA, TestChannelB, ReceivedSubscribedChannels, bListChannelsOpSuccess]()
    {
        TestTrue("ListUserSubscribedChannels (3) operation successful", *bListChannelsOpSuccess);
        if(*bListChannelsOpSuccess)
        {
            TestEqual("Should be subscribed to 1 channel after unsubscribing from A", ReceivedSubscribedChannels->Num(), 1);
            TestFalse(FString::Printf(TEXT("Should NOT contain '%s'"), *TestChannelA), ReceivedSubscribedChannels->Contains(TestChannelA));
            TestTrue(FString::Printf(TEXT("Should still contain '%s'"), *TestChannelB), ReceivedSubscribedChannels->Contains(TestChannelB));
        }
    }, 0.1f));

    // Step 7: Unsubscribe from Channel B
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelB, UnsubscribeFromChannelBCallback, bUnsubscribeFromChannelBDone]()
    {
        *bUnsubscribeFromChannelBDone = false;
        PubnubSubsystem->UnsubscribeFromChannel(TestChannelB, UnsubscribeFromChannelBCallback);
    }, 0.1f));

    //Wait until unsubscribe from channel B result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bUnsubscribeFromChannelBDone]() -> bool {
        return *bUnsubscribeFromChannelBDone;
    }, MAX_WAIT_TIME));

    //Check whether unsubscribe from channel B result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bUnsubscribeFromChannelBDone]()
    {
        if(!*bUnsubscribeFromChannelBDone)
        {
            AddError("UnsubscribeFromChannelB result callback was not received");
        }
    }, 2.0f));

    // Step 8: List and Verify Empty List
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, ListChannelsCallback, bListChannelsOpDone, bListChannelsOpSuccess, ReceivedSubscribedChannels]()
    {
        *bListChannelsOpDone = false; *bListChannelsOpSuccess = false; ReceivedSubscribedChannels->Empty();
        PubnubSubsystem->ListUserSubscribedChannels(TestUserID, ListChannelsCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListChannelsOpDone]() { return *bListChannelsOpDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedSubscribedChannels, bListChannelsOpSuccess]()
    {
        TestTrue("ListUserSubscribedChannels (4) operation successful", *bListChannelsOpSuccess);
        if(*bListChannelsOpSuccess)
        {
            TestEqual("Should be subscribed to 0 channels after unsubscribing from all", ReceivedSubscribedChannels->Num(), 0);
        }
    }, 0.1f));

    CleanUp();
    return true;
}

bool FPubnubChannelSetGetStateTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestUserID = SDK_PREFIX + "test_user_setget_state";
    const FString TestChannelName = SDK_PREFIX + "test_channel_setget_state";
    const FString InitialStateJson = "{\"mood\": \"happy\", \"level\": 10}";
    const FString UpdatedStateJson = "{\"mood\": \"focused\", \"level\": 11, \"item\": \"shield\"}";

    TSharedPtr<bool> bGetStateOperationDone = MakeShared<bool>(false);
    TSharedPtr<FString> ReceivedStateJson = MakeShared<FString>();
    TSharedPtr<bool> bSetInitialStateDone = MakeShared<bool>(false);
    TSharedPtr<bool> bSetInitialStateSuccess = MakeShared<bool>(false);
    TSharedPtr<bool> bSetUpdatedStateDone = MakeShared<bool>(false);
    TSharedPtr<bool> bSetUpdatedStateSuccess = MakeShared<bool>(false);

    TSharedPtr<bool> bSubscribeToChannelDone = MakeShared<bool>(false);
    TSharedPtr<bool> bUnsubscribeFromChannelDone = MakeShared<bool>(false);

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubChannelSetGetStateTest");
        return false;
    }

    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("General Pubnub Error in FPubnubChannelSetGetStateTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    FOnGetStateResponseNative GetStateCallback;
    GetStateCallback.BindLambda([this, bGetStateOperationDone, ReceivedStateJson](const FPubnubOperationResult& Result, FString JsonResponse)
    {
        *bGetStateOperationDone = true;
        *ReceivedStateJson = JsonResponse;
    });

    FOnSetStateResponseNative SetInitialStateCallback;
    SetInitialStateCallback.BindLambda([this, bSetInitialStateDone, bSetInitialStateSuccess](const FPubnubOperationResult& Result)
    {
        *bSetInitialStateDone = true;
        *bSetInitialStateSuccess = !Result.Error && Result.Status == 200;
        if (!*bSetInitialStateSuccess)
        {
            AddError(FString::Printf(TEXT("SetState (initial) failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    FOnSetStateResponseNative SetUpdatedStateCallback;
    SetUpdatedStateCallback.BindLambda([this, bSetUpdatedStateDone, bSetUpdatedStateSuccess](const FPubnubOperationResult& Result)
    {
        *bSetUpdatedStateDone = true;
        *bSetUpdatedStateSuccess = !Result.Error && Result.Status == 200;
        if (!*bSetUpdatedStateSuccess)
        {
            AddError(FString::Printf(TEXT("SetState (updated) failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    //Create subscribe result callback
    FOnSubscribeOperationResponseNative SubscribeToChannelCallback;
    SubscribeToChannelCallback.BindLambda([this, bSubscribeToChannelDone](const FPubnubOperationResult& Result)
    {
        *bSubscribeToChannelDone = true;
        TestFalse("SubscribeToChannel operation should not have failed", Result.Error);
        TestEqual("SubscribeToChannel HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("SubscribeToChannel failed with error: %s"), *Result.ErrorMessage));
        }
    });

    //Create unsubscribe result callback
    FOnSubscribeOperationResponseNative UnsubscribeFromChannelCallback;
    UnsubscribeFromChannelCallback.BindLambda([this, bUnsubscribeFromChannelDone](const FPubnubOperationResult& Result)
    {
        *bUnsubscribeFromChannelDone = true;
        TestFalse("UnsubscribeFromChannel operation should not have failed", Result.Error);
        TestEqual("UnsubscribeFromChannel HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("UnsubscribeFromChannel failed with error: %s"), *Result.ErrorMessage));
        }
    });

    // Set UserID first
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID]()
    {
        PubnubSubsystem->SetUserID(TestUserID);
    }, 0.1f));

    // Step 1: Subscribe to Channel
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, SubscribeToChannelCallback, bSubscribeToChannelDone]()
    {
        *bSubscribeToChannelDone = false;
        PubnubSubsystem->SubscribeToChannel(TestChannelName, SubscribeToChannelCallback);
    }, 0.1f));

    //Wait until subscribe to channel result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeToChannelDone]() -> bool {
        return *bSubscribeToChannelDone;
    }, MAX_WAIT_TIME));

    //Check whether subscribe to channel result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSubscribeToChannelDone]()
    {
        if(!*bSubscribeToChannelDone)
        {
            AddError("SubscribeToChannel result callback was not received");
        }
    }, 0.1f));

    // Step 2: Set Initial State
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, InitialStateJson, SetInitialStateCallback, bSetInitialStateDone, bSetInitialStateSuccess]()
    {
        *bSetInitialStateDone = false;
        *bSetInitialStateSuccess = false;
        PubnubSubsystem->SetState(TestChannelName, InitialStateJson, SetInitialStateCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSetInitialStateDone]() { return *bSetInitialStateDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSetInitialStateSuccess]()
    {
        TestTrue("SetState (initial) operation was successful", *bSetInitialStateSuccess);
    }, 0.1f));

    // Step 3: Get Initial State and Verify
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, TestUserID, GetStateCallback, bGetStateOperationDone, ReceivedStateJson]()
    {
        *bGetStateOperationDone = false;
        ReceivedStateJson->Empty();
        PubnubSubsystem->GetState(TestChannelName, "", TestUserID, GetStateCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetStateOperationDone]() { return *bGetStateOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, InitialStateJson, ReceivedStateJson]()
    {
        TestTrue("GetState (1) operation should have completed (received a response).", !ReceivedStateJson->IsEmpty());
        if (!ReceivedStateJson->IsEmpty())
        {
            TSharedPtr<FJsonObject> JsonObject;
            if(UPubnubJsonUtilities::StringToJsonObject(*ReceivedStateJson, JsonObject))
            {
                const TSharedPtr<FJsonObject>* PayloadObjectPtr = nullptr;
                if (JsonObject->TryGetObjectField(TEXT("payload"), PayloadObjectPtr) && PayloadObjectPtr && (*PayloadObjectPtr).IsValid())
                {
                    FString ExtractedPayloadJson = UPubnubJsonUtilities::JsonObjectToString(*PayloadObjectPtr);
                    bool StatesMatches = UPubnubJsonUtilities::AreJsonObjectStringsEqual(ExtractedPayloadJson, InitialStateJson);
                    TestTrue("Retrieved initial state payload should match set state", StatesMatches);
                    if(!StatesMatches)
                    {
                        AddError(FString::Printf(TEXT("States should match but exactracted state is: %s, original state is: %s"), *ExtractedPayloadJson, *InitialStateJson));
                    }
                }
                else
                {
                    AddError("GetState (1) response JSON does not contain a valid 'payload' object.");
                }
            }
            else
            {
                AddError(FString::Printf(TEXT("GetState (1) response JSON could not be deserialized: %s"), **ReceivedStateJson));
            }
        }
    }, 0.1f));

    // Step 4: Set Updated State
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, UpdatedStateJson, SetUpdatedStateCallback, bSetUpdatedStateDone, bSetUpdatedStateSuccess]()
    {
        *bSetUpdatedStateDone = false;
        *bSetUpdatedStateSuccess = false;
        PubnubSubsystem->SetState(TestChannelName, UpdatedStateJson, SetUpdatedStateCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSetUpdatedStateDone]() { return *bSetUpdatedStateDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSetUpdatedStateSuccess]()
    {
        TestTrue("SetState (updated) operation was successful", *bSetUpdatedStateSuccess);
    }, 0.1f));

    // Step 5: Get Updated State and Verify
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, TestUserID, GetStateCallback, bGetStateOperationDone, ReceivedStateJson]()
    {
        *bGetStateOperationDone = false;
        ReceivedStateJson->Empty();
        PubnubSubsystem->GetState(TestChannelName, TEXT(""), TestUserID, GetStateCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetStateOperationDone]() { return *bGetStateOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, UpdatedStateJson, ReceivedStateJson]()
    {
        TestTrue("GetState (2) operation should have completed (received a response).", !ReceivedStateJson->IsEmpty());
        if (!ReceivedStateJson->IsEmpty())
        {
            TSharedPtr<FJsonObject> JsonObject;
            if(UPubnubJsonUtilities::StringToJsonObject(*ReceivedStateJson, JsonObject))
            {
                const TSharedPtr<FJsonObject>* PayloadObjectPtr = nullptr;
                if (JsonObject->TryGetObjectField(TEXT("payload"), PayloadObjectPtr) && PayloadObjectPtr && (*PayloadObjectPtr).IsValid())
                {
                    FString ExtractedPayloadJson = UPubnubJsonUtilities::JsonObjectToString(*PayloadObjectPtr);
                    bool StatesMatches = UPubnubJsonUtilities::AreJsonObjectStringsEqual(ExtractedPayloadJson, UpdatedStateJson);
                    TestTrue("Retrieved initial state payload should match set state", StatesMatches);
                    if(!StatesMatches)
                    {
                        AddError(FString::Printf(TEXT("States should match but exactracted state is: %s, original state is: %s"), *ExtractedPayloadJson, *UpdatedStateJson));
                    }
                }
                else
                {
                    AddError("GetState (2) response JSON does not contain a valid 'payload' object.");
                }
            }
            else
            {
                AddError(FString::Printf(TEXT("GetState (2) response JSON could not be deserialized: %s"), **ReceivedStateJson));
            }
        }
    }, 0.1f));
    
    // Step 6: Unsubscribe (optional, but good for hygiene before CleanUp)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, UnsubscribeFromChannelCallback, bUnsubscribeFromChannelDone]()
    {
        *bUnsubscribeFromChannelDone = false;
        PubnubSubsystem->UnsubscribeFromChannel(TestChannelName, UnsubscribeFromChannelCallback);
    }, 0.1f));

    //Wait until unsubscribe from channel result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bUnsubscribeFromChannelDone]() -> bool {
        return *bUnsubscribeFromChannelDone;
    }, MAX_WAIT_TIME));

    //Check whether unsubscribe from channel result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bUnsubscribeFromChannelDone]()
    {
        if(!*bUnsubscribeFromChannelDone)
        {
            AddError("UnsubscribeFromChannel result callback was not received");
        }
    }, 0.1f));

    CleanUp();
    return true;
}

bool FPubnubChannelSetGetStateForMultipleTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestUserID = SDK_PREFIX + "test_user_multi_setget_state";
    const FString TestChannel1Name = SDK_PREFIX + "test_channel1_multi_setget_state";
    const FString TestChannel2Name = SDK_PREFIX + "test_channel2_multi_setget_state";
    const FString State1Json = "{\"item\": \"potion\", \"quantity\": 5}";
    const FString State2Json = "{\"status\": \"exploring\", \"location\": \"dungeon_level_3\"}";
    const FString CombinedChannelsString = FString::Printf(TEXT("%s,%s"), *TestChannel1Name, *TestChannel2Name);

    TSharedPtr<bool> bGetStateOperationDone = MakeShared<bool>(false);
    TSharedPtr<FString> ReceivedCombinedStateJson = MakeShared<FString>();
    TSharedPtr<bool> bSetState1Done = MakeShared<bool>(false);
    TSharedPtr<bool> bSetState1Success = MakeShared<bool>(false);
    TSharedPtr<bool> bSetState2Done = MakeShared<bool>(false);
    TSharedPtr<bool> bSetState2Success = MakeShared<bool>(false);

    TSharedPtr<bool> bSubscribeToChannel1Done = MakeShared<bool>(false);
    TSharedPtr<bool> bSubscribeToChannel2Done = MakeShared<bool>(false);
    TSharedPtr<bool> bUnsubscribeFromChannel1Done = MakeShared<bool>(false);
    TSharedPtr<bool> bUnsubscribeFromChannel2Done = MakeShared<bool>(false);

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubChannelSetGetStateForMultipleTest");
        return false;
    }

    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("General Pubnub Error in FPubnubChannelSetGetStateForMultipleTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    FOnGetStateResponseNative GetStateCallback;
    GetStateCallback.BindLambda([this, bGetStateOperationDone, ReceivedCombinedStateJson](const FPubnubOperationResult& Result, FString JsonResponse)
    {
        *bGetStateOperationDone = true;
        *ReceivedCombinedStateJson = JsonResponse;
    });

    FOnSetStateResponseNative SetState1Callback;
    SetState1Callback.BindLambda([this, bSetState1Done, bSetState1Success](const FPubnubOperationResult& Result)
    {
        *bSetState1Done = true;
        *bSetState1Success = !Result.Error && Result.Status == 200;
        if (!*bSetState1Success)
        {
            AddError(FString::Printf(TEXT("SetState (Channel1) failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    FOnSetStateResponseNative SetState2Callback;
    SetState2Callback.BindLambda([this, bSetState2Done, bSetState2Success](const FPubnubOperationResult& Result)
    {
        *bSetState2Done = true;
        *bSetState2Success = !Result.Error && Result.Status == 200;
        if (!*bSetState2Success)
        {
            AddError(FString::Printf(TEXT("SetState (Channel2) failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    //Create subscribe result callbacks
    FOnSubscribeOperationResponseNative SubscribeToChannel1Callback;
    SubscribeToChannel1Callback.BindLambda([this, bSubscribeToChannel1Done](const FPubnubOperationResult& Result)
    {
        *bSubscribeToChannel1Done = true;
        TestFalse("SubscribeToChannel1 operation should not have failed", Result.Error);
        TestEqual("SubscribeToChannel1 HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("SubscribeToChannel1 failed with error: %s"), *Result.ErrorMessage));
        }
    });

    FOnSubscribeOperationResponseNative SubscribeToChannel2Callback;
    SubscribeToChannel2Callback.BindLambda([this, bSubscribeToChannel2Done](const FPubnubOperationResult& Result)
    {
        *bSubscribeToChannel2Done = true;
        TestFalse("SubscribeToChannel2 operation should not have failed", Result.Error);
        TestEqual("SubscribeToChannel2 HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("SubscribeToChannel2 failed with error: %s"), *Result.ErrorMessage));
        }
    });

    //Create unsubscribe result callbacks
    FOnSubscribeOperationResponseNative UnsubscribeFromChannel1Callback;
    UnsubscribeFromChannel1Callback.BindLambda([this, bUnsubscribeFromChannel1Done](const FPubnubOperationResult& Result)
    {
        *bUnsubscribeFromChannel1Done = true;
        TestFalse("UnsubscribeFromChannel1 operation should not have failed", Result.Error);
        TestEqual("UnsubscribeFromChannel1 HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("UnsubscribeFromChannel1 failed with error: %s"), *Result.ErrorMessage));
        }
    });

    FOnSubscribeOperationResponseNative UnsubscribeFromChannel2Callback;
    UnsubscribeFromChannel2Callback.BindLambda([this, bUnsubscribeFromChannel2Done](const FPubnubOperationResult& Result)
    {
        *bUnsubscribeFromChannel2Done = true;
        TestFalse("UnsubscribeFromChannel2 operation should not have failed", Result.Error);
        TestEqual("UnsubscribeFromChannel2 HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("UnsubscribeFromChannel2 failed with error: %s"), *Result.ErrorMessage));
        }
    });

    // Set UserID first
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID]()
    {
        PubnubSubsystem->SetUserID(TestUserID);
    }, 0.1f));

    // Subscribe to Channel 1
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1Name, SubscribeToChannel1Callback, bSubscribeToChannel1Done]()
    {
        *bSubscribeToChannel1Done = false;
        PubnubSubsystem->SubscribeToChannel(TestChannel1Name, SubscribeToChannel1Callback);
    }, 0.1f));

    //Wait until subscribe to channel 1 result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeToChannel1Done]() -> bool {
        return *bSubscribeToChannel1Done;
    }, MAX_WAIT_TIME));

    //Check whether subscribe to channel 1 result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSubscribeToChannel1Done]()
    {
        if(!*bSubscribeToChannel1Done)
        {
            AddError("SubscribeToChannel1 result callback was not received");
        }
    }, 0.1f));

    // Subscribe to Channel 2
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel2Name, SubscribeToChannel2Callback, bSubscribeToChannel2Done]()
    {
        *bSubscribeToChannel2Done = false;
        PubnubSubsystem->SubscribeToChannel(TestChannel2Name, SubscribeToChannel2Callback);
    }, 0.1f));

    //Wait until subscribe to channel 2 result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeToChannel2Done]() -> bool {
        return *bSubscribeToChannel2Done;
    }, MAX_WAIT_TIME));

    //Check whether subscribe to channel 2 result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSubscribeToChannel2Done]()
    {
        if(!*bSubscribeToChannel2Done)
        {
            AddError("SubscribeToChannel2 result callback was not received");
        }
    }, 0.1f));

    // Set State for Channel 1
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1Name, State1Json, SetState1Callback, bSetState1Done, bSetState1Success]()
    {
        *bSetState1Done = false;
        *bSetState1Success = false;
        PubnubSubsystem->SetState(TestChannel1Name, State1Json, SetState1Callback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSetState1Done]() { return *bSetState1Done; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSetState1Success]()
    {
        TestTrue("SetState (Channel1) operation was successful", *bSetState1Success);
    }, 0.1f));

    // Set State for Channel 2
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel2Name, State2Json, SetState2Callback, bSetState2Done, bSetState2Success]()
    {
        *bSetState2Done = false;
        *bSetState2Success = false;
        PubnubSubsystem->SetState(TestChannel2Name, State2Json, SetState2Callback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSetState2Done]() { return *bSetState2Done; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSetState2Success]()
    {
        TestTrue("SetState (Channel2) operation was successful", *bSetState2Success);
    }, 0.1f));

    // Get Combined State for Channel 1 and Channel 2
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, CombinedChannelsString, TestUserID, GetStateCallback, bGetStateOperationDone, ReceivedCombinedStateJson]()
    {
        *bGetStateOperationDone = false;
        ReceivedCombinedStateJson->Empty();
        PubnubSubsystem->GetState(CombinedChannelsString, TEXT(""), TestUserID, GetStateCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetStateOperationDone]() { return *bGetStateOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1Name, State1Json, TestChannel2Name, State2Json, ReceivedCombinedStateJson]()
    {
        TestTrue("GetState (combined) operation should have completed (received a response).", !ReceivedCombinedStateJson->IsEmpty());
        if (ReceivedCombinedStateJson->IsEmpty()) { return; }

        TSharedPtr<FJsonObject> FullResponseObject;
        if (!UPubnubJsonUtilities::StringToJsonObject(*ReceivedCombinedStateJson, FullResponseObject) || !FullResponseObject.IsValid())
        {
            AddError(FString::Printf(TEXT("GetState (combined) response JSON could not be deserialized: %s"), **ReceivedCombinedStateJson));
            return;
        }

        const TSharedPtr<FJsonObject>* PayloadObjectPtr = nullptr;
        if (!FullResponseObject->TryGetObjectField(TEXT("payload"), PayloadObjectPtr) || !PayloadObjectPtr || !(*PayloadObjectPtr).IsValid())
        {
            AddError("GetState (combined) response JSON does not contain a valid 'payload' object.");
            return;
        }

        const TSharedPtr<FJsonObject>* ChannelsObjectPtr = nullptr;
        if (!(*PayloadObjectPtr)->TryGetObjectField(TEXT("channels"), ChannelsObjectPtr) || !ChannelsObjectPtr || !(*ChannelsObjectPtr).IsValid())
        {
            AddError("GetState (combined) payload does not contain a valid 'channels' object.");
            return;
        }
        const TSharedPtr<FJsonObject>& ChannelsObject = *ChannelsObjectPtr;

        // Verify State for Channel 1
        const TSharedPtr<FJsonObject>* Channel1StateObjectPtr = nullptr;
        if (ChannelsObject->TryGetObjectField(TestChannel1Name, Channel1StateObjectPtr) && Channel1StateObjectPtr && (*Channel1StateObjectPtr).IsValid())
        {
            FString ExtractedChannel1StateString = UPubnubJsonUtilities::JsonObjectToString(*Channel1StateObjectPtr);
            bool bState1Matches = UPubnubJsonUtilities::AreJsonObjectStringsEqual(ExtractedChannel1StateString, State1Json);
            TestTrue(FString::Printf(TEXT("State for '%s' should match"), *TestChannel1Name), bState1Matches);
            if (!bState1Matches)
            {
                AddError(FString::Printf(TEXT("State for '%s' mismatch. Expected: %s, Got: %s"), *TestChannel1Name, *State1Json, *ExtractedChannel1StateString));
            }
        }
        else
        {
            AddError(FString::Printf(TEXT("State for channel '%s' not found in combined response."), *TestChannel1Name));
        }

        // Verify State for Channel 2
        const TSharedPtr<FJsonObject>* Channel2StateObjectPtr = nullptr;
        if (ChannelsObject->TryGetObjectField(TestChannel2Name, Channel2StateObjectPtr) && Channel2StateObjectPtr && (*Channel2StateObjectPtr).IsValid())
        {
            FString ExtractedChannel2StateString = UPubnubJsonUtilities::JsonObjectToString(*Channel2StateObjectPtr);
            bool bState2Matches = UPubnubJsonUtilities::AreJsonObjectStringsEqual(ExtractedChannel2StateString, State2Json);
            TestTrue(FString::Printf(TEXT("State for '%s' should match"), *TestChannel2Name), bState2Matches);
            if (!bState2Matches)
            {
                AddError(FString::Printf(TEXT("State for '%s' mismatch. Expected: %s, Got: %s"), *TestChannel2Name, *State2Json, *ExtractedChannel2StateString));
            }
        }
        else
        {
            AddError(FString::Printf(TEXT("State for channel '%s' not found in combined response."), *TestChannel2Name));
        }
    }, 0.1f));
    
    // Unsubscribe from channels
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1Name, UnsubscribeFromChannel1Callback, bUnsubscribeFromChannel1Done]()
    {
        *bUnsubscribeFromChannel1Done = false;
        PubnubSubsystem->UnsubscribeFromChannel(TestChannel1Name, UnsubscribeFromChannel1Callback);
    }, 0.1f));

    //Wait until unsubscribe from channel 1 result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bUnsubscribeFromChannel1Done]() -> bool {
        return *bUnsubscribeFromChannel1Done;
    }, MAX_WAIT_TIME));

    //Check whether unsubscribe from channel 1 result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bUnsubscribeFromChannel1Done]()
    {
        if(!*bUnsubscribeFromChannel1Done)
        {
            AddError("UnsubscribeFromChannel1 result callback was not received");
        }
    }, 0.1f));

    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel2Name, UnsubscribeFromChannel2Callback, bUnsubscribeFromChannel2Done]()
    {
        *bUnsubscribeFromChannel2Done = false;
        PubnubSubsystem->UnsubscribeFromChannel(TestChannel2Name, UnsubscribeFromChannel2Callback);
    }, 0.1f));

    //Wait until unsubscribe from channel 2 result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bUnsubscribeFromChannel2Done]() -> bool {
        return *bUnsubscribeFromChannel2Done;
    }, MAX_WAIT_TIME));

    //Check whether unsubscribe from channel 2 result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bUnsubscribeFromChannel2Done]()
    {
        if(!*bUnsubscribeFromChannel2Done)
        {
            AddError("UnsubscribeFromChannel2 result callback was not received");
        }
    }, 0.1f));

    CleanUp();
    return true;
}

bool FPubnubListUsersFromChannelWithLimitTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestUserID = SDK_PREFIX + "test_user_limit";
    const FString TestChannelName = SDK_PREFIX + "test_channel_limit";

    TSharedPtr<bool> bListUsersOperationDone = MakeShared<bool>(false);
    TSharedPtr<bool> bListUsersOperationSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FString>> CurrentListedUserIDs = MakeShared<TArray<FString>>();
    TSharedPtr<int> CurrentOccupancy = MakeShared<int>(0);

    TSharedPtr<bool> bSubscribeToChannelDone = MakeShared<bool>(false);
    TSharedPtr<bool> bUnsubscribeFromChannelDone = MakeShared<bool>(false);

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubListUsersFromChannelWithLimitTest");
        return false;
    }

    // General error handler
    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("General Pubnub Error in FPubnubListUsersFromChannelWithLimitTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    // ListUsersFromChannel callback handler
    FOnListUsersFromChannelResponseNative ListUsersCallback;
    ListUsersCallback.BindLambda([this, bListUsersOperationDone, bListUsersOperationSuccess, CurrentListedUserIDs, CurrentOccupancy](FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper ResponseData)
    {
        *bListUsersOperationDone = true;
        CurrentListedUserIDs->Empty();
        *CurrentOccupancy = ResponseData.Occupancy;
        if (Result.Status == 200) 
        {
            *bListUsersOperationSuccess = true;
            ResponseData.UsersState.GetKeys(*CurrentListedUserIDs);
        }
        else
        {
            *bListUsersOperationSuccess = false;
            AddError(FString::Printf(TEXT("ListUsersFromChannel failed. Status: %d"), Result.Status));
        }
    });

    // Create subscribe result callback
    FOnSubscribeOperationResponseNative SubscribeToChannelCallback;
    SubscribeToChannelCallback.BindLambda([this, bSubscribeToChannelDone](const FPubnubOperationResult& Result)
    {
        *bSubscribeToChannelDone = true;
        TestFalse("SubscribeToChannel operation should not have failed", Result.Error);
        TestEqual("SubscribeToChannel HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("SubscribeToChannel failed with error: %s"), *Result.ErrorMessage));
        }
    });

    // Create unsubscribe result callback
    FOnSubscribeOperationResponseNative UnsubscribeFromChannelCallback;
    UnsubscribeFromChannelCallback.BindLambda([this, bUnsubscribeFromChannelDone](const FPubnubOperationResult& Result)
    {
        *bUnsubscribeFromChannelDone = true;
        TestFalse("UnsubscribeFromChannel operation should not have failed", Result.Error);
        TestEqual("UnsubscribeFromChannel HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("UnsubscribeFromChannel failed with error: %s"), *Result.ErrorMessage));
        }
    });

    // Set UserID first
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID]()
    {
        PubnubSubsystem->SetUserID(TestUserID);
    }, 0.1f));

    // Step 1: Subscribe to channel
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, SubscribeToChannelCallback, bSubscribeToChannelDone]()
    {
        *bSubscribeToChannelDone = false;
        PubnubSubsystem->SubscribeToChannel(TestChannelName, SubscribeToChannelCallback);
    }, 0.1f));

    // Wait until subscribe to channel result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeToChannelDone]() -> bool {
        return *bSubscribeToChannelDone;
    }, MAX_WAIT_TIME));

    // Check whether subscribe to channel result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSubscribeToChannelDone]()
    {
        if(!*bSubscribeToChannelDone)
        {
            AddError("SubscribeToChannel result callback was not received");
        }
    }, 0.1f));

    // Step 2: List users with Limit=1
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, ListUsersCallback, bListUsersOperationDone, bListUsersOperationSuccess]()
    {
        *bListUsersOperationDone = false;
        *bListUsersOperationSuccess = false;
        FPubnubListUsersFromChannelSettings Settings;
        Settings.DisableUserID = false;
        Settings.Limit = 1;
        PubnubSubsystem->ListUsersFromChannel(TestChannelName, ListUsersCallback, Settings);
    }, 0.1f)); 
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListUsersOperationDone]() { return *bListUsersOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, CurrentListedUserIDs, bListUsersOperationSuccess, CurrentOccupancy]()
    {
        TestTrue("ListUsersFromChannel with Limit=1 was successful", *bListUsersOperationSuccess);
        if (*bListUsersOperationSuccess)
        {
            TestEqual("Should return maximum 1 user when Limit=1", CurrentListedUserIDs->Num(), 1);
            TestTrue(FString::Printf(TEXT("TestUserID '%s' should be in the limited list"), *TestUserID), CurrentListedUserIDs->Contains(TestUserID));
            TestEqual("Occupancy should still be 1", *CurrentOccupancy, 1);
        }
    }, 0.1f));

    // Step 3: List users with Limit=0 (should use default limit of 1000)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, ListUsersCallback, bListUsersOperationDone, bListUsersOperationSuccess]()
    {
        *bListUsersOperationDone = false;
        *bListUsersOperationSuccess = false;
        FPubnubListUsersFromChannelSettings Settings;
        Settings.DisableUserID = false;
        Settings.Limit = 0;
        PubnubSubsystem->ListUsersFromChannel(TestChannelName, ListUsersCallback, Settings);
    }, 0.1f)); 
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListUsersOperationDone]() { return *bListUsersOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, CurrentListedUserIDs, bListUsersOperationSuccess, CurrentOccupancy]()
    {
        TestTrue("ListUsersFromChannel with Limit=0 was successful", *bListUsersOperationSuccess);
        if (*bListUsersOperationSuccess)
        {
            TestEqual("Should return 1 user when Limit=0 (uses default limit)", CurrentListedUserIDs->Num(), 1);
            TestTrue(FString::Printf(TEXT("TestUserID '%s' should be in the list"), *TestUserID), CurrentListedUserIDs->Contains(TestUserID));
            TestEqual("Occupancy should be 1", *CurrentOccupancy, 1);
        }
    }, 0.1f));

    // Step 4: List users with default limit (should work normally)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, ListUsersCallback, bListUsersOperationDone, bListUsersOperationSuccess]()
    {
        *bListUsersOperationDone = false;
        *bListUsersOperationSuccess = false;
        FPubnubListUsersFromChannelSettings Settings;
        Settings.DisableUserID = false;
        // Using default Limit (1000)
        PubnubSubsystem->ListUsersFromChannel(TestChannelName, ListUsersCallback, Settings);
    }, 0.1f)); 
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListUsersOperationDone]() { return *bListUsersOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, CurrentListedUserIDs, bListUsersOperationSuccess, CurrentOccupancy]()
    {
        TestTrue("ListUsersFromChannel with default limit was successful", *bListUsersOperationSuccess);
        if (*bListUsersOperationSuccess)
        {
            TestEqual("Should return 1 user with default limit", CurrentListedUserIDs->Num(), 1);
            TestTrue(FString::Printf(TEXT("TestUserID '%s' should be in the list"), *TestUserID), CurrentListedUserIDs->Contains(TestUserID));
        }
    }, 0.1f));

    // Step 5: Unsubscribe from channel
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, UnsubscribeFromChannelCallback, bUnsubscribeFromChannelDone]()
    {
        *bUnsubscribeFromChannelDone = false;
        PubnubSubsystem->UnsubscribeFromChannel(TestChannelName, UnsubscribeFromChannelCallback);
    }, 0.1f));

    // Wait until unsubscribe from channel result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bUnsubscribeFromChannelDone]() -> bool {
        return *bUnsubscribeFromChannelDone;
    }, MAX_WAIT_TIME));

    // Check whether unsubscribe from channel result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bUnsubscribeFromChannelDone]()
    {
        if(!*bUnsubscribeFromChannelDone)
        {
            AddError("UnsubscribeFromChannel result callback was not received");
        }
    }, 0.1f));

    CleanUp();
    return true;
}

bool FPubnubListUsersFromChannelWithOffsetTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestUserID = SDK_PREFIX + "test_user_offset";
    const FString TestChannelName = SDK_PREFIX + "test_channel_offset";

    TSharedPtr<bool> bListUsersOperationDone = MakeShared<bool>(false);
    TSharedPtr<bool> bListUsersOperationSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FString>> CurrentListedUserIDs = MakeShared<TArray<FString>>();
    TSharedPtr<int> CurrentOccupancy = MakeShared<int>(0);

    TSharedPtr<bool> bSubscribeToChannelDone = MakeShared<bool>(false);
    TSharedPtr<bool> bUnsubscribeFromChannelDone = MakeShared<bool>(false);

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubListUsersFromChannelWithOffsetTest");
        return false;
    }

    // General error handler
    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("General Pubnub Error in FPubnubListUsersFromChannelWithOffsetTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    // ListUsersFromChannel callback handler
    FOnListUsersFromChannelResponseNative ListUsersCallback;
    ListUsersCallback.BindLambda([this, bListUsersOperationDone, bListUsersOperationSuccess, CurrentListedUserIDs, CurrentOccupancy](FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper ResponseData)
    {
        *bListUsersOperationDone = true;
        CurrentListedUserIDs->Empty();
        *CurrentOccupancy = ResponseData.Occupancy;
        if (Result.Status == 200) 
        {
            *bListUsersOperationSuccess = true;
            ResponseData.UsersState.GetKeys(*CurrentListedUserIDs);
        }
        else
        {
            *bListUsersOperationSuccess = false;
            AddError(FString::Printf(TEXT("ListUsersFromChannel failed. Status: %d"), Result.Status));
        }
    });

    // Create subscribe result callback
    FOnSubscribeOperationResponseNative SubscribeToChannelCallback;
    SubscribeToChannelCallback.BindLambda([this, bSubscribeToChannelDone](const FPubnubOperationResult& Result)
    {
        *bSubscribeToChannelDone = true;
        TestFalse("SubscribeToChannel operation should not have failed", Result.Error);
        TestEqual("SubscribeToChannel HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("SubscribeToChannel failed with error: %s"), *Result.ErrorMessage));
        }
    });

    // Create unsubscribe result callback
    FOnSubscribeOperationResponseNative UnsubscribeFromChannelCallback;
    UnsubscribeFromChannelCallback.BindLambda([this, bUnsubscribeFromChannelDone](const FPubnubOperationResult& Result)
    {
        *bUnsubscribeFromChannelDone = true;
        TestFalse("UnsubscribeFromChannel operation should not have failed", Result.Error);
        TestEqual("UnsubscribeFromChannel HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("UnsubscribeFromChannel failed with error: %s"), *Result.ErrorMessage));
        }
    });

    // Set UserID first
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID]()
    {
        PubnubSubsystem->SetUserID(TestUserID);
    }, 0.1f));

    // Step 1: Subscribe to channel
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, SubscribeToChannelCallback, bSubscribeToChannelDone]()
    {
        *bSubscribeToChannelDone = false;
        PubnubSubsystem->SubscribeToChannel(TestChannelName, SubscribeToChannelCallback);
    }, 0.1f));

    // Wait until subscribe to channel result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeToChannelDone]() -> bool {
        return *bSubscribeToChannelDone;
    }, MAX_WAIT_TIME));

    // Check whether subscribe to channel result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSubscribeToChannelDone]()
    {
        if(!*bSubscribeToChannelDone)
        {
            AddError("SubscribeToChannel result callback was not received");
        }
    }, 0.1f));

    // Step 2: List users with Offset=0 (should return user)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, ListUsersCallback, bListUsersOperationDone, bListUsersOperationSuccess]()
    {
        *bListUsersOperationDone = false;
        *bListUsersOperationSuccess = false;
        FPubnubListUsersFromChannelSettings Settings;
        Settings.DisableUserID = false;
        Settings.Offset = 0;
        PubnubSubsystem->ListUsersFromChannel(TestChannelName, ListUsersCallback, Settings);
    }, 0.1f)); 
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListUsersOperationDone]() { return *bListUsersOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, CurrentListedUserIDs, bListUsersOperationSuccess, CurrentOccupancy]()
    {
        TestTrue("ListUsersFromChannel with Offset=0 was successful", *bListUsersOperationSuccess);
        if (*bListUsersOperationSuccess)
        {
            TestEqual("Should return 1 user when Offset=0", CurrentListedUserIDs->Num(), 1);
            TestTrue(FString::Printf(TEXT("TestUserID '%s' should be in the list"), *TestUserID), CurrentListedUserIDs->Contains(TestUserID));
            TestEqual("Occupancy should be 1", *CurrentOccupancy, 1);
        }
    }, 0.1f));

    // Step 3: List users with Offset=1 (should skip the only user and return empty list)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, ListUsersCallback, bListUsersOperationDone, bListUsersOperationSuccess]()
    {
        *bListUsersOperationDone = false;
        *bListUsersOperationSuccess = false;
        FPubnubListUsersFromChannelSettings Settings;
        Settings.DisableUserID = false;
        Settings.Offset = 1;
        PubnubSubsystem->ListUsersFromChannel(TestChannelName, ListUsersCallback, Settings);
    }, 0.1f)); 
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListUsersOperationDone]() { return *bListUsersOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, CurrentListedUserIDs, bListUsersOperationSuccess, CurrentOccupancy]()
    {
        TestTrue("ListUsersFromChannel with Offset=1 was successful", *bListUsersOperationSuccess);
        if (*bListUsersOperationSuccess)
        {
            TestEqual("Should return 0 users when Offset=1 (skipping the only user)", CurrentListedUserIDs->Num(), 0);
            TestEqual("Occupancy should still be 1 (indicates total users, not affected by offset)", *CurrentOccupancy, 1);
        }
    }, 0.1f));

    // Step 4: List users with Offset=10 (large offset, should return empty list)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, ListUsersCallback, bListUsersOperationDone, bListUsersOperationSuccess]()
    {
        *bListUsersOperationDone = false;
        *bListUsersOperationSuccess = false;
        FPubnubListUsersFromChannelSettings Settings;
        Settings.DisableUserID = false;
        Settings.Offset = 10;
        PubnubSubsystem->ListUsersFromChannel(TestChannelName, ListUsersCallback, Settings);
    }, 0.1f)); 
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListUsersOperationDone]() { return *bListUsersOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, CurrentListedUserIDs, bListUsersOperationSuccess]()
    {
        TestTrue("ListUsersFromChannel with Offset=10 was successful", *bListUsersOperationSuccess);
        if (*bListUsersOperationSuccess)
        {
            TestEqual("Should return 0 users when Offset exceeds available users", CurrentListedUserIDs->Num(), 0);
        }
    }, 0.1f));

    // Step 5: Unsubscribe from channel
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, UnsubscribeFromChannelCallback, bUnsubscribeFromChannelDone]()
    {
        *bUnsubscribeFromChannelDone = false;
        PubnubSubsystem->UnsubscribeFromChannel(TestChannelName, UnsubscribeFromChannelCallback);
    }, 0.1f));

    // Wait until unsubscribe from channel result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bUnsubscribeFromChannelDone]() -> bool {
        return *bUnsubscribeFromChannelDone;
    }, MAX_WAIT_TIME));

    // Check whether unsubscribe from channel result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bUnsubscribeFromChannelDone]()
    {
        if(!*bUnsubscribeFromChannelDone)
        {
            AddError("UnsubscribeFromChannel result callback was not received");
        }
    }, 0.1f));

    CleanUp();
    return true;
}

bool FPubnubListUsersFromChannelWithLimitAndOffsetTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestUserID = SDK_PREFIX + "test_user_limit_offset";
    const FString TestChannelName = SDK_PREFIX + "test_channel_limit_offset";

    TSharedPtr<bool> bListUsersOperationDone = MakeShared<bool>(false);
    TSharedPtr<bool> bListUsersOperationSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FString>> CurrentListedUserIDs = MakeShared<TArray<FString>>();
    TSharedPtr<int> CurrentOccupancy = MakeShared<int>(0);

    TSharedPtr<bool> bSubscribeToChannelDone = MakeShared<bool>(false);
    TSharedPtr<bool> bUnsubscribeFromChannelDone = MakeShared<bool>(false);

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubListUsersFromChannelWithLimitAndOffsetTest");
        return false;
    }

    // General error handler
    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("General Pubnub Error in FPubnubListUsersFromChannelWithLimitAndOffsetTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    // ListUsersFromChannel callback handler
    FOnListUsersFromChannelResponseNative ListUsersCallback;
    ListUsersCallback.BindLambda([this, bListUsersOperationDone, bListUsersOperationSuccess, CurrentListedUserIDs, CurrentOccupancy](FPubnubOperationResult Result, FPubnubListUsersFromChannelWrapper ResponseData)
    {
        *bListUsersOperationDone = true;
        CurrentListedUserIDs->Empty();
        *CurrentOccupancy = ResponseData.Occupancy;
        if (Result.Status == 200) 
        {
            *bListUsersOperationSuccess = true;
            ResponseData.UsersState.GetKeys(*CurrentListedUserIDs);
        }
        else
        {
            *bListUsersOperationSuccess = false;
            AddError(FString::Printf(TEXT("ListUsersFromChannel failed. Status: %d"), Result.Status));
        }
    });

    // Create subscribe result callback
    FOnSubscribeOperationResponseNative SubscribeToChannelCallback;
    SubscribeToChannelCallback.BindLambda([this, bSubscribeToChannelDone](const FPubnubOperationResult& Result)
    {
        *bSubscribeToChannelDone = true;
        TestFalse("SubscribeToChannel operation should not have failed", Result.Error);
        TestEqual("SubscribeToChannel HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("SubscribeToChannel failed with error: %s"), *Result.ErrorMessage));
        }
    });

    // Create unsubscribe result callback
    FOnSubscribeOperationResponseNative UnsubscribeFromChannelCallback;
    UnsubscribeFromChannelCallback.BindLambda([this, bUnsubscribeFromChannelDone](const FPubnubOperationResult& Result)
    {
        *bUnsubscribeFromChannelDone = true;
        TestFalse("UnsubscribeFromChannel operation should not have failed", Result.Error);
        TestEqual("UnsubscribeFromChannel HTTP status should be 200", Result.Status, 200);
        
        if (Result.Error)
        {
            AddError(FString::Printf(TEXT("UnsubscribeFromChannel failed with error: %s"), *Result.ErrorMessage));
        }
    });

    // Set UserID first
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID]()
    {
        PubnubSubsystem->SetUserID(TestUserID);
    }, 0.1f));

    // Step 1: Subscribe to channel
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, SubscribeToChannelCallback, bSubscribeToChannelDone]()
    {
        *bSubscribeToChannelDone = false;
        PubnubSubsystem->SubscribeToChannel(TestChannelName, SubscribeToChannelCallback);
    }, 0.1f));

    // Wait until subscribe to channel result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeToChannelDone]() -> bool {
        return *bSubscribeToChannelDone;
    }, MAX_WAIT_TIME));

    // Check whether subscribe to channel result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSubscribeToChannelDone]()
    {
        if(!*bSubscribeToChannelDone)
        {
            AddError("SubscribeToChannel result callback was not received");
        }
    }, 0.1f));

    // Step 2: List users with Limit=1 and Offset=0 (pagination: first page with 1 item)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, ListUsersCallback, bListUsersOperationDone, bListUsersOperationSuccess]()
    {
        *bListUsersOperationDone = false;
        *bListUsersOperationSuccess = false;
        FPubnubListUsersFromChannelSettings Settings;
        Settings.DisableUserID = false;
        Settings.Limit = 1;
        Settings.Offset = 0;
        PubnubSubsystem->ListUsersFromChannel(TestChannelName, ListUsersCallback, Settings);
    }, 0.1f)); 
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListUsersOperationDone]() { return *bListUsersOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, CurrentListedUserIDs, bListUsersOperationSuccess, CurrentOccupancy]()
    {
        TestTrue("ListUsersFromChannel with Limit=1, Offset=0 was successful", *bListUsersOperationSuccess);
        if (*bListUsersOperationSuccess)
        {
            TestEqual("Should return 1 user with Limit=1, Offset=0", CurrentListedUserIDs->Num(), 1);
            TestTrue(FString::Printf(TEXT("TestUserID '%s' should be in the first page"), *TestUserID), CurrentListedUserIDs->Contains(TestUserID));
            TestEqual("Occupancy should be 1", *CurrentOccupancy, 1);
        }
    }, 0.1f));

    // Step 3: List users with Limit=1 and Offset=1 (pagination: second page should be empty)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, ListUsersCallback, bListUsersOperationDone, bListUsersOperationSuccess]()
    {
        *bListUsersOperationDone = false;
        *bListUsersOperationSuccess = false;
        FPubnubListUsersFromChannelSettings Settings;
        Settings.DisableUserID = false;
        Settings.Limit = 1;
        Settings.Offset = 1;
        PubnubSubsystem->ListUsersFromChannel(TestChannelName, ListUsersCallback, Settings);
    }, 0.1f)); 
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListUsersOperationDone]() { return *bListUsersOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, CurrentListedUserIDs, bListUsersOperationSuccess, CurrentOccupancy]()
    {
        TestTrue("ListUsersFromChannel with Limit=1, Offset=1 was successful", *bListUsersOperationSuccess);
        if (*bListUsersOperationSuccess)
        {
            TestEqual("Should return 0 users with Limit=1, Offset=1 (second page is empty)", CurrentListedUserIDs->Num(), 0);
            TestEqual("Occupancy should still be 1", *CurrentOccupancy, 1);
        }
    }, 0.1f));

    // Step 4: List users with Limit=0 and Offset=0 (Limit=0 should use default limit of 1000)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, ListUsersCallback, bListUsersOperationDone, bListUsersOperationSuccess]()
    {
        *bListUsersOperationDone = false;
        *bListUsersOperationSuccess = false;
        FPubnubListUsersFromChannelSettings Settings;
        Settings.DisableUserID = false;
        Settings.Limit = 0;
        Settings.Offset = 0;
        PubnubSubsystem->ListUsersFromChannel(TestChannelName, ListUsersCallback, Settings);
    }, 0.1f)); 
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListUsersOperationDone]() { return *bListUsersOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, CurrentListedUserIDs, bListUsersOperationSuccess, CurrentOccupancy]()
    {
        TestTrue("ListUsersFromChannel with Limit=0, Offset=0 was successful", *bListUsersOperationSuccess);
        if (*bListUsersOperationSuccess)
        {
            TestEqual("Should return 1 user when Limit=0 (uses default limit), Offset=0", CurrentListedUserIDs->Num(), 1);
            TestTrue(FString::Printf(TEXT("TestUserID '%s' should be in the list"), *TestUserID), CurrentListedUserIDs->Contains(TestUserID));
            TestEqual("Occupancy should be 1", *CurrentOccupancy, 1);
        }
    }, 0.1f));

    // Step 5: List users with Limit=5 and Offset=2 (offset beyond available users)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, ListUsersCallback, bListUsersOperationDone, bListUsersOperationSuccess]()
    {
        *bListUsersOperationDone = false;
        *bListUsersOperationSuccess = false;
        FPubnubListUsersFromChannelSettings Settings;
        Settings.DisableUserID = false;
        Settings.Limit = 5;
        Settings.Offset = 2;
        PubnubSubsystem->ListUsersFromChannel(TestChannelName, ListUsersCallback, Settings);
    }, 0.1f)); 
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListUsersOperationDone]() { return *bListUsersOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, CurrentListedUserIDs, bListUsersOperationSuccess]()
    {
        TestTrue("ListUsersFromChannel with Limit=5, Offset=2 was successful", *bListUsersOperationSuccess);
        if (*bListUsersOperationSuccess)
        {
            TestEqual("Should return 0 users when Offset exceeds available users", CurrentListedUserIDs->Num(), 0);
        }
    }, 0.1f));

    // Step 6: Unsubscribe from channel
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, UnsubscribeFromChannelCallback, bUnsubscribeFromChannelDone]()
    {
        *bUnsubscribeFromChannelDone = false;
        PubnubSubsystem->UnsubscribeFromChannel(TestChannelName, UnsubscribeFromChannelCallback);
    }, 0.1f));

    // Wait until unsubscribe from channel result is received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bUnsubscribeFromChannelDone]() -> bool {
        return *bUnsubscribeFromChannelDone;
    }, MAX_WAIT_TIME));

    // Check whether unsubscribe from channel result was received
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bUnsubscribeFromChannelDone]()
    {
        if(!*bUnsubscribeFromChannelDone)
        {
            AddError("UnsubscribeFromChannel result callback was not received");
        }
    }, 0.1f));

    CleanUp();
    return true;
}


#endif // WITH_DEV_AUTOMATION_TESTS