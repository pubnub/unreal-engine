// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "FunctionLibraries/PubnubTimetokenUtilities.h"
#include "Dom/JsonObject.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddAndGetMessageActionsTest, FPubnubAutomationTestBase, "Pubnub.Integration.MessageReactions.AddAndGetMessageActions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveMessageActionTest, FPubnubAutomationTestBase, "Pubnub.Integration.MessageReactions.RemoveMessageAction", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubReceiveMessageActionEventTest, FPubnubAutomationTestBase, "Pubnub.Integration.MessageReactions.ReceiveMessageActionEvent", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FPubnubAddAndGetMessageActionsTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestUser = SDK_PREFIX + "user_msg_actions";
    const FString TestChannel = SDK_PREFIX + "chan_msg_actions";
    const FString TestMessageContent = "\"A message to add actions to\"";
    const FString Action1Type = "reaction";
    const FString Action1Value = "smiley_face";
    const FString Action2Type = "comment_ref";
    const FString Action2Value = "thread_123";

    const FString TestStartTimetoken = UPubnubTimetokenUtilities::GetCurrentUnixTimetoken();

    TSharedPtr<bool> bPublishMessageDone = MakeShared<bool>(false);
    TSharedPtr<bool> bPublishMessageSuccess = MakeShared<bool>(false);
    TSharedPtr<FString> PublishedMessageTimetoken = MakeShared<FString>();

    TSharedPtr<bool> bAddAction1Done = MakeShared<bool>(false);
    TSharedPtr<bool> bAddAction1Success = MakeShared<bool>(false);
    TSharedPtr<FString> ReceivedAction1Timetoken = MakeShared<FString>();

    TSharedPtr<bool> bAddAction2Done = MakeShared<bool>(false);
    TSharedPtr<bool> bAddAction2Success = MakeShared<bool>(false);
    TSharedPtr<FString> ReceivedAction2Timetoken = MakeShared<FString>();

    TSharedPtr<bool> bGetActionsDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetActionsSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FPubnubMessageActionData>> ReceivedActionsArray = MakeShared<TArray<FPubnubMessageActionData>>();

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubAddAndGetMessageActionsTest");
        return false;
    }

    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("Pubnub Error in FPubnubAddAndGetMessageActionsTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    PubnubSubsystem->SetUserID(TestUser);

    // Publish Message Callback
    FOnPublishMessageResponseNative PublishCallback;
    PublishCallback.BindLambda([this, bPublishMessageDone, bPublishMessageSuccess, PublishedMessageTimetoken](const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
    {
        *bPublishMessageDone = true;
        *bPublishMessageSuccess = !Result.Error && Result.Status == 200;
        if (*bPublishMessageSuccess)
        {
            *PublishedMessageTimetoken = PublishedMessage.Timetoken;
        }
        else
        {
            AddError(FString::Printf(TEXT("PublishMessage failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    // First Message Action Callback
    FOnAddMessageActionResponseNative AddAction1Callback;
    AddAction1Callback.BindLambda([this, bAddAction1Done, bAddAction1Success, ReceivedAction1Timetoken](const FPubnubOperationResult& Result, FPubnubMessageActionData MessageActionData)
    {
        *bAddAction1Done = true;
        *bAddAction1Success = !Result.Error && Result.Status == 200;
        if (*bAddAction1Success)
        {
            *ReceivedAction1Timetoken = MessageActionData.ActionTimetoken;
        }
        else
        {
            AddError(FString::Printf(TEXT("AddMessageAction1 failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    // Second Message Action Callback
    FOnAddMessageActionResponseNative AddAction2Callback;
    AddAction2Callback.BindLambda([this, bAddAction2Done, bAddAction2Success, ReceivedAction2Timetoken](const FPubnubOperationResult& Result, FPubnubMessageActionData MessageActionData)
    {
        *bAddAction2Done = true;
        *bAddAction2Success = !Result.Error && Result.Status == 200;
        if (*bAddAction2Success)
        {
            *ReceivedAction2Timetoken = MessageActionData.ActionTimetoken;
        }
        else
        {
            AddError(FString::Printf(TEXT("AddMessageAction2 failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    // Get Message Actions Callback
    FOnGetMessageActionsResponseNative GetActionsCallback;
    GetActionsCallback.BindLambda([this, bGetActionsDone, bGetActionsSuccess, ReceivedActionsArray](const FPubnubOperationResult& Result, const TArray<FPubnubMessageActionData>& MessageActions)
    {
        *bGetActionsDone = true;
        *bGetActionsSuccess = !Result.Error && Result.Status == 200;
        if (*bGetActionsSuccess)
        {
            *ReceivedActionsArray = MessageActions;
        }
        else
        {
            AddError(FString::Printf(TEXT("GetMessageActions failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    // Step 1: Publish Message
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessageContent, PublishCallback]()
    {
        FPubnubPublishSettings PublishSettings;
        PublishSettings.StoreInHistory = true;
        PubnubSubsystem->PublishMessage(TestChannel, TestMessageContent, PublishCallback, PublishSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bPublishMessageDone]() { return *bPublishMessageDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bPublishMessageSuccess, PublishedMessageTimetoken]()
    {
        TestTrue("PublishMessage operation was successful", *bPublishMessageSuccess);
        TestFalse("PublishedMessageTimetoken should not be empty", PublishedMessageTimetoken->IsEmpty());
        if (PublishedMessageTimetoken->IsEmpty())
        {
            AddError("Failed to get published message timetoken. AddMessageAction cannot proceed.");
        }
    }, 0.1f));
    
    // Step 2: Add First Message Action (immediate - request queue ensures publish completes first)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, PublishedMessageTimetoken, Action1Type, Action1Value, AddAction1Callback]()
    {
        if (PublishedMessageTimetoken->IsEmpty()) return; // Guard against proceeding without timetoken
        PubnubSubsystem->AddMessageAction(TestChannel, *PublishedMessageTimetoken, Action1Type, Action1Value, AddAction1Callback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bAddAction1Done]() { return *bAddAction1Done; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bAddAction1Success, ReceivedAction1Timetoken]()
    {
        TestTrue("AddAction1 operation was successful", *bAddAction1Success);
        TestFalse("AddAction1Timetoken should not be empty after AddMessageAction", ReceivedAction1Timetoken->IsEmpty());
    }, 0.1f));

    // Step 3: Add Second Message Action (immediate - request queue ensures action1 completes first)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, PublishedMessageTimetoken, Action2Type, Action2Value, AddAction2Callback]()
    {
        if (PublishedMessageTimetoken->IsEmpty()) return;
        PubnubSubsystem->AddMessageAction(TestChannel, *PublishedMessageTimetoken, Action2Type, Action2Value, AddAction2Callback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bAddAction2Done]() { return *bAddAction2Done; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bAddAction2Success, ReceivedAction2Timetoken]()
    {
        TestTrue("AddAction2 operation was successful", *bAddAction2Success);
        TestFalse("AddAction2Timetoken should not be empty after AddMessageAction", ReceivedAction2Timetoken->IsEmpty());
    }, 0.1f));
    
    // Step 4: Get Message Actions (immediate - request queue ensures action2 completes first)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, GetActionsCallback, TestStartTimetoken]()
    {
        // Use a wide enough time range to catch the actions
        FString StartTimetoken = UPubnubTimetokenUtilities::GetCurrentUnixTimetoken();
        PubnubSubsystem->GetMessageActions(TestChannel, GetActionsCallback, StartTimetoken, TestStartTimetoken, 10);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetActionsDone]() { return *bGetActionsDone; }, MAX_WAIT_TIME));

    // Step 5: Verify Received Actions
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
        [this, TestUser, PublishedMessageTimetoken, Action1Type, Action1Value, ReceivedAction1Timetoken, 
              Action2Type, Action2Value, ReceivedAction2Timetoken, ReceivedActionsArray, bGetActionsSuccess]()
    {
        TestTrue("GetMessageActions operation was successful", *bGetActionsSuccess);
        if (!*bGetActionsSuccess || PublishedMessageTimetoken->IsEmpty() || ReceivedAction1Timetoken->IsEmpty() || ReceivedAction2Timetoken->IsEmpty()) 
        {
            AddError("Skipping action verification due to previous errors or missing timetokens (message or action).");
            return;
        }

        bool bFoundAction1 = false;
        bool bFoundAction2 = false;

        for (const FPubnubMessageActionData& Action : *ReceivedActionsArray)
        {
            if (Action.ActionTimetoken == *ReceivedAction1Timetoken)
            {
                bFoundAction1 = true;
                TestEqual("Action1 Type (matched by ActionTimetoken)", Action.Type, Action1Type);
                TestEqual("Action1 Value (matched by ActionTimetoken)", Action.Value, Action1Value);
                TestEqual("Action1 UserID (matched by ActionTimetoken)", Action.UserID, TestUser);
                TestEqual("Action1 MessageTimetoken (matched by ActionTimetoken)", Action.MessageTimetoken, *PublishedMessageTimetoken);
            }
            else if (Action.ActionTimetoken == *ReceivedAction2Timetoken)
            {
                bFoundAction2  = true;
                TestEqual("Action2 Type (matched by ActionTimetoken)", Action.Type, Action2Type);
                TestEqual("Action2 Value (matched by ActionTimetoken)", Action.Value, Action2Value);
                TestEqual("Action2 UserID (matched by ActionTimetoken)", Action.UserID, TestUser);
                TestEqual("Action2 MessageTimetoken (matched by ActionTimetoken)", Action.MessageTimetoken, *PublishedMessageTimetoken);
            }
        }

        TestTrue(FString::Printf(TEXT("Found action 1 ('%s':'%s')"), *Action1Type, *Action1Value), bFoundAction1);
        TestTrue(FString::Printf(TEXT("Found action 2 ('%s':'%s')"), *Action2Type, *Action2Value), bFoundAction2);
        if(ReceivedActionsArray->Num() < 2 && (!bFoundAction1 || !bFoundAction2))
        {
             AddError(FString::Printf(TEXT("Expected at least 2 actions, found %d. Action1 found: %d, Action2 found: %d"), ReceivedActionsArray->Num(), bFoundAction1, bFoundAction2));
        }

    }, 0.1f));

    CleanUp();
    return true;
}

bool FPubnubReceiveMessageActionEventTest::RunTest(const FString& Parameters)
{
    const FString TestUser = SDK_PREFIX + "user_msg_action_event";
    const FString TestChannel = SDK_PREFIX + "chan_msg_action_event";
    const FString TestMessageContent = "\"Hello Message Action Event Test!\"";
    const FString TestActionType = "reaction";
    const FString TestActionValue = "event_heart";

    TSharedPtr<bool> bPublishMessageDone = MakeShared<bool>(false);
    TSharedPtr<bool> bPublishMessageSuccess = MakeShared<bool>(false);
    TSharedPtr<FString> PublishedMessageTimetoken = MakeShared<FString>();

    TSharedPtr<bool> bAddActionDone = MakeShared<bool>(false);
    TSharedPtr<bool> bAddActionSuccess = MakeShared<bool>(false);
    TSharedPtr<FString> AddedActionTimetoken = MakeShared<FString>();

    TSharedPtr<bool> bMessageActionEventReceived = MakeShared<bool>(false);
    TSharedPtr<FString> ReceivedEventActionTimetoken = MakeShared<FString>();
    TSharedPtr<FString> ReceivedEventMessageTimetoken = MakeShared<FString>();
    TSharedPtr<FString> ReceivedEventActionType = MakeShared<FString>();
    TSharedPtr<FString> ReceivedEventActionValue = MakeShared<FString>();
    TSharedPtr<FString> ReceivedEventUserID = MakeShared<FString>();

    TSharedPtr<bool> bSubscribeToChannelDone = MakeShared<bool>(false);

    if (!InitTest())
    {
        AddError(TEXT("TestInitialization failed for FPubnubReceiveMessageActionEventTest"));
        return false;
    }

    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("Pubnub Error in FPubnubReceiveMessageActionEventTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    PubnubSubsystem->SetUserID(TestUser);

    // Publish Message Callback
    FOnPublishMessageResponseNative PublishCallback;
    PublishCallback.BindLambda([this, bPublishMessageDone, bPublishMessageSuccess, PublishedMessageTimetoken](const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
    {
        *bPublishMessageDone = true;
        *bPublishMessageSuccess = !Result.Error && Result.Status == 200;
        if (*bPublishMessageSuccess)
        {
            *PublishedMessageTimetoken = PublishedMessage.Timetoken;
        }
        else
        {
            AddError(FString::Printf(TEXT("PublishMessage failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    // Listener for message action events only (not for published messages)
    PubnubSubsystem->OnMessageReceivedNative.AddLambda(
        [this, TestChannel, bMessageActionEventReceived, ReceivedEventActionTimetoken, ReceivedEventMessageTimetoken,
         ReceivedEventActionType, ReceivedEventActionValue, ReceivedEventUserID]
        (FPubnubMessageData ReceivedMessage)
        {
            if (ReceivedMessage.Channel == TestChannel && ReceivedMessage.MessageType == EPubnubMessageType::PMT_Action)
            {
                TSharedPtr<FJsonObject> JsonObject;
                if (UPubnubJsonUtilities::StringToJsonObject(ReceivedMessage.Message, JsonObject))
                {
                    const TSharedPtr<FJsonObject>* DataObject = nullptr;
                    if (JsonObject->TryGetObjectField(TEXT("data"), DataObject))
                    {
                        (*DataObject)->TryGetStringField(TEXT("actionTimetoken"), *ReceivedEventActionTimetoken);
                        (*DataObject)->TryGetStringField(TEXT("messageTimetoken"), *ReceivedEventMessageTimetoken);
                        (*DataObject)->TryGetStringField(TEXT("type"), *ReceivedEventActionType);
                        (*DataObject)->TryGetStringField(TEXT("value"), *ReceivedEventActionValue);
                        *ReceivedEventUserID = ReceivedMessage.UserID;
                        *bMessageActionEventReceived = true;
                    }
                    else
                    {
                        AddError(TEXT("Received PMT_MessageAction event JSON does not contain 'data' field."));
                    }
                }
                else
                {
                    AddError(FString::Printf(TEXT("Failed to parse PMT_MessageAction event JSON: %s"), *ReceivedMessage.Message));
                }
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

    // Callback for AddMessageAction
    FOnAddMessageActionResponseNative AddActionCallback;
    AddActionCallback.BindLambda([this, bAddActionDone, bAddActionSuccess, AddedActionTimetoken](const FPubnubOperationResult& Result, FPubnubMessageActionData MessageActionData)
    {
        *bAddActionDone = true;
        *bAddActionSuccess = !Result.Error && Result.Status == 200;
        if (*bAddActionSuccess)
        {
            *AddedActionTimetoken = MessageActionData.ActionTimetoken;
        }
        else
        {
            AddError(FString::Printf(TEXT("AddMessageAction failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    // Step 1: Subscribe to Channel
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SubscribeToChannelCallback, bSubscribeToChannelDone]()
    {
        *bSubscribeToChannelDone = false;
        PubnubSubsystem->SubscribeToChannel(TestChannel, SubscribeToChannelCallback);
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

    // Step 2: Publish Message (immediate - using publish callback to get timetoken)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessageContent, PublishCallback]()
    {
        FPubnubPublishSettings PublishSettings;
        PublishSettings.StoreInHistory = true; // Ensure it can have actions
        PubnubSubsystem->PublishMessage(TestChannel, TestMessageContent, PublishCallback, PublishSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bPublishMessageDone]() { return *bPublishMessageDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bPublishMessageSuccess, PublishedMessageTimetoken]()
    {
        TestTrue("PublishMessage operation was successful", *bPublishMessageSuccess);
        TestFalse("PublishedMessageTimetoken should not be empty", PublishedMessageTimetoken->IsEmpty());
        if (PublishedMessageTimetoken->IsEmpty())
        {
            AddError("Failed to get published message timetoken from publish callback. Cannot proceed to add action.");
        }
    }, 0.1f));
    
    // Step 3: Add Message Action (immediate - request queue ensures publish completes first)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, PublishedMessageTimetoken, TestActionType, TestActionValue, AddActionCallback]()
    {
        if (PublishedMessageTimetoken->IsEmpty()) return; // Guard
        PubnubSubsystem->AddMessageAction(TestChannel, *PublishedMessageTimetoken, TestActionType, TestActionValue, AddActionCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bAddActionDone, bAddActionSuccess, AddedActionTimetoken]() { return *bAddActionDone && *bAddActionSuccess && !AddedActionTimetoken->IsEmpty(); }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, AddedActionTimetoken]()
    {
        if (AddedActionTimetoken->IsEmpty())
        {
            AddError(TEXT("Failed to capture added action timetoken. Cannot verify event."));
        }
    }, 0.1f));

    // Step 4: Wait for Message Action Event to be received
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bMessageActionEventReceived]()
    {
        return *bMessageActionEventReceived;
    }, MAX_WAIT_TIME));

    // Step 5: Verify the received Message Action Event
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
        [this, TestUser, TestActionType, TestActionValue, PublishedMessageTimetoken, AddedActionTimetoken,
         bMessageActionEventReceived, ReceivedEventActionTimetoken, ReceivedEventMessageTimetoken,
         ReceivedEventActionType, ReceivedEventActionValue, ReceivedEventUserID]()
    {
        TestTrue(TEXT("Message Action Event was received."), *bMessageActionEventReceived);
        if (!*bMessageActionEventReceived)
        {
            AddError(TEXT("Message action event was NOT received."));
            return;
        }
        if (PublishedMessageTimetoken->IsEmpty() || AddedActionTimetoken->IsEmpty())
        {
            AddError(TEXT("Cannot verify event due to missing published message timetoken or added action timetoken."));
            return;
        }

        TestEqual(TEXT("Event: Action Timetoken matches"), *ReceivedEventActionTimetoken, *AddedActionTimetoken);
        TestEqual(TEXT("Event: Message Timetoken matches"), *ReceivedEventMessageTimetoken, *PublishedMessageTimetoken);
        TestEqual(TEXT("Event: Action Type matches"), *ReceivedEventActionType, TestActionType);
        TestEqual(TEXT("Event: Action Value matches"), *ReceivedEventActionValue, TestActionValue);
        TestEqual(TEXT("Event: User ID matches"), *ReceivedEventUserID, TestUser);
    }, 0.1f));

    CleanUp();
    return true;
}

bool FPubnubRemoveMessageActionTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestUser = SDK_PREFIX + "user_remove_action";
    const FString TestChannel = SDK_PREFIX + "chan_remove_action";
    const FString TestMessageContent = "\"Message for remove action test\"";
    const FString ActionTypeToRemove = "reaction";
    const FString ActionValueToRemove = "thumb_up_for_removal";

    TSharedPtr<FString> TestRunStartTimetoken = MakeShared<FString>(UPubnubTimetokenUtilities::GetCurrentUnixTimetoken());

    TSharedPtr<bool> bPublishMessageDone = MakeShared<bool>(false);
    TSharedPtr<bool> bPublishMessageSuccess = MakeShared<bool>(false);
    TSharedPtr<FString> PublishedMessageTimetoken = MakeShared<FString>();

    TSharedPtr<bool> bAddActionDone = MakeShared<bool>(false);
    TSharedPtr<bool> bAddActionSuccess = MakeShared<bool>(false);
    TSharedPtr<FString> AddedActionTimetoken = MakeShared<FString>();

    TSharedPtr<bool> bRemoveActionDone = MakeShared<bool>(false);
    TSharedPtr<bool> bRemoveActionSuccess = MakeShared<bool>(false);

    TSharedPtr<bool> bGetActionsDone_Initial = MakeShared<bool>(false);
    TSharedPtr<bool> bGetActionsSuccess_Initial = MakeShared<bool>(false);
    TSharedPtr<TArray<FPubnubMessageActionData>> ReceivedActionsArray_Initial = MakeShared<TArray<FPubnubMessageActionData>>();

    TSharedPtr<bool> bGetActionsDone_AfterRemove = MakeShared<bool>(false);
    TSharedPtr<bool> bGetActionsSuccess_AfterRemove = MakeShared<bool>(false);
    TSharedPtr<TArray<FPubnubMessageActionData>> ReceivedActionsArray_AfterRemove = MakeShared<TArray<FPubnubMessageActionData>>();

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubRemoveMessageActionTest");
        return false;
    }

    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("Pubnub Error in FPubnubRemoveMessageActionTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    PubnubSubsystem->SetUserID(TestUser);

    // Publish Message Callback
    FOnPublishMessageResponseNative PublishCallback;
    PublishCallback.BindLambda([this, bPublishMessageDone, bPublishMessageSuccess, PublishedMessageTimetoken](const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
    {
        *bPublishMessageDone = true;
        *bPublishMessageSuccess = !Result.Error && Result.Status == 200;
        if (*bPublishMessageSuccess)
        {
            *PublishedMessageTimetoken = PublishedMessage.Timetoken;
        }
        else
        {
            AddError(FString::Printf(TEXT("PublishMessage failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    // Add Message Action Callback
    FOnAddMessageActionResponseNative AddActionCallback;
    AddActionCallback.BindLambda([this, bAddActionDone, bAddActionSuccess, AddedActionTimetoken](const FPubnubOperationResult& Result, FPubnubMessageActionData MessageActionData)
    {
        *bAddActionDone = true;
        *bAddActionSuccess = !Result.Error && Result.Status == 200;
        if (*bAddActionSuccess)
        {
            *AddedActionTimetoken = MessageActionData.ActionTimetoken;
        }
        else
        {
            AddError(FString::Printf(TEXT("AddMessageAction failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    // Remove Message Action Callback
    FOnRemoveMessageActionResponseNative RemoveActionCallback;
    RemoveActionCallback.BindLambda([this, bRemoveActionDone, bRemoveActionSuccess](const FPubnubOperationResult& Result)
    {
        *bRemoveActionDone = true;
        *bRemoveActionSuccess = !Result.Error && Result.Status == 200;
        if (!*bRemoveActionSuccess)
        {
            AddError(FString::Printf(TEXT("RemoveMessageAction failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    // Get Message Actions Callbacks
    FOnGetMessageActionsResponseNative GetActionsCallback_Initial;
    GetActionsCallback_Initial.BindLambda([this, bGetActionsDone_Initial, bGetActionsSuccess_Initial, ReceivedActionsArray_Initial](const FPubnubOperationResult& Result, const TArray<FPubnubMessageActionData>& MessageActions)
    {
        *bGetActionsDone_Initial = true;
        *bGetActionsSuccess_Initial = !Result.Error && Result.Status == 200;
        if (*bGetActionsSuccess_Initial)
        {
            *ReceivedActionsArray_Initial = MessageActions;
        }
        else
        {
            AddError(FString::Printf(TEXT("GetMessageActions (Initial) failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });
    
    FOnGetMessageActionsResponseNative GetActionsCallback_AfterRemove;
    GetActionsCallback_AfterRemove.BindLambda([this, bGetActionsDone_AfterRemove, bGetActionsSuccess_AfterRemove, ReceivedActionsArray_AfterRemove](const FPubnubOperationResult& Result, const TArray<FPubnubMessageActionData>& MessageActions)
    {
        *bGetActionsDone_AfterRemove = true;
        *bGetActionsSuccess_AfterRemove = !Result.Error && Result.Status == 200;
        if (*bGetActionsSuccess_AfterRemove)
        {
            *ReceivedActionsArray_AfterRemove = MessageActions;
        }
        else
        {
            AddError(FString::Printf(TEXT("GetMessageActions (After Remove) failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    // Step 1: Publish Message
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessageContent, PublishCallback]()
    {
        FPubnubPublishSettings PublishSettings;
        PublishSettings.StoreInHistory = true;
        PubnubSubsystem->PublishMessage(TestChannel, TestMessageContent, PublishCallback, PublishSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bPublishMessageDone]() { return *bPublishMessageDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bPublishMessageSuccess, PublishedMessageTimetoken]()
    {
        TestTrue("PublishMessage operation was successful", *bPublishMessageSuccess);
        TestFalse("PublishedMessageTimetoken should not be empty", PublishedMessageTimetoken->IsEmpty());
        if (PublishedMessageTimetoken->IsEmpty())
        {
            AddError("Failed to get published message timetoken. Test cannot proceed.");
        }
    }, 0.1f));

    // Step 2: Add Message Action (immediate - request queue ensures publish completes first)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, PublishedMessageTimetoken, ActionTypeToRemove, ActionValueToRemove, AddActionCallback]()
    {
        if (PublishedMessageTimetoken->IsEmpty()) { AddError("Skipping AddMessageAction due to missing PublishedMessageTimetoken"); return; }
        PubnubSubsystem->AddMessageAction(TestChannel, *PublishedMessageTimetoken, ActionTypeToRemove, ActionValueToRemove, AddActionCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bAddActionDone]() { return *bAddActionDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bAddActionSuccess, AddedActionTimetoken]()
    {
        TestTrue("AddMessageAction operation was successful", *bAddActionSuccess);
        TestFalse("AddedActionTimetoken should not be empty after AddMessageAction", AddedActionTimetoken->IsEmpty());
        if(AddedActionTimetoken->IsEmpty()) AddError("AddedActionTimetoken is empty. Cannot proceed with verification or removal.");
    }, 0.1f));

    // Step 3: Get Message Actions (Initial) and Verify Action is Present (immediate - request queue ensures add completes first)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, GetActionsCallback_Initial, TestRunStartTimetoken]()
    {
        FString CurrentTimetoken = UPubnubTimetokenUtilities::GetCurrentUnixTimetoken();
        PubnubSubsystem->GetMessageActions(TestChannel, GetActionsCallback_Initial, CurrentTimetoken, *TestRunStartTimetoken, 10);
    }, 0.1f)); 
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetActionsDone_Initial]() { return *bGetActionsDone_Initial; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
        [this, TestUser, PublishedMessageTimetoken, ActionTypeToRemove, ActionValueToRemove, AddedActionTimetoken, ReceivedActionsArray_Initial, bGetActionsSuccess_Initial]()
    {
        TestTrue("GetMessageActions (Initial) operation was successful", *bGetActionsSuccess_Initial);
        if (!*bGetActionsSuccess_Initial || PublishedMessageTimetoken->IsEmpty() || AddedActionTimetoken->IsEmpty()) 
        {
            AddError("Skipping initial action verification due to previous errors or missing timetokens.");
            return;
        }

        bool bFoundActionInitially = false;
        for (const FPubnubMessageActionData& Action : *ReceivedActionsArray_Initial)
        {
            if (Action.ActionTimetoken == *AddedActionTimetoken)
            {
                bFoundActionInitially = true;
                TestEqual("Initial Action Type", Action.Type, ActionTypeToRemove);
                TestEqual("Initial Action Value", Action.Value, ActionValueToRemove);
                TestEqual("Initial Action UserID", Action.UserID, TestUser);
                TestEqual("Initial Action MessageTimetoken", Action.MessageTimetoken, *PublishedMessageTimetoken);
                break;
            }
        }
        TestTrue(FString::Printf(TEXT("Action with timetoken '%s' (Type:'%s', Value:'%s') should be present initially."), **AddedActionTimetoken, *ActionTypeToRemove, *ActionValueToRemove), bFoundActionInitially);
        if(!bFoundActionInitially) AddError("Action to be removed was not found in initial GetMessageActions call.");
    }, 0.1f));
    
    // Step 4: Remove Message Action (immediate - request queue ensures get actions completes first)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, PublishedMessageTimetoken, AddedActionTimetoken, RemoveActionCallback]()
    {
        if (PublishedMessageTimetoken->IsEmpty() || AddedActionTimetoken->IsEmpty())
        {
            AddError("Cannot remove action due to missing PublishedMessageTimetoken or AddedActionTimetoken.");
            return;
        }
        PubnubSubsystem->RemoveMessageAction(TestChannel, *PublishedMessageTimetoken, *AddedActionTimetoken, RemoveActionCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveActionDone]() { return *bRemoveActionDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bRemoveActionSuccess]()
    {
        TestTrue("RemoveMessageAction operation was successful", *bRemoveActionSuccess);
    }, 0.1f));

    // Step 5: Get Message Actions (After Remove) and Verify Action is NOT Present (immediate - request queue ensures remove completes first)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, GetActionsCallback_AfterRemove, TestRunStartTimetoken]()
    {
        FString CurrentTimetoken = UPubnubTimetokenUtilities::GetCurrentUnixTimetoken();
        PubnubSubsystem->GetMessageActions(TestChannel, GetActionsCallback_AfterRemove, CurrentTimetoken, *TestRunStartTimetoken, 10);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetActionsDone_AfterRemove]() { return *bGetActionsDone_AfterRemove; }, MAX_WAIT_TIME));
    
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
        [this, AddedActionTimetoken, ReceivedActionsArray_AfterRemove, bGetActionsSuccess_AfterRemove, ActionTypeToRemove, ActionValueToRemove]()
    {
        TestTrue("GetMessageActions (After Remove) operation was successful", *bGetActionsSuccess_AfterRemove);
        if (!*bGetActionsSuccess_AfterRemove || AddedActionTimetoken->IsEmpty())
        {
            AddError("Skipping post-removal action verification due to previous errors or missing AddedActionTimetoken.");
            return;
        }

        bool bActionStillPresent = false;
        for (const FPubnubMessageActionData& Action : *ReceivedActionsArray_AfterRemove)
        {
            if (Action.ActionTimetoken == *AddedActionTimetoken)
            {
                bActionStillPresent = true;
                AddError(FString::Printf(TEXT("Error: Action with timetoken '%s' (Type:'%s', Value:'%s') was found after it was supposed to be removed."), **AddedActionTimetoken, *ActionTypeToRemove, *ActionValueToRemove));
                break;
            }
        }
        TestFalse(FString::Printf(TEXT("Action with timetoken '%s' (Type:'%s', Value:'%s') should NOT be present after removal."), **AddedActionTimetoken, *ActionTypeToRemove, *ActionValueToRemove), bActionStillPresent);
    }, 0.1f));

    CleanUp();
    return true;
}


#endif // WITH_DEV_AUTOMATION_TESTS