#include "Misc/AutomationTest.h"
#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUsersFromChannelTest, FPubnubAutomationTestBase, "Pubnub.E2E.Presence.ListUsersFromChannel", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUserSubscribedChannelsTest, FPubnubAutomationTestBase, "Pubnub.E2E.Presence.ListUserSubscribedChannels", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubChannelSetGetStateTest, FPubnubAutomationTestBase, "Pubnub.E2E.Presence.ChannelSetGetState", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FPubnubListUsersFromChannelTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestUserID = SDK_PREFIX + "test_user_list_users";
    const FString TestChannelName = SDK_PREFIX + "test_channel_list_users";

    TSharedPtr<bool> bListUsersOperationDone = MakeShared<bool>(false);
    TSharedPtr<bool> bListUsersOperationSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FString>> CurrentListedUserIDs = MakeShared<TArray<FString>>();
    TSharedPtr<int> CurrentOccupancy = MakeShared<int>(0);

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
    ListUsersCallback.BindLambda([this, bListUsersOperationDone, bListUsersOperationSuccess, CurrentListedUserIDs, CurrentOccupancy](int ResponseStatus, FString ResponseMessage, FPubnubListUsersFromChannelWrapper ResponseData)
    {
        *bListUsersOperationDone = true;
        CurrentListedUserIDs->Empty();
        *CurrentOccupancy = ResponseData.Occupancy;
        if (ResponseStatus == 200) 
        {
            *bListUsersOperationSuccess = true;
            ResponseData.UsersState.GetKeys(*CurrentListedUserIDs);
        }
        else
        {
            *bListUsersOperationSuccess = false;
            AddError(FString::Printf(TEXT("ListUsersFromChannel failed. Status: %d"), ResponseStatus));
        }
    });

    // Set UserID first
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID]()
    {
        PubnubSubsystem->SetUserID(TestUserID);
    }, 0.1f));

    // Step 1: Subscribe to channel
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName]()
    {
        PubnubSubsystem->SubscribeToChannel(TestChannelName);
    }, 0.2f));

    // Step 2: List users and verify TestUserID is present
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, ListUsersCallback, bListUsersOperationDone, bListUsersOperationSuccess]()
    {
        *bListUsersOperationDone = false;
        *bListUsersOperationSuccess = false;
        FPubnubListUsersFromChannelSettings Settings;
        Settings.DisableUserID = false;
        PubnubSubsystem->ListUsersFromChannel(TestChannelName, ListUsersCallback, Settings);
    }, 0.5f)); 
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListUsersOperationDone]() { return *bListUsersOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, CurrentListedUserIDs, bListUsersOperationSuccess, CurrentOccupancy]()
    {
        TestTrue(TEXT("ListUsersFromChannel operation (1) was successful"), *bListUsersOperationSuccess);
        if (*bListUsersOperationSuccess)
        {
            TestTrue(FString::Printf(TEXT("TestUserID '%s' should be in the list after subscribe"), *TestUserID), CurrentListedUserIDs->Contains(TestUserID));
            TestEqual(TEXT("Occupancy should be 1 after subscribe"), *CurrentOccupancy, 1);
        }
    }, 0.1f));

    // Step 3: Unsubscribe from channel
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName]()
    {
        PubnubSubsystem->UnsubscribeFromChannel(TestChannelName);
    }, 0.3f));

    // Step 4: List users again and verify TestUserID is NOT present
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, ListUsersCallback, bListUsersOperationDone, bListUsersOperationSuccess]()
    {
        *bListUsersOperationDone = false;
        *bListUsersOperationSuccess = false;
        FPubnubListUsersFromChannelSettings Settings;
        Settings.DisableUserID = false;
        PubnubSubsystem->ListUsersFromChannel(TestChannelName, ListUsersCallback, Settings);
    }, 0.5f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListUsersOperationDone]() { return *bListUsersOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, CurrentListedUserIDs, bListUsersOperationSuccess, CurrentOccupancy]()
    {
        TestTrue(TEXT("ListUsersFromChannel operation (2) was successful"), *bListUsersOperationSuccess);
        if (*bListUsersOperationSuccess)
        {
            TestFalse(FString::Printf(TEXT("TestUserID '%s' should NOT be in the list after unsubscribe"), *TestUserID), CurrentListedUserIDs->Contains(TestUserID));
            TestEqual(TEXT("Occupancy should be 0 after unsubscribe"), *CurrentOccupancy, 0);
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
    ListChannelsCallback.BindLambda([this, bListChannelsOpDone, bListChannelsOpSuccess, ReceivedSubscribedChannels](int Status, FString Message, const TArray<FString>& Channels)
    {
        *bListChannelsOpDone = true;
        *bListChannelsOpSuccess = (Status == 200);
        if (*bListChannelsOpSuccess)
        {
            *ReceivedSubscribedChannels = Channels;
        }
        else
        {
            ReceivedSubscribedChannels->Empty();
            AddError(FString::Printf(TEXT("ListUserSubscribedChannels failed. Status: %d"), Status));
        }
    });

    // Set UserID first
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID]()
    {
        PubnubSubsystem->SetUserID(TestUserID);
    }, 0.1f));

    // Step 1: Subscribe to Channel A
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelA]()
    {
        PubnubSubsystem->SubscribeToChannel(TestChannelA);
    }, 0.2f));

    // Step 2: List and Verify Channel A
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, ListChannelsCallback, bListChannelsOpDone, bListChannelsOpSuccess, ReceivedSubscribedChannels]()
    {
        *bListChannelsOpDone = false; *bListChannelsOpSuccess = false; ReceivedSubscribedChannels->Empty();
        PubnubSubsystem->ListUserSubscribedChannels(TestUserID, ListChannelsCallback);
    }, 0.5f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListChannelsOpDone]() { return *bListChannelsOpDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelA, ReceivedSubscribedChannels, bListChannelsOpSuccess]()
    {
        TestTrue(TEXT("ListUserSubscribedChannels (1) operation successful"), *bListChannelsOpSuccess);
        if(*bListChannelsOpSuccess)
        {
            TestEqual(TEXT("Should be subscribed to 1 channel"), ReceivedSubscribedChannels->Num(), 1);
            TestTrue(FString::Printf(TEXT("Should contain '%s'"), *TestChannelA), ReceivedSubscribedChannels->Contains(TestChannelA));
        }
    }, 0.1f));

    // Step 3: Subscribe to Channel B
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelB]()
    {
        PubnubSubsystem->SubscribeToChannel(TestChannelB);
    }, 0.2f));

    // Step 4: List and Verify Channel A & B
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, ListChannelsCallback, bListChannelsOpDone, bListChannelsOpSuccess, ReceivedSubscribedChannels]()
    {
        *bListChannelsOpDone = false; *bListChannelsOpSuccess = false; ReceivedSubscribedChannels->Empty();
        PubnubSubsystem->ListUserSubscribedChannels(TestUserID, ListChannelsCallback);
    }, 0.5f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListChannelsOpDone]() { return *bListChannelsOpDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelA, TestChannelB, ReceivedSubscribedChannels, bListChannelsOpSuccess]()
    {
        TestTrue(TEXT("ListUserSubscribedChannels (2) operation successful"), *bListChannelsOpSuccess);
        if(*bListChannelsOpSuccess)
        {
            TestEqual(TEXT("Should be subscribed to 2 channels"), ReceivedSubscribedChannels->Num(), 2);
            TestTrue(FString::Printf(TEXT("Should contain '%s'"), *TestChannelA), ReceivedSubscribedChannels->Contains(TestChannelA));
            TestTrue(FString::Printf(TEXT("Should contain '%s'"), *TestChannelB), ReceivedSubscribedChannels->Contains(TestChannelB));
        }
    }, 0.1f));

    // Step 5: Unsubscribe from Channel A
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelA]()
    {
        PubnubSubsystem->UnsubscribeFromChannel(TestChannelA);
    }, 0.2f));

    // Step 6: List and Verify Only Channel B
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, ListChannelsCallback, bListChannelsOpDone, bListChannelsOpSuccess, ReceivedSubscribedChannels]()
    {
        *bListChannelsOpDone = false; *bListChannelsOpSuccess = false; ReceivedSubscribedChannels->Empty();
        PubnubSubsystem->ListUserSubscribedChannels(TestUserID, ListChannelsCallback);
    }, 0.5f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListChannelsOpDone]() { return *bListChannelsOpDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelA, TestChannelB, ReceivedSubscribedChannels, bListChannelsOpSuccess]()
    {
        TestTrue(TEXT("ListUserSubscribedChannels (3) operation successful"), *bListChannelsOpSuccess);
        if(*bListChannelsOpSuccess)
        {
            TestEqual(TEXT("Should be subscribed to 1 channel after unsubscribing from A"), ReceivedSubscribedChannels->Num(), 1);
            TestFalse(FString::Printf(TEXT("Should NOT contain '%s'"), *TestChannelA), ReceivedSubscribedChannels->Contains(TestChannelA));
            TestTrue(FString::Printf(TEXT("Should still contain '%s'"), *TestChannelB), ReceivedSubscribedChannels->Contains(TestChannelB));
        }
    }, 0.1f));

    // Step 7: Unsubscribe from Channel B
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelB]()
    {
        PubnubSubsystem->UnsubscribeFromChannel(TestChannelB);
    }, 0.2f));

    // Step 8: List and Verify Empty List
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, ListChannelsCallback, bListChannelsOpDone, bListChannelsOpSuccess, ReceivedSubscribedChannels]()
    {
        *bListChannelsOpDone = false; *bListChannelsOpSuccess = false; ReceivedSubscribedChannels->Empty();
        PubnubSubsystem->ListUserSubscribedChannels(TestUserID, ListChannelsCallback);
    }, 0.5f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListChannelsOpDone]() { return *bListChannelsOpDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedSubscribedChannels, bListChannelsOpSuccess]()
    {
        TestTrue(TEXT("ListUserSubscribedChannels (4) operation successful"), *bListChannelsOpSuccess);
        if(*bListChannelsOpSuccess)
        {
            TestEqual(TEXT("Should be subscribed to 0 channels after unsubscribing from all"), ReceivedSubscribedChannels->Num(), 0);
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
    const FString InitialStateJson = TEXT("{\"mood\": \"happy\", \"level\": 10}");
    const FString UpdatedStateJson = TEXT("{\"mood\": \"focused\", \"level\": 11, \"item\": \"shield\"}");

    TSharedPtr<bool> bGetStateOperationDone = MakeShared<bool>(false);
    // For GetState, the FOnPubnubResponse delegate's FString is the JSON response. Success can be inferred if it's not empty and parses, but there's no explicit success bool from that delegate.
    // We'll consider it a success if the callback is fired and we get some JSON back.
    // For more robust error handling, GetState would ideally have a status/error parameter in its callback.
    TSharedPtr<FString> ReceivedStateJson = MakeShared<FString>();

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubChannelSetGetStateTest");
        return false;
    }

    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        // Avoid flagging errors from GetState if it uses PNER_OK with an empty JSON for no state, handle that in callback.
        AddError(FString::Printf(TEXT("General Pubnub Error in FPubnubChannelSetGetStateTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    FOnPubnubResponseNative GetStateCallback;
    GetStateCallback.BindLambda([this, bGetStateOperationDone, ReceivedStateJson](FString JsonResponse)
    {
        *bGetStateOperationDone = true;
        *ReceivedStateJson = JsonResponse;
    });

    // Set UserID first
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID]()
    {
        PubnubSubsystem->SetUserID(TestUserID);
    }, 0.1f));

    // Step 1: Subscribe to Channel
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName]()
    {
        PubnubSubsystem->SubscribeToChannel(TestChannelName);
    }, 0.2f));
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f)); // Allow subscription to process

    // Step 2: Set Initial State
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, InitialStateJson]()
    {
        PubnubSubsystem->SetState(TestChannelName, InitialStateJson);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.5f)); // Allow SetState to propagate before GetState

    // Step 3: Get Initial State and Verify
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, TestUserID, GetStateCallback, bGetStateOperationDone, ReceivedStateJson]()
    {
        *bGetStateOperationDone = false;
        ReceivedStateJson->Empty();
        PubnubSubsystem->GetState(TestChannelName, TEXT(""), TestUserID, GetStateCallback); // ChannelGroup is empty for channel state
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetStateOperationDone]() { return *bGetStateOperationDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, InitialStateJson, ReceivedStateJson]()
    {
        TestTrue(TEXT("GetState (1) operation should have completed (received a response)."), !ReceivedStateJson->IsEmpty());
        if (!ReceivedStateJson->IsEmpty())
        {
             // For robust comparison, parse JSON. For simplicity here, direct string comparison (might be flaky due to key order/whitespace)
            TestEqual(TEXT("Retrieved initial state should match set state"), *ReceivedStateJson, InitialStateJson);
        }
    }, 0.1f));

    // Step 4: Set Updated State
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, UpdatedStateJson]()
    {
        PubnubSubsystem->SetState(TestChannelName, UpdatedStateJson);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.5f)); // Allow SetState to propagate

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
        TestTrue(TEXT("GetState (2) operation should have completed (received a response)."), !ReceivedStateJson->IsEmpty());
        if (!ReceivedStateJson->IsEmpty())
        {
            TestEqual(TEXT("Retrieved updated state should match set state"), *ReceivedStateJson, UpdatedStateJson);
        }
    }, 0.1f));
    
    // Step 6: Unsubscribe (optional, but good for hygiene before CleanUp)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName]()
    {
        PubnubSubsystem->UnsubscribeFromChannel(TestChannelName);
    }, 0.2f));

    CleanUp();
    return true;
}


