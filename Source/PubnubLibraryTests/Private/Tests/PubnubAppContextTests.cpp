// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubUserMetadataFlowTest, FPubnubAutomationTestBase, "Pubnub.Integration.AppContext.UserMetadataFlow", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubChannelMetadataFlowTest, FPubnubAutomationTestBase, "Pubnub.Integration.AppContext.ChannelMetadataFlow", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetAllChannelMetadataWithOptionsTest, FPubnubAutomationTestBase, "Pubnub.Integration.AppContext.GetAllChannelMetadataWithOptions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGetAllUsersMetadataWithOptionsTest, FPubnubAutomationTestBase, "Pubnub.Integration.AppContext.GetAllUsersMetadataWithOptions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubMembershipManagementWithOptionsTest, FPubnubAutomationTestBase, "Pubnub.Integration.AppContext.MembershipManagementWithOptions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubChannelMembersManagementWithOptionsTest, FPubnubAutomationTestBase, "Pubnub.Integration.AppContext.ChannelMembersManagementWithOptions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);


bool FPubnubUserMetadataFlowTest::RunTest(const FString& Parameters)
{
    // Initial variables
    const FString TestUserID = SDK_PREFIX + "appctx_user_meta_test";
    
    FPubnubUserData UserDataToSet;
    UserDataToSet.UserName = "E2E AppContext User";
    UserDataToSet.Email = "appcontext.user@pubnubsdk.test";
    UserDataToSet.ExternalID = SDK_PREFIX + "ext_id_123";
    UserDataToSet.ProfileUrl = "https://example.com/avatar.png";
    UserDataToSet.Status = "Online";
    UserDataToSet.Type = "PremiumUser";
    UserDataToSet.Custom = "{\"mood\": \"elated\", \"points\": 1000}";
    
    // Shared state for operation callbacks
    TSharedPtr<bool> bSetUserMetaDone = MakeShared<bool>(false);
    TSharedPtr<bool> bSetUserMetaSuccess = MakeShared<bool>(false);
    TSharedPtr<bool> bRemoveUserMetaDone = MakeShared<bool>(false);
    TSharedPtr<bool> bRemoveUserMetaSuccess = MakeShared<bool>(false);
    
    TSharedPtr<bool> bGetUserMetaDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetUserMetaSuccess = MakeShared<bool>(false);
    TSharedPtr<FPubnubUserData> ReceivedUserData = MakeShared<FPubnubUserData>();

    TSharedPtr<bool> bGetAllUserMetaDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetAllUserMetaSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FPubnubUserData>> ReceivedAllUsersData = MakeShared<TArray<FPubnubUserData>>();
    
    TSharedPtr<bool> bGetUserMetaAfterRemoveDone = MakeShared<bool>(false);
    TSharedPtr<int> GetUserMetaAfterRemoveStatus = MakeShared<int>(0);

    // Define callbacks
    FOnSetUserMetadataResponseNative SetUserMetadataCallback;
    SetUserMetadataCallback.BindLambda([this, bSetUserMetaDone, bSetUserMetaSuccess](const FPubnubOperationResult& Result, const FPubnubUserData& UserData)
    {
        *bSetUserMetaDone = true;
        if (!Result.Error && Result.Status == 200)
        {
            *bSetUserMetaSuccess = true;
        }
        else
        {
            AddError(FString::Printf(TEXT("SetUserMetadata failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    FOnRemoveUserMetadataResponseNative RemoveUserMetadataCallback;
    RemoveUserMetadataCallback.BindLambda([this, bRemoveUserMetaDone, bRemoveUserMetaSuccess](const FPubnubOperationResult& Result)
    {
        *bRemoveUserMetaDone = true;
        if (!Result.Error && Result.Status == 200)
        {
            *bRemoveUserMetaSuccess = true;
        }
        else
        {
            AddError(FString::Printf(TEXT("RemoveUserMetadata failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    FOnGetUserMetadataResponseNative GetUserMetadataCallback;
    GetUserMetadataCallback.BindLambda([this, bGetUserMetaDone, bGetUserMetaSuccess, ReceivedUserData](FPubnubOperationResult Result, FPubnubUserData UserData)
    {
        *bGetUserMetaDone = true;
        *bGetUserMetaSuccess = (Result.Status == 200);
        if (*bGetUserMetaSuccess)
        {
            *ReceivedUserData = UserData;
        }
        else
        {
            AddError(FString::Printf(TEXT("GetUserMetadata failed. Status: %d"), Result.Status));
        }
    });

    FOnGetAllUserMetadataResponseNative GetAllUserMetadataCallback;
    GetAllUserMetadataCallback.BindLambda([this, bGetAllUserMetaDone, bGetAllUserMetaSuccess, ReceivedAllUsersData](const FPubnubOperationResult& Result, const TArray<FPubnubUserData>& UsersData, FString PageNext, FString PagePrev)
    {
        *bGetAllUserMetaDone = true;
        *bGetAllUserMetaSuccess = (Result.Status == 200);
        if (*bGetAllUserMetaSuccess)
        {
            *ReceivedAllUsersData = UsersData;
        }
        else
        {
            AddError(FString::Printf(TEXT("GetAllUserMetadata failed. Status: %d. Next: %s, Prev: %s"), Result.Status, *PageNext, *PagePrev));
        }
    });
    
    FOnGetUserMetadataResponseNative GetUserMetadataCallback_AfterRemove;
    GetUserMetadataCallback_AfterRemove.BindLambda([this, bGetUserMetaAfterRemoveDone, GetUserMetaAfterRemoveStatus](FPubnubOperationResult Result, FPubnubUserData UserData)
    {
        *bGetUserMetaAfterRemoveDone = true;
        *GetUserMetaAfterRemoveStatus = Result.Status; // We expect this to be non-200 for a removed user
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
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, UserDataToSet, SetUserMetadataCallback, bSetUserMetaDone, bSetUserMetaSuccess]()
    {
        *bSetUserMetaDone = false;
        *bSetUserMetaSuccess = false;
        PubnubSubsystem->SetUserMetadata(TestUserID, UserDataToSet, SetUserMetadataCallback, FPubnubGetMetadataInclude::FromValue(true)); 
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSetUserMetaDone]() { return *bSetUserMetaDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSetUserMetaSuccess]()
    {
        TestTrue("SetUserMetadata should succeed", *bSetUserMetaSuccess);
    }, 0.1f));

    // Step 2: GetUserMetadata and Compare
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetUserMetadataCallback, bGetUserMetaDone, bGetUserMetaSuccess, ReceivedUserData]()
    {
        *bGetUserMetaDone = false;
        *bGetUserMetaSuccess = false;
        ReceivedUserData->UserID.Empty(); // Reset
        PubnubSubsystem->GetUserMetadata(TestUserID, GetUserMetadataCallback, FPubnubGetMetadataInclude::FromValue(true));
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetUserMetaDone]() { return *bGetUserMetaDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, UserDataToSet, ReceivedUserData, bGetUserMetaSuccess]()
    {
        TestTrue("GetUserMetadata operation was successful after Set.", *bGetUserMetaSuccess);
        if (*bGetUserMetaSuccess)
        {
            TestEqual("GetUserMetadata: UserID match", ReceivedUserData->UserID, TestUserID);
            TestEqual("GetUserMetadata: UserName match", ReceivedUserData->UserName, UserDataToSet.UserName);
            TestEqual("GetUserMetadata: Email match", ReceivedUserData->Email, UserDataToSet.Email);
            TestEqual("GetUserMetadata: ExternalID match", ReceivedUserData->ExternalID, UserDataToSet.ExternalID);
            TestEqual("GetUserMetadata: ProfileUrl match", ReceivedUserData->ProfileUrl, UserDataToSet.ProfileUrl);
            TestEqual("GetUserMetadata: Status match", ReceivedUserData->Status, UserDataToSet.Status);
            TestEqual("GetUserMetadata: Type match", ReceivedUserData->Type, UserDataToSet.Type);
            TestFalse("GetUserMetadata: Updated should not be empty", ReceivedUserData->Updated.IsEmpty());
            TestFalse("GetUserMetadata: ETag should not be empty", ReceivedUserData->ETag.IsEmpty());
            bool bCustomJsonMatch = UPubnubJsonUtilities::AreJsonObjectStringsEqual(ReceivedUserData->Custom, UserDataToSet.Custom);
            TestTrue("GetUserMetadata: Custom JSON match", bCustomJsonMatch);
            if(!bCustomJsonMatch)
            {
                AddError(FString::Printf(TEXT("Custom JSON mismatch. Expected: %s, Got: %s"), *UserDataToSet.Custom, *ReceivedUserData->Custom));
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
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, UserDataToSet, ReceivedAllUsersData, bGetAllUserMetaSuccess]()
    {
        TestTrue("GetAllUserMetadata (filtered) operation was successful.", *bGetAllUserMetaSuccess);
        if (*bGetAllUserMetaSuccess)
        {
            TestEqual("GetAllUserMetadata (filtered): Expected 1 user.", ReceivedAllUsersData->Num(), 1);
            if (ReceivedAllUsersData->Num() == 1)
            {
                const FPubnubUserData& User = (*ReceivedAllUsersData)[0];
                TestEqual("GetAllUserMetadata (filtered): UserID match", User.UserID, TestUserID);
                TestEqual("GetAllUserMetadata (filtered): UserName match", User.UserName, UserDataToSet.UserName);
                TestEqual("GetAllUserMetadata (filtered): Email match", User.Email, UserDataToSet.Email);
                TestEqual("GetAllUserMetadata (filtered): ExternalID match", User.ExternalID, UserDataToSet.ExternalID);
                TestEqual("GetAllUserMetadata (filtered): ProfileUrl match", User.ProfileUrl, UserDataToSet.ProfileUrl);
                TestEqual("GetAllUserMetadata (filtered): Status match", User.Status, UserDataToSet.Status);
                TestEqual("GetAllUserMetadata (filtered): Type match", User.Type, UserDataToSet.Type);
                TestFalse("GetAllUserMetadata (filtered): Updated should not be empty", User.Updated.IsEmpty());
                TestFalse("GetAllUserMetadata (filtered): ETag should not be empty", User.ETag.IsEmpty());
                bool bCustomJsonMatch = UPubnubJsonUtilities::AreJsonObjectStringsEqual(User.Custom, UserDataToSet.Custom);
                TestTrue("GetAllUserMetadata (filtered): Custom JSON match", bCustomJsonMatch);
                 if(!bCustomJsonMatch)
                {
                    AddError(FString::Printf(TEXT("Filtered GetAll Custom JSON mismatch. Expected: %s, Got: %s"), *UserDataToSet.Custom, *User.Custom));
                }
            }
        }
    }, 0.1f));
    
    // Step 4: RemoveUserMetadata
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, RemoveUserMetadataCallback, bRemoveUserMetaDone, bRemoveUserMetaSuccess]()
    {
        *bRemoveUserMetaDone = false;
        *bRemoveUserMetaSuccess = false;
        PubnubSubsystem->RemoveUserMetadata(TestUserID, RemoveUserMetadataCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveUserMetaDone]() { return *bRemoveUserMetaDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bRemoveUserMetaSuccess]()
    {
        TestTrue("RemoveUserMetadata should succeed", *bRemoveUserMetaSuccess);
    }, 0.1f));

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
    
    FPubnubChannelData ChannelDataToSet;
	ChannelDataToSet.ChannelID = SDK_PREFIX + "appctx_chan_meta_test";
    ChannelDataToSet.ChannelName = "E2E AppContext Channel";
    ChannelDataToSet.Description = "This is a test channel for App Context E2E tests.";
    ChannelDataToSet.Status = "Active";
    ChannelDataToSet.Type = "PublicDiscussion";
    ChannelDataToSet.Custom = "{\"topic\":\"testing\",\"moderated\":true}";

    // Shared state for operation callbacks
    TSharedPtr<bool> bSetChannelMetaDone = MakeShared<bool>(false);
    TSharedPtr<bool> bSetChannelMetaSuccess = MakeShared<bool>(false);
    TSharedPtr<bool> bRemoveChannelMetaDone = MakeShared<bool>(false);
    TSharedPtr<bool> bRemoveChannelMetaSuccess = MakeShared<bool>(false);

    TSharedPtr<bool> bGetChannelMetaDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetChannelMetaSuccess = MakeShared<bool>(false);
    TSharedPtr<FPubnubChannelData> ReceivedChannelData = MakeShared<FPubnubChannelData>();

    TSharedPtr<bool> bGetAllChannelMetaDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetAllChannelMetaSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FPubnubChannelData>> ReceivedAllChannelsData = MakeShared<TArray<FPubnubChannelData>>();

    TSharedPtr<bool> bGetChannelMetaAfterRemoveDone = MakeShared<bool>(false);
    TSharedPtr<int> GetChannelMetaAfterRemoveStatus = MakeShared<int>(0);

    // Define callbacks
    FOnSetChannelMetadataResponseNative SetChannelMetadataCallback;
    SetChannelMetadataCallback.BindLambda([this, bSetChannelMetaDone, bSetChannelMetaSuccess](const FPubnubOperationResult& Result, const FPubnubChannelData& ChannelData)
    {
        *bSetChannelMetaDone = true;
        if (!Result.Error && Result.Status == 200)
        {
            *bSetChannelMetaSuccess = true;
        }
        else
        {
            AddError(FString::Printf(TEXT("SetChannelMetadata failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    FOnRemoveChannelMetadataResponseNative RemoveChannelMetadataCallback;
    RemoveChannelMetadataCallback.BindLambda([this, bRemoveChannelMetaDone, bRemoveChannelMetaSuccess](const FPubnubOperationResult& Result)
    {
        *bRemoveChannelMetaDone = true;
        if (!Result.Error && Result.Status == 200)
        {
            *bRemoveChannelMetaSuccess = true;
        }
        else
        {
            AddError(FString::Printf(TEXT("RemoveChannelMetadata failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    FOnGetChannelMetadataResponseNative GetChannelMetadataCallback;
    GetChannelMetadataCallback.BindLambda([this, bGetChannelMetaDone, bGetChannelMetaSuccess, ReceivedChannelData](const FPubnubOperationResult& Result, FPubnubChannelData ChannelData)
    {
        *bGetChannelMetaDone = true;
        *bGetChannelMetaSuccess = (Result.Status == 200);
        if (*bGetChannelMetaSuccess)
        {
            *ReceivedChannelData = ChannelData;
        }
        else
        {
            AddError(FString::Printf(TEXT("GetChannelMetadata failed. Status: %d"), Result.Status));
        }
    });

    FOnGetAllChannelMetadataResponseNative GetAllChannelMetadataCallback;
    GetAllChannelMetadataCallback.BindLambda([this, bGetAllChannelMetaDone, bGetAllChannelMetaSuccess, ReceivedAllChannelsData](const FPubnubOperationResult& Result, const TArray<FPubnubChannelData>& ChannelsData, FString PageNext, FString PagePrev)
    {
        *bGetAllChannelMetaDone = true;
        *bGetAllChannelMetaSuccess = (Result.Status == 200);
        if (*bGetAllChannelMetaSuccess)
        {
            *ReceivedAllChannelsData = ChannelsData;
        }
        else
        {
            AddError(FString::Printf(TEXT("GetAllChannelMetadata failed. Status: %d. Next: %s, Prev: %s"), Result.Status, *PageNext, *PagePrev));
        }
    });

    FOnGetChannelMetadataResponseNative GetChannelMetadataCallback_AfterRemove;
    GetChannelMetadataCallback_AfterRemove.BindLambda([this, bGetChannelMetaAfterRemoveDone, GetChannelMetaAfterRemoveStatus](const FPubnubOperationResult& Result, FPubnubChannelData ChannelData)
    {
        *bGetChannelMetaAfterRemoveDone = true;
        *GetChannelMetaAfterRemoveStatus = Result.Status;
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
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelDataToSet, SetChannelMetadataCallback, bSetChannelMetaDone, bSetChannelMetaSuccess]()
    {
        *bSetChannelMetaDone = false;
        *bSetChannelMetaSuccess = false;
        PubnubSubsystem->SetChannelMetadata(ChannelDataToSet.ChannelID, ChannelDataToSet, SetChannelMetadataCallback, FPubnubGetMetadataInclude::FromValue(true));
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSetChannelMetaDone]() { return *bSetChannelMetaDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSetChannelMetaSuccess]()
    {
        TestTrue("SetChannelMetadata should succeed", *bSetChannelMetaSuccess);
    }, 0.1f));

    // Step 2: GetChannelMetadata and Compare
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelDataToSet, GetChannelMetadataCallback, bGetChannelMetaDone, bGetChannelMetaSuccess, ReceivedChannelData]()
    {
        *bGetChannelMetaDone = false;
        *bGetChannelMetaSuccess = false;
        ReceivedChannelData->ChannelID.Empty(); // Reset
        PubnubSubsystem->GetChannelMetadata(ChannelDataToSet.ChannelID, GetChannelMetadataCallback, FPubnubGetMetadataInclude::FromValue(true));
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetChannelMetaDone]() { return *bGetChannelMetaDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelDataToSet, ReceivedChannelData, bGetChannelMetaSuccess]()
    {
        TestTrue("GetChannelMetadata operation was successful after Set.", *bGetChannelMetaSuccess);
        if (*bGetChannelMetaSuccess)
        {
            TestEqual("GetChannelMetadata: ChannelID match", ReceivedChannelData->ChannelID, ChannelDataToSet.ChannelID);
            TestEqual("GetChannelMetadata: ChannelName match", ReceivedChannelData->ChannelName, ChannelDataToSet.ChannelName);
            TestEqual("GetChannelMetadata: Description match", ReceivedChannelData->Description, ChannelDataToSet.Description);
            TestEqual("GetChannelMetadata: Status match", ReceivedChannelData->Status, ChannelDataToSet.Status);
            TestEqual("GetChannelMetadata: Type match", ReceivedChannelData->Type, ChannelDataToSet.Type);
            TestFalse("GetChannelMetadata: Updated should not be empty", ReceivedChannelData->Updated.IsEmpty());
            TestFalse("GetChannelMetadata: ETag should not be empty", ReceivedChannelData->ETag.IsEmpty());
            bool bCustomJsonMatch = UPubnubJsonUtilities::AreJsonObjectStringsEqual(ReceivedChannelData->Custom, ChannelDataToSet.Custom);
            TestTrue("GetChannelMetadata: Custom JSON match", bCustomJsonMatch);
            if(!bCustomJsonMatch)
            {
                AddError(FString::Printf(TEXT("Custom JSON mismatch. Expected: %s, Got: %s"), *ChannelDataToSet.Custom, *ReceivedChannelData->Custom));
            }
        }
    }, 0.1f));

    // Step 3: GetAllChannelMetadata with filter
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelDataToSet, GetAllChannelMetadataCallback, bGetAllChannelMetaDone, bGetAllChannelMetaSuccess, ReceivedAllChannelsData]()
    {
        *bGetAllChannelMetaDone = false;
        *bGetAllChannelMetaSuccess = false;
        ReceivedAllChannelsData->Empty();
        FPubnubGetAllInclude IncludeSettings;
        IncludeSettings.IncludeCustom = true;
        IncludeSettings.IncludeStatus = true;
        IncludeSettings.IncludeType = true;
        FString Filter = FString::Printf(TEXT("id == '%s'"), *ChannelDataToSet.ChannelID);
        PubnubSubsystem->GetAllChannelMetadata(GetAllChannelMetadataCallback, IncludeSettings, 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllChannelMetaDone]() { return *bGetAllChannelMetaDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelDataToSet, ReceivedAllChannelsData, bGetAllChannelMetaSuccess]()
    {
        TestTrue("GetAllChannelMetadata (filtered) operation was successful.", *bGetAllChannelMetaSuccess);
        if (*bGetAllChannelMetaSuccess)
        {
            TestEqual("GetAllChannelMetadata (filtered): Expected 1 channel.", ReceivedAllChannelsData->Num(), 1);
            if (ReceivedAllChannelsData->Num() == 1)
            {
                const FPubnubChannelData& Channel = (*ReceivedAllChannelsData)[0];
                TestEqual("GetAllChannelMetadata (filtered): ChannelID match", Channel.ChannelID, ChannelDataToSet.ChannelID);
                TestEqual("GetAllChannelMetadata (filtered): ChannelName match", Channel.ChannelName, ChannelDataToSet.ChannelName);
                TestEqual("GetAllChannelMetadata (filtered): Description match", Channel.Description, ChannelDataToSet.Description);
                TestEqual("GetAllChannelMetadata (filtered): Status match", Channel.Status, ChannelDataToSet.Status);
                TestEqual("GetAllChannelMetadata (filtered): Type match", Channel.Type, ChannelDataToSet.Type);
                TestFalse("GetAllChannelMetadata (filtered): Updated should not be empty", Channel.Updated.IsEmpty());
                TestFalse("GetAllChannelMetadata (filtered): ETag should not be empty", Channel.ETag.IsEmpty());
                bool bCustomJsonMatch = UPubnubJsonUtilities::AreJsonObjectStringsEqual(Channel.Custom, ChannelDataToSet.Custom);
                TestTrue("GetAllChannelMetadata (filtered): Custom JSON match", bCustomJsonMatch);
                 if(!bCustomJsonMatch)
                {
                    AddError(FString::Printf(TEXT("Filtered GetAll Custom JSON mismatch. Expected: %s, Got: %s"), *ChannelDataToSet.Custom, *Channel.Custom));
                }
            }
        }
    }, 0.1f));
    
    // Step 4: RemoveChannelMetadata
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelDataToSet, RemoveChannelMetadataCallback, bRemoveChannelMetaDone, bRemoveChannelMetaSuccess]()
    {
        *bRemoveChannelMetaDone = false;
        *bRemoveChannelMetaSuccess = false;
        PubnubSubsystem->RemoveChannelMetadata(ChannelDataToSet.ChannelID, RemoveChannelMetadataCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveChannelMetaDone]() { return *bRemoveChannelMetaDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bRemoveChannelMetaSuccess]()
    {
        TestTrue("RemoveChannelMetadata should succeed", *bRemoveChannelMetaSuccess);
    }, 0.1f));

    // Step 6: Verify Removal (using GetAllChannelMetadata Filtered)
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelDataToSet, GetAllChannelMetadataCallback, bGetAllChannelMetaDone, bGetAllChannelMetaSuccess, ReceivedAllChannelsData]()
    {
        *bGetAllChannelMetaDone = false;
        *bGetAllChannelMetaSuccess = false; // Reset for this call
        ReceivedAllChannelsData->Empty();
        FPubnubGetAllInclude IncludeSettings;
        IncludeSettings.IncludeCustom = true;
        IncludeSettings.IncludeStatus = true;
        IncludeSettings.IncludeType = true;
        FString Filter = FString::Printf(TEXT("id == '%s'"), *ChannelDataToSet.ChannelID);
        PubnubSubsystem->GetAllChannelMetadata(GetAllChannelMetadataCallback, IncludeSettings, 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllChannelMetaDone]() { return *bGetAllChannelMetaDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedAllChannelsData, bGetAllChannelMetaSuccess, ChannelDataToSet]()
    {
        TestTrue("GetAllChannelMetadata (filtered, after remove) operation was successful (API call itself).", *bGetAllChannelMetaSuccess);
        if (*bGetAllChannelMetaSuccess)
        {
            TestEqual("GetAllChannelMetadata (filtered, after remove): Expected 0 channels.", ReceivedAllChannelsData->Num(), 0);
            if (ReceivedAllChannelsData->Num() != 0)
            {
                AddError(FString::Printf(TEXT("Channel metadata with ID '%s' was still found via GetAllChannelMetadata after RemoveChannelMetadata was called."), *ChannelDataToSet.ChannelID));
            }
        }
    }, 0.1f));

    CleanUp();
    return true;
}

bool FPubnubGetAllChannelMetadataWithOptionsTest::RunTest(const FString& Parameters)
{
    const FString TestRunPrefix = SDK_PREFIX + "gacm_opts_";
    const FString TestUserID = TestRunPrefix + "user";

    // Channel A
    FPubnubChannelData ChannelAData;
    ChannelAData.ChannelID = TestRunPrefix + "ChannelA";
    ChannelAData.ChannelName = "Channel A - Tech";
    ChannelAData.Custom = "{\"category\":\"Tech\", \"priority\":1}";
    ChannelAData.Status = "Active";
    ChannelAData.Type = "TechnicalDiscussion";

    // Channel B
    FPubnubChannelData ChannelBData;
    ChannelBData.ChannelID = TestRunPrefix + "ChannelB";
    ChannelBData.ChannelName = "Channel B - Finance";
    ChannelBData.Custom = "{\"category\":\"Finance\", \"priority\":2}";

    // Channels for Sorting & Limit Test
    const FString ChannelSortPrefix = TestRunPrefix + "SortChan_";
    FPubnubChannelData ChannelSortAData;
    ChannelSortAData.ChannelID = ChannelSortPrefix + "Alpha";
    ChannelSortAData.ChannelName = "Sort Channel Alpha";

    FPubnubChannelData ChannelSortBData;
    ChannelSortBData.ChannelID = ChannelSortPrefix + "Beta";
    ChannelSortBData.ChannelName = "Sort Channel Beta";

    FPubnubChannelData ChannelSortCData;
    ChannelSortCData.ChannelID = ChannelSortPrefix + "Gamma";
    ChannelSortCData.ChannelName = "Sort Channel Gamma";

    // Shared state for operations
    TSharedPtr<bool> bGetAllDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetAllSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FPubnubChannelData>> ReceivedChannels = MakeShared<TArray<FPubnubChannelData>>();
    TSharedPtr<FString> NextPage = MakeShared<FString>();
    TSharedPtr<FString> PrevPage = MakeShared<FString>();

    // Shared state for setup operations
    TSharedPtr<int> SetupCounter = MakeShared<int>(0);
    TSharedPtr<int> SetupErrors = MakeShared<int>(0);
    const int TotalSetupOperations = 5; // 5 channels to set up

    // Shared state for cleanup operations
    TSharedPtr<int> CleanupCounter = MakeShared<int>(0);
    TSharedPtr<int> CleanupErrors = MakeShared<int>(0);
    const int TotalCleanupOperations = 5; // 5 channels to clean up

    // Callback for GetAllChannelMetadata
    FOnGetAllChannelMetadataResponseNative GetAllCallback;
    GetAllCallback.BindLambda([this, bGetAllDone, bGetAllSuccess, ReceivedChannels, NextPage, PrevPage](const FPubnubOperationResult& Result, const TArray<FPubnubChannelData>& ChannelsData, FString PageNextStr, FString PagePrevStr)
    {
        *bGetAllDone = true;
        *bGetAllSuccess = (Result.Status == 200);
        if (*bGetAllSuccess)
        {
            *ReceivedChannels = ChannelsData;
            *NextPage = PageNextStr;
            *PrevPage = PagePrevStr;
        }
        else
        {
            ReceivedChannels->Empty();
            AddError(FString::Printf(TEXT("GetAllChannelMetadata call failed. Status: %d. Next: '%s', Prev: '%s'"), Result.Status, *PageNextStr, *PagePrevStr));
        }
    });

    // Callback for SetChannelMetadata operations
    FOnSetChannelMetadataResponseNative SetChannelMetadataCallback;
    SetChannelMetadataCallback.BindLambda([this, SetupCounter, SetupErrors](const FPubnubOperationResult& Result, const FPubnubChannelData& ChannelData)
    {
        (*SetupCounter)++;
        if (Result.Error || Result.Status != 200)
        {
            (*SetupErrors)++;
            AddError(FString::Printf(TEXT("SetChannelMetadata failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    // Callback for RemoveChannelMetadata operations
    FOnRemoveChannelMetadataResponseNative RemoveChannelMetadataCallback;
    RemoveChannelMetadataCallback.BindLambda([this, CleanupCounter, CleanupErrors](const FPubnubOperationResult& Result)
    {
        (*CleanupCounter)++;
        if (Result.Error || Result.Status != 200)
        {
            (*CleanupErrors)++;
            AddError(FString::Printf(TEXT("RemoveChannelMetadata failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
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

    // Initial Setup: Set metadata for all test channels using callbacks
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelAData, SetChannelMetadataCallback, SetupCounter, SetupErrors]()
    {
        *SetupCounter = 0;
        *SetupErrors = 0;
        PubnubSubsystem->SetChannelMetadata(ChannelAData.ChannelID, ChannelAData, SetChannelMetadataCallback, FPubnubGetMetadataInclude::FromValue(true));
    }, 0.1f));

    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelBData, SetChannelMetadataCallback]()
    {
        PubnubSubsystem->SetChannelMetadata(ChannelBData.ChannelID, ChannelBData, SetChannelMetadataCallback, FPubnubGetMetadataInclude(true, false, false));
    }, 0.1f));

    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelSortAData, SetChannelMetadataCallback]()
    {
        PubnubSubsystem->SetChannelMetadata(ChannelSortAData.ChannelID, ChannelSortAData, SetChannelMetadataCallback, FPubnubGetMetadataInclude(true, false, false));
    }, 0.1f));

    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelSortBData, SetChannelMetadataCallback]()
    {
        PubnubSubsystem->SetChannelMetadata(ChannelSortBData.ChannelID, ChannelSortBData, SetChannelMetadataCallback, FPubnubGetMetadataInclude(true, false, false));
    }, 0.1f));

    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelSortCData, SetChannelMetadataCallback]()
    {
        PubnubSubsystem->SetChannelMetadata(ChannelSortCData.ChannelID, ChannelSortCData, SetChannelMetadataCallback, FPubnubGetMetadataInclude(true, false, false));
    }, 0.1f));

    // Wait for all setup operations to complete
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([SetupCounter, TotalSetupOperations]() { return *SetupCounter >= TotalSetupOperations; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SetupErrors]()
    {
        TestEqual("Setup operations should complete without errors", *SetupErrors, 0);
    }, 0.1f));

    // --- Scenario 1: Filter by a Custom Field ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, bGetAllDone, bGetAllSuccess, ReceivedChannels]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedChannels->Empty();
        FPubnubGetAllInclude IncludeSettings; IncludeSettings.IncludeCustom = true;
        FString Filter = "custom.category == 'Tech'";
        PubnubSubsystem->GetAllChannelMetadata(GetAllCallback, IncludeSettings, 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedChannels, bGetAllSuccess, ChannelAData]()
    {
        TestTrue("S1: GetAllChannelMetadata with custom filter success.", *bGetAllSuccess);
        if(*bGetAllSuccess)
        {
            bool bFoundChannelA = false;
            for(const auto& Chan : *ReceivedChannels) { if(Chan.ChannelID == ChannelAData.ChannelID) { bFoundChannelA = true; break; } }
            TestTrue(FString::Printf(TEXT("S1: Channel A (ID: %s) should be in results for custom.category == 'Tech'. Count: %d"), *ChannelAData.ChannelID, ReceivedChannels->Num()), bFoundChannelA);
            if (bFoundChannelA && ReceivedChannels->Num() == 1) // Stricter check if only one is expected
            {
                 TestEqual("S1: Channel A Name match", (*ReceivedChannels)[0].ChannelName, ChannelAData.ChannelName);
                 TestTrue("S1: Channel A Custom match", UPubnubJsonUtilities::AreJsonObjectStringsEqual((*ReceivedChannels)[0].Custom, ChannelAData.Custom));
            }
             else if (ReceivedChannels->Num() > 1) AddWarning("S1: Filter custom.category == 'Tech' returned multiple channels. Check keyset for conflicts.");
        }
    }, 0.1f));

    // --- Scenario 2: Filter by Name ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, ChannelAData, bGetAllDone, bGetAllSuccess, ReceivedChannels]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedChannels->Empty();
        FString Filter = FString::Printf(TEXT("name == '%s'"), *ChannelAData.ChannelName);
        PubnubSubsystem->GetAllChannelMetadata(GetAllCallback, FPubnubGetAllInclude(), 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedChannels, bGetAllSuccess, ChannelAData]()
    {
        TestTrue("S2: GetAllChannelMetadata with name filter success.", *bGetAllSuccess);
        if(*bGetAllSuccess)
        {
             bool bFoundChannelA = false;
             for(const auto& Chan : *ReceivedChannels) { if(Chan.ChannelID == ChannelAData.ChannelID) { bFoundChannelA = true; break; } }
             TestTrue(FString::Printf(TEXT("S2: Channel A (ID: %s) should be in results for name filter. Count: %d"), *ChannelAData.ChannelID, ReceivedChannels->Num()), bFoundChannelA);
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
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedChannels, bGetAllSuccess, ChannelSortAData, ChannelSortBData, ChannelSortCData]()
    {
        TestTrue("S3a: GetAllChannelMetadata with name:asc sort success.", *bGetAllSuccess);
        if(*bGetAllSuccess && ReceivedChannels->Num() == 3)
        {
            TestEqual("S3a: Sort Order [0] ID", (*ReceivedChannels)[0].ChannelID, ChannelSortAData.ChannelID);
            TestEqual("S3a: Sort Order [1] ID", (*ReceivedChannels)[1].ChannelID, ChannelSortBData.ChannelID);
            TestEqual("S3a: Sort Order [2] ID", (*ReceivedChannels)[2].ChannelID, ChannelSortCData.ChannelID);
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
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedChannels, bGetAllSuccess, ChannelSortAData, ChannelSortBData, ChannelSortCData]()
    {
        TestTrue("S3b: GetAllChannelMetadata with name:desc sort success.", *bGetAllSuccess);
        if(*bGetAllSuccess && ReceivedChannels->Num() == 3)
        {
            TestEqual("S3b: Sort Order [0] ID", (*ReceivedChannels)[0].ChannelID, ChannelSortCData.ChannelID);
            TestEqual("S3b: Sort Order [1] ID", (*ReceivedChannels)[1].ChannelID, ChannelSortBData.ChannelID);
            TestEqual("S3b: Sort Order [2] ID", (*ReceivedChannels)[2].ChannelID, ChannelSortAData.ChannelID);
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
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, ChannelAData, bGetAllDone, bGetAllSuccess, ReceivedChannels]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedChannels->Empty();
        FPubnubGetAllInclude IncludeSettings; 
        IncludeSettings.IncludeCustom = true;
        IncludeSettings.IncludeStatus = true;
        IncludeSettings.IncludeType = true;
        FString Filter = FString::Printf(TEXT("id == '%s'"), *ChannelAData.ChannelID);
        PubnubSubsystem->GetAllChannelMetadata(GetAllCallback, IncludeSettings, 1, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedChannels, bGetAllSuccess, ChannelAData]()
    {
        TestTrue("S5: GetAllChannelMetadata with specific Includes success.", *bGetAllSuccess);
        if(*bGetAllSuccess && ReceivedChannels->Num() == 1)
        {
            const auto& Chan = (*ReceivedChannels)[0];
            TestTrue("S5: Custom data match", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Chan.Custom, ChannelAData.Custom));
            TestEqual("S5: Status match", Chan.Status, ChannelAData.Status);
            TestEqual("S5: Type match", Chan.Type, ChannelAData.Type);
        }
        else if(*bGetAllSuccess) AddError(FString::Printf(TEXT("S5: Expected 1 channel for include test, got %d"), ReceivedChannels->Num()));
    }, 0.1f));

    // Cleanup: Remove metadata for all test channels using callbacks
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelAData, RemoveChannelMetadataCallback, CleanupCounter, CleanupErrors]()
    {
        *CleanupCounter = 0;
        *CleanupErrors = 0;
        PubnubSubsystem->RemoveChannelMetadata(ChannelAData.ChannelID, RemoveChannelMetadataCallback);
    }, 0.1f));

    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelBData, RemoveChannelMetadataCallback]()
    {
        PubnubSubsystem->RemoveChannelMetadata(ChannelBData.ChannelID, RemoveChannelMetadataCallback);
    }, 0.1f));

    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelSortAData, RemoveChannelMetadataCallback]()
    {
        PubnubSubsystem->RemoveChannelMetadata(ChannelSortAData.ChannelID, RemoveChannelMetadataCallback);
    }, 0.1f));

    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelSortBData, RemoveChannelMetadataCallback]()
    {
        PubnubSubsystem->RemoveChannelMetadata(ChannelSortBData.ChannelID, RemoveChannelMetadataCallback);
    }, 0.1f));

    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ChannelSortCData, RemoveChannelMetadataCallback]()
    {
        PubnubSubsystem->RemoveChannelMetadata(ChannelSortCData.ChannelID, RemoveChannelMetadataCallback);
    }, 0.1f));

    // Wait for all cleanup operations to complete
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([CleanupCounter, TotalCleanupOperations]() { return *CleanupCounter >= TotalCleanupOperations; }, MAX_WAIT_TIME));

    CleanUp();
    return true;
}

bool FPubnubGetAllUsersMetadataWithOptionsTest::RunTest(const FString& Parameters)
{
    const FString TestRunPrefix = SDK_PREFIX + "gaum_opts_";
    const FString TestAdminUserID = TestRunPrefix + "admin_user";

    // User A
    FPubnubUserData UserAData;
    UserAData.UserID = TestRunPrefix + "UserA";
    UserAData.UserName = "User A - Engineering Dept";
    UserAData.Email = "user.a@example.com";
    UserAData.Custom = "{\"department\":\"Engineering\", \"level\":5, \"active_project\":\"ProjectPhoenix\"}";
    UserAData.Status = "ActiveProject";
    UserAData.Type = "Engineer";

    // User B
    FPubnubUserData UserBData;
    UserBData.UserID = TestRunPrefix + "UserB";
    UserBData.UserName = "User B - Marketing Dept";
    UserBData.Email = "user.b@example.com";
    UserBData.Custom = "{\"department\":\"Marketing\", \"level\":3, \"campaign\":\"SummerSale\"}";

    // Users for Sorting & Limit Test
    const FString UserSortPrefix = TestRunPrefix + "SortUser_";
    FPubnubUserData UserSortAData;
    UserSortAData.UserID = UserSortPrefix + "Charlie";
    UserSortAData.UserName = "Charlie Brown";

    FPubnubUserData UserSortBData;
    UserSortBData.UserID = UserSortPrefix + "Alice";
    UserSortBData.UserName = "Alice Wonderland";

    FPubnubUserData UserSortCData;
    UserSortCData.UserID = UserSortPrefix + "Bob";
    UserSortCData.UserName = "Bob The Builder";
    
    FPubnubUserData UserSortDData;
    UserSortDData.UserID = UserSortPrefix + "David";
    UserSortDData.UserName = "David Copperfield";

    TArray<FPubnubUserData> AllTestUsers = {UserAData, UserBData, UserSortAData, UserSortBData, UserSortCData, UserSortDData};

    // Shared state for operations
    TSharedPtr<bool> bGetAllDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetAllSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FPubnubUserData>> ReceivedUsers = MakeShared<TArray<FPubnubUserData>>();
    TSharedPtr<FString> NextPage = MakeShared<FString>();
    TSharedPtr<FString> PrevPage = MakeShared<FString>();

    // Shared state for setup operations
    TSharedPtr<int> SetupCounter = MakeShared<int>(0);
    TSharedPtr<int> SetupErrors = MakeShared<int>(0);
    const int TotalSetupOperations = AllTestUsers.Num(); // 6 users to set up

    // Shared state for cleanup operations
    TSharedPtr<int> CleanupCounter = MakeShared<int>(0);
    TSharedPtr<int> CleanupErrors = MakeShared<int>(0);
    const int TotalCleanupOperations = AllTestUsers.Num(); // 6 users to clean up

    // Callback for GetAllUserMetadata
    FOnGetAllUserMetadataResponseNative GetAllCallback;
    GetAllCallback.BindLambda([this, bGetAllDone, bGetAllSuccess, ReceivedUsers, NextPage, PrevPage](const FPubnubOperationResult& Result, const TArray<FPubnubUserData>& UsersData, FString PageNextStr, FString PagePrevStr)
    {
        *bGetAllDone = true;
        *bGetAllSuccess = (Result.Status == 200);
        if (*bGetAllSuccess)
        {
            *ReceivedUsers = UsersData;
            *NextPage = PageNextStr;
            *PrevPage = PagePrevStr;
        }
        else
        {
            ReceivedUsers->Empty();
            AddError(FString::Printf(TEXT("GetAllUserMetadata call failed. Status: %d. Next: '%s', Prev: '%s'"), Result.Status, *PageNextStr, *PagePrevStr));
        }
    });

    // Callback for SetUserMetadata operations
    FOnSetUserMetadataResponseNative SetUserMetadataCallback;
    SetUserMetadataCallback.BindLambda([this, SetupCounter, SetupErrors](const FPubnubOperationResult& Result, const FPubnubUserData& UserData)
    {
        (*SetupCounter)++;
        if (Result.Error || Result.Status != 200)
        {
            (*SetupErrors)++;
            AddError(FString::Printf(TEXT("SetUserMetadata failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    // Callback for RemoveUserMetadata operations
    FOnRemoveUserMetadataResponseNative RemoveUserMetadataCallback;
    RemoveUserMetadataCallback.BindLambda([this, CleanupCounter, CleanupErrors](const FPubnubOperationResult& Result)
    {
        (*CleanupCounter)++;
        if (Result.Error || Result.Status != 200)
        {
            (*CleanupErrors)++;
            AddError(FString::Printf(TEXT("RemoveUserMetadata failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
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

    // Initial Setup: Set metadata for all test users using callbacks
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SetUserMetadataCallback, SetupCounter, SetupErrors, AllTestUsers]()
    {
        *SetupCounter = 0;
        *SetupErrors = 0;
        
        // Set metadata for all users
        for (const auto& User : AllTestUsers)
        {
            PubnubSubsystem->SetUserMetadata(User.UserID, User, SetUserMetadataCallback, FPubnubGetMetadataInclude::FromValue(true));
        }
    }, 0.1f));

    // Wait for all setup operations to complete
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([SetupCounter, TotalSetupOperations]() { return *SetupCounter >= TotalSetupOperations; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SetupErrors]()
    {
        TestEqual("Setup operations should complete without errors", *SetupErrors, 0);
    }, 0.1f));

    // --- Scenario 1: Filter by a Custom Field ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, bGetAllDone, bGetAllSuccess, ReceivedUsers]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedUsers->Empty();
        FPubnubGetAllInclude IncludeSettings; IncludeSettings.IncludeCustom = true;
        FString Filter = "custom.department == 'Engineering'";
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, IncludeSettings, 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, bGetAllSuccess, UserAData]()
    {
        TestTrue("S1: GetAllUserMetadata with custom.department filter success.", *bGetAllSuccess);
        if(*bGetAllSuccess)
        {
            bool bFoundUserA = false;
            for(const auto& User : *ReceivedUsers) { if(User.UserID == UserAData.UserID) { bFoundUserA = true; break; } }
            TestTrue(FString::Printf(TEXT("S1: User A (ID: %s) should be in results for custom.department == 'Engineering'. Count: %d"), *UserAData.UserID, ReceivedUsers->Num()), bFoundUserA);
            if (bFoundUserA && ReceivedUsers->Num() == 1)
            {
                 TestEqual("S1: User A Name match", (*ReceivedUsers)[0].UserName, UserAData.UserName);
                 TestTrue("S1: User A Custom match", UPubnubJsonUtilities::AreJsonObjectStringsEqual((*ReceivedUsers)[0].Custom, UserAData.Custom));
            }
             else if (ReceivedUsers->Num() > 1) AddWarning("S1: Filter custom.department == 'Engineering' returned multiple users. Check keyset for conflicts.");
        }
    }, 0.1f));

    // --- Scenario 2: Filter by Standard Field (Name) ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, UserAData, bGetAllDone, bGetAllSuccess, ReceivedUsers]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedUsers->Empty();
        FString Filter = FString::Printf(TEXT("name == '%s'"), *UserAData.UserName);
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, FPubnubGetAllInclude(), 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, bGetAllSuccess, UserAData]()
    {
        TestTrue("S2: GetAllUserMetadata with name filter success.", *bGetAllSuccess);
        if(*bGetAllSuccess)
        {
             bool bFoundUserA = false;
             for(const auto& User : *ReceivedUsers) { if(User.UserID == UserAData.UserID) { bFoundUserA = true; break; } }
             TestTrue(FString::Printf(TEXT("S2: User A (ID: %s) should be in results for name filter. Count: %d"), *UserAData.UserID, ReceivedUsers->Num()), bFoundUserA);
             if (ReceivedUsers->Num() > 1) AddWarning("S2: Name filter returned multiple users. Ensure name is unique for this test or filter is more specific.");
        }
    }, 0.1f));

    // --- Scenario 3: Sort by Name (Ascending/Descending) ---
    const FString SortTestFilter = FString::Printf(TEXT("id == '%s' || id == '%s' || id == '%s' || id == '%s'"), *UserSortAData.UserID, *UserSortBData.UserID, *UserSortCData.UserID, *UserSortDData.UserID);
    // Ascending
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, SortTestFilter, bGetAllDone, bGetAllSuccess, ReceivedUsers]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedUsers->Empty();
        FPubnubGetAllSort SortSettings; SortSettings.GetAllSort.Add({EPubnubGetAllSortType::PGAST_Name, false});
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, FPubnubGetAllInclude(), 10, SortTestFilter, SortSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, bGetAllSuccess, UserSortAData, UserSortBData, UserSortCData, UserSortDData]()
    {
        TestTrue("S3a: GetAllUserMetadata with name:asc sort success.", *bGetAllSuccess);
        if(*bGetAllSuccess && ReceivedUsers->Num() == 4)
        {
            TestEqual("S3a: Sort Order [0] ID (Alice)", (*ReceivedUsers)[0].UserID, UserSortBData.UserID);
            TestEqual("S3a: Sort Order [1] ID (Bob)", (*ReceivedUsers)[1].UserID, UserSortCData.UserID);
            TestEqual("S3a: Sort Order [2] ID (Charlie)", (*ReceivedUsers)[2].UserID, UserSortAData.UserID);
            TestEqual("S3a: Sort Order [3] ID (David)", (*ReceivedUsers)[3].UserID, UserSortDData.UserID);
        }
        else if (*bGetAllSuccess) AddError(FString::Printf(TEXT("S3a: Expected 4 users for sort test, got %d"), ReceivedUsers->Num()));
    }, 0.1f));
    // Descending
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, SortTestFilter, bGetAllDone, bGetAllSuccess, ReceivedUsers]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedUsers->Empty();
        FPubnubGetAllSort SortSettings; SortSettings.GetAllSort.Add({EPubnubGetAllSortType::PGAST_Name, true});
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, FPubnubGetAllInclude(), 10, SortTestFilter, SortSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, bGetAllSuccess, UserSortAData, UserSortBData, UserSortCData, UserSortDData]()
    {
        TestTrue("S3b: GetAllUserMetadata with name:desc sort success.", *bGetAllSuccess);
        if(*bGetAllSuccess && ReceivedUsers->Num() == 4)
        {
            TestEqual("S3b: Sort Order [0] ID (David)", (*ReceivedUsers)[0].UserID, UserSortDData.UserID);
            TestEqual("S3b: Sort Order [1] ID (Charlie)", (*ReceivedUsers)[1].UserID, UserSortAData.UserID);
            TestEqual("S3b: Sort Order [2] ID (Bob)", (*ReceivedUsers)[2].UserID, UserSortCData.UserID);
            TestEqual("S3b: Sort Order [3] ID (Alice)", (*ReceivedUsers)[3].UserID, UserSortBData.UserID);
        }
         else if (*bGetAllSuccess) AddError(FString::Printf(TEXT("S3b: Expected 4 users for sort test, got %d"), ReceivedUsers->Num()));
    }, 0.1f));

    // --- Scenario 4: Limit Number of Results & PageNext ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, SortTestFilter, NextPage, bGetAllDone, bGetAllSuccess, ReceivedUsers]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedUsers->Empty(); NextPage->Empty();
        FPubnubGetAllSort SortSettings; SortSettings.GetAllSort.Add({EPubnubGetAllSortType::PGAST_Name, false}); 
        FPubnubGetAllInclude IncludeSettings; IncludeSettings.IncludeTotalCount = true;
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, IncludeSettings, 2, SortTestFilter, SortSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, NextPage, bGetAllSuccess, UserSortBData, UserSortCData]()
    {
        TestTrue("S4a: GetAllUserMetadata with Limit=2 (Page 1) success.", *bGetAllSuccess);
        if(*bGetAllSuccess)
        {
            TestEqual("S4a: Received user count with Limit=2.", ReceivedUsers->Num(), 2);
            if(ReceivedUsers->Num() == 2)
            {
                TestEqual("S4a: Page 1 User [0] ID (Alice)", (*ReceivedUsers)[0].UserID, UserSortBData.UserID);
                TestEqual("S4a: Page 1 User [1] ID (Bob)", (*ReceivedUsers)[1].UserID, UserSortCData.UserID);
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
        NextPage->Empty();
        FPubnubGetAllSort SortSettings; SortSettings.GetAllSort.Add({EPubnubGetAllSortType::PGAST_Name, false});
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, FPubnubGetAllInclude(), 2, SortTestFilter, SortSettings, CurrentNextPage);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, NextPage, bGetAllSuccess, UserSortAData, UserSortDData]()
    {
        TestTrue("S4b: GetAllUserMetadata with Limit=2 (Page 2) success.", *bGetAllSuccess);
        if(*bGetAllSuccess)
        {
            TestEqual("S4b: Received user count with Limit=2 for Page 2.", ReceivedUsers->Num(), 2);
             if(ReceivedUsers->Num() == 2)
            {
                TestEqual("S4b: Page 2 User [0] ID (Charlie)", (*ReceivedUsers)[0].UserID, UserSortAData.UserID);
                TestEqual("S4b: Page 2 User [1] ID (David)", (*ReceivedUsers)[1].UserID, UserSortDData.UserID);
            }
        }
    }, 0.1f));

    // --- Scenario 5: Test Include Options (Custom, Status, Type) ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, UserAData, bGetAllDone, bGetAllSuccess, ReceivedUsers]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedUsers->Empty();
        FPubnubGetAllInclude IncludeSettings; 
        IncludeSettings.IncludeCustom = true;
        IncludeSettings.IncludeStatus = true;
        IncludeSettings.IncludeType = true;
        FString Filter = FString::Printf(TEXT("id == '%s'"), *UserAData.UserID);
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, IncludeSettings, 1, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, bGetAllSuccess, UserAData]()
    {
        TestTrue("S5a: GetAllUserMetadata with specific Includes (Custom, Status, Type) success.", *bGetAllSuccess);
        if(*bGetAllSuccess && ReceivedUsers->Num() == 1)
        {
            const auto& User = (*ReceivedUsers)[0];
            TestTrue("S5a: Custom data match", UPubnubJsonUtilities::AreJsonObjectStringsEqual(User.Custom, UserAData.Custom));
            TestEqual("S5a: Status match", User.Status, UserAData.Status);
            TestEqual("S5a: Type match", User.Type, UserAData.Type);
        }
        else if(*bGetAllSuccess) AddError(FString::Printf(TEXT("S5a: Expected 1 user for include test, got %d"), ReceivedUsers->Num()));
    }, 0.1f));
    // Test with some includes false
     ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, GetAllCallback, UserAData, bGetAllDone, bGetAllSuccess, ReceivedUsers]()
    {
        *bGetAllDone = false; *bGetAllSuccess = false; ReceivedUsers->Empty();
        FPubnubGetAllInclude IncludeSettings; 
        IncludeSettings.IncludeCustom = true;
        IncludeSettings.IncludeStatus = false;
        IncludeSettings.IncludeType = false;
        FString Filter = FString::Printf(TEXT("id == '%s'"), *UserAData.UserID);
        PubnubSubsystem->GetAllUserMetadata(GetAllCallback, IncludeSettings, 1, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetAllDone]() { return *bGetAllDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedUsers, bGetAllSuccess, UserAData]()
    {
        TestTrue("S5b: GetAllUserMetadata with Includes (Status=false, Type=false) success.", *bGetAllSuccess);
        if(*bGetAllSuccess && ReceivedUsers->Num() == 1)
        {
            const auto& User = (*ReceivedUsers)[0];
            TestTrue("S5b: Custom data should still match", UPubnubJsonUtilities::AreJsonObjectStringsEqual(User.Custom, UserAData.Custom));
            TestTrue("S5b: Status should be empty as it was not included", User.Status.IsEmpty());
            TestTrue("S5b: Type should be empty as it was not included", User.Type.IsEmpty());
        }
        else if(*bGetAllSuccess) AddError(FString::Printf(TEXT("S5b: Expected 1 user for include test (custom only), got %d"), ReceivedUsers->Num()));
    }, 0.1f));

    // Cleanup: Remove metadata for all test users using callbacks
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, RemoveUserMetadataCallback, CleanupCounter, CleanupErrors, AllTestUsers]()
    {
        *CleanupCounter = 0;
        *CleanupErrors = 0;
        
        // Remove metadata for all users
        for(const auto& User : AllTestUsers)
        {
            PubnubSubsystem->RemoveUserMetadata(User.UserID, RemoveUserMetadataCallback);
        }
    }, 0.1f));

    // Wait for all cleanup operations to complete
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([CleanupCounter, TotalCleanupOperations]() { return *CleanupCounter >= TotalCleanupOperations; }, MAX_WAIT_TIME));

    CleanUp();
    return true;
}

bool FPubnubMembershipManagementWithOptionsTest::RunTest(const FString& Parameters)
{
    const FString TestRunPrefix = SDK_PREFIX + "memb_opts_";
    const FString TestUserID = TestRunPrefix + "user_main";
    const FString TestAdminUserID = TestRunPrefix + "admin_for_setuser";

    // Channel Definitions
    FPubnubChannelData ChannelAData;
    ChannelAData.ChannelID = TestRunPrefix + "ChannelA_Docs";
    ChannelAData.ChannelName = "Documentation Central";
    ChannelAData.Custom = "{\"topic\":\"sdk_docs\", \"priority\":\"high\"}";
    ChannelAData.Status = "active";

    FPubnubChannelData ChannelBData;
    ChannelBData.ChannelID = TestRunPrefix + "ChannelB_Dev";
    ChannelBData.ChannelName = "Development Zone";
    ChannelBData.Custom = "{\"topic\":\"core_dev\", \"priority\":\"medium\"}";
    ChannelBData.Status = "active";

    FPubnubChannelData ChannelCData;
    ChannelCData.ChannelID = TestRunPrefix + "ChannelC_General";
    ChannelCData.ChannelName = "General Discussion";
    ChannelCData.Custom = "{\"topic\":\"community\", \"priority\":\"low\"}";
    ChannelCData.Status = "archived";
    
    FPubnubChannelData ChannelDData;
    ChannelDData.ChannelID = TestRunPrefix + "ChannelD_Support";
	ChannelDData.ChannelName = "Support Hub";
	ChannelDData.Custom = "{\"topic\":\"user_support\", \"priority\":\"high\"}";
    ChannelDData.Status = TEXT("active");

    TArray<FPubnubChannelData> AllTestChannels = {ChannelAData, ChannelBData, ChannelCData, ChannelDData};
    TArray<FString> AllTestChannelIDs;
    for (const auto& Channel : AllTestChannels)
    {
        AllTestChannelIDs.Add(Channel.ChannelID);
    }

    // Membership specific data
    FPubnubMembershipInputData MembershipA;
    MembershipA.Channel = ChannelAData.ChannelID;
    MembershipA.Custom = TEXT("{\"role\":\"editor\", \"last_access\":\"2024-01-01\"}");
    MembershipA.Status = TEXT("activeEditor");
    MembershipA.Type = TEXT("contentContributor");

    FPubnubMembershipInputData MembershipB;
    MembershipB.Channel = ChannelBData.ChannelID;
    MembershipB.Custom = TEXT("{\"role\":\"viewer\", \"last_access\":\"2024-01-15\"}");
    MembershipB.Status = TEXT("passiveViewer");

    FPubnubMembershipInputData MembershipC;
    MembershipC.Channel = ChannelCData.ChannelID;
    MembershipC.Custom = TEXT("{\"role\":\"moderator\", \"last_access\":\"2023-12-01\"}");
    MembershipC.Type = TEXT("channelAdmin");
    
    FPubnubMembershipInputData MembershipD;
    MembershipD.Channel = ChannelDData.ChannelID;
    MembershipD.Custom = TEXT("{\"role\":\"agent\", \"last_access\":\"2024-01-20\"}");

    TArray<FPubnubMembershipInputData> AllMemberships = {MembershipA, MembershipB, MembershipC, MembershipD};

    // Shared state for operations
    TSharedPtr<bool> bGetMembershipsDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetMembershipsSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FPubnubMembershipData>> ReceivedMemberships = MakeShared<TArray<FPubnubMembershipData>>();
    TSharedPtr<FString> NextPageToken = MakeShared<FString>();
    TSharedPtr<FString> PrevPageToken = MakeShared<FString>();

    // Shared state for setup operations
    TSharedPtr<int> SetupCounter = MakeShared<int>(0);
    TSharedPtr<int> SetupErrors = MakeShared<int>(0);
    const int TotalSetupOperations = AllTestChannels.Num(); // 4 channels to set up

    // Shared state for cleanup operations
    TSharedPtr<int> CleanupCounter = MakeShared<int>(0);
    TSharedPtr<int> CleanupErrors = MakeShared<int>(0);
    const int TotalCleanupOperations = AllTestChannels.Num(); // 4 channels to clean up

    // Shared state for membership operations
    TSharedPtr<bool> bSetMembershipsDone = MakeShared<bool>(false);
    TSharedPtr<bool> bSetMembershipsSuccess = MakeShared<bool>(false);
    TSharedPtr<bool> bRemoveMembershipsDone = MakeShared<bool>(false);
    TSharedPtr<bool> bRemoveMembershipsSuccess = MakeShared<bool>(false);

    // Callback for GetMemberships
    FOnGetMembershipsResponseNative GetMembershipsCallback;
    GetMembershipsCallback.BindLambda(
        [this, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships, NextPageToken, PrevPageToken]
        (const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
    {
        *bGetMembershipsDone = true;
        *bGetMembershipsSuccess = (Result.Status == 200);
        if (*bGetMembershipsSuccess)
        {
            *ReceivedMemberships = MembershipsData;
            *NextPageToken = PageNext;
            *PrevPageToken = PagePrev;
        }
        else
        {
            ReceivedMemberships->Empty();
            AddError(FString::Printf(TEXT("GetMemberships call failed. Status: %d. Next: '%s', Prev: '%s'"), Result.Status, *PageNext, *PagePrev));
        }
    });

    // Callback for SetChannelMetadata operations
    FOnSetChannelMetadataResponseNative SetChannelMetadataCallback;
    SetChannelMetadataCallback.BindLambda([this, SetupCounter, SetupErrors](const FPubnubOperationResult& Result, const FPubnubChannelData& ChannelData)
    {
        (*SetupCounter)++;
        if (Result.Error || Result.Status != 200)
        {
            (*SetupErrors)++;
            AddError(FString::Printf(TEXT("SetChannelMetadata failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    // Callback for SetMemberships operations
    FOnSetMembershipsResponseNative SetMembershipsCallback;
    SetMembershipsCallback.BindLambda([this, bSetMembershipsDone, bSetMembershipsSuccess](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
    {
        *bSetMembershipsDone = true;
        if (Result.Error || Result.Status != 200)
        {
            *bSetMembershipsSuccess = false;
            AddError(FString::Printf(TEXT("SetMemberships failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
        else
        {
            *bSetMembershipsSuccess = true;
        }
    });

    // Callback for RemoveMemberships operations
    FOnRemoveMembershipsResponseNative RemoveMembershipsCallback;
    RemoveMembershipsCallback.BindLambda([this, bRemoveMembershipsDone, bRemoveMembershipsSuccess](const FPubnubOperationResult& Result, const TArray<FPubnubMembershipData>& MembershipsData, FString PageNext, FString PagePrev)
    {
        *bRemoveMembershipsDone = true;
        if (Result.Error || Result.Status != 200)
        {
            *bRemoveMembershipsSuccess = false;
            AddError(FString::Printf(TEXT("RemoveMemberships failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
        else
        {
            *bRemoveMembershipsSuccess = true;
        }
    });

    // Callback for RemoveChannelMetadata operations
    FOnRemoveChannelMetadataResponseNative RemoveChannelMetadataCallback;
    RemoveChannelMetadataCallback.BindLambda([this, CleanupCounter, CleanupErrors](const FPubnubOperationResult& Result)
    {
        (*CleanupCounter)++;
        if (Result.Error || Result.Status != 200)
        {
            (*CleanupErrors)++;
            AddError(FString::Printf(TEXT("RemoveChannelMetadata failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubMembershipManagementWithOptionsTest");
        return false;
    }

    PubnubSubsystem->SetUserID(TestAdminUserID);
    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("General Pubnub Error in MembershipManagementTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    // --- Initial Setup: Create Channel Metadata using callbacks ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SetChannelMetadataCallback, SetupCounter, SetupErrors, AllTestChannels]()
    {
        *SetupCounter = 0;
        *SetupErrors = 0;
        
        // Set metadata for all channels
        for(const auto& Channel : AllTestChannels)
        {
            PubnubSubsystem->SetChannelMetadata(Channel.ChannelID, Channel, SetChannelMetadataCallback, FPubnubGetMetadataInclude::FromValue(true));
        }
    }, 0.1f));

    // Wait for all setup operations to complete
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([SetupCounter, TotalSetupOperations]() { return *SetupCounter >= TotalSetupOperations; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SetupErrors]()
    {
        TestEqual("Setup operations should complete without errors", *SetupErrors, 0);
    }, 0.1f));

    // --- Scenario 1: SetMemberships ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, AllMemberships, SetMembershipsCallback, bSetMembershipsDone, bSetMembershipsSuccess]()
    {
        *bSetMembershipsDone = false;
        *bSetMembershipsSuccess = false;
        PubnubSubsystem->SetMemberships(TestUserID, AllMemberships, SetMembershipsCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSetMembershipsDone]() { return *bSetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSetMembershipsSuccess]()
    {
        TestTrue("SetMemberships should succeed", *bSetMembershipsSuccess);
    }, 0.1f));

    // --- Scenario 1b: Basic GetMemberships & Verification ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetMembershipsCallback, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships]()
    {
        *bGetMembershipsDone = false; *bGetMembershipsSuccess = false; ReceivedMemberships->Empty();
        FPubnubMembershipInclude IncludeAll; 
        IncludeAll.IncludeCustom = true; IncludeAll.IncludeStatus = true; IncludeAll.IncludeType = true;
        IncludeAll.IncludeChannel = true; IncludeAll.IncludeChannelCustom = true; IncludeAll.IncludeChannelStatus = true; IncludeAll.IncludeChannelType = true;
        PubnubSubsystem->GetMemberships(TestUserID, GetMembershipsCallback, IncludeAll, 10);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembershipsDone]() { return *bGetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, bGetMembershipsSuccess, AllTestChannels, AllMemberships]()
    {
        TestTrue("S1b: GetMemberships (initial) success.", *bGetMembershipsSuccess);
        if (!*bGetMembershipsSuccess) return;
        TestEqual("S1b: Expected 4 memberships.", ReceivedMemberships->Num(), 4);

        bool bFoundA = false, bFoundB = false, bFoundC = false, bFoundD = false;
        for (const auto& Membership : *ReceivedMemberships)
        {
            if (Membership.Channel.ChannelID == AllTestChannels[0].ChannelID)
            {
                bFoundA = true;
                TestEqual("S1b_A: Channel ID", Membership.Channel.ChannelID, AllMemberships[0].Channel);
                TestTrue("S1b_A: Membership Custom", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Membership.Custom, AllMemberships[0].Custom));
                TestEqual("S1b_A: Membership Status", Membership.Status, AllMemberships[0].Status);
                TestEqual("S1b_A: Membership Type", Membership.Type, AllMemberships[0].Type);
            }
            else if (Membership.Channel.ChannelID == AllTestChannels[1].ChannelID)
            {
                bFoundB = true;
                TestEqual("S1b_B: Channel ID", Membership.Channel.ChannelID, AllMemberships[1].Channel);
                TestTrue("S1b_B: Membership Custom", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Membership.Custom, AllMemberships[1].Custom));
                TestEqual("S1b_B: Membership Status", Membership.Status, AllMemberships[1].Status);
                TestTrue("S1b_B: Membership Type (should be empty)", Membership.Type.IsEmpty());
            }
            else if (Membership.Channel.ChannelID == AllTestChannels[2].ChannelID)
            {
                bFoundC = true;
                TestEqual("S1b_C: Channel ID", Membership.Channel.ChannelID, AllMemberships[2].Channel);
                TestTrue("S1b_C: Membership Custom", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Membership.Custom, AllMemberships[2].Custom));
                TestTrue("S1b_C: Membership Status (should be empty)", Membership.Status.IsEmpty());
                TestEqual("S1b_C: Membership Type", Membership.Type, AllMemberships[2].Type);
            }
            else if (Membership.Channel.ChannelID == AllTestChannels[3].ChannelID)
			{
				bFoundD = true;
				TestEqual("S1b_D: Channel ID", Membership.Channel.ChannelID, AllMemberships[3].Channel);
				TestTrue("S1b_D: Membership Custom", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Membership.Custom, AllMemberships[3].Custom));
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
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetMembershipsCallback, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships, ChannelAData]()
    {
        *bGetMembershipsDone = false; *bGetMembershipsSuccess = false; ReceivedMemberships->Empty();
        FPubnubMembershipInclude IncludeOpts;
        IncludeOpts.IncludeCustom = true;
        IncludeOpts.IncludeStatus = false;
        IncludeOpts.IncludeType = true;
        IncludeOpts.IncludeChannel = true;
        IncludeOpts.IncludeChannelCustom = false;
        IncludeOpts.IncludeChannelStatus = true;
        FString Filter = FString::Printf(TEXT("channel.id == '%s'"), *ChannelAData.ChannelID);
        PubnubSubsystem->GetMemberships(TestUserID, GetMembershipsCallback, IncludeOpts, 1, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembershipsDone]() { return *bGetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, bGetMembershipsSuccess, MembershipA]()
    {
        TestTrue("S2: GetMemberships with specific includes success.", *bGetMembershipsSuccess);
        if (!*bGetMembershipsSuccess || ReceivedMemberships->Num() != 1)
        {
             if (*bGetMembershipsSuccess) AddError(FString::Printf(TEXT("S2: Expected 1 membership for include test, got %d"), ReceivedMemberships->Num()));
             return;
        }
        const auto& Membership = (*ReceivedMemberships)[0];
        TestEqual("S2: Channel ID", Membership.Channel.ChannelID, MembershipA.Channel);
        TestTrue("S2: Channel Custom (NOT Included, should be empty)", Membership.Channel.Custom.IsEmpty());
        
        TestTrue("S2: Membership Custom (Included)", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Membership.Custom, MembershipA.Custom));
        TestTrue("S2: Membership Status (NOT Included, should be empty)", Membership.Status.IsEmpty());
        TestEqual("S2: Membership Type (Included)", Membership.Type, MembershipA.Type);
    }, 0.1f));
    
    // --- Scenario 3: GetMemberships with Filtering ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetMembershipsCallback, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships]()
    {
        *bGetMembershipsDone = false; *bGetMembershipsSuccess = false; ReceivedMemberships->Empty();
        FPubnubMembershipInclude IncludeAll; IncludeAll.IncludeCustom = true; IncludeAll.IncludeChannel = true;
        FString Filter = TEXT("custom.role == 'editor'");
        PubnubSubsystem->GetMemberships(TestUserID, GetMembershipsCallback, IncludeAll, 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembershipsDone]() { return *bGetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, bGetMembershipsSuccess, ChannelAData]()
    {
        TestTrue("S3a: GetMemberships filter by custom.role success.", *bGetMembershipsSuccess);
        if (*bGetMembershipsSuccess)
        {
            TestEqual("S3a: Expected 1 membership for custom.role='editor'.", ReceivedMemberships->Num(), 1);
            if (ReceivedMemberships->Num() == 1)
            {
                TestEqual("S3a: Channel ID for 'editor' role", (*ReceivedMemberships)[0].Channel.ChannelID, ChannelAData.ChannelID);
            }
        }
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetMembershipsCallback, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships]()
    {
        *bGetMembershipsDone = false; *bGetMembershipsSuccess = false; ReceivedMemberships->Empty();
        FPubnubMembershipInclude IncludeChannelMeta; IncludeChannelMeta.IncludeChannel = true; IncludeChannelMeta.IncludeChannelStatus = true;
        FString Filter = TEXT("channel.status == 'archived'");
        PubnubSubsystem->GetMemberships(TestUserID, GetMembershipsCallback, IncludeChannelMeta, 10, Filter);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembershipsDone]() { return *bGetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, bGetMembershipsSuccess, ChannelCData]()
    {
        TestTrue("S3b: GetMemberships filter by channel.status success.", *bGetMembershipsSuccess);
        if (*bGetMembershipsSuccess)
        {
            TestEqual("S3b: Expected 1 membership for channel.status='archived'.", ReceivedMemberships->Num(), 1);
            if (ReceivedMemberships->Num() == 1)
            {
                TestEqual("S3b: Channel ID for 'archived' status", (*ReceivedMemberships)[0].Channel.ChannelID, ChannelCData.ChannelID);
            }
        }
    }, 0.1f));

    // --- Scenario 4: GetMemberships with Sorting ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetMembershipsCallback, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships]()
    {
        *bGetMembershipsDone = false; *bGetMembershipsSuccess = false; ReceivedMemberships->Empty();
        FPubnubMembershipInclude IncludeChannelName; IncludeChannelName.IncludeChannel = true;
        FPubnubMembershipSort SortSettings; SortSettings.MembershipSort.Add({EPubnubMembershipSortType::PMST_ChannelName, false});
        PubnubSubsystem->GetMemberships(TestUserID, GetMembershipsCallback, IncludeChannelName, 10, TEXT(""), SortSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembershipsDone]() { return *bGetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, bGetMembershipsSuccess, ChannelAData, ChannelBData, ChannelCData, ChannelDData]()
    {
        TestTrue("S4a: GetMemberships sort by channel.name:asc success.", *bGetMembershipsSuccess);
        if (*bGetMembershipsSuccess && ReceivedMemberships->Num() == 4)
        {
            TestEqual("S4a: Sort [0] (Dev)", (*ReceivedMemberships)[0].Channel.ChannelID, ChannelBData.ChannelID);
            TestEqual("S4a: Sort [1] (Docs)", (*ReceivedMemberships)[1].Channel.ChannelID, ChannelAData.ChannelID);
            TestEqual("S4a: Sort [2] (General)", (*ReceivedMemberships)[2].Channel.ChannelID, ChannelCData.ChannelID);
            TestEqual("S4a: Sort [3] (Support)", (*ReceivedMemberships)[3].Channel.ChannelID, ChannelDData.ChannelID);
        } else if (*bGetMembershipsSuccess) { AddError(FString::Printf(TEXT("S4a: Expected 4 memberships for sort, got %d"), ReceivedMemberships->Num())); }
    }, 0.1f));

    // --- Scenario 5: GetMemberships with Pagination ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetMembershipsCallback, NextPageToken, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships]()
    {
        *bGetMembershipsDone = false; *bGetMembershipsSuccess = false; ReceivedMemberships->Empty(); NextPageToken->Empty();
        FPubnubMembershipSort SortSettings; SortSettings.MembershipSort.Add({EPubnubMembershipSortType::PMST_ChannelID, false});
        FPubnubMembershipInclude IncludeChannel; IncludeChannel.IncludeChannel = true; IncludeChannel.IncludeTotalCount = true;
        PubnubSubsystem->GetMemberships(TestUserID, GetMembershipsCallback, IncludeChannel, 2, TEXT(""), SortSettings);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembershipsDone]() { return *bGetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, NextPageToken, bGetMembershipsSuccess, ChannelAData, ChannelBData]()
    {
        TestTrue("S5a: GetMemberships with Limit=2 (Page 1) success.", *bGetMembershipsSuccess);
        if (*bGetMembershipsSuccess)
        {
            TestEqual("S5a: Page 1 count.", ReceivedMemberships->Num(), 2);
            if (ReceivedMemberships->Num() == 2)
            {
                TestEqual("S5a: Page 1 Item [0]", (*ReceivedMemberships)[0].Channel.ChannelID, ChannelAData.ChannelID);
                TestEqual("S5a: Page 1 Item [1]", (*ReceivedMemberships)[1].Channel.ChannelID, ChannelBData.ChannelID);
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
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, NextPageToken, bGetMembershipsSuccess, ChannelCData, ChannelDData]()
    {
        TestTrue("S5b: GetMemberships (Page 2) success.", *bGetMembershipsSuccess);
        if (*bGetMembershipsSuccess)
        {
            TestEqual("S5b: Page 2 count.", ReceivedMemberships->Num(), 2);
             if (ReceivedMemberships->Num() == 2)
            {
                TestEqual("S5b: Page 2 Item [0]", (*ReceivedMemberships)[0].Channel.ChannelID, ChannelCData.ChannelID);
                TestEqual("S5b: Page 2 Item [1]", (*ReceivedMemberships)[1].Channel.ChannelID, ChannelDData.ChannelID);
            }
        }
    }, 0.1f));

    // --- Scenario 6: RemoveMemberships ---
    TArray<FString> ChannelsToRemove = {ChannelAData.ChannelID, ChannelCData.ChannelID};
     ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, ChannelsToRemove, RemoveMembershipsCallback, bRemoveMembershipsDone, bRemoveMembershipsSuccess]()
    {
        *bRemoveMembershipsDone = false;
        *bRemoveMembershipsSuccess = false;
        PubnubSubsystem->RemoveMemberships(TestUserID, ChannelsToRemove, RemoveMembershipsCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveMembershipsDone]() { return *bRemoveMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bRemoveMembershipsSuccess]()
    {
        TestTrue("RemoveMemberships should succeed", *bRemoveMembershipsSuccess);
    }, 0.1f));

    // Verify removal
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, GetMembershipsCallback, bGetMembershipsDone, bGetMembershipsSuccess, ReceivedMemberships]()
    {
        *bGetMembershipsDone = false; *bGetMembershipsSuccess = false; ReceivedMemberships->Empty();
        FPubnubMembershipInclude IncludeChannel; IncludeChannel.IncludeChannel = true;
        PubnubSubsystem->GetMemberships(TestUserID, GetMembershipsCallback, IncludeChannel, 10);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembershipsDone]() { return *bGetMembershipsDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMemberships, bGetMembershipsSuccess, ChannelAData, ChannelBData, ChannelCData, ChannelDData]()
    {
        TestTrue("S6: GetMemberships after removal success.", *bGetMembershipsSuccess);
        if (!*bGetMembershipsSuccess) return;
        TestEqual("S6: Expected 2 memberships after removal.", ReceivedMemberships->Num(), 2);
        bool bFoundA = false, bFoundC = false;
        bool bFoundB = false, bFoundD = false;
        for (const auto& Membership : *ReceivedMemberships)
        {
            if (Membership.Channel.ChannelID == ChannelAData.ChannelID) bFoundA = true;
            else if (Membership.Channel.ChannelID == ChannelBData.ChannelID) bFoundB = true;
            else if (Membership.Channel.ChannelID == ChannelCData.ChannelID) bFoundC = true;
            else if (Membership.Channel.ChannelID == ChannelDData.ChannelID) bFoundD = true;
        }
        TestFalse("S6: Membership A should be removed.", bFoundA);
        TestTrue("S6: Membership B should still exist.", bFoundB);
        TestFalse("S6: Membership C should be removed.", bFoundC);
        TestTrue("S6: Membership D should still exist.", bFoundD);
    }, 0.1f));

    // Cleanup: Remove remaining memberships
    TArray<FString> FinalChannelsToRemove = {ChannelBData.ChannelID, ChannelDData.ChannelID};
     ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestUserID, FinalChannelsToRemove, RemoveMembershipsCallback, bRemoveMembershipsDone, bRemoveMembershipsSuccess]()
    {
        *bRemoveMembershipsDone = false;
        *bRemoveMembershipsSuccess = false;
        PubnubSubsystem->RemoveMemberships(TestUserID, FinalChannelsToRemove, RemoveMembershipsCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveMembershipsDone]() { return *bRemoveMembershipsDone; }, MAX_WAIT_TIME));
    
    // Cleanup: Remove channel metadata using callbacks
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, RemoveChannelMetadataCallback, CleanupCounter, CleanupErrors, AllTestChannels]()
    {
        *CleanupCounter = 0;
        *CleanupErrors = 0;
        
        // Remove metadata for all channels
        for(const auto& Channel : AllTestChannels)
        {
            PubnubSubsystem->RemoveChannelMetadata(Channel.ChannelID, RemoveChannelMetadataCallback);
        }
    }, 0.1f));

    // Wait for all cleanup operations to complete
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([CleanupCounter, TotalCleanupOperations]() { return *CleanupCounter >= TotalCleanupOperations; }, MAX_WAIT_TIME));

    CleanUp();
    return true;
}


bool FPubnubChannelMembersManagementWithOptionsTest::RunTest(const FString& Parameters)
{
    const FString TestRunPrefix = SDK_PREFIX + TEXT("cm_opts_");
    const FString TestAdminUserID = TestRunPrefix + TEXT("admin");
    const FString TestChannelID = TestRunPrefix + TEXT("Channel_Main");

    // User Definitions
    FPubnubUserData UserAData;
    UserAData.UserID = TestRunPrefix + TEXT("UserA_Eng");
    UserAData.UserName = TEXT("Alice Engineer");
    UserAData.Email = TEXT("alice.eng@example.com");
    UserAData.Custom = TEXT("{\"department\":\"Engineering\", \"skill\":\"C++\"}");
    UserAData.Status = TEXT("ActiveProject");
    UserAData.Type = TEXT("SeniorDev");

    FPubnubUserData UserBData;
    UserBData.UserID = TestRunPrefix + TEXT("UserB_Sales");
    UserBData.UserName = TEXT("Bob Salesman");
    UserBData.Email = TEXT("bob.sales@example.com");
    UserBData.Custom = TEXT("{\"department\":\"Sales\", \"region\":\"North\"}");
    UserBData.Status = TEXT("OnQuota");
    UserBData.Type = TEXT("AccountExec");

    FPubnubUserData UserCData;
    UserCData.UserID = TestRunPrefix + TEXT("UserC_Support");
    UserCData.UserName = TEXT("Charlie Support");
    UserCData.Email = TEXT("charlie.support@example.com");
    UserCData.Custom = TEXT("{\"department\":\"Support\", \"tier\":2}");
    UserCData.Status = TEXT("Available"); 
    UserCData.Type = TEXT("SupportAgent"); 

    FPubnubUserData UserDData;
    UserDData.UserID = TestRunPrefix + TEXT("UserD_Marketing");
    UserDData.UserName = TEXT("Diana Marketing");
    UserDData.Email = TEXT("diana.marketing@example.com");
    UserDData.Custom = TEXT("{\"department\":\"Marketing\", \"focus\":\"Digital\"}");
    UserDData.Status = TEXT("CampaignLive"); 
    UserDData.Type = TEXT("Specialist"); 
    
    FPubnubUserData UserEData;
	UserEData.UserID = TestRunPrefix + TEXT("UserE_Ops");
	UserEData.UserName = TEXT("Evan Operations");
	UserEData.Email = TEXT("evan.ops@example.com");
	UserEData.Custom = TEXT("{\"department\":\"Operations\", \"role\":\"Manager\"}");
    UserEData.Status = TEXT("Overseeing"); 
    UserEData.Type = TEXT("LeadManager"); 

    TArray<FPubnubUserData> AllTestUsers = {UserAData, UserBData, UserCData, UserDData, UserEData};

    // Member-specific data definitions
    FPubnubChannelMemberInputData MemberA;
    MemberA.User = UserAData.UserID;
    MemberA.Custom = TEXT("{\"channel_role\":\"LeadDev\", \"join_date\":\"2024-01-01\"}");
    MemberA.Status = TEXT("ActiveInChannel");
    MemberA.Type = TEXT("PrimaryContributor");

    FPubnubChannelMemberInputData MemberB;
    MemberB.User = UserBData.UserID;
    MemberB.Custom = TEXT("{\"channel_role\":\"Participant\", \"last_active\":\"2024-01-15\"}");
    MemberB.Status = TEXT("Watching"); 
    MemberB.Type = TEXT("RegularMember");
    
    FPubnubChannelMemberInputData MemberC;
    MemberC.User = UserCData.UserID;
    MemberC.Custom = TEXT("{\"channel_role\":\"Moderator\"}");
    MemberC.Status = TEXT("MonitoringChannel"); 
    MemberC.Type = TEXT("ChannelAdminAssist"); 

    FPubnubChannelMemberInputData MemberD;
    MemberD.User = UserDData.UserID;
    MemberD.Custom = TEXT("{\"channel_role\":\"Reader\"}");
    MemberD.Status = TEXT("Lurking");
    MemberD.Type = TEXT("SilentObserver");

    FPubnubChannelMemberInputData MemberE;
    MemberE.User = UserEData.UserID;
    MemberE.Custom = TEXT("{\"channel_role\":\"Newbie\"}");
    MemberE.Status = TEXT("JustJoined");
    MemberE.Type = TEXT("FreshMeat");

    // Shared state for operations
    TSharedPtr<bool> bGetMembersDone = MakeShared<bool>(false);
    TSharedPtr<bool> bGetMembersSuccess = MakeShared<bool>(false);
    TSharedPtr<TArray<FPubnubChannelMemberData>> ReceivedMembers = MakeShared<TArray<FPubnubChannelMemberData>>();
    TSharedPtr<FString> NextPage = MakeShared<FString>();
    TSharedPtr<FString> PrevPage = MakeShared<FString>();

    // Shared state for setup operations
    TSharedPtr<int> SetupCounter = MakeShared<int>(0);
    TSharedPtr<int> SetupErrors = MakeShared<int>(0);
    const int TotalSetupOperations = AllTestUsers.Num(); // 5 users to set up

    // Shared state for cleanup operations
    TSharedPtr<int> CleanupCounter = MakeShared<int>(0);
    TSharedPtr<int> CleanupErrors = MakeShared<int>(0);
    const int TotalCleanupOperations = AllTestUsers.Num(); // 5 users to clean up

    // Shared state for channel member operations
    TSharedPtr<bool> bSetChannelMembersDone = MakeShared<bool>(false);
    TSharedPtr<bool> bSetChannelMembersSuccess = MakeShared<bool>(false);
    TSharedPtr<bool> bRemoveChannelMembersDone = MakeShared<bool>(false);
    TSharedPtr<bool> bRemoveChannelMembersSuccess = MakeShared<bool>(false);

    // Callback for GetChannelMembers
    FOnGetChannelMembersResponseNative GetMembersCallback;
    GetMembersCallback.BindLambda(
        [this, bGetMembersDone, bGetMembersSuccess, ReceivedMembers, NextPage, PrevPage]
        (const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNextStr, FString PagePrevStr)
    {
        *bGetMembersDone = true;
        *bGetMembersSuccess = (Result.Status == 200);
        if (*bGetMembersSuccess)
        {
            *ReceivedMembers = MembersData;
            *NextPage = PageNextStr;
            *PrevPage = PagePrevStr;
        }
        else
        {
            ReceivedMembers->Empty();
            AddError(FString::Printf(TEXT("GetChannelMembers call failed. Status: %d. Next: '%s', Prev: '%s'"), Result.Status, *PageNextStr, *PagePrevStr));
        }
    });

    // Callback for SetUserMetadata operations
    FOnSetUserMetadataResponseNative SetUserMetadataCallback;
    SetUserMetadataCallback.BindLambda([this, SetupCounter, SetupErrors](const FPubnubOperationResult& Result, const FPubnubUserData& UserData)
    {
        (*SetupCounter)++;
        if (Result.Error || Result.Status != 200)
        {
            (*SetupErrors)++;
            AddError(FString::Printf(TEXT("SetUserMetadata failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    // Callback for SetChannelMembers operations
    FOnSetChannelMembersResponseNative SetChannelMembersCallback;
    SetChannelMembersCallback.BindLambda([this, bSetChannelMembersDone, bSetChannelMembersSuccess](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
    {
        *bSetChannelMembersDone = true;
        if (Result.Error || Result.Status != 200)
        {
            *bSetChannelMembersSuccess = false;
            AddError(FString::Printf(TEXT("SetChannelMembers failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
        else
        {
            *bSetChannelMembersSuccess = true;
        }
    });

    // Callback for RemoveChannelMembers operations
    FOnRemoveChannelMembersResponseNative RemoveChannelMembersCallback;
    RemoveChannelMembersCallback.BindLambda([this, bRemoveChannelMembersDone, bRemoveChannelMembersSuccess](const FPubnubOperationResult& Result, const TArray<FPubnubChannelMemberData>& MembersData, FString PageNext, FString PagePrev)
    {
        *bRemoveChannelMembersDone = true;
        if (Result.Error || Result.Status != 200)
        {
            *bRemoveChannelMembersSuccess = false;
            AddError(FString::Printf(TEXT("RemoveChannelMembers failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
        else
        {
            *bRemoveChannelMembersSuccess = true;
        }
    });

    // Callback for RemoveUserMetadata operations
    FOnRemoveUserMetadataResponseNative RemoveUserMetadataCallback;
    RemoveUserMetadataCallback.BindLambda([this, CleanupCounter, CleanupErrors](const FPubnubOperationResult& Result)
    {
        (*CleanupCounter)++;
        if (Result.Error || Result.Status != 200)
        {
            (*CleanupErrors)++;
            AddError(FString::Printf(TEXT("RemoveUserMetadata failed. Status: %d, Error: %s"), Result.Status, *Result.ErrorMessage));
        }
    });

    if (!InitTest())
    {
        AddError("TestInitialization failed for FPubnubChannelMembersManagementWithOptionsTest");
        return false;
    }

    PubnubSubsystem->SetUserID(TestAdminUserID);
    PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
    {
        AddError(FString::Printf(TEXT("General Pubnub Error in ChannelMembersManagementTest: %s, Type: %d"), *ErrorMessage, ErrorType));
    });

    // --- Initial Setup: Create User Metadata using callbacks ---
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SetUserMetadataCallback, SetupCounter, SetupErrors, AllTestUsers]()
    {
        *SetupCounter = 0;
        *SetupErrors = 0;
        
        // Set metadata for all users
        for (const auto& User : AllTestUsers)
        {
            PubnubSubsystem->SetUserMetadata(User.UserID, User, SetUserMetadataCallback, FPubnubGetMetadataInclude::FromValue(true));
        }
    }, 0.1f));

    // Wait for all setup operations to complete
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([SetupCounter, TotalSetupOperations]() { return *SetupCounter >= TotalSetupOperations; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SetupErrors]()
    {
        TestEqual("Setup operations should complete without errors", *SetupErrors, 0);
    }, 0.1f)); 

    // --- Scenario 1: SetChannelMembers (Initial members A, B) ---
    TArray<FPubnubChannelMemberInputData> SetMembers_AB = {MemberA, MemberB};
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, SetMembers_AB, SetChannelMembersCallback, bSetChannelMembersDone, bSetChannelMembersSuccess]()
    {
        *bSetChannelMembersDone = false;
        *bSetChannelMembersSuccess = false;
        PubnubSubsystem->SetChannelMembers(TestChannelID, SetMembers_AB, SetChannelMembersCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSetChannelMembersDone]() { return *bSetChannelMembersDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSetChannelMembersSuccess]()
    {
        TestTrue("SetChannelMembers (A,B) should succeed", *bSetChannelMembersSuccess);
    }, 0.1f));

    // Verify A, B are members
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, GetMembersCallback, bGetMembersDone, bGetMembersSuccess, ReceivedMembers]()
    {
        *bGetMembersDone = false; *bGetMembersSuccess = false; ReceivedMembers->Empty();
        FPubnubMemberInclude IncludeAll; IncludeAll.IncludeCustom = true; IncludeAll.IncludeStatus = true; IncludeAll.IncludeType = true;
        IncludeAll.IncludeUser = true; IncludeAll.IncludeUserCustom = true; IncludeAll.IncludeUserStatus = true; IncludeAll.IncludeUserType = true;
        PubnubSubsystem->GetChannelMembers(TestChannelID, GetMembersCallback, IncludeAll, 10);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembersDone]() { return *bGetMembersDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMembers, bGetMembersSuccess, MemberA, MemberB]()
    {
        TestTrue("S1: SetChannelMembers (A,B) & Get success.", *bGetMembersSuccess);
        if (!*bGetMembersSuccess) return;
        TestEqual("S1: Expected 2 members (A,B).", ReceivedMembers->Num(), 2);
        bool bFoundA = false, bFoundB = false;
        for (const auto& Member : *ReceivedMembers) {
            if (Member.User.UserID == MemberA.User) { 
                bFoundA = true;
                TestTrue("S1_A: Member Custom", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Member.Custom, MemberA.Custom));
                TestEqual("S1_A: Member Status", Member.Status, MemberA.Status);
                TestEqual("S1_A: Member Type", Member.Type, MemberA.Type);
            } else if (Member.User.UserID == MemberB.User) { 
                bFoundB = true;
                TestTrue("S1_B: Member Custom", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Member.Custom, MemberB.Custom));
                TestEqual("S1_B: Member Status", Member.Status, MemberB.Status);
                TestEqual("S1_B: Member Type", Member.Type, MemberB.Type);
            }
        }
        TestTrue("S1: Found Member A.", bFoundA);
        TestTrue("S1: Found Member B.", bFoundB);
    }, 0.1f));

    // --- Scenario 1b: SetChannelMembers (Upserting UserC) ---
    TArray<FPubnubChannelMemberInputData> SetMembers_C = {MemberC};
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, SetMembers_C, SetChannelMembersCallback, bSetChannelMembersDone, bSetChannelMembersSuccess]()
    {
        *bSetChannelMembersDone = false;
        *bSetChannelMembersSuccess = false;
        // This should ADD UserC because SetChannelMembers acts as an UPSERT
        PubnubSubsystem->SetChannelMembers(TestChannelID, SetMembers_C, SetChannelMembersCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSetChannelMembersDone]() { return *bSetChannelMembersDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSetChannelMembersSuccess]()
    {
        TestTrue("SetChannelMembers (Upsert C) should succeed", *bSetChannelMembersSuccess);
    }, 0.1f));

    // Verify A, B, C are members
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, GetMembersCallback, bGetMembersDone, bGetMembersSuccess, ReceivedMembers]()
    {
        *bGetMembersDone = false; *bGetMembersSuccess = false; ReceivedMembers->Empty();
        FPubnubMemberInclude IncludeAll; IncludeAll.IncludeCustom = true; IncludeAll.IncludeStatus = true; IncludeAll.IncludeType = true;
        IncludeAll.IncludeUser = true; IncludeAll.IncludeUserCustom = true; IncludeAll.IncludeUserStatus = true; IncludeAll.IncludeUserType = true;
        PubnubSubsystem->GetChannelMembers(TestChannelID, GetMembersCallback, IncludeAll, 10);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembersDone]() { return *bGetMembersDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMembers, bGetMembersSuccess, MemberA, MemberB, MemberC]()
    {
        TestTrue("S1b: SetChannelMembers (Upsert C) & Get success.", *bGetMembersSuccess);
        if (!*bGetMembersSuccess) return;
        TestEqual("S1b: Expected 3 members (A,B,C).", ReceivedMembers->Num(), 3);
        bool bFoundA = false, bFoundB = false, bFoundC = false;
        for (const auto& Member : *ReceivedMembers) {
            if(Member.User.UserID == MemberA.User) bFoundA = true;
            else if(Member.User.UserID == MemberB.User) bFoundB = true;
            else if (Member.User.UserID == MemberC.User) { 
                bFoundC = true;
                TestTrue("S1b_C: Member Custom", UPubnubJsonUtilities::AreJsonObjectStringsEqual(Member.Custom, MemberC.Custom));
                TestEqual("S1b_C: Member Status", Member.Status, MemberC.Status);
                TestEqual("S1b_C: Member Type", Member.Type, MemberC.Type);
            }
        }
        TestTrue("S1b: UserA still member.", bFoundA);
        TestTrue("S1b: UserB still member.", bFoundB);
        TestTrue("S1b: Found Member C.", bFoundC);
    }, 0.1f));


    // --- Scenario 2: Explicit Remove (A,B) then Set (D,E) to achieve replacement ---
    TArray<FString> RemoveMembers_AB = {MemberA.User, MemberB.User};
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, RemoveMembers_AB, RemoveChannelMembersCallback, bRemoveChannelMembersDone, bRemoveChannelMembersSuccess]()
    {
        *bRemoveChannelMembersDone = false;
        *bRemoveChannelMembersSuccess = false;
        PubnubSubsystem->RemoveChannelMembers(TestChannelID, RemoveMembers_AB, RemoveChannelMembersCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveChannelMembersDone]() { return *bRemoveChannelMembersDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bRemoveChannelMembersSuccess]()
    {
        TestTrue("RemoveChannelMembers (A,B) should succeed", *bRemoveChannelMembersSuccess);
    }, 0.1f));

    // Verify C is the only member
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, GetMembersCallback, bGetMembersDone, bGetMembersSuccess, ReceivedMembers]()
    {
        *bGetMembersDone = false; *bGetMembersSuccess = false; ReceivedMembers->Empty();
        PubnubSubsystem->GetChannelMembers(TestChannelID, GetMembersCallback, FPubnubMemberInclude(), 10);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembersDone]() { return *bGetMembersDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMembers, bGetMembersSuccess, MemberC]()
    {
        TestTrue("S2a: GetChannelMembers after Remove (A,B) success.", *bGetMembersSuccess);
        if (!*bGetMembersSuccess) return;
        TestEqual("S2a: Expected 1 member (C).", ReceivedMembers->Num(), 1);
        if (ReceivedMembers->Num() == 1)
        {
            TestEqual("S2a: Remaining member is C.", (*ReceivedMembers)[0].User.UserID, MemberC.User);
        }
    }, 0.1f));

    // Now Set D, E. Members should become C, D, E
    TArray<FPubnubChannelMemberInputData> SetMembers_DE = {MemberD, MemberE};
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, SetMembers_DE, SetChannelMembersCallback, bSetChannelMembersDone, bSetChannelMembersSuccess]()
    {
        *bSetChannelMembersDone = false;
        *bSetChannelMembersSuccess = false;
        PubnubSubsystem->SetChannelMembers(TestChannelID, SetMembers_DE, SetChannelMembersCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSetChannelMembersDone]() { return *bSetChannelMembersDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSetChannelMembersSuccess]()
    {
        TestTrue("SetChannelMembers (D,E) should succeed", *bSetChannelMembersSuccess);
    }, 0.1f));

    // Verify C,D,E are members
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, GetMembersCallback, bGetMembersDone, bGetMembersSuccess, ReceivedMembers]()
    {
        *bGetMembersDone = false; *bGetMembersSuccess = false; ReceivedMembers->Empty();
        PubnubSubsystem->GetChannelMembers(TestChannelID, GetMembersCallback, FPubnubMemberInclude(), 10);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembersDone]() { return *bGetMembersDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMembers, bGetMembersSuccess, MemberC, MemberD, MemberE]()
    {
        TestTrue("S2b: GetChannelMembers after Set (D,E) success.", *bGetMembersSuccess);
        if (!*bGetMembersSuccess) return;
        TestEqual("S2b: Expected 3 members (C,D,E).", ReceivedMembers->Num(), 3);
        bool bFoundC = false, bFoundD = false, bFoundE = false;
        for (const auto& Member : *ReceivedMembers) {
            if(Member.User.UserID == MemberC.User) bFoundC = true;
            else if(Member.User.UserID == MemberD.User) bFoundD = true;
            else if(Member.User.UserID == MemberE.User) bFoundE = true;
        }
        TestTrue("S2b: Found Member C.", bFoundC);
        TestTrue("S2b: Found Member D.", bFoundD);
        TestTrue("S2b: Found Member E.", bFoundE);
    }, 0.1f));
    
    // Setup for Filtering, Sorting, Pagination: Ensure all 5 users (A,B,C,D,E) are members
    TArray<FPubnubChannelMemberInputData> AllMembers;
    AllMembers.Add(MemberA);
    AllMembers.Add(MemberB);
    AllMembers.Add(MemberC);
    AllMembers.Add(MemberD);
    AllMembers.Add(MemberE);
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, AllMembers, SetChannelMembersCallback, bSetChannelMembersDone, bSetChannelMembersSuccess]()
    {
        *bSetChannelMembersDone = false;
        *bSetChannelMembersSuccess = false;
        PubnubSubsystem->SetChannelMembers(TestChannelID, AllMembers, SetChannelMembersCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSetChannelMembersDone]() { return *bSetChannelMembersDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bSetChannelMembersSuccess]()
    {
        TestTrue("SetChannelMembers (All members) should succeed", *bSetChannelMembersSuccess);
    }, 0.1f));

    // --- Scenario 7: RemoveChannelMembers & Verification ---
    TArray<FString> UsersToRemove_AD = {MemberA.User, MemberD.User};
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, UsersToRemove_AD, RemoveChannelMembersCallback, bRemoveChannelMembersDone, bRemoveChannelMembersSuccess]()
    {
        *bRemoveChannelMembersDone = false;
        *bRemoveChannelMembersSuccess = false;
        PubnubSubsystem->RemoveChannelMembers(TestChannelID, UsersToRemove_AD, RemoveChannelMembersCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveChannelMembersDone]() { return *bRemoveChannelMembersDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bRemoveChannelMembersSuccess]()
    {
        TestTrue("RemoveChannelMembers (A,D) should succeed", *bRemoveChannelMembersSuccess);
    }, 0.1f));
    
    // Verify B,C,E are members
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, GetMembersCallback, bGetMembersDone, bGetMembersSuccess, ReceivedMembers]()
    {
        *bGetMembersDone = false; *bGetMembersSuccess = false; ReceivedMembers->Empty();
        PubnubSubsystem->GetChannelMembers(TestChannelID, GetMembersCallback, FPubnubMemberInclude(), 10);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bGetMembersDone]() { return *bGetMembersDone; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedMembers, bGetMembersSuccess, MemberA, MemberB, MemberC, MemberD, MemberE]()
    {
        TestTrue("S7: GetChannelMembers after Remove (A,D) success.", *bGetMembersSuccess);
        if (!*bGetMembersSuccess) return;
        TestEqual("S7: Expected 3 members (B,C,E).", ReceivedMembers->Num(), 3);
        bool bFoundA = false, bFoundB = false, bFoundC = false, bFoundD = false, bFoundE = false;
        for (const auto& Member : *ReceivedMembers) {
            if(Member.User.UserID == MemberA.User) bFoundA = true;
            else if(Member.User.UserID == MemberB.User) bFoundB = true;
            else if(Member.User.UserID == MemberC.User) bFoundC = true;
            else if(Member.User.UserID == MemberD.User) bFoundD = true;
            else if(Member.User.UserID == MemberE.User) bFoundE = true;
        }
        TestFalse("S7: Member A should be removed.", bFoundA);
        TestTrue("S7: Member B should still exist.", bFoundB);
        TestTrue("S7: Member C should still exist.", bFoundC);
        TestFalse("S7: Member D should be removed.", bFoundD);
        TestTrue("S7: Member E should still exist.", bFoundE);
    }, 0.1f));

    // Cleanup: Remove remaining members
    TArray<FString> FinalUsersToRemove = {MemberB.User, MemberC.User, MemberE.User};
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannelID, FinalUsersToRemove, RemoveChannelMembersCallback, bRemoveChannelMembersDone, bRemoveChannelMembersSuccess]()
    {
        *bRemoveChannelMembersDone = false;
        *bRemoveChannelMembersSuccess = false;
        PubnubSubsystem->RemoveChannelMembers(TestChannelID, FinalUsersToRemove, RemoveChannelMembersCallback);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bRemoveChannelMembersDone]() { return *bRemoveChannelMembersDone; }, MAX_WAIT_TIME));

    // Cleanup: Remove user metadata using callbacks
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, RemoveUserMetadataCallback, CleanupCounter, CleanupErrors, AllTestUsers]()
    {
        *CleanupCounter = 0;
        *CleanupErrors = 0;
        
        // Remove metadata for all users
        for(const auto& User : AllTestUsers)
        {
            PubnubSubsystem->RemoveUserMetadata(User.UserID, RemoveUserMetadataCallback);
        }
    }, 0.1f));

    // Wait for all cleanup operations to complete
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([CleanupCounter, TotalCleanupOperations]() { return *CleanupCounter >= TotalCleanupOperations; }, MAX_WAIT_TIME));
    
    CleanUp();
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
