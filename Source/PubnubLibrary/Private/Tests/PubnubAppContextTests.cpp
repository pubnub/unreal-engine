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
        FPubnubGetAllSort SortSettings; SortSettings.GetAllSort.Add({EPubnubGetAllSortType::PGAST_Name, false}); // Consistent order for limit
        PubnubSubsystem->GetAllChannelMetadata(GetAllCallback, FPubnubGetAllInclude(), 2, SortTestFilter, SortSettings);
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


