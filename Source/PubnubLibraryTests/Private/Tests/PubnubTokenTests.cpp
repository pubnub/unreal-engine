// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "PubnubClient.h"
#include "PubnubStructLibrary.h"
#include "PubnubEnumLibrary.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "Dom/JsonObject.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;

namespace PubnubTokenTestsPrivate
{
	static FPubnubConfig MakePamRestrictedConfig(const FString& UserId)
	{
		FPubnubConfig Config;
		Config.LoggerConfig.DefaultLoggerMinLevel = EPubnubLogLevel::PLL_Debug;
		Config.UserID = UserId;
		Config.PublishKey = PubnubTests::GetTestPublishKeyWithPAM();
		Config.SubscribeKey = PubnubTests::GetTestSubscribeKeyWithPAM();
		Config.SecretKey = FString();
		Config.SetSecretKeyAutomatically = false;
		return Config;
	}

	static void BuildMinimalChannelWritePermissions(const FString& ChannelName, FPubnubGrantTokenPermissions& OutPerms)
	{
		OutPerms = FPubnubGrantTokenPermissions();
		FChannelGrant Grant;
		Grant.Channel = ChannelName;
		Grant.Permissions.Write = true;
		OutPerms.Channels.Add(Grant);
	}

	static bool PublishDenied(const FPubnubPublishMessageResult& R)
	{
		return R.Result.Error || R.Result.Status != 200;
	}

	static bool GetChannelWriteFromParsedToken(const FString& ParsedJson, const FString& ChannelName, bool& OutWrite)
	{
		TSharedPtr<FJsonObject> Root;
		if (!UPubnubJsonUtilities::StringToJsonObject(ParsedJson, Root) || !Root.IsValid())
		{
			return false;
		}
		const TSharedPtr<FJsonObject>* ResourcesPtr = nullptr;
		if (!Root->TryGetObjectField(TEXT("Resources"), ResourcesPtr) || !ResourcesPtr || !(*ResourcesPtr).IsValid())
		{
			return false;
		}
		const TSharedPtr<FJsonObject>* ChannelsPtr = nullptr;
		if (!(*ResourcesPtr)->TryGetObjectField(TEXT("Channels"), ChannelsPtr) || !ChannelsPtr || !(*ChannelsPtr).IsValid())
		{
			return false;
		}
		const TSharedPtr<FJsonObject>* PerChannelPtr = nullptr;
		if (!(*ChannelsPtr)->TryGetObjectField(ChannelName, PerChannelPtr) || !PerChannelPtr || !(*PerChannelPtr).IsValid())
		{
			return false;
		}
		return (*PerChannelPtr)->TryGetBoolField(TEXT("Write"), OutWrite);
	}

	static bool GetChannelReadFromParsedToken(const FString& ParsedJson, const FString& ChannelName, bool& OutRead)
	{
		TSharedPtr<FJsonObject> Root;
		if (!UPubnubJsonUtilities::StringToJsonObject(ParsedJson, Root) || !Root.IsValid())
		{
			return false;
		}
		const TSharedPtr<FJsonObject>* ResourcesPtr = nullptr;
		if (!Root->TryGetObjectField(TEXT("Resources"), ResourcesPtr) || !ResourcesPtr || !(*ResourcesPtr).IsValid())
		{
			return false;
		}
		const TSharedPtr<FJsonObject>* ChannelsPtr = nullptr;
		if (!(*ResourcesPtr)->TryGetObjectField(TEXT("Channels"), ChannelsPtr) || !ChannelsPtr || !(*ChannelsPtr).IsValid())
		{
			return false;
		}
		const TSharedPtr<FJsonObject>* PerChannelPtr = nullptr;
		if (!(*ChannelsPtr)->TryGetObjectField(ChannelName, PerChannelPtr) || !PerChannelPtr || !(*PerChannelPtr).IsValid())
		{
			return false;
		}
		return (*PerChannelPtr)->TryGetBoolField(TEXT("Read"), OutRead);
	}
}

using namespace PubnubTokenTestsPrivate;

// ---------------------------------------------------------------------------
// UPubnubClient::GrantToken
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGrantToken_EmptyAuthorizedUser_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.GrantToken.1Validation.EmptyAuthorizedUser",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGrantToken_EmptyPermissions_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.GrantToken.1Validation.EmptyPermissions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGrantToken_ClientNotInitialized_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.GrantToken.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGrantToken_WithoutSetSecretKey_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.GrantToken.1Validation.SecretKeyNotApplied",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGrantToken_HappyPath_RequiredParamsOnly, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.GrantToken.2HappyPath.RequiredParamsOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGrantToken_WithMetaAndExtendedPermissions, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.GrantToken.3FullParameters.MetaChannelGroupAndUser",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGrantToken_ParseTokenReflectsGrantedChannelPermissions, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.GrantToken.4Advanced.ParseMatchesGrant",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubGrantToken_ConcurrentSyncWhileAsyncInProgress_ReturnsMutexOrSucceeds, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.GrantToken.4Advanced.ConcurrentSyncWhileAsyncInProgress",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::RevokeToken
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRevokeToken_EmptyToken_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.RevokeToken.1Validation.EmptyToken",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRevokeToken_ClientNotInitialized_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.RevokeToken.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRevokeToken_AfterGrantToken_Succeeds, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.RevokeToken.2HappyPath.AfterGrantToken",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubRevokeToken_InvalidToken_ReturnsError, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.RevokeToken.4Advanced.InvalidToken",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// UPubnubClient::SetAuthToken
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetAuthToken_ClientNotInitialized_NoCrash, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.SetAuthToken.1Validation.NotInitialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubSetAuthToken_EmptyString_ClearsAuthPublishFailsUnderPAM, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.SetAuthToken.4Advanced.EmptyTokenClearsClientAuth",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// ---------------------------------------------------------------------------
// Cross-function PAM scenario (deny without token, grant, publish, revoke succeeds)
// Post-revoke publish is not asserted (server propagation timing is too variable).
// ---------------------------------------------------------------------------

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubToken_PublishLifecycleWithGrantAndRevoke, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.4Advanced.PublishLifecycleWithGrantAndRevoke",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubToken_PublishWrongChannel_StillDeniedWithLimitedGrant, FPubnubAutomationTestBase,
	"Pubnub.Integration.Token.4Advanced.PublishWrongChannelDenied",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FPubnubGrantToken_EmptyAuthorizedUser_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	PubnubClient->SetSecretKey();
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("pam_grant_empty_auth"));

	FPubnubGrantTokenPermissions Perms;
	BuildMinimalChannelWritePermissions(SDK_PREFIX + TEXT("pam_ch"), Perms);

	const FPubnubGrantTokenResult R = PubnubClient->GrantToken(60, FString(), Perms);

	TestTrue(TEXT("GrantToken should error for empty authorized user"), R.Result.Error);
	TestTrue(TEXT("Error should mention empty permission payload"),
		R.Result.ErrorMessage.Contains(TEXT("PermissionObject")) || R.Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubGrantToken_EmptyPermissions_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	PubnubClient->SetSecretKey();
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("pam_grant_empty_perms"));

	const FPubnubGrantTokenPermissions EmptyPerms;
	const FPubnubGrantTokenResult R = PubnubClient->GrantToken(60, SDK_PREFIX + TEXT("some_user"), EmptyPerms);

	TestTrue(TEXT("GrantToken should error for empty permissions"), R.Result.Error);
	TestTrue(TEXT("Error should mention PermissionObject"),
		R.Result.ErrorMessage.Contains(TEXT("PermissionObject")) || R.Result.ErrorMessage.Contains(TEXT("empty")));

	CleanUp();
	return true;
}

bool FPubnubGrantToken_ClientNotInitialized_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();

	FPubnubGrantTokenPermissions Perms;
	BuildMinimalChannelWritePermissions(SDK_PREFIX + TEXT("x"), Perms);
	const FPubnubGrantTokenResult R = Client->GrantToken(60, SDK_PREFIX + TEXT("u"), Perms);

	TestTrue(TEXT("Result should indicate error"), R.Result.Error);
	TestTrue(TEXT("ErrorMessage should mention not initialized"),
		R.Result.ErrorMessage.Contains(TEXT("not initialized")) || R.Result.ErrorMessage.Contains(TEXT("invalid")));

	CleanUp();
	return true;
}

bool FPubnubGrantToken_WithoutSetSecretKey_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("pam_grant_no_secret_call"));

	FPubnubGrantTokenPermissions Perms;
	BuildMinimalChannelWritePermissions(SDK_PREFIX + TEXT("pam_ch_ns"), Perms);

	const FPubnubGrantTokenResult R = PubnubClient->GrantToken(60, SDK_PREFIX + TEXT("pam_target_user"), Perms);

	TestTrue(TEXT("GrantToken without SetSecretKey should fail against PAM"), R.Result.Error);
	TestTrue(TEXT("Token should be empty on failure"), R.Token.IsEmpty());

	CleanUp();
	return true;
}

bool FPubnubGrantToken_HappyPath_RequiredParamsOnly::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});

	const FString AuthUser = SDK_PREFIX + TEXT("pam_grant_happy_user");
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("pam_grant_happy_admin"));
	PubnubClient->SetSecretKey();

	FPubnubGrantTokenPermissions Perms;
	BuildMinimalChannelWritePermissions(SDK_PREFIX + TEXT("pam_grant_happy_ch"), Perms);

	const FPubnubGrantTokenResult R = PubnubClient->GrantToken(60, AuthUser, Perms);

	TestFalse(TEXT("GrantToken should succeed"), R.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), R.Result.Status, 200);
	TestTrue(TEXT("Returned token should be non-empty"), R.Token.Len() > 10);

	CleanUp();
	return true;
}

bool FPubnubGrantToken_WithMetaAndExtendedPermissions::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});

	const FString AuthUser = SDK_PREFIX + TEXT("pam_grant_full_user");
	const FString ChannelName = SDK_PREFIX + TEXT("pam_grant_full_ch");
	const FString GroupName = SDK_PREFIX + TEXT("pam_grant_full_grp");
	const FString UserRes = SDK_PREFIX + TEXT("pam_grant_full_ures");

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("pam_grant_full_admin"));
	PubnubClient->SetSecretKey();

	FPubnubGrantTokenPermissions Perms;

	FChannelGrant Ch;
	Ch.Channel = ChannelName;
	Ch.Permissions.Read = true;
	Ch.Permissions.Write = true;
	Perms.Channels.Add(Ch);

	FChannelGroupGrant Gg;
	Gg.ChannelGroup = GroupName;
	Gg.Permissions.Read = true;
	Gg.Permissions.Manage = true;
	Perms.ChannelGroups.Add(Gg);

	FUserGrant Ug;
	Ug.User = UserRes;
	Ug.Permissions.Get = true;
	Ug.Permissions.Update = true;
	Perms.Users.Add(Ug);

	const FString MetaJson = TEXT("{\"env\":\"integration\",\"suite\":\"token\"}");
	const FPubnubGrantTokenResult R = PubnubClient->GrantToken(120, AuthUser, Perms, MetaJson);

	TestFalse(TEXT("GrantToken with full parameter set should succeed"), R.Result.Error);
	TestEqual(TEXT("HTTP status should be 200"), R.Result.Status, 200);
	TestTrue(TEXT("Token should be non-empty"), R.Token.Len() > 10);

	CleanUp();
	return true;
}

// ParseToken should expose the same channel-level Read/Write flags we encoded in the grant.
bool FPubnubGrantToken_ParseTokenReflectsGrantedChannelPermissions::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});

	const FString AuthUser = SDK_PREFIX + TEXT("pam_parse_user");
	const FString ChannelName = SDK_PREFIX + TEXT("pam_parse_ch");

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("pam_parse_admin"));
	PubnubClient->SetSecretKey();

	FPubnubGrantTokenPermissions Perms;
	FChannelGrant Ch;
	Ch.Channel = ChannelName;
	Ch.Permissions.Read = true;
	Ch.Permissions.Write = true;
	Perms.Channels.Add(Ch);

	const int32 TtlMinutes = 90;
	const FPubnubGrantTokenResult GrantRes = PubnubClient->GrantToken(TtlMinutes, AuthUser, Perms);

	TestFalse(TEXT("GrantToken should succeed"), GrantRes.Result.Error);
	TestEqual(TEXT("HTTP 200"), GrantRes.Result.Status, 200);

	// C-Core parse payload does not expose aud as AuthorizedUuid in the reworked JSON; validate TTL + channel permissions only.
	const FString Parsed = PubnubClient->ParseToken(GrantRes.Token);
	TestFalse(TEXT("ParseToken should return non-empty JSON"), Parsed.IsEmpty());

	TSharedPtr<FJsonObject> Root;
	TestTrue(TEXT("Parsed token should be valid JSON object"),
		UPubnubJsonUtilities::StringToJsonObject(Parsed, Root) && Root.IsValid());

	double ParsedTtl = 0.0;
	TestTrue(TEXT("Parsed token should include TTL"), Root->TryGetNumberField(TEXT("TTL"), ParsedTtl));
	TestTrue(TEXT("TTL in parsed token should match granted minutes"),
		FMath::IsNearlyEqual(ParsedTtl, static_cast<double>(TtlMinutes), 0.51));

	bool bWrite = false;
	bool bRead = false;
	TestTrue(TEXT("Parsed token should contain Write on granted channel"),
		GetChannelWriteFromParsedToken(Parsed, ChannelName, bWrite) && bWrite);
	TestTrue(TEXT("Parsed token should contain Read on granted channel"),
		GetChannelReadFromParsedToken(Parsed, ChannelName, bRead) && bRead);

	CleanUp();
	return true;
}

bool FPubnubGrantToken_ConcurrentSyncWhileAsyncInProgress_ReturnsMutexOrSucceeds::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("pam_grant_mutex_admin"));
	PubnubClient->SetSecretKey();

	FPubnubGrantTokenPermissions Perms;
	BuildMinimalChannelWritePermissions(SDK_PREFIX + TEXT("pam_mutex_ch"), Perms);

	const FString AuthUser = SDK_PREFIX + TEXT("pam_mutex_target");

	TSharedPtr<FPubnubGrantTokenResult> SyncResult = MakeShared<FPubnubGrantTokenResult>();

	FOnPubnubGrantTokenResponseNative AsyncCb;
	AsyncCb.BindLambda([](const FPubnubOperationResult&, FString) {});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, AuthUser, Perms, AsyncCb]()
	{
		PubnubClient->GrantTokenAsync(60, AuthUser, Perms, AsyncCb);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, AuthUser, Perms, SyncResult]()
	{
		*SyncResult = PubnubClient->GrantToken(60, AuthUser, Perms);
	}, 0.15f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, SyncResult]()
	{
		if (SyncResult->Result.Error)
		{
			TestTrue(TEXT("When sync fails during concurrent op, message should mention mutex / in progress"),
				SyncResult->Result.ErrorMessage.Contains(TEXT("operation is in progress")) ||
				SyncResult->Result.ErrorMessage.Contains(TEXT("concurrently")) ||
				SyncResult->Result.ErrorMessage.Contains(TEXT("locked")));
		}
	}, 0.25f));

	CleanUp();
	return true;
}

bool FPubnubRevokeToken_EmptyToken_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	PubnubClient->SetSecretKey();
	PubnubClient->SetUserID(SDK_PREFIX + TEXT("pam_revoke_empty"));

	const FPubnubOperationResult R = PubnubClient->RevokeToken(FString());

	TestTrue(TEXT("RevokeToken should error on empty token"), R.Error);
	TestTrue(TEXT("Error should mention Token"), R.ErrorMessage.Contains(TEXT("Token")));

	CleanUp();
	return true;
}

bool FPubnubRevokeToken_ClientNotInitialized_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();

	const FPubnubOperationResult R = Client->RevokeToken(TEXT("any_token"));

	TestTrue(TEXT("RevokeToken should error when client destroyed"), R.Error);
	TestTrue(TEXT("ErrorMessage should mention not initialized"),
		R.ErrorMessage.Contains(TEXT("not initialized")) || R.ErrorMessage.Contains(TEXT("invalid")));

	CleanUp();
	return true;
}

bool FPubnubRevokeToken_AfterGrantToken_Succeeds::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("pam_revoke_ok_admin"));
	PubnubClient->SetSecretKey();

	FPubnubGrantTokenPermissions Perms;
	BuildMinimalChannelWritePermissions(SDK_PREFIX + TEXT("pam_revoke_ok_ch"), Perms);

	const FPubnubGrantTokenResult G = PubnubClient->GrantToken(60, SDK_PREFIX + TEXT("pam_revoke_ok_user"), Perms);
	TestFalse(TEXT("Grant should succeed before revoke"), G.Result.Error);
	TestEqual(TEXT("Grant HTTP 200"), G.Result.Status, 200);

	const FPubnubOperationResult R = PubnubClient->RevokeToken(G.Token);
	TestFalse(TEXT("RevokeToken should succeed for freshly granted token"), R.Error);
	TestEqual(TEXT("Revoke HTTP status 200"), R.Status, 200);

	CleanUp();
	return true;
}

bool FPubnubRevokeToken_InvalidToken_ReturnsError::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	PubnubClient->SetUserID(SDK_PREFIX + TEXT("pam_revoke_bad_admin"));
	PubnubClient->SetSecretKey();

	const FPubnubOperationResult R = PubnubClient->RevokeToken(TEXT("definitely_not_a_valid_pubnub_v3_token"));

	TestTrue(TEXT("RevokeToken should report error for garbage token"), R.Error);
	TestTrue(TEXT("HTTP status should not be success"), R.Status != 200);

	CleanUp();
	return true;
}

bool FPubnubSetAuthToken_ClientNotInitialized_NoCrash::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	UPubnubClient* Client = PubnubClient;
	Client->DestroyClient();
	Client->SetAuthToken(TEXT("some_token"));

	CleanUp();
	return true;
}

bool FPubnubSetAuthToken_EmptyString_ClearsAuthPublishFailsUnderPAM::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});

	const FString RestrictedUser = SDK_PREFIX + TEXT("pam_clear_auth_user");
	const FString ChannelName = SDK_PREFIX + TEXT("pam_clear_auth_ch");

	UPubnubClient* Admin = PubnubClient;
	Admin->SetUserID(SDK_PREFIX + TEXT("pam_clear_auth_admin"));
	Admin->SetSecretKey();

	FPubnubGrantTokenPermissions Perms;
	BuildMinimalChannelWritePermissions(ChannelName, Perms);
	const FPubnubGrantTokenResult G = Admin->GrantToken(60, RestrictedUser, Perms);
	TestFalse(TEXT("Grant for clear-auth scenario should succeed"), G.Result.Error);

	UPubnubClient* Restricted = PubnubSubsystem->CreatePubnubClient(MakePamRestrictedConfig(RestrictedUser));
	TestNotNull(TEXT("Restricted client should be created"), Restricted);

	Restricted->SetAuthToken(G.Token);
	FPubnubPublishMessageResult OkPublish = Restricted->PublishMessage(ChannelName, TEXT("\"before clear\""));
	TestFalse(TEXT("Publish should succeed while token is set"), OkPublish.Result.Error);
	TestEqual(TEXT("Publish status 200 with token"), OkPublish.Result.Status, 200);

	Restricted->SetAuthToken(FString());
	FPubnubPublishMessageResult BadPublish = Restricted->PublishMessage(ChannelName, TEXT("\"after clear\""));
	TestTrue(TEXT("After clearing auth token, publish should be denied under PAM"), PublishDenied(BadPublish));

	PubnubSubsystem->DestroyPubnubClient(Restricted);

	CleanUp();
	return true;
}

bool FPubnubToken_PublishLifecycleWithGrantAndRevoke::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});

	const FString RestrictedUser = SDK_PREFIX + TEXT("pam_pubcycle_user");
	const FString ChannelName = SDK_PREFIX + TEXT("pam_pubcycle_ch");

	UPubnubClient* Admin = PubnubClient;
	Admin->SetUserID(SDK_PREFIX + TEXT("pam_pubcycle_admin"));
	Admin->SetSecretKey();

	UPubnubClient* Restricted = PubnubSubsystem->CreatePubnubClient(MakePamRestrictedConfig(RestrictedUser));
	TestNotNull(TEXT("Restricted client exists"), Restricted);

	FPubnubPublishMessageResult DeniedNoToken = Restricted->PublishMessage(ChannelName, TEXT("\"no token\""));
	TestTrue(TEXT("Without auth token, publish should be denied"), PublishDenied(DeniedNoToken));

	FPubnubGrantTokenPermissions Perms;
	BuildMinimalChannelWritePermissions(ChannelName, Perms);
	const FPubnubGrantTokenResult G = Admin->GrantToken(60, RestrictedUser, Perms);
	TestFalse(TEXT("GrantToken should succeed"), G.Result.Error);
	TestEqual(TEXT("Grant status 200"), G.Result.Status, 200);

	Restricted->SetAuthToken(G.Token);
	FPubnubPublishMessageResult Allowed = Restricted->PublishMessage(ChannelName, TEXT("\"with token\""));
	TestFalse(TEXT("With granted token, publish should succeed"), Allowed.Result.Error);
	TestEqual(TEXT("Publish status 200"), Allowed.Result.Status, 200);

	const FPubnubOperationResult RevokeRes = Admin->RevokeToken(G.Token);
	TestFalse(TEXT("RevokeToken should succeed"), RevokeRes.Error);
	TestEqual(TEXT("Revoke status 200"), RevokeRes.Status, 200);

	PubnubSubsystem->DestroyPubnubClient(Restricted);

	CleanUp();
	return true;
}

// Grant write only on channel A; publishing to B must still fail even after SetAuthToken.
bool FPubnubToken_PublishWrongChannel_StillDeniedWithLimitedGrant::RunTest(const FString& Parameters)
{
	if (!InitTestWithPAM())
	{
		AddError(TEXT("InitTestWithPAM failed"));
		return false;
	}

	PubnubSubsystem->OnPubnubErrorNative.AddLambda([this](FString ErrorMessage, EPubnubErrorType ErrorType)
	{
		AddError(ErrorMessage);
	});

	const FString RestrictedUser = SDK_PREFIX + TEXT("pam_wrongch_user");
	const FString AllowedChannel = SDK_PREFIX + TEXT("pam_wrongch_allowed");
	const FString OtherChannel = SDK_PREFIX + TEXT("pam_wrongch_other");

	UPubnubClient* Admin = PubnubClient;
	Admin->SetUserID(SDK_PREFIX + TEXT("pam_wrongch_admin"));
	Admin->SetSecretKey();

	FPubnubGrantTokenPermissions Perms;
	BuildMinimalChannelWritePermissions(AllowedChannel, Perms);
	const FPubnubGrantTokenResult G = Admin->GrantToken(60, RestrictedUser, Perms);
	TestFalse(TEXT("Grant should succeed"), G.Result.Error);

	UPubnubClient* Restricted = PubnubSubsystem->CreatePubnubClient(MakePamRestrictedConfig(RestrictedUser));
	Restricted->SetAuthToken(G.Token);

	FPubnubPublishMessageResult WrongCh = Restricted->PublishMessage(OtherChannel, TEXT("\"wrong\""));
	TestTrue(TEXT("Publish to non-granted channel should be denied"), PublishDenied(WrongCh));

	FPubnubPublishMessageResult RightCh = Restricted->PublishMessage(AllowedChannel, TEXT("\"right\""));
	TestFalse(TEXT("Publish to granted channel should succeed"), RightCh.Result.Error);
	TestEqual(TEXT("Allowed channel publish status 200"), RightCh.Result.Status, 200);

	PubnubSubsystem->DestroyPubnubClient(Restricted);

	CleanUp();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
