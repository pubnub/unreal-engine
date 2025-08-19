// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.h"
#include "Kismet/GameplayStatics.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddChannelToGroupAndReceiveMessageTest, FPubnubAutomationTestBase, "Pubnub.Integration.ChannelGroups.AddChannelToGroupAndReceiveMessage", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubChannelAddRemoveListGroupTest, FPubnubAutomationTestBase, "Pubnub.Integration.ChannelGroups.AddRemoveListChannelsInGroup", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUnsubscribeFromGroupTest, FPubnubAutomationTestBase, "Pubnub.Integration.ChannelGroups.UnsubscribeFromGroup", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUnsubscribeFromAllTest, FPubnubAutomationTestBase, "Pubnub.Integration.ChannelGroups.UnsubscribeFromAll", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRemoveGroupTest, FPubnubAutomationTestBase, "Pubnub.Integration.ChannelGroups.RemoveGroup", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FPubnubAddChannelToGroupAndReceiveMessageTest::RunTest(const FString& Parameters)
{
	//Initial variables
	const FString TestMessage = "\"Message for group test\"";
	const FString TestUser = SDK_PREFIX + "test_user_group";
	const FString TestChannel = SDK_PREFIX + "test_channel_for_group";
	const FString TestGroup = SDK_PREFIX + "test_group";
	TSharedPtr<bool> TestMessageReceived = MakeShared<bool>(false);
	
	// Shared state for operation callbacks
	TSharedPtr<bool> bAddChannelToGroupDone = MakeShared<bool>(false);
	TSharedPtr<bool> bAddChannelToGroupSuccess = MakeShared<bool>(false);
	TSharedPtr<bool> bPublishDone = MakeShared<bool>(false);
	TSharedPtr<bool> bPublishSuccess = MakeShared<bool>(false);
	TSharedPtr<bool> bRemoveChannelFromGroupDone = MakeShared<bool>(false);
	TSharedPtr<bool> bSubscribeToGroupDone = MakeShared<bool>(false);
	TSharedPtr<bool> bUnsubscribeFromGroupDone = MakeShared<bool>(false);
	
	if(!InitTest())
	{
		AddError("TestInitialization failed for FPubnubAddChannelToGroupAndReceiveMessageTest");
		return false;
	}

	//Check for any errors on the way
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(FString::Printf(TEXT("Pubnub Error in FPubnubAddChannelToGroupAndReceiveMessageTest: %s, Type: %d"), *ErrorMessage, ErrorType));
	});
	
	//Set User ID - it's necessary for any operation
	PubnubSubsystem->SetUserID(TestUser);

	//Add listener for received messages
	PubnubSubsystem->OnMessageReceivedNative.AddLambda([this, TestMessage, TestChannel, TestUser, TestMessageReceived](FPubnubMessageData ReceivedMessage)
	{
		// Message should still report the original channel it was published to
		if (ReceivedMessage.Channel == TestChannel)
		{
			*TestMessageReceived = true;
			TestEqual("Group test: Published message content", TestMessage, ReceivedMessage.Message);
			TestEqual("Group test: Published message original channel", TestChannel, ReceivedMessage.Channel);
			TestEqual("Group test: Published message UserID", TestUser, ReceivedMessage.UserID);
			TestEqual("Group test: Published message MessageType", EPubnubMessageType::PMT_Published, ReceivedMessage.MessageType);
		}
	});

	// Define callbacks
	FOnAddChannelToGroupResponseNative AddChannelCallback;
	AddChannelCallback.BindLambda([this, bAddChannelToGroupDone, bAddChannelToGroupSuccess](const FPubnubOperationResult& Result)
	{
		*bAddChannelToGroupDone = true;
		if (!Result.Error && Result.Status == 200)
		{
			*bAddChannelToGroupSuccess = true;
		}
		else
		{
			AddError(FString::Printf(TEXT("AddChannelToGroup failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
		}
	});

	FOnSubscribeOperationResponseNative SubscribeToGroupCallback;
	SubscribeToGroupCallback.BindLambda([this, bSubscribeToGroupDone](const FPubnubOperationResult& Result)
	{
		*bSubscribeToGroupDone = true;
		TestFalse("SubscribeToGroup operation should not have failed", Result.Error);
		TestEqual("SubscribeToGroup HTTP status should be 200", Result.Status, 200);
		
		if (Result.Error)
		{
			AddError(FString::Printf(TEXT("SubscribeToGroup failed with error: %s"), *Result.ErrorMessage));
		}
	});

	FOnPublishMessageResponseNative PublishCallback;
	PublishCallback.BindLambda([this, bPublishDone, bPublishSuccess](const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
	{
		*bPublishDone = true;
		if (!Result.Error && Result.Status == 200)
		{
			*bPublishSuccess = true;
		}
		else
		{
			AddError(FString::Printf(TEXT("PublishMessage failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
		}
	});

	FOnSubscribeOperationResponseNative UnsubscribeFromGroupCallback;
	UnsubscribeFromGroupCallback.BindLambda([this, bUnsubscribeFromGroupDone](const FPubnubOperationResult& Result)
	{
		*bUnsubscribeFromGroupDone = true;
		TestFalse("UnsubscribeFromGroup operation should not have failed", Result.Error);
		TestEqual("UnsubscribeFromGroup HTTP status should be 200", Result.Status, 200);
		
		if (Result.Error)
		{
			AddError(FString::Printf(TEXT("UnsubscribeFromGroup failed with error: %s"), *Result.ErrorMessage));
		}
	});

	FOnRemoveChannelFromGroupResponseNative RemoveChannelCallback;
	RemoveChannelCallback.BindLambda([this, bRemoveChannelFromGroupDone](const FPubnubOperationResult& Result)
	{
		*bRemoveChannelFromGroupDone = true;
		if (Result.Error || Result.Status != 200)
		{
			AddError(FString::Printf(TEXT("RemoveChannelFromGroup failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
		}
	});

	// Step 1: Add channel to group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestGroup, AddChannelCallback, bAddChannelToGroupDone, bAddChannelToGroupSuccess]()
	{
		*bAddChannelToGroupDone = false;
		*bAddChannelToGroupSuccess = false;
		PubnubSubsystem->AddChannelToGroup(TestChannel, TestGroup, AddChannelCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bAddChannelToGroupDone]() { return *bAddChannelToGroupDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bAddChannelToGroupSuccess]()
	{
		TestTrue("AddChannelToGroup should succeed", *bAddChannelToGroupSuccess);
	}, 0.1f));

	// Step 2: Subscribe to the group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, SubscribeToGroupCallback, bSubscribeToGroupDone]()
	{
		*bSubscribeToGroupDone = false;
		PubnubSubsystem->SubscribeToGroup(TestGroup, SubscribeToGroupCallback);
	}, 0.1f));

	//Wait until subscribe to group result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeToGroupDone]() -> bool {
		return *bSubscribeToGroupDone;
	}, MAX_WAIT_TIME));

	//Check whether subscribe to group result was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSubscribeToGroupDone]()
	{
		if(!*bSubscribeToGroupDone)
		{
			AddError("SubscribeToGroup result callback was not received");
		}
	}, 0.1f));

	// Step 3: Publish a message to the channel (which is in the subscribed group)
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage, PublishCallback, bPublishDone, bPublishSuccess]()
	{
		*bPublishDone = false;
		*bPublishSuccess = false;
		PubnubSubsystem->PublishMessage(TestChannel, TestMessage, PublishCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bPublishDone]() { return *bPublishDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bPublishSuccess]()
	{
		TestTrue("PublishMessage should succeed", *bPublishSuccess);
	}, 0.1f));

	// Step 4: Wait until message is received or timeout is reached
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestMessageReceived]() -> bool {
		return *TestMessageReceived;
	}, MAX_WAIT_TIME));

	// Step 5: Check whether message was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestMessageReceived]()
	{
		TestTrue("Message should be received on channel within a group", *TestMessageReceived);
	}, 0.1f));

	// Cleanup: Unsubscribe from group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, UnsubscribeFromGroupCallback, bUnsubscribeFromGroupDone]()
	{
		*bUnsubscribeFromGroupDone = false;
		PubnubSubsystem->UnsubscribeFromGroup(TestGroup, UnsubscribeFromGroupCallback);
	}, 0.1f));

	//Wait until unsubscribe from group result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bUnsubscribeFromGroupDone]() -> bool {
		return *bUnsubscribeFromGroupDone;
	}, MAX_WAIT_TIME));

	//Check whether unsubscribe from group result was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bUnsubscribeFromGroupDone]()
	{
		if(!*bUnsubscribeFromGroupDone)
		{
			AddError("UnsubscribeFromGroup result callback was not received");
		}
	}, 0.1f));
	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestGroup, RemoveChannelCallback, bRemoveChannelFromGroupDone]()
	{
		*bRemoveChannelFromGroupDone = false;
		PubnubSubsystem->RemoveChannelFromGroup(TestChannel, TestGroup, RemoveChannelCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveChannelFromGroupDone]() { return *bRemoveChannelFromGroupDone; }, MAX_WAIT_TIME));

	CleanUp();
	return true;
}

bool FPubnubChannelAddRemoveListGroupTest::RunTest(const FString& Parameters)
{
	//Initial variables
	const FString TestUser = SDK_PREFIX + "test_user_cg_mgmt";
	const FString TestGroup = SDK_PREFIX + "test_group_mgmt";
	const FString TestChannel1 = SDK_PREFIX + "cg_ch1_mgmt";
	const FString TestChannel2 = SDK_PREFIX + "cg_ch2_mgmt";

	TSharedPtr<bool> bListOperationCompleted = MakeShared<bool>(false);
	TSharedPtr<TArray<FString>> ListedChannels = MakeShared<TArray<FString>>();
	TSharedPtr<bool> bListOperationSuccess = MakeShared<bool>(false);

	// Shared state for operation callbacks
	TSharedPtr<bool> bAddChannelDone = MakeShared<bool>(false);
	TSharedPtr<bool> bAddChannelSuccess = MakeShared<bool>(false);
	TSharedPtr<bool> bRemoveChannelDone = MakeShared<bool>(false);
	TSharedPtr<bool> bRemoveChannelSuccess = MakeShared<bool>(false);

	if(!InitTest())
	{
		AddError("TestInitialization failed for FPubnubChannelGroupManagementTest");
		return false;
	}

	//Check for any errors on the way
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		// Only log general errors not specific to list operations handled by its own callback
		AddError(FString::Printf(TEXT("General Pubnub Error in FPubnubChannelGroupManagementTest: %s, Type: %d"), *ErrorMessage, ErrorType));
	});
	
	//Set User ID
	PubnubSubsystem->SetUserID(TestUser);
	
	FOnListChannelsFromGroupResponseNative ListDelegate;
	ListDelegate.BindLambda([this, bListOperationCompleted, ListedChannels, bListOperationSuccess](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		*bListOperationCompleted = true;
		if (!Result.Error)
		{
			*ListedChannels = Channels;
			*bListOperationSuccess = true;
		}
		else
		{
			*bListOperationSuccess = false;
			AddError(FString::Printf(TEXT("ListChannelsInGroup for group failed. Status: %d"), Result.Status));
		}
	});

	FOnAddChannelToGroupResponseNative AddChannelCallback;
	AddChannelCallback.BindLambda([this, bAddChannelDone, bAddChannelSuccess](const FPubnubOperationResult& Result)
	{
		*bAddChannelDone = true;
		if (!Result.Error && Result.Status == 200)
		{
			*bAddChannelSuccess = true;
		}
		else
		{
			*bAddChannelSuccess = false;
			AddError(FString::Printf(TEXT("AddChannelToGroup failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
		}
	});

	FOnRemoveChannelFromGroupResponseNative RemoveChannelCallback;
	RemoveChannelCallback.BindLambda([this, bRemoveChannelDone, bRemoveChannelSuccess](const FPubnubOperationResult& Result)
	{
		*bRemoveChannelDone = true;
		if (!Result.Error && Result.Status == 200)
		{
			*bRemoveChannelSuccess = true;
		}
		else
		{
			*bRemoveChannelSuccess = false;
			AddError(FString::Printf(TEXT("RemoveChannelFromGroup failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
		}
	});

	// Ensure group is empty to start (clean up from previous tests if necessary)
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, TestChannel1, TestChannel2, RemoveChannelCallback, bRemoveChannelDone]()
	{
		*bRemoveChannelDone = false;
		PubnubSubsystem->RemoveChannelFromGroup(TestChannel1, TestGroup, RemoveChannelCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveChannelDone]() { return *bRemoveChannelDone; }, MAX_WAIT_TIME));
	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, TestChannel2, RemoveChannelCallback, bRemoveChannelDone]()
	{
		*bRemoveChannelDone = false;
		PubnubSubsystem->RemoveChannelFromGroup(TestChannel2, TestGroup, RemoveChannelCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveChannelDone]() { return *bRemoveChannelDone; }, MAX_WAIT_TIME));

	// 1. Add Channel1 to Group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1, TestGroup, AddChannelCallback, bAddChannelDone, bAddChannelSuccess]()
	{
		*bAddChannelDone = false;
		*bAddChannelSuccess = false;
		PubnubSubsystem->AddChannelToGroup(TestChannel1, TestGroup, AddChannelCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bAddChannelDone]() { return *bAddChannelDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bAddChannelSuccess]()
	{
		TestTrue("AddChannelToGroup (Channel1) should succeed", *bAddChannelSuccess);
	}, 0.1f));

	// 2. List and Verify Channel1
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, ListDelegate, bListOperationCompleted, bListOperationSuccess, ListedChannels]()
	{
		*bListOperationCompleted = false;
		*bListOperationSuccess = false;
		ListedChannels->Empty();
		PubnubSubsystem->ListChannelsFromGroup(TestGroup, ListDelegate);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListOperationCompleted]() { return *bListOperationCompleted; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1, ListedChannels, bListOperationSuccess]()
	{
		TestTrue("ListChannelsInGroup operation was marked successful (1)", *bListOperationSuccess);
		if(*bListOperationSuccess)
		{
			TestEqual("Number of channels after adding Channel1", ListedChannels->Num(), 1);
			TestTrue(FString::Printf(TEXT("Channel1 ('%s') found in group"), *TestChannel1), ListedChannels->Contains(TestChannel1));
		}
	}, 0.1f));

	// 3. Add Channel2 to Group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel2, TestGroup, AddChannelCallback, bAddChannelDone, bAddChannelSuccess]()
	{
		*bAddChannelDone = false;
		*bAddChannelSuccess = false;
		PubnubSubsystem->AddChannelToGroup(TestChannel2, TestGroup, AddChannelCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bAddChannelDone]() { return *bAddChannelDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bAddChannelSuccess]()
	{
		TestTrue("AddChannelToGroup (Channel2) should succeed", *bAddChannelSuccess);
	}, 0.1f));

	// 4. List and Verify Channel1 and Channel2
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, ListDelegate, bListOperationCompleted, bListOperationSuccess, ListedChannels]()
	{
		*bListOperationCompleted = false;
		*bListOperationSuccess = false;
		ListedChannels->Empty();
		PubnubSubsystem->ListChannelsFromGroup(TestGroup, ListDelegate);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListOperationCompleted]() { return *bListOperationCompleted; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1, TestChannel2, ListedChannels, bListOperationSuccess]()
	{
		TestTrue("ListChannelsInGroup operation was marked successful (2)", *bListOperationSuccess);
		if(*bListOperationSuccess)
		{
			TestEqual("Number of channels after adding Channel2", ListedChannels->Num(), 2);
			TestTrue(FString::Printf(TEXT("Channel1 ('%s') found in group"), *TestChannel1), ListedChannels->Contains(TestChannel1));
			TestTrue(FString::Printf(TEXT("Channel2 ('%s') found in group"), *TestChannel2), ListedChannels->Contains(TestChannel2));
		}
	}, 0.1f));

	// 5. Remove Channel1 from Group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1, TestGroup, RemoveChannelCallback, bRemoveChannelDone, bRemoveChannelSuccess]()
	{
		*bRemoveChannelDone = false;
		*bRemoveChannelSuccess = false;
		PubnubSubsystem->RemoveChannelFromGroup(TestChannel1, TestGroup, RemoveChannelCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveChannelDone]() { return *bRemoveChannelDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bRemoveChannelSuccess]()
	{
		TestTrue("RemoveChannelFromGroup (Channel1) should succeed", *bRemoveChannelSuccess);
	}, 0.1f));

	// 6. List and Verify Only Channel2 Remains
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, ListDelegate, bListOperationCompleted, bListOperationSuccess, ListedChannels]()
	{
		*bListOperationCompleted = false;
		*bListOperationSuccess = false;
		ListedChannels->Empty();
		PubnubSubsystem->ListChannelsFromGroup(TestGroup, ListDelegate);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListOperationCompleted]() { return *bListOperationCompleted; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1, TestChannel2, ListedChannels, bListOperationSuccess]()
	{
		TestTrue("ListChannelsInGroup operation was marked successful (3)", *bListOperationSuccess);
		if(*bListOperationSuccess)
		{
			TestEqual("Number of channels after removing Channel1", ListedChannels->Num(), 1);
			TestFalse(FString::Printf(TEXT("Channel1 ('%s') NOT found in group"), *TestChannel1), ListedChannels->Contains(TestChannel1));
			TestTrue(FString::Printf(TEXT("Channel2 ('%s') still in group"), *TestChannel2), ListedChannels->Contains(TestChannel2));
		}
	}, 0.1f));

	// 7. Remove Channel2 from Group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel2, TestGroup, RemoveChannelCallback, bRemoveChannelDone, bRemoveChannelSuccess]()
	{
		*bRemoveChannelDone = false;
		*bRemoveChannelSuccess = false;
		PubnubSubsystem->RemoveChannelFromGroup(TestChannel2, TestGroup, RemoveChannelCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveChannelDone]() { return *bRemoveChannelDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bRemoveChannelSuccess]()
	{
		TestTrue("RemoveChannelFromGroup (Channel2) should succeed", *bRemoveChannelSuccess);
	}, 0.1f));

	// 8. List and Verify Group is Empty
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, ListDelegate, bListOperationCompleted, bListOperationSuccess, ListedChannels]()
	{
		*bListOperationCompleted = false;
		*bListOperationSuccess = false;
		ListedChannels->Empty();
		PubnubSubsystem->ListChannelsFromGroup(TestGroup, ListDelegate);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListOperationCompleted]() { return *bListOperationCompleted; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ListedChannels, bListOperationSuccess]()
	{
		TestTrue("ListChannelsInGroup operation was marked successful (4)", *bListOperationSuccess);
		if(*bListOperationSuccess)
		{
			TestEqual("Number of channels after removing Channel2 (group should be empty)", ListedChannels->Num(), 0);
		}
	}, 0.1f));
	
	CleanUp();
	return true;
}

bool FPubnubUnsubscribeFromGroupTest::RunTest(const FString& Parameters)
{
	//Initial variables
	const FString TestMessage = "\"Message for group unsubscribe test\"";
	const FString TestUser = SDK_PREFIX + "test_user_group_unsubscribe";
	const FString TestChannel = SDK_PREFIX + "test_channel_for_group_unsubscribe";
	const FString TestGroup = SDK_PREFIX + "test_group_unsubscribe";
	TSharedPtr<bool> TestMessageReceivedAfterUnsubscribe = MakeShared<bool>(false);

	// Shared state for operation callbacks
	TSharedPtr<bool> bAddChannelToGroupDone = MakeShared<bool>(false);
	TSharedPtr<bool> bAddChannelToGroupSuccess = MakeShared<bool>(false);
	TSharedPtr<bool> bPublishDone = MakeShared<bool>(false);
	TSharedPtr<bool> bPublishSuccess = MakeShared<bool>(false);
	TSharedPtr<bool> bRemoveChannelFromGroupDone = MakeShared<bool>(false);
	TSharedPtr<bool> bSubscribeToGroupDone = MakeShared<bool>(false);
	TSharedPtr<bool> bUnsubscribeFromGroupDone = MakeShared<bool>(false);

	if (!InitTest())
	{
		AddError("TestInitialization failed for FPubnubUnsubscribeFromGroupTest");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(FString::Printf(TEXT("Unexpected Pubnub Error in FPubnubUnsubscribeFromGroupTest: %s"), *ErrorMessage));
	});

	PubnubSubsystem->SetUserID(TestUser);

	PubnubSubsystem->OnMessageReceivedNative.AddLambda([this, TestMessage, TestChannel, TestGroup, TestMessageReceivedAfterUnsubscribe](FPubnubMessageData ReceivedMessage)
	{
		if (ReceivedMessage.Channel == TestChannel && ReceivedMessage.Message == TestMessage)
		{
			*TestMessageReceivedAfterUnsubscribe = true;
			AddError(FString::Printf(TEXT("Message '%s' received on channel '%s' via group AFTER unsubscribing from group '%s'."), *TestMessage, *TestChannel, *TestGroup));
		}
	});

	// Define callbacks
	FOnAddChannelToGroupResponseNative AddChannelCallback;
	AddChannelCallback.BindLambda([this, bAddChannelToGroupDone, bAddChannelToGroupSuccess](const FPubnubOperationResult& Result)
	{
		*bAddChannelToGroupDone = true;
		if (!Result.Error && Result.Status == 200)
		{
			*bAddChannelToGroupSuccess = true;
		}
		else
		{
			AddError(FString::Printf(TEXT("AddChannelToGroup failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
		}
	});

	FOnSubscribeOperationResponseNative SubscribeToGroupCallback;
	SubscribeToGroupCallback.BindLambda([this, bSubscribeToGroupDone](const FPubnubOperationResult& Result)
	{
		*bSubscribeToGroupDone = true;
		TestFalse("SubscribeToGroup operation should not have failed", Result.Error);
		TestEqual("SubscribeToGroup HTTP status should be 200", Result.Status, 200);
		
		if (Result.Error)
		{
			AddError(FString::Printf(TEXT("SubscribeToGroup failed with error: %s"), *Result.ErrorMessage));
		}
	});

	FOnSubscribeOperationResponseNative UnsubscribeFromGroupCallback;
	UnsubscribeFromGroupCallback.BindLambda([this, bUnsubscribeFromGroupDone](const FPubnubOperationResult& Result)
	{
		*bUnsubscribeFromGroupDone = true;
		TestFalse("UnsubscribeFromGroup operation should not have failed", Result.Error);
		TestEqual("UnsubscribeFromGroup HTTP status should be 200", Result.Status, 200);
		
		if (Result.Error)
		{
			AddError(FString::Printf(TEXT("UnsubscribeFromGroup failed with error: %s"), *Result.ErrorMessage));
		}
	});

	FOnPublishMessageResponseNative PublishCallback;
	PublishCallback.BindLambda([this, bPublishDone, bPublishSuccess](const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
	{
		*bPublishDone = true;
		if (!Result.Error && Result.Status == 200)
		{
			*bPublishSuccess = true;
		}
		else
		{
			AddError(FString::Printf(TEXT("PublishMessage failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
		}
	});

	FOnRemoveChannelFromGroupResponseNative RemoveChannelCallback;
	RemoveChannelCallback.BindLambda([this, bRemoveChannelFromGroupDone](const FPubnubOperationResult& Result)
	{
		*bRemoveChannelFromGroupDone = true;
		if (Result.Error || Result.Status != 200)
		{
			AddError(FString::Printf(TEXT("RemoveChannelFromGroup failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
		}
	});

	// Step 1: Add channel to group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestGroup, AddChannelCallback, bAddChannelToGroupDone, bAddChannelToGroupSuccess]()
	{
		*bAddChannelToGroupDone = false;
		*bAddChannelToGroupSuccess = false;
		PubnubSubsystem->AddChannelToGroup(TestChannel, TestGroup, AddChannelCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bAddChannelToGroupDone]() { return *bAddChannelToGroupDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bAddChannelToGroupSuccess]()
	{
		TestTrue("AddChannelToGroup should succeed", *bAddChannelToGroupSuccess);
	}, 0.1f));

	// Step 2: Subscribe to the group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, SubscribeToGroupCallback, bSubscribeToGroupDone]()
	{
		*bSubscribeToGroupDone = false;
		PubnubSubsystem->SubscribeToGroup(TestGroup, SubscribeToGroupCallback);
	}, 0.1f));

	//Wait until subscribe to group result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeToGroupDone]() -> bool {
		return *bSubscribeToGroupDone;
	}, MAX_WAIT_TIME));

	//Check whether subscribe to group result was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSubscribeToGroupDone]()
	{
		if(!*bSubscribeToGroupDone)
		{
			AddError("SubscribeToGroup result callback was not received");
		}
	}, 0.1f));

	// Step 3: Unsubscribe from the group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, UnsubscribeFromGroupCallback, bUnsubscribeFromGroupDone]()
	{
		*bUnsubscribeFromGroupDone = false;
		PubnubSubsystem->UnsubscribeFromGroup(TestGroup, UnsubscribeFromGroupCallback);
	}, 0.1f));

	//Wait until unsubscribe from group result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bUnsubscribeFromGroupDone]() -> bool {
		return *bUnsubscribeFromGroupDone;
	}, MAX_WAIT_TIME));

	//Check whether unsubscribe from group result was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bUnsubscribeFromGroupDone]()
	{
		if(!*bUnsubscribeFromGroupDone)
		{
			AddError("UnsubscribeFromGroup result callback was not received");
		}
	}, 0.1f));

	// Step 4: Publish a message to the channel (that was in the group)
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage, PublishCallback, bPublishDone, bPublishSuccess]()
	{
		*bPublishDone = false;
		*bPublishSuccess = false;
		PubnubSubsystem->PublishMessage(TestChannel, TestMessage, PublishCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bPublishDone]() { return *bPublishDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bPublishSuccess]()
	{
		TestTrue("PublishMessage should succeed", *bPublishSuccess);
	}, 0.1f));

	// Step 5: Wait for a period to see if the message is incorrectly received
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(MAX_WAIT_TIME / 2.0f));

	// Step 6: Check that the message was NOT received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestMessageReceivedAfterUnsubscribe, TestGroup]()
	{
		TestFalse(FString::Printf(TEXT("Message should NOT have been received via group '%s' after unsubscribing."), *TestGroup), *TestMessageReceivedAfterUnsubscribe);
	}, 0.1f));

	// Cleanup: Remove channel from group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestGroup, RemoveChannelCallback, bRemoveChannelFromGroupDone]()
	{
		*bRemoveChannelFromGroupDone = false;
		PubnubSubsystem->RemoveChannelFromGroup(TestChannel, TestGroup, RemoveChannelCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveChannelFromGroupDone]() { return *bRemoveChannelFromGroupDone; }, MAX_WAIT_TIME));

	CleanUp();
	return true;
}

bool FPubnubUnsubscribeFromAllTest::RunTest(const FString& Parameters)
{
	//Initial variables
	const FString TestUser = SDK_PREFIX + "test_user_unsubscribe_all";
	const FString TestMessageForChannel = "\"Message for direct channel (unsubscribe all test)\"";
	const FString TestMessageForGroupChannel = "\"Message for group channel (unsubscribe all test)\"";
	
	const FString TestChannelForAll = SDK_PREFIX + "channel_direct_unsubscribe_all";
	const FString TestGroupForAll = SDK_PREFIX + "group_unsubscribe_all";
	const FString TestChannelInGroupForAll = SDK_PREFIX + "channel_in_group_unsubscribe_all";

	TSharedPtr<bool> bMessageReceivedOnDirectChannel = MakeShared<bool>(false);
	TSharedPtr<bool> bMessageReceivedViaGroup = MakeShared<bool>(false);

	// Shared state for operation callbacks
	TSharedPtr<bool> bAddChannelToGroupDone = MakeShared<bool>(false);
	TSharedPtr<bool> bAddChannelToGroupSuccess = MakeShared<bool>(false);
	TSharedPtr<bool> bPublish1Done = MakeShared<bool>(false);
	TSharedPtr<bool> bPublish1Success = MakeShared<bool>(false);
	TSharedPtr<bool> bPublish2Done = MakeShared<bool>(false);
	TSharedPtr<bool> bPublish2Success = MakeShared<bool>(false);
	TSharedPtr<bool> bRemoveChannelFromGroupDone = MakeShared<bool>(false);
	TSharedPtr<bool> bSubscribeToChannelDone = MakeShared<bool>(false);
	TSharedPtr<bool> bSubscribeToGroupDone = MakeShared<bool>(false);
	TSharedPtr<bool> bUnsubscribeFromAllDone = MakeShared<bool>(false);


	if (!InitTest())
	{
		AddError("TestInitialization failed for FPubnubUnsubscribeFromAllTest");
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(FString::Printf(TEXT("Unexpected Pubnub Error in FPubnubUnsubscribeFromAllTest: %s"), *ErrorMessage));
	});

	PubnubSubsystem->SetUserID(TestUser);

	PubnubSubsystem->OnMessageReceivedNative.AddLambda([this, TestMessageForChannel, TestChannelForAll, TestMessageForGroupChannel, TestChannelInGroupForAll, bMessageReceivedOnDirectChannel, bMessageReceivedViaGroup](FPubnubMessageData ReceivedMessage)
	{
		if (ReceivedMessage.Channel == TestChannelForAll && ReceivedMessage.Message == TestMessageForChannel)
		{
			*bMessageReceivedOnDirectChannel = true;
			AddError(FString::Printf(TEXT("Message '%s' received on direct channel '%s' AFTER UnsubscribeFromAll."), *TestMessageForChannel, *TestChannelForAll));
		}
		else if (ReceivedMessage.Channel == TestChannelInGroupForAll && ReceivedMessage.Message == TestMessageForGroupChannel)
		{
			*bMessageReceivedViaGroup = true;
			AddError(FString::Printf(TEXT("Message '%s' received on channel '%s' (via group) AFTER UnsubscribeFromAll."), *TestMessageForGroupChannel, *TestChannelInGroupForAll));
		}
	});

	// Define callbacks
	FOnAddChannelToGroupResponseNative AddChannelCallback;
	AddChannelCallback.BindLambda([this, bAddChannelToGroupDone, bAddChannelToGroupSuccess](const FPubnubOperationResult& Result)
	{
		*bAddChannelToGroupDone = true;
		if (!Result.Error && Result.Status == 200)
		{
			*bAddChannelToGroupSuccess = true;
		}
		else
		{
			AddError(FString::Printf(TEXT("AddChannelToGroup failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
		}
	});

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

	FOnSubscribeOperationResponseNative SubscribeToGroupCallback;
	SubscribeToGroupCallback.BindLambda([this, bSubscribeToGroupDone](const FPubnubOperationResult& Result)
	{
		*bSubscribeToGroupDone = true;
		TestFalse("SubscribeToGroup operation should not have failed", Result.Error);
		TestEqual("SubscribeToGroup HTTP status should be 200", Result.Status, 200);
		
		if (Result.Error)
		{
			AddError(FString::Printf(TEXT("SubscribeToGroup failed with error: %s"), *Result.ErrorMessage));
		}
	});

	FOnSubscribeOperationResponseNative UnsubscribeFromAllCallback;
	UnsubscribeFromAllCallback.BindLambda([this, bUnsubscribeFromAllDone](const FPubnubOperationResult& Result)
	{
		*bUnsubscribeFromAllDone = true;
		TestFalse("UnsubscribeFromAll operation should not have failed", Result.Error);
		TestEqual("UnsubscribeFromAll HTTP status should be 200", Result.Status, 200);
		
		if (Result.Error)
		{
			AddError(FString::Printf(TEXT("UnsubscribeFromAll failed with error: %s"), *Result.ErrorMessage));
		}
	});

	FOnPublishMessageResponseNative PublishCallback1;
	PublishCallback1.BindLambda([this, bPublish1Done, bPublish1Success](const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
	{
		*bPublish1Done = true;
		if (!Result.Error && Result.Status == 200)
		{
			*bPublish1Success = true;
		}
		else
		{
			AddError(FString::Printf(TEXT("PublishMessage 1 failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
		}
	});

	FOnPublishMessageResponseNative PublishCallback2;
	PublishCallback2.BindLambda([this, bPublish2Done, bPublish2Success](const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
	{
		*bPublish2Done = true;
		if (!Result.Error && Result.Status == 200)
		{
			*bPublish2Success = true;
		}
		else
		{
			AddError(FString::Printf(TEXT("PublishMessage 2 failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
		}
	});

	FOnRemoveChannelFromGroupResponseNative RemoveChannelCallback;
	RemoveChannelCallback.BindLambda([this, bRemoveChannelFromGroupDone](const FPubnubOperationResult& Result)
	{
		*bRemoveChannelFromGroupDone = true;
		if (Result.Error || Result.Status != 200)
		{
			AddError(FString::Printf(TEXT("RemoveChannelFromGroup failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
		}
	});
	
	// Step 1: Add a channel to the group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelInGroupForAll, TestGroupForAll, AddChannelCallback, bAddChannelToGroupDone, bAddChannelToGroupSuccess]()
	{
		*bAddChannelToGroupDone = false;
		*bAddChannelToGroupSuccess = false;
		PubnubSubsystem->AddChannelToGroup(TestChannelInGroupForAll, TestGroupForAll, AddChannelCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bAddChannelToGroupDone]() { return *bAddChannelToGroupDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bAddChannelToGroupSuccess]()
	{
		TestTrue("AddChannelToGroup should succeed", *bAddChannelToGroupSuccess);
	}, 0.1f));

	// Step 2: Subscribe to the channel
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelForAll, SubscribeToChannelCallback, bSubscribeToChannelDone]()
	{
		*bSubscribeToChannelDone = false;
		PubnubSubsystem->SubscribeToChannel(TestChannelForAll, SubscribeToChannelCallback);
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

	// Step 3: Subscribe to the group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroupForAll, SubscribeToGroupCallback, bSubscribeToGroupDone]()
	{
		*bSubscribeToGroupDone = false;
		PubnubSubsystem->SubscribeToGroup(TestGroupForAll, SubscribeToGroupCallback);
	}, 0.1f));

	//Wait until subscribe to group result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribeToGroupDone]() -> bool {
		return *bSubscribeToGroupDone;
	}, MAX_WAIT_TIME));

	//Check whether subscribe to group result was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSubscribeToGroupDone]()
	{
		if(!*bSubscribeToGroupDone)
		{
			AddError("SubscribeToGroup result callback was not received");
		}
	}, 0.1f));

	// Step 4: Unsubscribe from all
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, UnsubscribeFromAllCallback, bUnsubscribeFromAllDone]()
	{
		*bUnsubscribeFromAllDone = false;
		PubnubSubsystem->UnsubscribeFromAll(UnsubscribeFromAllCallback);
	}, 0.1f));

	//Wait until unsubscribe from all result is received
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bUnsubscribeFromAllDone]() -> bool {
		return *bUnsubscribeFromAllDone;
	}, MAX_WAIT_TIME));

	//Check whether unsubscribe from all result was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bUnsubscribeFromAllDone]()
	{
		if(!*bUnsubscribeFromAllDone)
		{
			AddError("UnsubscribeFromAll result callback was not received");
		}
	}, 0.1f));

	// Step 5: Publish a message to the direct channel
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelForAll, TestMessageForChannel, PublishCallback1, bPublish1Done, bPublish1Success]()
	{
		*bPublish1Done = false;
		*bPublish1Success = false;
		PubnubSubsystem->PublishMessage(TestChannelForAll, TestMessageForChannel, PublishCallback1);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bPublish1Done]() { return *bPublish1Done; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bPublish1Success]()
	{
		TestTrue("PublishMessage 1 should succeed", *bPublish1Success);
	}, 0.1f));

	// Step 6: Publish a message to the channel that was in the group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelInGroupForAll, TestMessageForGroupChannel, PublishCallback2, bPublish2Done, bPublish2Success]()
	{
		*bPublish2Done = false;
		*bPublish2Success = false;
		PubnubSubsystem->PublishMessage(TestChannelInGroupForAll, TestMessageForGroupChannel, PublishCallback2);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bPublish2Done]() { return *bPublish2Done; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bPublish2Success]()
	{
		TestTrue("PublishMessage 2 should succeed", *bPublish2Success);
	}, 0.1f));

	// Step 7: Wait for a period to see if messages are incorrectly received
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(MAX_WAIT_TIME / 2.0f));

	// Step 8: Check that messages were NOT received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bMessageReceivedOnDirectChannel, TestChannelForAll, bMessageReceivedViaGroup, TestGroupForAll]()
	{
		TestFalse(FString::Printf(TEXT("Message should NOT have been received on direct channel '%s' after UnsubscribeFromAll."), *TestChannelForAll), *bMessageReceivedOnDirectChannel);
		TestFalse(FString::Printf(TEXT("Message should NOT have been received via group '%s' after UnsubscribeFromAll."), *TestGroupForAll), *bMessageReceivedViaGroup);
	}, 0.1f));

	// Cleanup: Remove channel from group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelInGroupForAll, TestGroupForAll, RemoveChannelCallback, bRemoveChannelFromGroupDone]()
	{
		*bRemoveChannelFromGroupDone = false;
		PubnubSubsystem->RemoveChannelFromGroup(TestChannelInGroupForAll, TestGroupForAll, RemoveChannelCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveChannelFromGroupDone]() { return *bRemoveChannelFromGroupDone; }, MAX_WAIT_TIME));

	CleanUp();
	return true;
}

bool FPubnubRemoveGroupTest::RunTest(const FString& Parameters)
{
	// Initial variables
	const FString TestUser = SDK_PREFIX + "test_user_remove_group";
	const FString TestGroup = SDK_PREFIX + "test_group_to_remove";
	const FString TestChannel1 = SDK_PREFIX + "rg_ch1";
	const FString TestChannel2 = SDK_PREFIX + "rg_ch2";

	TSharedPtr<bool> bListOperationCompleted = MakeShared<bool>(false);
	TSharedPtr<TArray<FString>> ListedChannels = MakeShared<TArray<FString>>();
	TSharedPtr<bool> bListOperationSuccess = MakeShared<bool>(false);
	TSharedPtr<int> ListOperationStatus = MakeShared<int>(0);

	// Shared state for operation callbacks
	TSharedPtr<bool> bAddChannel1Done = MakeShared<bool>(false);
	TSharedPtr<bool> bAddChannel1Success = MakeShared<bool>(false);
	TSharedPtr<bool> bAddChannel2Done = MakeShared<bool>(false);
	TSharedPtr<bool> bAddChannel2Success = MakeShared<bool>(false);
	TSharedPtr<bool> bRemoveGroupDone = MakeShared<bool>(false);
	TSharedPtr<bool> bRemoveGroupSuccess = MakeShared<bool>(false);

	if(!InitTest())
	{
		AddError("TestInitialization failed for FPubnubRemoveGroupTest");
		return false;
	}
	
	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(FString::Printf(TEXT("General Pubnub Error in FPubnubRemoveGroupTest: %s, Type: %d"), *ErrorMessage, ErrorType));
	});
	
	PubnubSubsystem->SetUserID(TestUser);
	
	FOnListChannelsFromGroupResponseNative ListDelegate;
	ListDelegate.BindLambda([this, bListOperationCompleted, ListedChannels, bListOperationSuccess, ListOperationStatus](const FPubnubOperationResult& Result, const TArray<FString>& Channels)
	{
		*bListOperationCompleted = true;
		*ListOperationStatus = Result.Status;
		if (!Result.Error)
		{
			*ListedChannels = Channels;
			*bListOperationSuccess = true;
		}
		else
		{
			*bListOperationSuccess = false;
			// This can be expected after group removal, so not an immediate AddError here.
		}
	});

	FOnAddChannelToGroupResponseNative AddChannelCallback;
	AddChannelCallback.BindLambda([this, bAddChannel1Done, bAddChannel1Success, bAddChannel2Done, bAddChannel2Success](const FPubnubOperationResult& Result)
	{
		// This callback will be used for both channels, so we need to track which one completed
		// We'll use the done flags to determine which operation this is for
		if (!*bAddChannel1Done)
		{
			*bAddChannel1Done = true;
			if (!Result.Error && Result.Status == 200)
			{
				*bAddChannel1Success = true;
			}
			else
			{
				AddError(FString::Printf(TEXT("AddChannelToGroup (Channel1) failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
			}
		}
		else if (!*bAddChannel2Done)
		{
			*bAddChannel2Done = true;
			if (!Result.Error && Result.Status == 200)
			{
				*bAddChannel2Success = true;
			}
			else
			{
				AddError(FString::Printf(TEXT("AddChannelToGroup (Channel2) failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
			}
		}
	});

	FOnRemoveChannelGroupResponseNative RemoveGroupCallback;
	RemoveGroupCallback.BindLambda([this, bRemoveGroupDone, bRemoveGroupSuccess](const FPubnubOperationResult& Result)
	{
		*bRemoveGroupDone = true;
		if (!Result.Error && Result.Status == 200)
		{
			*bRemoveGroupSuccess = true;
		}
		else
		{
			AddError(FString::Printf(TEXT("RemoveChannelGroup failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
		}
	});

	// Pre-cleanup: Ensure the group doesn't exist from a previous failed run.
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, RemoveGroupCallback, bRemoveGroupDone]()
	{
		*bRemoveGroupDone = false;
		PubnubSubsystem->RemoveChannelGroup(TestGroup, RemoveGroupCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveGroupDone]() { return *bRemoveGroupDone; }, MAX_WAIT_TIME));

	// 1. Add Channel1 to Group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1, TestGroup, AddChannelCallback, bAddChannel1Done, bAddChannel1Success]()
	{
		*bAddChannel1Done = false;
		*bAddChannel1Success = false;
		PubnubSubsystem->AddChannelToGroup(TestChannel1, TestGroup, AddChannelCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bAddChannel1Done]() { return *bAddChannel1Done; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bAddChannel1Success]()
	{
		TestTrue("AddChannelToGroup (Channel1) should succeed", *bAddChannel1Success);
	}, 0.1f));

	// 2. Add Channel2 to Group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel2, TestGroup, AddChannelCallback, bAddChannel2Done, bAddChannel2Success]()
	{
		*bAddChannel2Done = false;
		*bAddChannel2Success = false;
		PubnubSubsystem->AddChannelToGroup(TestChannel2, TestGroup, AddChannelCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bAddChannel2Done]() { return *bAddChannel2Done; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bAddChannel2Success]()
	{
		TestTrue("AddChannelToGroup (Channel2) should succeed", *bAddChannel2Success);
	}, 0.1f));

	// 3. List and Verify Channels are in the Group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, ListDelegate, bListOperationCompleted, bListOperationSuccess, ListedChannels, ListOperationStatus]()
	{
		*bListOperationCompleted = false;
		*bListOperationSuccess = false;
		*ListOperationStatus = 0;
		ListedChannels->Empty();
		PubnubSubsystem->ListChannelsFromGroup(TestGroup, ListDelegate);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListOperationCompleted]() { return *bListOperationCompleted; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1, TestChannel2, ListedChannels, bListOperationSuccess, ListOperationStatus]()
	{
		TestTrue(FString::Printf(TEXT("ListChannelsFromGroup (before removal) operation should be successful. Status: %d"), *ListOperationStatus), *bListOperationSuccess);
		if(*bListOperationSuccess)
		{
			TestEqual("Number of channels before removal", ListedChannels->Num(), 2);
			TestTrue(FString::Printf(TEXT("Channel1 ('%s') found in group before removal"), *TestChannel1), ListedChannels->Contains(TestChannel1));
			TestTrue(FString::Printf(TEXT("Channel2 ('%s') found in group before removal"), *TestChannel2), ListedChannels->Contains(TestChannel2));
		}
	}, 0.1f));

	// 4. Remove the entire Group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, RemoveGroupCallback, bRemoveGroupDone, bRemoveGroupSuccess]()
	{
		*bRemoveGroupDone = false;
		*bRemoveGroupSuccess = false;
		PubnubSubsystem->RemoveChannelGroup(TestGroup, RemoveGroupCallback);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveGroupDone]() { return *bRemoveGroupDone; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bRemoveGroupSuccess]()
	{
		TestTrue("RemoveChannelGroup should succeed", *bRemoveGroupSuccess);
	}, 0.1f));

	// 5. Attempt to List Channels from the (now removed) Group and Verify
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, ListDelegate, bListOperationCompleted, bListOperationSuccess, ListedChannels, ListOperationStatus]()
	{
		*bListOperationCompleted = false;
		*bListOperationSuccess = true; 
		*ListOperationStatus = 0;
		ListedChannels->Empty();
		PubnubSubsystem->ListChannelsFromGroup(TestGroup, ListDelegate);
	}, 0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bListOperationCompleted]() { return *bListOperationCompleted; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ListedChannels, bListOperationSuccess, ListOperationStatus, TestGroup]()
	{
		bool bGroupSuccessfullyRemoved = false;
		if (!(*bListOperationSuccess)) // ListChannelsFromGroup indicated an error
		{
			bGroupSuccessfullyRemoved = true;
		}
		else // ListChannelsFromGroup call was "successful" (no transport error, etc.)
		{
			if (ListedChannels->Num() == 0)
			{
				bGroupSuccessfullyRemoved = true;
			}
		}

		TestTrue(FString::Printf(TEXT("Group '%s' should be removed. Verification: ListChannels (after remove) Error: %s, Status: %d, Listed Count: %d"),
			*TestGroup, !(*bListOperationSuccess) ? TEXT("true") : TEXT("false"), *ListOperationStatus, ListedChannels->Num()), bGroupSuccessfullyRemoved);

		if (!bGroupSuccessfullyRemoved)
		{
			AddError(FString::Printf(TEXT("Group '%s' NOT removed. List (after remove): Success=%s, Status=%d, Count=%d. Channels: %s"),
				*TestGroup, *bListOperationSuccess ? TEXT("true") : TEXT("false"), *ListOperationStatus, ListedChannels->Num(), *FString::Join(*ListedChannels, TEXT(","))
			));
		}
	}, 0.1f));
	
	CleanUp();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS