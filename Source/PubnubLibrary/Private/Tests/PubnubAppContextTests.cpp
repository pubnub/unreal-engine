#include "Misc/AutomationTest.h"
#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "FunctionLibraries/PubnubTimetokenUtilities.h"
#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUserMetadataFlowTest, FPubnubAutomationTestBase, "Pubnub.E2E.AppContext.UserMetadataFlow", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubChannelMetadataFlowTest, FPubnubAutomationTestBase, "Pubnub.E2E.AppContext.ChannelMetadataFlow", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetAllChannelMetadataWithOptionsTest, FPubnubAutomationTestBase, "Pubnub.E2E.AppContext.GetAllChannelMetadataWithOptions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetAllUsersMetadataWithOptionsTest, FPubnubAutomationTestBase, "Pubnub.E2E.AppContext.GetAllUsersMetadataWithOptions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubMembershipManagementWithOptionsTest, FPubnubAutomationTestBase, "Pubnub.E2E.AppContext.MembershipManagementWithOptions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FPubnubUserMetadataFlowTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestUserID = SDK_PREFIX + "appctx_user_meta_test";
    const FString TestUserName = "E2E AppContext User";
    const FString TestUserEmail = "appcontext.user@pubnubsdk.test";
    const FString TestUserExternalID = SDK_PREFIX + "ext_id_123";
    const FString TestUserProfileUrl = "https://example.com/avatar.png";
    const FString TestUserStatus = "Online";
    const FString TestUserType = "PremiumUser";
    const FString TestUserCustomJson = TEXT("{\"mood\": \"elated\", \"points\": 1000}");
    
    const FString FullMetadataToSet = FString::Printf(TEXT("{")
        TEXT("\"name\":\"%s\",")
        TEXT("\"email\":\"%s\",")
        TEXT("\"externalId\":\"%s\",")
        TEXT("\"profileUrl\":\"%s\",")
        TEXT("\"status\":\"%s\",")
        TEXT("\"type\":\"%s\",")
        TEXT("\"custom\":%s") 
        TEXT("}"), 
        *TestUserName, 
        *TestUserEmail, 
        *TestUserExternalID, 
        *TestUserProfileUrl, 
        *TestUserStatus,
        *TestUserType,
        *TestUserCustomJson);

    TSharedPtr<bool> bGetUserMetaDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetUserMetaSuccess = MakeShared<bool>(false);
    TSharedPtr<FPubnubUserData> ReceivedUserData = MakeShared<FPubnubUserData>();

    TSharedPtr<bool> bGetAllUserMetaDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetAllUserMetaSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FPubnubUserData>> ReceivedAllUsersData = MakeShared<TArray<FPubnubUserData>>();
    
    TSharedPtr<bool> bGetUserMetaAfterRemoveDone = MakeShared<bool>(false);
    TSharedPtr<int> GetUserMetaAfterRemoveStatus = MakeShared<int>(0);


    // Callbacks
    FOnGetUserMetadataResponseNative GetUserMetadataCallback;
    GetUserMetadataCallback.BindLambda([this, bGetUserMetaDone, bGetUserMetaSuccess, ReceivedUserData](int Status, FPubnubUserData UserData)
    {
        *bGetUserMetaDone = true;
        *bGetUserMetaSuccess = (Status == 200);
        if (*bGetUserMetaSuccess)
        {
            *ReceivedUserData = UserData;
        }
        else
        {
            AddError(FString::Printf(TEXT("GetUserMetadata failed. Status: %d"), Status));
        }
    });

    FOnGetAllUserMetadataResponseNative GetAllUserMetadataCallback;
    GetAllUserMetadataCallback.BindLambda([this, bGetAllUserMetaDone, bGetAllUserMetaSuccess, ReceivedAllUsersData](int Status, const TArray<FPubnubUserData>& UsersData, FString PageNext, FString PagePrev)
    {
        *bGetAllUserMetaDone = true;
        *bGetAllUserMetaSuccess = (Status == 200);
        if (*bGetAllUserMetaSuccess)
        {
            *ReceivedAllUsersData = UsersData;
        }
        else
        {
            AddError(FString::Printf(TEXT("GetAllUserMetadata failed. Status: %d. Next: %s, Prev: %s"), Status, *PageNext, *PagePrev));
        }
    });
    
    FOnGetUserMetadataResponseNative GetUserMetadataCallback_AfterRemove;
    GetUserMetadataCallback_AfterRemove.BindLambda([this, bGetUserMetaAfterRemoveDone, GetUserMetaAfterRemoveStatus](int Status, FPubnubUserData UserData)
    {
        *bGetUserMetaAfterRemoveDone = true;
        *GetUserMetaAfterRemoveStatus = Status; // We expect this to be non-200 for a removed user
    });


    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubUserMetadataFlowTest");
        return false;
    }

    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this, bGetAllUserMetaDone](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("General Pubnub Error: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    // Set UserID
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID]()
	{
		PubnubSubsystem->SetUserID(TestUserID);
	}, 0.1f));

    // Step 1: SetUserMetadata
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, FullMetadataToSet]()
    {
        PubnubSubsystem->SetUserMetadata(TestUserID, FullMetadataToSet, TEXT("custom,externalId,profileUrl,status,type")); 
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f)); // Allow time for SetUserMetadata to process

    // Step 2: GetUserMetadata and Compare
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetUserMetadataCallback, bGetUserMetaDone, bGetUserMetaSuccess, ReceivedUserData]()
    {
        *bGetUserMetaDone = false;
        *bGetUserMetaSuccess = false;
        ReceivedUserData->UserID.Empty(); // Reset
        PubnubSubsystem->GetUserMetadata(TestUserID, GetUserMetadataCallback, TEXT("custom,externalId,profileUrl,status,type"));
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetUserMetaDone]() { return *bGetUserMetaDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, TestUserName, TestUserEmail, TestUserExternalID, TestUserProfileUrl, TestUserStatus, TestUserType, TestUserCustomJson, ReceivedUserData, bGetUserMetaSuccess]()
    {
        TestTrue("GetUserMetadata operation was successful after Set.", *bGetUserMetaSuccess);
        if (*bGetUserMetaSuccess)
        {
            TestEqual("GetUserMetadata: UserID match", ReceivedUserData->UserID, TestUserID);
            TestEqual("GetUserMetadata: UserName match", ReceivedUserData->UserName, TestUserName);
            TestEqual("GetUserMetadata: Email match", ReceivedUserData->Email, TestUserEmail);
            TestEqual("GetUserMetadata: ExternalID match", ReceivedUserData->ExternalID, TestUserExternalID);
            TestEqual("GetUserMetadata: ProfileUrl match", ReceivedUserData->ProfileUrl, TestUserProfileUrl);
            TestEqual("GetUserMetadata: Status match", ReceivedUserData->Status, TestUserStatus);
            TestEqual("GetUserMetadata: Type match", ReceivedUserData->Type, TestUserType);
            TestFalse("GetUserMetadata: Updated should not be empty", ReceivedUserData->Updated.IsEmpty());
            TestFalse("GetUserMetadata: ETag should not be empty", ReceivedUserData->ETag.IsEmpty());
            bool bCustomJsonMatch = UPubnubJsonUtilities::AreJsonObjectStringsEqual(ReceivedUserData->Custom, TestUserCustomJson);
            TestTrue("GetUserMetadata: Custom JSON match", bCustomJsonMatch);
            if(!bCustomJsonMatch)
            {
                AddError(FString::Printf(TEXT("Custom JSON mismatch. Expected: %s, Got: %s"), *TestUserCustomJson, *ReceivedUserData->Custom));
            }
        }
    }, 0.1f));

    // Step 3: GetAllUsersMetadata with filter
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetAllUserMetadataCallback, bGetAllUserMetaDone, bGetAllUserMetaSuccess, ReceivedAllUsersData]()
    {
        *bGetAllUserMetaDone = false;
        *bGetAllUserMetaSuccess = false;
        ReceivedAllUsersData->Empty();
        FPubnubGetAllInclude IncludeSettings;
        IncludeSettings.IncludeCustom = true;
        IncludeSettings.IncludeStatus = true; // Explicitly include if available in struct
        IncludeSettings.IncludeType = true;   // Explicitly include if available in struct
        FString Filter = FString::Printf(TEXT("id == '%s'"), *TestUserID);
        PubnubSubsystem->GetAllUserMetadata(GetAllUserMetadataCallback, IncludeSettings, 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllUserMetaDone]() { return *bGetAllUserMetaDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, TestUserName, TestUserEmail, TestUserExternalID, TestUserProfileUrl, TestUserStatus, TestUserType, TestUserCustomJson, ReceivedAllUsersData, bGetAllUserMetaSuccess]()
    {
        TestTrue("GetAllUserMetadata (filtered) operation was successful.", *bGetAllUserMetaSuccess);
        if (*bGetAllUserMetaSuccess)
        {
            TestEqual("GetAllUserMetadata (filtered): Expected 1 user.", ReceivedAllUsersData->Num(), 1);
            if (ReceivedAllUsersData->Num() == 1)
            {
                const FPubnubUserData& User = (*ReceivedAllUsersData)[0];
                TestEqual("GetAllUserMetadata (filtered): UserID match", User.UserID, TestUserID);
                TestEqual("GetAllUserMetadata (filtered): UserName match", User.UserName, TestUserName);
                TestEqual("GetAllUserMetadata (filtered): Email match", User.Email, TestUserEmail);
                TestEqual("GetAllUserMetadata (filtered): ExternalID match", User.ExternalID, TestUserExternalID);
                TestEqual("GetAllUserMetadata (filtered): ProfileUrl match", User.ProfileUrl, TestUserProfileUrl);
                TestEqual("GetAllUserMetadata (filtered): Status match", User.Status, TestUserStatus);
                TestEqual("GetAllUserMetadata (filtered): Type match", User.Type, TestUserType);
                TestFalse("GetAllUserMetadata (filtered): Updated should not be empty", User.Updated.IsEmpty());
                TestFalse("GetAllUserMetadata (filtered): ETag should not be empty", User.ETag.IsEmpty());
                bool bCustomJsonMatch = UPubnubJsonUtilities::AreJsonObjectStringsEqual(User.Custom, TestUserCustomJson);
                TestTrue("GetAllUserMetadata (filtered): Custom JSON match", bCustomJsonMatch);
                 if(!bCustomJsonMatch)
                {
                    AddError(FString::Printf(TEXT("Filtered GetAll Custom JSON mismatch. Expected: %s, Got: %s"), *TestUserCustomJson, *User.Custom));
                }
            }
        }
    }, 0.1f));
    
    // Step 4: RemoveUserMetadata
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID]()
    {
        PubnubSubsystem->RemoveUserMetadata(TestUserID);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f)); // Allow time for RemoveUserMetadata to process

    // Step 6: Verify Removal (using GetAllUserMetadata Filtered)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetAllUserMetadataCallback, bGetAllUserMetaDone, bGetAllUserMetaSuccess, ReceivedAllUsersData]()
    {
        *bGetAllUserMetaDone = false;
        *bGetAllUserMetaSuccess = false; // Reset for this call
        ReceivedAllUsersData->Empty();
        FPubnubGetAllInclude IncludeSettings;
        IncludeSettings.IncludeCustom = true; // Include custom just in case, though we expect no user
        FString Filter = FString::Printf(TEXT("id == '%s'"), *TestUserID);
        PubnubSubsystem->GetAllUserMetadata(GetAllUserMetadataCallback, IncludeSettings, 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllUserMetaDone]() { return *bGetAllUserMetaDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedAllUsersData, bGetAllUserMetaSuccess, TestUserID]()
    {
        TestTrue("GetAllUserMetadata (filtered, after remove) operation was successful (API call itself).", *bGetAllUserMetaSuccess);
        if (*bGetAllUserMetaSuccess)
        {
            TestEqual("GetAllUserMetadata (filtered, after remove): Expected 0 users.", ReceivedAllUsersData->Num(), 0);
            if (ReceivedAllUsersData->Num() != 0)
            {
                AddError(FString::Printf(TEXT("User metadata with ID '%s' was still found via GetAllUserMetadata after RemoveUserMetadata was called."), *TestUserID));
            }
        }
    }, 0.1f));

    CleanUp();
    return true;
}

bool FPubnubChannelMetadataFlowTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestUserID = SDK_PREFIX + "appctx_channel_meta_test";
    const FString TestChannelID = SDK_PREFIX + "appctx_chan_meta_test";
    const FString TestChannelName = "E2E AppContext Channel";
    const FString TestChannelDescription = "This is a test channel for App Context E2E tests.";
    const FString TestChannelStatus = "Active";
    const FString TestChannelType = "PublicDiscussion";
    const FString TestChannelCustomJson = TEXT("{\"topic\":\"testing\",\"moderated\":true}");

    const FString FullChannelMetadataToSet = FString::Printf(TEXT("{")
        TEXT("\"name\":\"%s\",")
        TEXT("\"description\":\"%s\",")
        TEXT("\"status\":\"%s\",")
        TEXT("\"type\":\"%s\",")
        TEXT("\"custom\":%s")
        TEXT("}"),
        *TestChannelName,
        *TestChannelDescription,
        *TestChannelStatus,
        *TestChannelType,
        *TestChannelCustomJson);

    TSharedPtr<bool> bGetChannelMetaDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetChannelMetaSuccess = MakeShared<bool>(false);
    TSharedPtr<FPubnubChannelData> ReceivedChannelData = MakeShared<FPubnubChannelData>();

    TSharedPtr<bool> bGetAllChannelMetaDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetAllChannelMetaSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FPubnubChannelData>> ReceivedAllChannelsData = MakeShared<TArray<FPubnubChannelData>>();

    TSharedPtr<bool> bGetChannelMetaAfterRemoveDone = MakeShared<bool>(false);
    TSharedPtr<int> GetChannelMetaAfterRemoveStatus = MakeShared<int>(0);

    // Callbacks
    FOnGetChannelMetadataResponseNative GetChannelMetadataCallback;
    GetChannelMetadataCallback.BindLambda([this, bGetChannelMetaDone, bGetChannelMetaSuccess, ReceivedChannelData](int Status, FPubnubChannelData ChannelData)
    {
        *bGetChannelMetaDone = true;
        *bGetChannelMetaSuccess = (Status == 200);
        if (*bGetChannelMetaSuccess)
        {
            *ReceivedChannelData = ChannelData;
        }
        else
        {
            AddError(FString::Printf(TEXT("GetChannelMetadata failed. Status: %d"), Status));
        }
    });

    FOnGetAllChannelMetadataResponseNative GetAllChannelMetadataCallback;
    GetAllChannelMetadataCallback.BindLambda([this, bGetAllChannelMetaDone, bGetAllChannelMetaSuccess, ReceivedAllChannelsData](int Status, const TArray<FPubnubChannelData>& ChannelsData, FString PageNext, FString PagePrev)
    {
        *bGetAllChannelMetaDone = true;
        *bGetAllChannelMetaSuccess = (Status == 200);
        if (*bGetAllChannelMetaSuccess)
        {
            *ReceivedAllChannelsData = ChannelsData;
        }
        else
        {
            AddError(FString::Printf(TEXT("GetAllChannelMetadata failed. Status: %d. Next: %s, Prev: %s"), Status, *PageNext, *PagePrev));
        }
    });

    FOnGetChannelMetadataResponseNative GetChannelMetadataCallback_AfterRemove;
    GetChannelMetadataCallback_AfterRemove.BindLambda([this, bGetChannelMetaAfterRemoveDone, GetChannelMetaAfterRemoveStatus](int Status, FPubnubChannelData ChannelData)
    {
        *bGetChannelMetaAfterRemoveDone = true;
        *GetChannelMetaAfterRemoveStatus = Status;
    });

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubChannelMetadataFlowTest");
        return false;
    }

    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("General Pubnub Error: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    // Set UserID
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID]()
    {
        PubnubSubsystem->SetUserID(TestUserID);
    }, 0.1f));

    // Step 1: SetChannelMetadata
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, FullChannelMetadataToSet]()
    {
        PubnubSubsystem->SetChannelMetadata(TestChannelID, FullChannelMetadataToSet, TEXT("custom,status,type"));
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f)); // Allow time for SetChannelMetadata to process

    // Step 2: GetChannelMetadata and Compare
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, GetChannelMetadataCallback, bGetChannelMetaDone, bGetChannelMetaSuccess, ReceivedChannelData]()
    {
        *bGetChannelMetaDone = false;
        *bGetChannelMetaSuccess = false;
        ReceivedChannelData->ChannelID.Empty(); // Reset
        PubnubSubsystem->GetChannelMetadata(TestChannelID, GetChannelMetadataCallback, TEXT("custom,status,type"));
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetChannelMetaDone]() { return *bGetChannelMetaDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, TestChannelName, TestChannelDescription, TestChannelStatus, TestChannelType, TestChannelCustomJson, ReceivedChannelData, bGetChannelMetaSuccess]()
    {
        TestTrue("GetChannelMetadata operation was successful after Set.", *bGetChannelMetaSuccess);
        if (*bGetChannelMetaSuccess)
        {
            TestEqual("GetChannelMetadata: ChannelID match", ReceivedChannelData->ChannelID, TestChannelID);
            TestEqual("GetChannelMetadata: ChannelName match", ReceivedChannelData->ChannelName, TestChannelName);
            TestEqual("GetChannelMetadata: Description match", ReceivedChannelData->Description, TestChannelDescription);
            TestEqual("GetChannelMetadata: Status match", ReceivedChannelData->Status, TestChannelStatus);
            TestEqual("GetChannelMetadata: Type match", ReceivedChannelData->Type, TestChannelType);
            TestFalse("GetChannelMetadata: Updated should not be empty", ReceivedChannelData->Updated.IsEmpty());
            TestFalse("GetChannelMetadata: ETag should not be empty", ReceivedChannelData->ETag.IsEmpty());
            bool bCustomJsonMatch = UPubnubJsonUtilities::AreJsonObjectStringsEqual(ReceivedChannelData->Custom, TestChannelCustomJson);
            TestTrue("GetChannelMetadata: Custom JSON match", bCustomJsonMatch);
            if(!bCustomJsonMatch)
            {
                AddError(FString::Printf(TEXT("Custom JSON mismatch. Expected: %s, Got: %s"), *TestChannelCustomJson, *ReceivedChannelData->Custom));
            }
        }
    }, 0.1f));

    // Step 3: GetAllChannelMetadata with filter
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, GetAllChannelMetadataCallback, bGetAllChannelMetaDone, bGetAllChannelMetaSuccess, ReceivedAllChannelsData]()
    {
        *bGetAllChannelMetaDone = false;
        *bGetAllChannelMetaSuccess = false;
        ReceivedAllChannelsData->Empty();
        FPubnubGetAllInclude IncludeSettings;
        IncludeSettings.IncludeCustom = true;
        IncludeSettings.IncludeStatus = true;
        IncludeSettings.IncludeType = true;
        FString Filter = FString::Printf(TEXT("id == '%s'"), *TestChannelID);
        PubnubSubsystem->GetAllChannelMetadata(GetAllChannelMetadataCallback, IncludeSettings, 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllChannelMetaDone]() { return *bGetAllChannelMetaDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, TestChannelName, TestChannelDescription, TestChannelStatus, TestChannelType, TestChannelCustomJson, ReceivedAllChannelsData, bGetAllChannelMetaSuccess]()
    {
        TestTrue("GetAllChannelMetadata (filtered) operation was successful.", *bGetAllChannelMetaSuccess);
        if (*bGetAllChannelMetaSuccess)
        {
            TestEqual("GetAllChannelMetadata (filtered): Expected 1 channel.", ReceivedAllChannelsData->Num(), 1);
            if (ReceivedAllChannelsData->Num() == 1)
            {
                const FPubnubChannelData& Channel = (*ReceivedAllChannelsData)[0];
                TestEqual("GetAllChannelMetadata (filtered): ChannelID match", Channel.ChannelID, TestChannelID);
                TestEqual("GetAllChannelMetadata (filtered): ChannelName match", Channel.ChannelName, TestChannelName);
                TestEqual("GetAllChannelMetadata (filtered): Description match", Channel.Description, TestChannelDescription);
                TestEqual("GetAllChannelMetadata (filtered): Status match", Channel.Status, TestChannelStatus);
                TestEqual("GetAllChannelMetadata (filtered): Type match", Channel.Type, TestChannelType);
                TestFalse("GetAllChannelMetadata (filtered): Updated should not be empty", Channel.Updated.IsEmpty());
                TestFalse("GetAllChannelMetadata (filtered): ETag should not be empty", Channel.ETag.IsEmpty());
                bool bCustomJsonMatch = UPubnubJsonUtilities::AreJsonObjectStringsEqual(Channel.Custom, TestChannelCustomJson);
                TestTrue("GetAllChannelMetadata (filtered): Custom JSON match", bCustomJsonMatch);
                 if(!bCustomJsonMatch)
                {
                    AddError(FString::Printf(TEXT("Filtered GetAll Custom JSON mismatch. Expected: %s, Got: %s"), *TestChannelCustomJson, *Channel.Custom));
                }
            }
        }
    }, 0.1f));
    
    // Step 4: RemoveChannelMetadata
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID]()
    {
        PubnubSubsystem->RemoveChannelMetadata(TestChannelID);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f)); // Allow time for RemoveChannelMetadata to process

    // Step 6: Verify Removal (using GetAllChannelMetadata Filtered)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, GetAllChannelMetadataCallback, bGetAllChannelMetaDone, bGetAllChannelMetaSuccess, ReceivedAllChannelsData]()
    {
        *bGetAllChannelMetaDone = false;
        *bGetAllChannelMetaSuccess = false; // Reset for this call
        ReceivedAllChannelsData->Empty();
        FPubnubGetAllInclude IncludeSettings;
        IncludeSettings.IncludeCustom = true;
        IncludeSettings.IncludeStatus = true;
        IncludeSettings.IncludeType = true;
        FString Filter = FString::Printf(TEXT("id == '%s'"), *TestChannelID);
        PubnubSubsystem->GetAllChannelMetadata(GetAllChannelMetadataCallback, IncludeSettings, 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllChannelMetaDone]() { return *bGetAllChannelMetaDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedAllChannelsData, bGetAllChannelMetaSuccess, TestChannelID]()
    {
        TestTrue("GetAllChannelMetadata (filtered, after remove) operation was successful (API call itself).", *bGetAllChannelMetaSuccess);
        if (*bGetAllChannelMetaSuccess)
        {
            TestEqual("GetAllChannelMetadata (filtered, after remove): Expected 0 channels.", ReceivedAllChannelsData->Num(), 0);
            if (ReceivedAllChannelsData->Num() != 0)
            {
                AddError(FString::Printf(TEXT("Channel metadata with ID '%s' was still found via GetAllChannelMetadata after RemoveChannelMetadata was called."), *TestChannelID));
            }
        }
    }, 0.1f));

    CleanUp();
    return true;
}

bool FPubnubGetAllChannelMetadataWithOptionsTest::RunTest(const FString& Parameters)
{
    const FString TestRunPrefix = SDK_PREFIX + TEXT("gacm_opts_");
    const FString TestUserID = TestRunPrefix + TEXT("user");

    // Channel A: For custom field filtering & specific include test
    const FString ChannelAID = TestRunPrefix + TEXT("ChannelA");
    const FString ChannelAName = TEXT("Channel A - Tech");
    const FString ChannelACustom = TEXT("{\"category\":\"Tech\", \"priority\":1}");
    const FString ChannelAStatus = TEXT("Active");
    const FString ChannelAType = TEXT("TechnicalDiscussion");
    const FString ChannelAMetadata = FString::Printf(TEXT("{\"name\":\"%s\", \"custom\":%s, \"status\":\"%s\", \"type\":\"%s\"}"), *ChannelAName, *ChannelACustom, *ChannelAStatus, *ChannelAType);

    // Channel B: For custom field filtering (different category)
    const FString ChannelBID = TestRunPrefix + TEXT("ChannelB");
    const FString ChannelBName = TEXT("Channel B - Finance");
    const FString ChannelBCustom = TEXT("{\"category\":\"Finance\", \"priority\":2}");
    const FString ChannelBMetadata = FString::Printf(TEXT("{\"name\":\"%s\", \"custom\":%s}"), *ChannelBName, *ChannelBCustom);

    // Channels for Sorting & Limit Test (Channel_Sort_X)
    const FString ChannelSortPrefix = TestRunPrefix + TEXT("SortChan_");
    const FString ChannelSortAID = ChannelSortPrefix + TEXT("Alpha");
    const FString ChannelSortAName = TEXT("Sort Channel Alpha");
    const FString ChannelSortAMetadata = FString::Printf(TEXT("{\"name\":\"%s\"}"), *ChannelSortAName);

    const FString ChannelSortBID = ChannelSortPrefix + TEXT("Beta");
    const FString ChannelSortBName = TEXT("Sort Channel Beta");
    const FString ChannelSortBMetadata = FString::Printf(TEXT("{\"name\":\"%s\"}"), *ChannelSortBName);

    const FString ChannelSortCID = ChannelSortPrefix + TEXT("Gamma");
    const FString ChannelSortCName = TEXT("Sort Channel Gamma");
    const FString ChannelSortCMetadata = FString::Printf(TEXT("{\"name\":\"%s\"}"), *ChannelSortCName);

    TSharedPtr<bool> bGetAllDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetAllSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FPubnubChannelData>> ReceivedChannels = MakeShared<TArray<FPubnubChannelData>>();
    TSharedPtr<FString> NextPage = MakeShared<FString>();
    TSharedPtr<FString> PrevPage = MakeShared<FString>();

    // Callback
    FOnGetAllChannelMetadataResponseNative GetAllCallback;
    GetAllCallback.BindLambda([this, bGetAllDone, bGetAllSuccess, ReceivedChannels, NextPage, PrevPage](int Status, const TArray<FPubnubChannelData>& ChannelsData, FString PageNextStr, FString PagePrevStr)
    {
        *bGetAllDone = true;
        *bGetAllSuccess = (Status == 200);
        if (*bGetAllSuccess)
        {
            *ReceivedChannels = ChannelsData;
            *NextPage = PageNextStr;
            *PrevPage = PagePrevStr;
        }
        else
        {
            ReceivedChannels->Empty();
            AddError(FString::Printf(TEXT("GetAllChannelMetadata call failed. Status: %d. Next: '%s', Prev: '%s'"), Status, *PageNextStr, *PagePrevStr));
        }
    });

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubGetAllChannelMetadataWithOptionsTest");
        return false;
    }
    
    PubnubSubsystem->SetUserID(TestUserID);
    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("General Pubnub Error: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    // Initial Setup: Set metadata for all test channels
    auto SetMeta = [this](const FString& ChanID, const FString& Meta, const FString& IncludeFields = TEXT("custom"))
    {
        ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChanID, Meta, IncludeFields]()
        {
            PubnubSubsystem->SetChannelMetadata(ChanID, Meta, IncludeFields);
        }, 0.1f));
    };

    SetMeta(ChannelAID, ChannelAMetadata, TEXT("custom,status,type"));
    SetMeta(ChannelBID, ChannelBMetadata);
    SetMeta(ChannelSortAID, ChannelSortAMetadata);
    SetMeta(ChannelSortBID, ChannelSortBMetadata);
    SetMeta(ChannelSortCID, ChannelSortCMetadata);
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(2.0f)); // Wait for all SetChannelMetadata to process

    // --- Scenario 1: Filter by a Custom Field ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, bGetAllDone, bGetAllSuccess, ReceivedChannels]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedChannels->Empty();
        FPubnubGetAllInclude IncludeSettings; IncludeSettings.IncludeCustom = true;
        FString Filter = TEXT("custom.category == 'Tech'");
        PubnubSubsystem->GetAllChannelMetadata(GetAllCallback, IncludeSettings, 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedChannels, bGetAllSuccess, ChannelAID, ChannelAName, ChannelACustom]()
    {
        TestTrue("S1: GetAllChannelMetadata with custom filter success.", *bGetAllSuccess);
        if(*bGetAllSuccess)
        {
            bool bFoundChannelA = false;
            for(const auto& Chan : *ReceivedChannels) { if(Chan.ChannelID == ChannelAID) { bFoundChannelA = true; break; } }
            TestTrue(FString::Printf(TEXT("S1: Channel A (ID: %s) should be in results for custom.category == 'Tech'. Count: %d"), *ChannelAID, ReceivedChannels->Num()), bFoundChannelA);
            if (bFoundChannelA && ReceivedChannels->Num() == 1) // Stricter check if only one is expected
            {
                 TestEqual("S1: Channel A Name match", (*ReceivedChannels)[0].ChannelName, ChannelAName);
                 TestTrue("S1: Channel A Custom match", UPubnubJsonUtilities::AreJsonObjectStringsEqual((*ReceivedChannels)[0].Custom, ChannelACustom));
            }
             else if (ReceivedChannels->Num() > 1) AddWarning("S1: Filter custom.category == 'Tech' returned multiple channels. Check keyset for conflicts.");
        }
    }, 0.1f));

    // --- Scenario 2: Filter by Name ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, ChannelAName, bGetAllDone, bGetAllSuccess, ReceivedChannels]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedChannels->Empty();
        FString Filter = FString::Printf(TEXT("name == '%s'"), *ChannelAName);
        PubnubSubsystem->GetAllChannelMetadata(GetAllCallback, FPubnubGetAllInclude(), 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedChannels, bGetAllSuccess, ChannelAID]()
    {
        TestTrue("S2: GetAllChannelMetadata with name filter success.", *bGetAllSuccess);
        if(*bGetAllSuccess)
        {
             bool bFoundChannelA = false;
             for(const auto& Chan : *ReceivedChannels) { if(Chan.ChannelID == ChannelAID) { bFoundChannelA = true; break; } }
             TestTrue(FString::Printf(TEXT("S2: Channel A (ID: %s) should be in results for name filter. Count: %d"), *ChannelAID, ReceivedChannels->Num()), bFoundChannelA);
             if (ReceivedChannels->Num() > 1) AddWarning("S2: Name filter returned multiple channels. Ensure name is unique for this test or filter is more specific.");
        }
    }, 0.1f));

    // --- Scenario 3: Sort by Name (Ascending/Descending) ---
    const FString SortTestFilter = FString::Printf(TEXT("id LIKE \"%s*\""), *ChannelSortPrefix); // Filter for our sort test channels
    // Ascending Sort
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, SortTestFilter, bGetAllDone, bGetAllSuccess, ReceivedChannels]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedChannels->Empty();
        FPubnubGetAllSort SortSettings; SortSettings.GetAllSort.Add({EPubnubGetAllSortType::PGAST_Name, false}); // name:asc
        PubnubSubsystem->GetAllChannelMetadata(GetAllCallback, FPubnubGetAllInclude(), 10, SortTestFilter, SortSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedChannels, bGetAllSuccess, ChannelSortAID, ChannelSortBID, ChannelSortCID]()
    {
        TestTrue("S3a: GetAllChannelMetadata with name:asc sort success.", *bGetAllSuccess);
        if(*bGetAllSuccess && ReceivedChannels->Num() == 3)
        {
            TestEqual("S3a: Sort Order [0] ID", (*ReceivedChannels)[0].ChannelID, ChannelSortAID);
            TestEqual("S3a: Sort Order [1] ID", (*ReceivedChannels)[1].ChannelID, ChannelSortBID);
            TestEqual("S3a: Sort Order [2] ID", (*ReceivedChannels)[2].ChannelID, ChannelSortCID);
        }
        else if (*bGetAllSuccess) AddError(FString::Printf(TEXT("S3a: Expected 3 channels for sort test, got %d"), ReceivedChannels->Num()));
    }, 0.1f));
    // Descending Sort
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, SortTestFilter, bGetAllDone, bGetAllSuccess, ReceivedChannels]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedChannels->Empty();
        FPubnubGetAllSort SortSettings; SortSettings.GetAllSort.Add({EPubnubGetAllSortType::PGAST_Name, true}); // name:desc
        PubnubSubsystem->GetAllChannelMetadata(GetAllCallback, FPubnubGetAllInclude(), 10, SortTestFilter, SortSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedChannels, bGetAllSuccess, ChannelSortAID, ChannelSortBID, ChannelSortCID]()
    {
        TestTrue("S3b: GetAllChannelMetadata with name:desc sort success.", *bGetAllSuccess);
        if(*bGetAllSuccess && ReceivedChannels->Num() == 3)
        {
            TestEqual("S3b: Sort Order [0] ID", (*ReceivedChannels)[0].ChannelID, ChannelSortCID);
            TestEqual("S3b: Sort Order [1] ID", (*ReceivedChannels)[1].ChannelID, ChannelSortBID);
            TestEqual("S3b: Sort Order [2] ID", (*ReceivedChannels)[2].ChannelID, ChannelSortAID);
        }
         else if (*bGetAllSuccess) AddError(FString::Printf(TEXT("S3b: Expected 3 channels for sort test, got %d"), ReceivedChannels->Num()));
    }, 0.1f));

    // --- Scenario 4: Limit Number of Results & PageNext ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, SortTestFilter, NextPage, bGetAllDone, bGetAllSuccess, ReceivedChannels]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedChannels->Empty(); NextPage->Empty();
        FPubnubGetAllSort SortSettings; SortSettings.GetAllSort.Add({EPubnubGetAllSortType::PGAST_Name, false}); 
        FPubnubGetAllInclude IncludeSettings; IncludeSettings.IncludeTotalCount = true; // Test IncludeTotalCount flag processing
        PubnubSubsystem->GetAllChannelMetadata(GetAllCallback, IncludeSettings, 2, SortTestFilter, SortSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedChannels, NextPage, bGetAllSuccess]()
    {
        TestTrue("S4: GetAllChannelMetadata with Limit=2 success.", *bGetAllSuccess);
        if(*bGetAllSuccess)
        {
            TestEqual("S4: Received channel count with Limit=2.", ReceivedChannels->Num(), 2);
            TestFalse("S4: PageNext should be populated if more results exist.", NextPage->IsEmpty());
        }
    }, 0.1f));

    // --- Scenario 5: Test Include Options ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, ChannelAID, bGetAllDone, bGetAllSuccess, ReceivedChannels]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedChannels->Empty();
        FPubnubGetAllInclude IncludeSettings; 
        IncludeSettings.IncludeCustom = true;
        IncludeSettings.IncludeStatus = true;
        IncludeSettings.IncludeType = true;
        FString Filter = FString::Printf(TEXT("id == '%s'"), *ChannelAID);
        PubnubSubsystem->GetAllChannelMetadata(GetAllCallback, IncludeSettings, 1, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedChannels, bGetAllSuccess, ChannelACustom, ChannelAStatus, ChannelAType]()
    {
        TestTrue("S5: GetAllChannelMetadata with specific Includes success.", *bGetAllSuccess);
        if(*bGetAllSuccess && ReceivedChannels->Num() == 1)
        {
            const auto& Chan = (*ReceivedChannels)[0];
            TestTrue("S5: Custom data match", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Chan.Custom, ChannelACustom));
            TestEqual("S5: Status match", Chan.Status, ChannelAStatus);
            TestEqual("S5: Type match", Chan.Type, ChannelAType);
        }
        else if(*bGetAllSuccess) AddError(FString::Printf(TEXT("S5: Expected 1 channel for include test, got %d"), ReceivedChannels->Num()));
    }, 0.1f));

    // Cleanup
    auto RemoveMeta = [this](const FString& ChanID)
    {
        ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChanID]()
        {
            PubnubSubsystem->RemoveChannelMetadata(ChanID);
        }, 0.1f));
    };
    RemoveMeta(ChannelAID); RemoveMeta(ChannelBID);
    RemoveMeta(ChannelSortAID); RemoveMeta(ChannelSortBID); RemoveMeta(ChannelSortCID);
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f)); // Wait for removals

    CleanUp();
    return true;
}

bool FPubnubGetAllUsersMetadataWithOptionsTest::RunTest(const FString& Parameters)
{
    const FString TestRunPrefix = SDK_PREFIX + TEXT("gaum_opts_");
    const FString TestAdminUserID = TestRunPrefix + TEXT("admin_user");

    // User A: For custom field filtering & specific include test
    const FString UserAID = TestRunPrefix + TEXT("UserA");
    const FString UserAName = TEXT("User A - Engineering Dept");
    const FString UserAEmail = TEXT("user.a@example.com");
    const FString UserACustom = TEXT("{\"department\":\"Engineering\", \"level\":5, \"active_project\":\"ProjectPhoenix\"}");
    const FString UserAStatus = TEXT("Active");
    const FString UserAType = TEXT("Engineer");
    const FString UserAMetadata = FString::Printf(TEXT("{\"name\":\"%s\", \"email\":\"%s\", \"custom\":%s, \"status\":\"%s\", \"type\":\"%s\"}"), 
        *UserAName, *UserAEmail, *UserACustom, *UserAStatus, *UserAType);

    // User B: For custom field filtering (different department)
    const FString UserBID = TestRunPrefix + TEXT("UserB");
    const FString UserBName = TEXT("User B - Marketing Dept");
    const FString UserBEmail = TEXT("user.b@example.com");
    const FString UserBCustom = TEXT("{\"department\":\"Marketing\", \"level\":3, \"campaign\":\"SummerSale\"}");
    const FString UserBMetadata = FString::Printf(TEXT("{\"name\":\"%s\", \"email\":\"%s\", \"custom\":%s}"), 
        *UserBName, *UserBEmail, *UserBCustom);

    // Users for Sorting & Limit Test (User_Sort_X)
    const FString UserSortPrefix = TestRunPrefix + TEXT("SortUser_");
    const FString UserSortAID = UserSortPrefix + TEXT("Charlie"); // Names chosen for easy sort verification
    const FString UserSortAName = TEXT("Charlie Brown");
    const FString UserSortAMetadata = FString::Printf(TEXT("{\"name\":\"%s\"}"), *UserSortAName);

    const FString UserSortBID = UserSortPrefix + TEXT("Alice");
    const FString UserSortBName = TEXT("Alice Wonderland");
    const FString UserSortBMetadata = FString::Printf(TEXT("{\"name\":\"%s\"}"), *UserSortBName);

    const FString UserSortCID = UserSortPrefix + TEXT("Bob");
    const FString UserSortCName = TEXT("Bob The Builder");
    const FString UserSortCMetadata = FString::Printf(TEXT("{\"name\":\"%s\"}"), *UserSortCName);
    
    // User D and E for pagination testing (enough to require a second page with limit 2)
    const FString UserSortDID = UserSortPrefix + TEXT("David");
    const FString UserSortDName = TEXT("David Copperfield");
    const FString UserSortDMetadata = FString::Printf(TEXT("{\"name\":\"%s\"}"), *UserSortDName);


    TArray<FString> AllTestUserIDs = { UserAID, UserBID, UserSortAID, UserSortBID, UserSortCID, UserSortDID };

    TSharedPtr<bool> bGetAllDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetAllSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FPubnubUserData>> ReceivedUsers = MakeShared<TArray<FPubnubUserData>>();
    TSharedPtr<FString> NextPage = MakeShared<FString>();
    TSharedPtr<FString> PrevPage = MakeShared<FString>(); // Though not explicitly tested for navigation, it's part of callback

    // Callback
    FOnGetAllUserMetadataResponseNative GetAllCallback;
    GetAllCallback.BindLambda([this, bGetAllDone, bGetAllSuccess, ReceivedUsers, NextPage, PrevPage](int Status, const TArray<FPubnubUserData>& UsersData, FString PageNextStr, FString PagePrevStr)
    {
        *bGetAllDone = true;
        *bGetAllSuccess = (Status == 200);
        if (*bGetAllSuccess)
        {
            *ReceivedUsers = UsersData;
            *NextPage = PageNextStr;
            *PrevPage = PagePrevStr;
        }
        else
        {
            ReceivedUsers->Empty();
            AddError(FString::Printf(TEXT("GetAllUserMetadata call failed. Status: %d. Next: '%s', Prev: '%s'"), Status, *PageNextStr, *PagePrevStr));
        }
    });

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubGetAllUsersMetadataWithOptionsTest");
        return false;
    }
    
    PubnubSubsystem->SetUserID(TestAdminUserID);
    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("General Pubnub Error: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    // Initial Setup: Set metadata for all test users
    auto SetMeta = [this](const FString& UserID, const FString& Meta, const FString& IncludeFields = TEXT("custom,status,type"))
    {
        ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, UserID, Meta, IncludeFields]()
        {
            PubnubSubsystem->SetUserMetadata(UserID, Meta, IncludeFields);
        }, 0.05f)); // Shorter delay for setup
    };

    SetMeta(UserAID, UserAMetadata);
    SetMeta(UserBID, UserBMetadata);
    SetMeta(UserSortAID, UserSortAMetadata);
    SetMeta(UserSortBID, UserSortBMetadata);
    SetMeta(UserSortCID, UserSortCMetadata);
    SetMeta(UserSortDID, UserSortDMetadata);
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(2.0f)); // Wait for all SetUserMetadata to process

    // --- Scenario 1: Filter by a Custom Field --- (e.g. department)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, bGetAllDone, bGetAllSuccess, ReceivedUsers]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedUsers->Empty();
        FPubnubGetAllInclude IncludeSettings; IncludeSettings.IncludeCustom = true;
        FString Filter = TEXT("custom.department == 'Engineering'");
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, IncludeSettings, 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, bGetAllSuccess, UserAID, UserAName, UserACustom]()
    {
        TestTrue("S1: GetAllUserMetadata with custom.department filter success.", *bGetAllSuccess);
        if(*bGetAllSuccess)
        {
            bool bFoundUserA = false;
            for(const auto& User : *ReceivedUsers) { if(User.UserID == UserAID) { bFoundUserA = true; break; } }
            TestTrue(FString::Printf(TEXT("S1: User A (ID: %s) should be in results for custom.department == 'Engineering'. Count: %d"), *UserAID, ReceivedUsers->Num()), bFoundUserA);
            if (bFoundUserA && ReceivedUsers->Num() == 1)
            {
                 TestEqual("S1: User A Name match", (*ReceivedUsers)[0].UserName, UserAName);
                 TestTrue("S1: User A Custom match", UPubnubJsonUtilities::AreJsonObjectStringsEqual((*ReceivedUsers)[0].Custom, UserACustom));
            }
             else if (ReceivedUsers->Num() > 1) AddWarning("S1: Filter custom.department == 'Engineering' returned multiple users. Check keyset for conflicts.");
        }
    }, 0.1f));

    // --- Scenario 2: Filter by Standard Field (Name) ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, UserAName, bGetAllDone, bGetAllSuccess, ReceivedUsers]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedUsers->Empty();
        FString Filter = FString::Printf(TEXT("name == '%s'"), *UserAName);
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, FPubnubGetAllInclude(), 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, bGetAllSuccess, UserAID]()
    {
        TestTrue("S2: GetAllUserMetadata with name filter success.", *bGetAllSuccess);
        if(*bGetAllSuccess)
        {
             bool bFoundUserA = false;
             for(const auto& User : *ReceivedUsers) { if(User.UserID == UserAID) { bFoundUserA = true; break; } }
             TestTrue(FString::Printf(TEXT("S2: User A (ID: %s) should be in results for name filter. Count: %d"), *UserAID, ReceivedUsers->Num()), bFoundUserA);
             if (ReceivedUsers->Num() > 1) AddWarning("S2: Name filter returned multiple users. Ensure name is unique for this test or filter is more specific.");
        }
    }, 0.1f));

    // --- Scenario 3: Sort by Name (Ascending/Descending) ---
    const FString SortTestFilter = FString::Printf(TEXT("id == '%s' || id == '%s' || id == '%s' || id == '%s'"), *UserSortAID, *UserSortBID, *UserSortCID, *UserSortDID); // Filter for our sort test users (Alice, Bob, Charlie, David)
    // Ascending Sort by Name
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, SortTestFilter, bGetAllDone, bGetAllSuccess, ReceivedUsers]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedUsers->Empty();
        FPubnubGetAllSort SortSettings; SortSettings.GetAllSort.Add({EPubnubGetAllSortType::PGAST_Name, false}); // name:asc
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, FPubnubGetAllInclude(), 10, SortTestFilter, SortSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, bGetAllSuccess, UserSortAID, UserSortBID, UserSortCID, UserSortDID]()
    {
        TestTrue("S3a: GetAllUserMetadata with name:asc sort success.", *bGetAllSuccess);
        if(*bGetAllSuccess && ReceivedUsers->Num() == 4)
        {
            TestEqual("S3a: Sort Order [0] ID (Alice)", (*ReceivedUsers)[0].UserID, UserSortBID);
            TestEqual("S3a: Sort Order [1] ID (Bob)", (*ReceivedUsers)[1].UserID, UserSortCID);
            TestEqual("S3a: Sort Order [2] ID (Charlie)", (*ReceivedUsers)[2].UserID, UserSortAID);
            TestEqual("S3a: Sort Order [3] ID (David)", (*ReceivedUsers)[3].UserID, UserSortDID);
        }
        else if (*bGetAllSuccess) AddError(FString::Printf(TEXT("S3a: Expected 4 users for sort test, got %d"), ReceivedUsers->Num()));
    }, 0.1f));
    // Descending Sort by Name
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, SortTestFilter, bGetAllDone, bGetAllSuccess, ReceivedUsers]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedUsers->Empty();
        FPubnubGetAllSort SortSettings; SortSettings.GetAllSort.Add({EPubnubGetAllSortType::PGAST_Name, true}); // name:desc
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, FPubnubGetAllInclude(), 10, SortTestFilter, SortSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, bGetAllSuccess, UserSortAID, UserSortBID, UserSortCID, UserSortDID]()
    {
        TestTrue("S3b: GetAllUserMetadata with name:desc sort success.", *bGetAllSuccess);
        if(*bGetAllSuccess && ReceivedUsers->Num() == 4)
        {
            TestEqual("S3b: Sort Order [0] ID (David)", (*ReceivedUsers)[0].UserID, UserSortDID);
            TestEqual("S3b: Sort Order [1] ID (Charlie)", (*ReceivedUsers)[1].UserID, UserSortAID);
            TestEqual("S3b: Sort Order [2] ID (Bob)", (*ReceivedUsers)[2].UserID, UserSortCID);
            TestEqual("S3b: Sort Order [3] ID (Alice)", (*ReceivedUsers)[3].UserID, UserSortBID);
        }
         else if (*bGetAllSuccess) AddError(FString::Printf(TEXT("S3b: Expected 4 users for sort test, got %d"), ReceivedUsers->Num()));
    }, 0.1f));

    // --- Scenario 4: Limit Number of Results & PageNext ---
    // Using the same SortTestFilter and name:asc order for consistent pagination
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, SortTestFilter, NextPage, bGetAllDone, bGetAllSuccess, ReceivedUsers]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedUsers->Empty(); NextPage->Empty();
        FPubnubGetAllSort SortSettings; SortSettings.GetAllSort.Add({EPubnubGetAllSortType::PGAST_Name, false}); 
        FPubnubGetAllInclude IncludeSettings; IncludeSettings.IncludeTotalCount = true; // Test IncludeTotalCount flag processing
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, IncludeSettings, 2, SortTestFilter, SortSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, NextPage, bGetAllSuccess, UserSortBID, UserSortCID]()
    {
        TestTrue("S4a: GetAllUserMetadata with Limit=2 (Page 1) success.", *bGetAllSuccess);
        if(*bGetAllSuccess)
        {
            TestEqual("S4a: Received user count with Limit=2.", ReceivedUsers->Num(), 2);
            if(ReceivedUsers->Num() == 2)
            {
                TestEqual("S4a: Page 1 User [0] ID (Alice)", (*ReceivedUsers)[0].UserID, UserSortBID);
                TestEqual("S4a: Page 1 User [1] ID (Bob)", (*ReceivedUsers)[1].UserID, UserSortCID);
            }
            TestFalse("S4a: PageNext should be populated as more results exist.", NextPage->IsEmpty());
        }
    }, 0.1f));
    // Fetch Page 2
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, SortTestFilter, NextPage, bGetAllDone, bGetAllSuccess, ReceivedUsers]()
    {
        if (NextPage->IsEmpty()) { AddError("S4b: Cannot fetch page 2, NextPage token is empty."); *bGetAllDone = true; return; }
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedUsers->Empty(); 
        FString CurrentNextPage = *NextPage;
        NextPage->Empty(); // Clear for next potential pagination, though not expected here
        FPubnubGetAllSort SortSettings; SortSettings.GetAllSort.Add({EPubnubGetAllSortType::PGAST_Name, false});
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, FPubnubGetAllInclude(), 2, SortTestFilter, SortSettings, CurrentNextPage);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, NextPage, bGetAllSuccess, UserSortAID, UserSortDID]()
    {
        TestTrue("S4b: GetAllUserMetadata with Limit=2 (Page 2) success.", *bGetAllSuccess);
        if(*bGetAllSuccess)
        {
            TestEqual("S4b: Received user count with Limit=2 for Page 2.", ReceivedUsers->Num(), 2);
             if(ReceivedUsers->Num() == 2)
            {
                TestEqual("S4b: Page 2 User [0] ID (Charlie)", (*ReceivedUsers)[0].UserID, UserSortAID);
                TestEqual("S4b: Page 2 User [1] ID (David)", (*ReceivedUsers)[1].UserID, UserSortDID);
            }
            // NextPage might be empty now if these are all users for the filter.
            // AddTestEqual("S4b: PageNext should be empty as no more results.", NextPage->IsEmpty(), true); // This depends on exact total number of users matching the filter vs limit
        }
    }, 0.1f));

    // --- Scenario 5: Test Include Options (Custom, Status, Type) ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, UserAID, bGetAllDone, bGetAllSuccess, ReceivedUsers]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedUsers->Empty();
        FPubnubGetAllInclude IncludeSettings; 
        IncludeSettings.IncludeCustom = true;
        IncludeSettings.IncludeStatus = true;
        IncludeSettings.IncludeType = true;
        FString Filter = FString::Printf(TEXT("id == '%s'"), *UserAID);
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, IncludeSettings, 1, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, bGetAllSuccess, UserACustom, UserAStatus, UserAType]()
    {
        TestTrue("S5a: GetAllUserMetadata with specific Includes (Custom, Status, Type) success.", *bGetAllSuccess);
        if(*bGetAllSuccess && ReceivedUsers->Num() == 1)
        {
            const auto& User = (*ReceivedUsers)[0];
            TestTrue("S5a: Custom data match", UPubnubJsonUtilities::AreJsonObjectStringsEqual(User.Custom, UserACustom));
            TestEqual("S5a: Status match", User.Status, UserAStatus);
            TestEqual("S5a: Type match", User.Type, UserAType);
        }
        else if(*bGetAllSuccess) AddError(FString::Printf(TEXT("S5a: Expected 1 user for include test, got %d"), ReceivedUsers->Num()));
    }, 0.1f));
    // Test with some includes false
     ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, UserAID, bGetAllDone, bGetAllSuccess, ReceivedUsers]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedUsers->Empty();
        FPubnubGetAllInclude IncludeSettings; 
        IncludeSettings.IncludeCustom = true; // Keep custom true
        IncludeSettings.IncludeStatus = false;
        IncludeSettings.IncludeType = false;
        FString Filter = FString::Printf(TEXT("id == '%s'"), *UserAID);
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, IncludeSettings, 1, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, bGetAllSuccess, UserACustom, UserAStatus, UserAType]()
    {
        TestTrue("S5b: GetAllUserMetadata with Includes (Status=false, Type=false) success.", *bGetAllSuccess);
        if(*bGetAllSuccess && ReceivedUsers->Num() == 1)
        {
            const auto& User = (*ReceivedUsers)[0];
            TestTrue("S5b: Custom data should still match", UPubnubJsonUtilities::AreJsonObjectStringsEqual(User.Custom, UserACustom));
            TestTrue("S5b: Status should be empty as it was not included", User.Status.IsEmpty());
            TestTrue("S5b: Type should be empty as it was not included", User.Type.IsEmpty());
        }
        else if(*bGetAllSuccess) AddError(FString::Printf(TEXT("S5b: Expected 1 user for include test (custom only), got %d"), ReceivedUsers->Num()));
    }, 0.1f));


    // Cleanup
    auto RemoveMeta = [this](const FString& UserID)
    {
        ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, UserID]()
        {
            PubnubSubsystem->RemoveUserMetadata(UserID);
        }, 0.05f));
    };
    for(const FString& UserID : AllTestUserIDs)
    {
        RemoveMeta(UserID);
    }
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(2.0f)); // Wait for all removals

    CleanUp();
    return true;
}

bool FPubnubMembershipManagementWithOptionsTest::RunTest(const FString& Parameters)
{
    const FString TestRunPrefix = SDK_PREFIX + TEXT("memb_opts_");
    const FString TestUserID = TestRunPrefix + TEXT("user_main");
    const FString TestAdminUserID = TestRunPrefix + TEXT("admin_for_setuser"); // UserID for the PubnubSubsystem instance

    // Channel Definitions
    const FString ChannelAID = TestRunPrefix + TEXT("ChannelA_Docs");
    const FString ChannelAName = TEXT("Documentation Central");
    const FString ChannelACustomJson = TEXT("{\"topic\":\"sdk_docs\", \"priority\":\"high\"}");
    const FString ChannelAStatusVal = TEXT("active");
    const FString ChannelAMetadata = FString::Printf(TEXT("{\"name\":\"%s\", \"custom\":%s, \"status\":\"%s\"}"), *ChannelAName, *ChannelACustomJson, *ChannelAStatusVal);

    const FString ChannelBID = TestRunPrefix + TEXT("ChannelB_Dev");
    const FString ChannelBName = TEXT("Development Zone");
    const FString ChannelBCustomJson = TEXT("{\"topic\":\"core_dev\", \"priority\":\"medium\"}");
    const FString ChannelBStatusVal = TEXT("active");
    const FString ChannelBMetadata = FString::Printf(TEXT("{\"name\":\"%s\", \"custom\":%s, \"status\":\"%s\"}"), *ChannelBName, *ChannelBCustomJson, *ChannelBStatusVal);

    const FString ChannelCID = TestRunPrefix + TEXT("ChannelC_General");
    const FString ChannelCName = TEXT("General Discussion");
    const FString ChannelCCustomJson = TEXT("{\"topic\":\"community\", \"priority\":\"low\"}");
    const FString ChannelCStatusVal = TEXT("archived");
    const FString ChannelCMetadata = FString::Printf(TEXT("{\"name\":\"%s\", \"custom\":%s, \"status\":\"%s\"}"), *ChannelCName, *ChannelCCustomJson, *ChannelCStatusVal);
    
    const FString ChannelDID = TestRunPrefix + TEXT("ChannelD_Support");
	const FString ChannelDName = TEXT("Support Hub");
	const FString ChannelDCustomJson = TEXT("{\"topic\":\"user_support\", \"priority\":\"high\"}");
    const FString ChannelDStatusVal = TEXT("active");
	const FString ChannelDMetadata = FString::Printf(TEXT("{\"name\":\"%s\", \"custom\":%s, \"status\":\"%s\"}"), *ChannelDName, *ChannelDCustomJson, *ChannelDStatusVal);

    TArray<FString> AllTestChannelIDs = {ChannelAID, ChannelBID, ChannelCID, ChannelDID};

    // Membership specific data
    const FString MembershipACustom = TEXT("{\"role\":\"editor\", \"last_access\":\"2024-01-01\"}");
    const FString MembershipAStatus = TEXT("activeEditor");
    const FString MembershipAType = TEXT("contentContributor");

    const FString MembershipBCustom = TEXT("{\"role\":\"viewer\", \"last_access\":\"2024-01-15\"}");
    const FString MembershipBStatus = TEXT("passiveViewer");
    // MembershipBType will be default/empty

    const FString MembershipCCustom = TEXT("{\"role\":\"moderator\", \"last_access\":\"2023-12-01\"}");
    // MembershipCStatus will be default/empty
    const FString MembershipCType = TEXT("channelAdmin");
    
    const FString MembershipDCustom = TEXT("{\"role\":\"agent\", \"last_access\":\"2024-01-20\"}");

    TSharedPtr<bool> bGetMembershipsDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetMembershipsSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FPubnubGetMembershipsWrapper>> ReceivedMemberships = MakeShared<TArray<FPubnubGetMembershipsWrapper>>();
    TSharedPtr<FString> NextPageToken = MakeShared<FString>();
    TSharedPtr<FString> PrevPageToken = MakeShared<FString>();

    FOnGetMembershipsResponseNative GetMembershipsCallback;
    GetMembershipsCallback.BindLambda(
        [this, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships, NextPageToken, PrevPageToken]
        (int Status, const TArray<FPubnubGetMembershipsWrapper>& MembershipsData, FString PageNext, FString PagePrev)
    {
        *bGetMembershipsDone = true;
        *bGetMembershipsSuccess = (Status == 200);
        if (*bGetMembershipsSuccess)
        {
            *ReceivedMemberships = MembershipsData;
            *NextPageToken = PageNext;
            *PrevPageToken = PagePrev;
        }
        else
        {
            ReceivedMemberships->Empty();
            AddError(FString::Printf(TEXT("GetMemberships call failed. Status: %d. Next: '%s', Prev: '%s'"), Status, *PageNext, *PagePrev));
        }
    });

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubMembershipManagementWithOptionsTest");
        return false;
    }

    PubnubSubsystem->SetUserID(TestAdminUserID); // User performing Set/Get/Remove operations
    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("General Pubnub Error in MembershipManagementTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    // --- Initial Setup: Create Channel Metadata ---
    auto CreateChannelMeta = [this](const FString& ChanID, const FString& Meta)
    {
        ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChanID, Meta]()
        {
            PubnubSubsystem->SetChannelMetadata(ChanID, Meta, TEXT("custom,status,type"));
        }, 0.05f));
    };
    CreateChannelMeta(ChannelAID, ChannelAMetadata);
    CreateChannelMeta(ChannelBID, ChannelBMetadata);
    CreateChannelMeta(ChannelCID, ChannelCMetadata);
    CreateChannelMeta(ChannelDID, ChannelDMetadata);
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.5f)); // Wait for channel metadata setup

    // --- Scenario 1: SetMemberships ---
    // Constructing channel objects for SetMemberships payload based on previously set metadata
    const FString ChannelAContextForSet = FString::Printf(TEXT("{\"id\":\"%s\", \"name\":\"%s\", \"custom\":%s, \"status\":\"%s\"}"), *ChannelAID, *ChannelAName, *ChannelACustomJson, *ChannelAStatusVal);
    const FString ChannelBContextForSet = FString::Printf(TEXT("{\"id\":\"%s\", \"name\":\"%s\", \"custom\":%s, \"status\":\"%s\"}"), *ChannelBID, *ChannelBName, *ChannelBCustomJson, *ChannelBStatusVal);
    const FString ChannelCContextForSet = FString::Printf(TEXT("{\"id\":\"%s\", \"name\":\"%s\", \"custom\":%s, \"status\":\"%s\"}"), *ChannelCID, *ChannelCName, *ChannelCCustomJson, *ChannelCStatusVal);
    const FString ChannelDContextForSet = FString::Printf(TEXT("{\"id\":\"%s\", \"name\":\"%s\", \"custom\":%s, \"status\":\"%s\"}"), *ChannelDID, *ChannelDName, *ChannelDCustomJson, *ChannelDStatusVal);

    const FString SetMembershipsJson = FString::Printf(TEXT("[")
        TEXT("{\"channel\":%s, \"custom\":%s, \"status\":\"%s\", \"type\":\"%s\"},")
        TEXT("{\"channel\":%s, \"custom\":%s, \"status\":\"%s\"},") 
        TEXT("{\"channel\":%s, \"custom\":%s, \"type\":\"%s\"},")   
        TEXT("{\"channel\":%s, \"custom\":%s}") 
        TEXT("]"),
        *ChannelAContextForSet, *MembershipACustom, *MembershipAStatus, *MembershipAType,
        *ChannelBContextForSet, *MembershipBCustom, *MembershipBStatus,
        *ChannelCContextForSet, *MembershipCCustom, *MembershipCType,
        *ChannelDContextForSet, *MembershipDCustom
    );

    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, SetMembershipsJson]()
    {
        PubnubSubsystem->SetMemberships(TestUserID, SetMembershipsJson);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f)); // Wait for SetMemberships to process

    // --- Scenario 1b: Basic GetMemberships & Verification ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetMembershipsCallback, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships]()
    {
        *bGetMembershipsDone = false; *bGetMembershipsSuccess = false; ReceivedMemberships->Empty();
        FPubnubMembershipInclude IncludeAll; 
        IncludeAll.IncludeCustom = true; IncludeAll.IncludeStatus = true; IncludeAll.IncludeType = true;
        IncludeAll.IncludeChannel = true; IncludeAll.IncludeChannelCustom = true; IncludeAll.IncludeChannelStatus = true; IncludeAll.IncludeChannelType = true; // Although Channel Type is not explicitly set in our metadata example, include it.
        PubnubSubsystem->GetMemberships(TestUserID, GetMembershipsCallback, IncludeAll, 10);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembershipsDone]() { return *bGetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, bGetMembershipsSuccess, ChannelAID, ChannelAName, ChannelACustomJson, ChannelAStatusVal, MembershipACustom, MembershipAStatus, MembershipAType, ChannelBID, ChannelBName, ChannelBCustomJson, ChannelBStatusVal, MembershipBCustom, MembershipBStatus, ChannelCID, ChannelCName, ChannelCCustomJson, ChannelCStatusVal, MembershipCCustom, MembershipCType, ChannelDID, ChannelDName, ChannelDCustomJson, ChannelDStatusVal, MembershipDCustom]()
    {
        TestTrue("S1b: GetMemberships (initial) success.", *bGetMembershipsSuccess);
        if (!*bGetMembershipsSuccess) return;
        TestEqual("S1b: Expected 4 memberships.", ReceivedMemberships->Num(), 4);

        bool bFoundA = false, bFoundB = false, bFoundC = false, bFoundD = false;
        for (const auto& Membership : *ReceivedMemberships)
        {
            if (Membership.Channel.ChannelID == ChannelAID)
            {
                bFoundA = true;
                TestEqual("S1b_A: Channel Name", Membership.Channel.ChannelName, ChannelAName);
                TestTrue("S1b_A: Channel Custom", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Membership.Channel.Custom, ChannelACustomJson));
                TestEqual("S1b_A: Channel Status", Membership.Channel.Status, ChannelAStatusVal);
                TestTrue("S1b_A: Membership Custom", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Membership.Custom, MembershipACustom));
                TestEqual("S1b_A: Membership Status", Membership.Status, MembershipAStatus);
                TestEqual("S1b_A: Membership Type", Membership.Type, MembershipAType);
            }
            else if (Membership.Channel.ChannelID == ChannelBID)
            {
                bFoundB = true;
                TestEqual("S1b_B: Channel Name", Membership.Channel.ChannelName, ChannelBName);
                TestTrue("S1b_B: Channel Custom", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Membership.Channel.Custom, ChannelBCustomJson));
                TestEqual("S1b_B: Channel Status", Membership.Channel.Status, ChannelBStatusVal);
                TestTrue("S1b_B: Membership Custom", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Membership.Custom, MembershipBCustom));
                TestEqual("S1b_B: Membership Status", Membership.Status, MembershipBStatus);
                TestTrue("S1b_B: Membership Type (should be empty)", Membership.Type.IsEmpty());
            }
            else if (Membership.Channel.ChannelID == ChannelCID)
            {
                bFoundC = true;
                TestEqual("S1b_C: Channel Name", Membership.Channel.ChannelName, ChannelCName);
                TestTrue("S1b_C: Channel Custom", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Membership.Channel.Custom, ChannelCCustomJson));
                TestEqual("S1b_C: Channel Status", Membership.Channel.Status, ChannelCStatusVal);
                TestTrue("S1b_C: Membership Custom", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Membership.Custom, MembershipCCustom));
                TestTrue("S1b_C: Membership Status (should be empty)", Membership.Status.IsEmpty());
                TestEqual("S1b_C: Membership Type", Membership.Type, MembershipCType);
            }
            else if (Membership.Channel.ChannelID == ChannelDID)
			{
				bFoundD = true;
				TestEqual("S1b_D: Channel Name", Membership.Channel.ChannelName, ChannelDName);
                TestTrue("S1b_D: Channel Custom", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Membership.Channel.Custom, ChannelDCustomJson));
                TestEqual("S1b_D: Channel Status", Membership.Channel.Status, ChannelDStatusVal);
				TestTrue("S1b_D: Membership Custom", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Membership.Custom, MembershipDCustom));
				TestTrue("S1b_D: Membership Status (should be empty)", Membership.Status.IsEmpty());
				TestTrue("S1b_D: Membership Type (should be empty)", Membership.Type.IsEmpty());
			}
        }
        TestTrue("S1b: Found and verified Membership A.", bFoundA);
        TestTrue("S1b: Found and verified Membership B.", bFoundB);
        TestTrue("S1b: Found and verified Membership C.", bFoundC);
        TestTrue("S1b: Found and verified Membership D.", bFoundD);
    }, 0.1f));

    // --- Scenario 2: GetMemberships with Include Options ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetMembershipsCallback, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships, ChannelAID]()
    {
        *bGetMembershipsDone = false; *bGetMembershipsSuccess = false; ReceivedMemberships->Empty();
        FPubnubMembershipInclude IncludeOpts;
        IncludeOpts.IncludeCustom = true;      // Membership custom: YES
        IncludeOpts.IncludeStatus = false;     // Membership status: NO
        IncludeOpts.IncludeType = true;        // Membership type:  YES
        IncludeOpts.IncludeChannel = true;     // Channel basic data: YES
        IncludeOpts.IncludeChannelCustom = false; // Channel custom:   NO
        IncludeOpts.IncludeChannelStatus = true;  // Channel status:   YES
        FString Filter = FString::Printf(TEXT("channel.id == '%s'"), *ChannelAID); // Target Channel A
        PubnubSubsystem->GetMemberships(TestUserID, GetMembershipsCallback, IncludeOpts, 1, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembershipsDone]() { return *bGetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, bGetMembershipsSuccess, ChannelAID, ChannelAName, MembershipACustom, MembershipAType, ChannelAStatusVal /*Use ChannelAStatusVal here*/]()
    {
        TestTrue("S2: GetMemberships with specific includes success.", *bGetMembershipsSuccess);
        if (!*bGetMembershipsSuccess || ReceivedMemberships->Num() != 1)
        {
             if (*bGetMembershipsSuccess) AddError(FString::Printf(TEXT("S2: Expected 1 membership for include test, got %d"), ReceivedMemberships->Num()));
             return;
        }
        const auto& Membership = (*ReceivedMemberships)[0];
        TestEqual("S2: Channel ID", Membership.Channel.ChannelID, ChannelAID);
        TestEqual("S2: Channel Name (Included)", Membership.Channel.ChannelName, ChannelAName);
        TestTrue("S2: Channel Custom (NOT Included, should be empty)", Membership.Channel.Custom.IsEmpty());
        TestEqual("S2: Channel Status (Included)", Membership.Channel.Status, ChannelAStatusVal); // Compare with actual channel status
        
        TestTrue("S2: Membership Custom (Included)", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Membership.Custom, MembershipACustom));
        TestTrue("S2: Membership Status (NOT Included, should be empty)", Membership.Status.IsEmpty());
        TestEqual("S2: Membership Type (Included)", Membership.Type, MembershipAType);
    }, 0.1f));
    
    // --- Scenario 3: GetMemberships with Filtering ---
    // Filter by membership custom field
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetMembershipsCallback, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships]()
    {
        *bGetMembershipsDone = false; *bGetMembershipsSuccess = false; ReceivedMemberships->Empty();
        FPubnubMembershipInclude IncludeAll; IncludeAll.IncludeCustom = true; IncludeAll.IncludeChannel = true;
        FString Filter = TEXT("custom.role == 'editor'"); // Should only find Membership A
        PubnubSubsystem->GetMemberships(TestUserID, GetMembershipsCallback, IncludeAll, 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembershipsDone]() { return *bGetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, bGetMembershipsSuccess, ChannelAID]()
    {
        TestTrue("S3a: GetMemberships filter by custom.role success.", *bGetMembershipsSuccess);
        if (*bGetMembershipsSuccess)
        {
            TestEqual("S3a: Expected 1 membership for custom.role='editor'.", ReceivedMemberships->Num(), 1);
            if (ReceivedMemberships->Num() == 1)
            {
                TestEqual("S3a: Channel ID for 'editor' role", (*ReceivedMemberships)[0].Channel.ChannelID, ChannelAID);
            }
        }
    }, 0.1f));
    // Filter by channel status (which is part of channel metadata)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetMembershipsCallback, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships]()
    {
        *bGetMembershipsDone = false; *bGetMembershipsSuccess = false; ReceivedMemberships->Empty();
        FPubnubMembershipInclude IncludeChannelMeta; IncludeChannelMeta.IncludeChannel = true; IncludeChannelMeta.IncludeChannelStatus = true;
        FString Filter = TEXT("channel.status == 'archived'"); // Should only find Channel C membership
        PubnubSubsystem->GetMemberships(TestUserID, GetMembershipsCallback, IncludeChannelMeta, 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembershipsDone]() { return *bGetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, bGetMembershipsSuccess, ChannelCID]()
    {
        TestTrue("S3b: GetMemberships filter by channel.status success.", *bGetMembershipsSuccess);
        if (*bGetMembershipsSuccess)
        {
            TestEqual("S3b: Expected 1 membership for channel.status='archived'.", ReceivedMemberships->Num(), 1);
            if (ReceivedMemberships->Num() == 1)
            {
                TestEqual("S3b: Channel ID for 'archived' status", (*ReceivedMemberships)[0].Channel.ChannelID, ChannelCID);
            }
        }
    }, 0.1f));

    // --- Scenario 4: GetMemberships with Sorting ---
    // Sort by Channel Name (Ascending: B, A, C, D -> Dev Zone, Docs Central, General Disc, Support Hub) - Need to adjust for actual set names.
    // ChannelBName (Dev Zone), ChannelAName (Docs Central), ChannelCName (General Discussion), ChannelDName (Support Hub)
    // Sorted: ChannelAName, ChannelBName, ChannelCName, ChannelDName
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetMembershipsCallback, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships]()
    {
        *bGetMembershipsDone = false; *bGetMembershipsSuccess = false; ReceivedMemberships->Empty();
        FPubnubMembershipInclude IncludeChannelName; IncludeChannelName.IncludeChannel = true; IncludeChannelName.IncludeChannel = true;
        FPubnubMembershipSort SortSettings; SortSettings.MembershipSort.Add({EPubnubMembershipSortType::PMST_ChannelName, false /*asc*/});
        PubnubSubsystem->GetMemberships(TestUserID, GetMembershipsCallback, IncludeChannelName, 10, TEXT(""), SortSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembershipsDone]() { return *bGetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, bGetMembershipsSuccess, ChannelAID, ChannelBID, ChannelCID, ChannelDID]()
    {
        TestTrue("S4a: GetMemberships sort by channel.name:asc success.", *bGetMembershipsSuccess);
        if (*bGetMembershipsSuccess && ReceivedMemberships->Num() == 4)
        {
            TestEqual("S4a: Sort [0] (Dev)", (*ReceivedMemberships)[0].Channel.ChannelID, ChannelBID);
            TestEqual("S4a: Sort [1] (Docs)", (*ReceivedMemberships)[1].Channel.ChannelID, ChannelAID);
            TestEqual("S4a: Sort [2] (General)", (*ReceivedMemberships)[2].Channel.ChannelID, ChannelCID);
            TestEqual("S4a: Sort [3] (Support)", (*ReceivedMemberships)[3].Channel.ChannelID, ChannelDID);
        } else if (*bGetMembershipsSuccess) { AddError(FString::Printf(TEXT("S4a: Expected 4 memberships for sort, got %d"), ReceivedMemberships->Num())); }
    }, 0.1f));

    // --- Scenario 5: GetMemberships with Pagination ---
    // Using ChannelID:asc sort (A, B, C, D)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetMembershipsCallback, NextPageToken, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships]()
    {
        *bGetMembershipsDone = false; *bGetMembershipsSuccess = false; ReceivedMemberships->Empty(); NextPageToken->Empty();
        FPubnubMembershipSort SortSettings; SortSettings.MembershipSort.Add({EPubnubMembershipSortType::PMST_ChannelID, false});
        FPubnubMembershipInclude IncludeChannel; IncludeChannel.IncludeChannel = true; IncludeChannel.IncludeTotalCount = true;
        PubnubSubsystem->GetMemberships(TestUserID, GetMembershipsCallback, IncludeChannel, 2, TEXT(""), SortSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembershipsDone]() { return *bGetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, NextPageToken, bGetMembershipsSuccess, ChannelAID, ChannelBID]()
    {
        TestTrue("S5a: GetMemberships with Limit=2 (Page 1) success.", *bGetMembershipsSuccess);
        if (*bGetMembershipsSuccess)
        {
            TestEqual("S5a: Page 1 count.", ReceivedMemberships->Num(), 2);
            if (ReceivedMemberships->Num() == 2)
            {
                TestEqual("S5a: Page 1 Item [0]", (*ReceivedMemberships)[0].Channel.ChannelID, ChannelAID);
                TestEqual("S5a: Page 1 Item [1]", (*ReceivedMemberships)[1].Channel.ChannelID, ChannelBID);
            }
            TestFalse("S5a: NextPageToken should be populated as more results exist.", NextPageToken->IsEmpty());
        }
    }, 0.1f));
    // Fetch Page 2
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetMembershipsCallback, NextPageToken, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships]()
    {
        if (NextPageToken->IsEmpty()) { AddError("S5b: NextPageToken is empty, cannot fetch page 2."); *bGetMembershipsDone = true; return; }
        *bGetMembershipsDone = false; *bGetMembershipsSuccess = false; ReceivedMemberships->Empty();
        FString PageTokenToUse = *NextPageToken; NextPageToken->Empty();
        FPubnubMembershipSort SortSettings; SortSettings.MembershipSort.Add({EPubnubMembershipSortType::PMST_ChannelID, false});
        FPubnubMembershipInclude IncludeChannel; IncludeChannel.IncludeChannel = true;
        PubnubSubsystem->GetMemberships(TestUserID, GetMembershipsCallback, IncludeChannel, 2, TEXT(""), SortSettings, PageTokenToUse);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembershipsDone]() { return *bGetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, NextPageToken, bGetMembershipsSuccess, ChannelCID, ChannelDID]()
    {
        TestTrue("S5b: GetMemberships (Page 2) success.", *bGetMembershipsSuccess);
        if (*bGetMembershipsSuccess)
        {
            TestEqual("S5b: Page 2 count.", ReceivedMemberships->Num(), 2);
             if (ReceivedMemberships->Num() == 2)
            {
                TestEqual("S5b: Page 2 Item [0]", (*ReceivedMemberships)[0].Channel.ChannelID, ChannelCID);
                TestEqual("S5b: Page 2 Item [1]", (*ReceivedMemberships)[1].Channel.ChannelID, ChannelDID);
            }
        }
    }, 0.1f));

    // --- Scenario 6: RemoveMemberships ---
    // Remove membership for Channel A and Channel C
    const FString RemoveMembershipsJson = FString::Printf(TEXT("[")
        TEXT("{\"channel\":{\"id\":\"%s\"}},")
        TEXT("{\"channel\":{\"id\":\"%s\"}}") 
        TEXT("]"), *ChannelAID, *ChannelCID);
     ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, RemoveMembershipsJson]()
    {
        PubnubSubsystem->RemoveMemberships(TestUserID, RemoveMembershipsJson);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f));

    // Verify removal
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetMembershipsCallback, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships]()
    {
        *bGetMembershipsDone = false; *bGetMembershipsSuccess = false; ReceivedMemberships->Empty();
        FPubnubMembershipInclude IncludeChannel; IncludeChannel.IncludeChannel = true;
        PubnubSubsystem->GetMemberships(TestUserID, GetMembershipsCallback, IncludeChannel, 10);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembershipsDone]() { return *bGetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, bGetMembershipsSuccess, ChannelAID, ChannelBID, ChannelCID, ChannelDID]()
    {
        TestTrue("S6: GetMemberships after removal success.", *bGetMembershipsSuccess);
        if (!*bGetMembershipsSuccess) return;
        TestEqual("S6: Expected 2 memberships after removal.", ReceivedMemberships->Num(), 2);
        bool bFoundA = false, bFoundC = false;
        bool bFoundB = false, bFoundD = false;
        for (const auto& Membership : *ReceivedMemberships)
        {
            if (Membership.Channel.ChannelID == ChannelAID) bFoundA = true;
            else if (Membership.Channel.ChannelID == ChannelBID) bFoundB = true;
            else if (Membership.Channel.ChannelID == ChannelCID) bFoundC = true;
            else if (Membership.Channel.ChannelID == ChannelDID) bFoundD = true;
        }
        TestFalse("S6: Membership A should be removed.", bFoundA);
        TestTrue("S6: Membership B should still exist.", bFoundB);
        TestFalse("S6: Membership C should be removed.", bFoundC);
        TestTrue("S6: Membership D should still exist.", bFoundD);
    }, 0.1f));

    // Cleanup: Remove remaining memberships (B and D)
    const FString FinalRemoveJson = FString::Printf(TEXT("[{\"channel\":{\"id\":\"%s\"}},{\"channel\":{\"id\":\"%s\"}}]"), *ChannelBID, *ChannelDID);
     ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, FinalRemoveJson]()
    {
        PubnubSubsystem->RemoveMemberships(TestUserID, FinalRemoveJson);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(0.5f));
    
    // Cleanup: Remove channel metadata
    auto RemoveChannelMeta = [this](const FString& ChanID)
    {
        ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChanID]()
        {
            PubnubSubsystem->RemoveChannelMetadata(ChanID);
        }, 0.05f));
    };
    for(const FString& ChanID : AllTestChannelIDs)
    {
        RemoveChannelMeta(ChanID);
    }
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f));


    CleanUp();
    return true;
}


