#include "Misc/AutomationTest.h"
#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUsersFromChannelTest, FPubnubAutomationTestBase, "Pubnub.Integration.Presence.ListUsersFromChannel", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubListUserSubscribedChannelsTest, FPubnubAutomationTestBase, "Pubnub.Integration.Presence.ListUserSubscribedChannels", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubChannelSetGetStateTest, FPubnubAutomationTestBase, "Pubnub.Integration.Presence.SetGetState", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubChannelSetGetStateForMultipleTest, FPubnubAutomationTestBase, "Pubnub.Integration.Presence.SetGetStateMultipleChannels", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

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
        TestTrue("ListUsersFromChannel operation (1) was successful", *bListUsersOperationSuccess);
        if (*bListUsersOperationSuccess)
        {
            TestTrue(FString::Printf(TEXT("TestUserID '%s' should be in the list after subscribe"), *TestUserID), CurrentListedUserIDs->Contains(TestUserID));
            TestEqual("Occupancy should be 1 after subscribe", *CurrentOccupancy, 1);
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
        TestTrue("ListUserSubscribedChannels (1) operation successful", *bListChannelsOpSuccess);
        if(*bListChannelsOpSuccess)
        {
            TestEqual("Should be subscribed to 1 channel", ReceivedSubscribedChannels->Num(), 1);
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
        TestTrue("ListUserSubscribedChannels (2) operation successful", *bListChannelsOpSuccess);
        if(*bListChannelsOpSuccess)
        {
            TestEqual("Should be subscribed to 2 channels", ReceivedSubscribedChannels->Num(), 2);
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
        TestTrue("ListUserSubscribedChannels (3) operation successful", *bListChannelsOpSuccess);
        if(*bListChannelsOpSuccess)
        {
            TestEqual("Should be subscribed to 1 channel after unsubscribing from A", ReceivedSubscribedChannels->Num(), 1);
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

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubChannelSetGetStateTest");
        return false;
    }

    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
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
    // Step 2: Set Initial State
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName, InitialStateJson]()
    {
        PubnubSubsystem->SetState(TestChannelName, InitialStateJson);
    }, 0.2f));
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f)); // Allow SetState to propagate before GetState

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
                const TSharedPtr<FJsonObject>* PayloadObjectPtr;
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
        TestTrue("GetState (2) operation should have completed (received a response).", !ReceivedStateJson->IsEmpty());
        if (!ReceivedStateJson->IsEmpty())
        {
            TSharedPtr<FJsonObject> JsonObject;
            if(UPubnubJsonUtilities::StringToJsonObject(*ReceivedStateJson, JsonObject))
            {
                const TSharedPtr<FJsonObject>* PayloadObjectPtr;
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
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelName]()
    {
        PubnubSubsystem->UnsubscribeFromChannel(TestChannelName);
    }, 0.2f));

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

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubChannelSetGetStateForMultipleTest");
        return false;
    }

    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("General Pubnub Error in FPubnubChannelSetGetStateForMultipleTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    FOnPubnubResponseNative GetStateCallback;
    GetStateCallback.BindLambda([this, bGetStateOperationDone, ReceivedCombinedStateJson](FString JsonResponse)
    {
        *bGetStateOperationDone = true;
        *ReceivedCombinedStateJson = JsonResponse;
    });

    // Set UserID first
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID]()
    {
        PubnubSubsystem->SetUserID(TestUserID);
    }, 0.1f));

    // Subscribe to Channel 1
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1Name]()
    {
        PubnubSubsystem->SubscribeToChannel(TestChannel1Name);
    }, 0.2f));

    // Subscribe to Channel 2
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel2Name]()
    {
        PubnubSubsystem->SubscribeToChannel(TestChannel2Name);
    }, 0.5f));

    // Set State for Channel 1
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1Name, State1Json]()
    {
        PubnubSubsystem->SetState(TestChannel1Name, State1Json);
    }, 0.3f));

    // Set State for Channel 2
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel2Name, State2Json]()
    {
        PubnubSubsystem->SetState(TestChannel2Name, State2Json);
    }, 0.3f));

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

        const TSharedPtr<FJsonObject>* PayloadObjectPtr;
        if (!FullResponseObject->TryGetObjectField(TEXT("payload"), PayloadObjectPtr) || !PayloadObjectPtr || !(*PayloadObjectPtr).IsValid())
        {
            AddError("GetState (combined) response JSON does not contain a valid 'payload' object.");
            return;
        }

        const TSharedPtr<FJsonObject>* ChannelsObjectPtr;
        if (!(*PayloadObjectPtr)->TryGetObjectField(TEXT("channels"), ChannelsObjectPtr) || !ChannelsObjectPtr || !(*ChannelsObjectPtr).IsValid())
        {
            AddError("GetState (combined) payload does not contain a valid 'channels' object.");
            return;
        }
        const TSharedPtr<FJsonObject>& ChannelsObject = *ChannelsObjectPtr;

        // Verify State for Channel 1
        const TSharedPtr<FJsonObject>* Channel1StateObjectPtr;
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
        const TSharedPtr<FJsonObject>* Channel2StateObjectPtr;
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
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1Name]()
    {
        PubnubSubsystem->UnsubscribeFromChannel(TestChannel1Name);
    }, 0.2f));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel2Name]()
    {
        PubnubSubsystem->UnsubscribeFromChannel(TestChannel2Name);
    }, 0.2f));

    CleanUp();
    return true;
}


