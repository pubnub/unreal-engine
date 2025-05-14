#include "Misc/AutomationTest.h"
#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubAddChannelToGroupAndReceiveMessageTest, FPubnubAutomationTestBase, "Pubnub.E2E.ChannelGroups.AddChannelToGroupAndReceiveMessage", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubChannelAddRemoveListGroupTest, FPubnubAutomationTestBase, "Pubnub.E2E.ChannelGroups.AddRemoveListChannelsInGroup", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FPubnubAddChannelToGroupAndReceiveMessageTest::RunTest(const FString& Parameters)
{
	//Initial variables
	const FString TestMessage = "\"Message for group test\"";
	const FString TestUser = SDK_PREFIX + "test_user_group";
	const FString TestChannel = SDK_PREFIX + "test_channel_for_group";
	const FString TestGroup = SDK_PREFIX + "test_group";
	TSharedPtr<bool> TestMessageReceived = MakeShared<bool>(false);
	
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

	// Step 1: Add channel to group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestGroup]()
	{
		PubnubSubsystem->AddChannelToGroup(TestChannel, TestGroup);
	}, 0.2f));
	

	// Step 2: Subscribe to the group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup]()
	{
		PubnubSubsystem->SubscribeToGroup(TestGroup);
	}, 0.5f));
	

	// Step 3: Publish a message to the channel (which is in the subscribed group)
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		PubnubSubsystem->PublishMessage(TestChannel, TestMessage);
	}, 1.f));

	// Step 4: Wait until message is received or timeout is reached
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([TestMessageReceived]() -> bool {
		return *TestMessageReceived;
	}, MAX_WAIT_TIME));

	// Step 5: Check whether message was received
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestMessageReceived]()
	{
		if(!*TestMessageReceived)
		{
			AddError("Message was not received on channel within a group in FPubnubAddChannelToGroupAndReceiveMessageTest.");
		}
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup]()
	{
		PubnubSubsystem->UnsubscribeFromGroup(TestGroup);
	}, 0.5f));
	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestGroup]()
	{
		PubnubSubsystem->RemoveChannelFromGroup(TestChannel, TestGroup);
	}, 0.5f));
	

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
	ListDelegate.BindLambda([this, bListOperationCompleted, ListedChannels, bListOperationSuccess](bool Error, int Status, const TArray<FString>& Channels)
	{
		*bListOperationCompleted = true;
		if (!Error)
		{
			*ListedChannels = Channels;
			*bListOperationSuccess = true;
		}
		else
		{
			*bListOperationSuccess = false;
			AddError(FString::Printf(TEXT("ListChannelsInGroup for group failed. Status: %d"), Status));
		}
	});

	// Ensure group is empty to start (clean up from previous tests if necessary)
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestGroup, TestChannel1, TestChannel2]()
	{
		PubnubSubsystem->RemoveChannelFromGroup(TestChannel1, TestGroup);
		PubnubSubsystem->RemoveChannelFromGroup(TestChannel2, TestGroup); 
	}, 0.1f));

	// 1. Add Channel1 to Group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1, TestGroup]()
	{
		PubnubSubsystem->AddChannelToGroup(TestChannel1, TestGroup);
	}, 0.2f));

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
		TestTrue(TEXT("ListChannelsInGroup operation was marked successful (1)"), *bListOperationSuccess);
		if(*bListOperationSuccess)
		{
			TestEqual(TEXT("Number of channels after adding Channel1"), ListedChannels->Num(), 1);
			TestTrue(FString::Printf(TEXT("Channel1 ('%s') found in group"), *TestChannel1), ListedChannels->Contains(TestChannel1));
		}
	}, 0.1f));

	// 3. Add Channel2 to Group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel2, TestGroup]()
	{
		PubnubSubsystem->AddChannelToGroup(TestChannel2, TestGroup);
	}, 0.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(0.5f));

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
		TestTrue(TEXT("ListChannelsInGroup operation was marked successful (2)"), *bListOperationSuccess);
		if(*bListOperationSuccess)
		{
			TestEqual(TEXT("Number of channels after adding Channel2"), ListedChannels->Num(), 2);
			TestTrue(FString::Printf(TEXT("Channel1 ('%s') found in group"), *TestChannel1), ListedChannels->Contains(TestChannel1));
			TestTrue(FString::Printf(TEXT("Channel2 ('%s') found in group"), *TestChannel2), ListedChannels->Contains(TestChannel2));
		}
	}, 0.1f));

	// 5. Remove Channel1 from Group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel1, TestGroup]()
	{
		PubnubSubsystem->RemoveChannelFromGroup(TestChannel1, TestGroup);
	}, 0.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(0.5f));

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
		TestTrue(TEXT("ListChannelsInGroup operation was marked successful (3)"), *bListOperationSuccess);
		if(*bListOperationSuccess)
		{
			TestEqual(TEXT("Number of channels after removing Channel1"), ListedChannels->Num(), 1);
			TestFalse(FString::Printf(TEXT("Channel1 ('%s') NOT found in group"), *TestChannel1), ListedChannels->Contains(TestChannel1));
			TestTrue(FString::Printf(TEXT("Channel2 ('%s') still in group"), *TestChannel2), ListedChannels->Contains(TestChannel2));
		}
	}, 0.1f));

	// 7. Remove Channel2 from Group
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel2, TestGroup]()
	{
		PubnubSubsystem->RemoveChannelFromGroup(TestChannel2, TestGroup);
	}, 0.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(0.5f));

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
		TestTrue(TEXT("ListChannelsInGroup operation was marked successful (4)"), *bListOperationSuccess);
		if(*bListOperationSuccess)
		{
			TestEqual(TEXT("Number of channels after removing Channel2 (group should be empty)"), ListedChannels->Num(), 0);
		}
	}, 0.1f));
	
	CleanUp();
	return true;
}


