// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "PubnubClient.h"
#include "PubnubEnumLibrary.h"
#include "PubnubStructLibrary.h"
#include "FunctionLibraries/PubnubTimetokenUtilities.h"
#include "Crypto/PubnubCryptoModule.h"
#include "Crypto/PubnubAesCryptor.h"
#include "Crypto/PubnubLegacyCryptor.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PubnubTestsUtils.h"
#include "Misc/AutomationTest.h"

using namespace PubnubTests;


IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubCryptoAesDefaultTest, FPubnubAutomationTestBase, "Pubnub.Integration.Crypto.AESDefaultEncryption", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubCryptoLegacyRandomIvTest, FPubnubAutomationTestBase, "Pubnub.Integration.Crypto.LegacyDefaultRandomIVEncryption", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubCryptoLegacyFixedIvTest, FPubnubAutomationTestBase, "Pubnub.Integration.Crypto.LegacyDefaultFixedIVEncryption", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubFetchHistoryWithEncryptionTest, FPubnubAutomationTestBase, "Pubnub.Integration.Crypto.FetchHistoryWithEncryption", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubCryptoAesManualEncryptionTest, FPubnubAutomationTestBase, "Pubnub.Integration.Crypto.AesManualEncryption", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubCryptoLegacyRandomManualEncryptionTest, FPubnubAutomationTestBase, "Pubnub.Integration.Crypto.LegacyRandomManualEncryption", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubCryptoLegacyFixedManualEncryptionTest, FPubnubAutomationTestBase, "Pubnub.Integration.Crypto.LegacyFixedManualEncryption", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FPubnubCryptoAesDefaultTest::RunTest(const FString& Parameters)
{
	const FString TestMessage = "\"AES default encrypted message\"";
	const FString TestUser = SDK_PREFIX + "crypto_user_aes";
	const FString TestChannel = SDK_PREFIX + "crypto_channel_aes";
	const FString AesKey = "test-aes-key";
	TSharedPtr<bool> bReceived = MakeShared<bool>(false);
	TSharedPtr<bool> bSubscribed = MakeShared<bool>(false);

	if(!InitTest())
	{
		AddError("TestInitialization failed");
		return false;
	}

	UPubnubAesCryptor* Aes = NewObject<UPubnubAesCryptor>(GameInstance);
	Aes->SetCipherKey(AesKey);
	TScriptInterface<IPubnubCryptorInterface> AesIntf; AesIntf.SetObject(Aes); AesIntf.SetInterface(Cast<IPubnubCryptorInterface>(Aes));

	UPubnubCryptoModule* Module = NewObject<UPubnubCryptoModule>(GameInstance);
	Module->InitCryptoModule(AesIntf, TArray<TScriptInterface<IPubnubCryptorInterface>>());

	if (!RecreateClient(Module))
	{
		AddError("RecreateClient with crypto module failed");
		CleanUp();
		return false;
	}

	PubnubClient->SetUserID(TestUser);
	FPubnubHandshakeTracker Handshake;
	TSharedPtr<int32> HandshakeBaseline = MakeShared<int32>(0);
	TrackHandshake(PubnubClient, Handshake);
	PubnubClient->OnMessageReceivedNative.AddLambda([this, TestMessage, TestChannel, TestUser, bReceived](FPubnubMessageData ReceivedMessage)
	{
		if(ReceivedMessage.Channel == TestChannel)
		{
			*bReceived = true;
			TestEqual("AES default - content", TestMessage, ReceivedMessage.Message);
			TestEqual("AES default - channel", TestChannel, ReceivedMessage.Channel);
			TestEqual("AES default - user", TestUser, ReceivedMessage.UserID);
			TestEqual("AES default - type", EPubnubMessageType::PMT_Published, ReceivedMessage.MessageType);
		}
	});

	FOnPubnubSubscribeOperationResponseNative SubscribeCb;
	SubscribeCb.BindLambda([this, bSubscribed](const FPubnubOperationResult& Result)
	{
		*bSubscribed = true;
		TestFalse("Subscribe should not fail (AES)", Result.Error);
		TestEqual("Subscribe status (AES)", Result.Status, 200);
	});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SubscribeCb, Handshake, HandshakeBaseline]()
	{
		*HandshakeBaseline = *Handshake.Epoch;
		PubnubClient->SubscribeToChannelAsync(TestChannel, SubscribeCb);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribed]() { return *bSubscribed; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForPubnubHandshakeCommand(Handshake, HandshakeBaseline, { TestChannel }, {}, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Handshake, TestChannel]()
	{
		TestFalse(FString::Printf(TEXT("AES handshake failed: %s"), **Handshake.FailureReason), *Handshake.bFailed);
		TestTrue("AES handshake should include the channel", Handshake.Channels->Contains(TestChannel));
	}, 0.0f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		PubnubClient->PublishMessage(TestChannel, TestMessage);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bReceived]() { return *bReceived; }, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bReceived]()
	{
		if(!*bReceived) { AddError("AES default: Message was not received"); }
	}, 0.1f));

	CleanUp();
	return true;
}

bool FPubnubCryptoLegacyRandomIvTest::RunTest(const FString& Parameters)
{
	const FString TestMessage = "\"Legacy default (random IV) encrypted message\"";
	const FString TestUser = SDK_PREFIX + "crypto_user_legacy_rand";
	const FString TestChannel = SDK_PREFIX + "crypto_channel_legacy_rand";
	const FString LegacyKey = "test-legacy-key";
	TSharedPtr<bool> bReceived = MakeShared<bool>(false);
	TSharedPtr<bool> bSubscribed = MakeShared<bool>(false);

	if(!InitTest())
	{
		AddError("TestInitialization failed");
		return false;
	}

	UPubnubLegacyCryptor* Legacy = NewObject<UPubnubLegacyCryptor>(GameInstance);
	Legacy->UseRandomIV = true;
	Legacy->SetCipherKey(LegacyKey);
	TScriptInterface<IPubnubCryptorInterface> LegacyIntf; LegacyIntf.SetObject(Legacy); LegacyIntf.SetInterface(Cast<IPubnubCryptorInterface>(Legacy));

	UPubnubCryptoModule* Module = NewObject<UPubnubCryptoModule>(GameInstance);
	Module->InitCryptoModule(LegacyIntf, TArray<TScriptInterface<IPubnubCryptorInterface>>());

	if (!RecreateClient(Module))
	{
		AddError("RecreateClient with crypto module failed");
		CleanUp();
		return false;
	}

	PubnubClient->SetUserID(TestUser);
	FPubnubHandshakeTracker Handshake;
	TSharedPtr<int32> HandshakeBaseline = MakeShared<int32>(0);
	TrackHandshake(PubnubClient, Handshake);
	PubnubClient->OnMessageReceivedNative.AddLambda([this, TestMessage, TestChannel, TestUser, bReceived](FPubnubMessageData ReceivedMessage)
	{
		if(ReceivedMessage.Channel == TestChannel)
		{
			*bReceived = true;
			TestEqual("Legacy(random) - content", TestMessage, ReceivedMessage.Message);
			TestEqual("Legacy(random) - channel", TestChannel, ReceivedMessage.Channel);
			TestEqual("Legacy(random) - user", TestUser, ReceivedMessage.UserID);
			TestEqual("Legacy(random) - type", EPubnubMessageType::PMT_Published, ReceivedMessage.MessageType);
		}
	});

	FOnPubnubSubscribeOperationResponseNative SubscribeCb;
	SubscribeCb.BindLambda([this, bSubscribed](const FPubnubOperationResult& Result)
	{
		*bSubscribed = true;
		TestFalse("Subscribe should not fail (Legacy random)", Result.Error);
		TestEqual("Subscribe status (Legacy random)", Result.Status, 200);
	});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SubscribeCb, Handshake, HandshakeBaseline]()
	{
		*HandshakeBaseline = *Handshake.Epoch;
		PubnubClient->SubscribeToChannelAsync(TestChannel, SubscribeCb);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribed]() { return *bSubscribed; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForPubnubHandshakeCommand(Handshake, HandshakeBaseline, { TestChannel }, {}, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Handshake, TestChannel]()
	{
		TestFalse(FString::Printf(TEXT("Legacy random handshake failed: %s"), **Handshake.FailureReason), *Handshake.bFailed);
		TestTrue("Legacy random handshake should include the channel", Handshake.Channels->Contains(TestChannel));
	}, 0.0f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		PubnubClient->PublishMessage(TestChannel, TestMessage);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bReceived]() { return *bReceived; }, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bReceived]()
	{
		if(!*bReceived) { AddError("Legacy(random): Message was not received"); }
	}, 0.1f));

	CleanUp();
	return true;
}

bool FPubnubCryptoLegacyFixedIvTest::RunTest(const FString& Parameters)
{
	const FString TestMessage = "\"Legacy default (fixed IV) encrypted message\"";
	const FString TestUser = SDK_PREFIX + "crypto_user_legacy_fixed";
	const FString TestChannel = SDK_PREFIX + "crypto_channel_legacy_fixed";
	const FString LegacyKey = "test-legacy-key-fixed";
	TSharedPtr<bool> bReceived = MakeShared<bool>(false);
	TSharedPtr<bool> bSubscribed = MakeShared<bool>(false);

	if(!InitTest())
	{
		AddError("TestInitialization failed");
		return false;
	}

	UPubnubLegacyCryptor* Legacy = NewObject<UPubnubLegacyCryptor>(GameInstance);
	Legacy->UseRandomIV = false;
	Legacy->SetCipherKey(LegacyKey);
	TScriptInterface<IPubnubCryptorInterface> LegacyIntf; LegacyIntf.SetObject(Legacy); LegacyIntf.SetInterface(Cast<IPubnubCryptorInterface>(Legacy));

	UPubnubCryptoModule* Module = NewObject<UPubnubCryptoModule>(GameInstance);
	Module->InitCryptoModule(LegacyIntf, TArray<TScriptInterface<IPubnubCryptorInterface>>());

	if (!RecreateClient(Module))
	{
		AddError("RecreateClient with crypto module failed");
		CleanUp();
		return false;
	}

	PubnubClient->SetUserID(TestUser);
	FPubnubHandshakeTracker Handshake;
	TSharedPtr<int32> HandshakeBaseline = MakeShared<int32>(0);
	TrackHandshake(PubnubClient, Handshake);
	PubnubClient->OnMessageReceivedNative.AddLambda([this, TestMessage, TestChannel, TestUser, bReceived](FPubnubMessageData ReceivedMessage)
	{
		if(ReceivedMessage.Channel == TestChannel)
		{
			*bReceived = true;
			TestEqual("Legacy(fixed) - content", TestMessage, ReceivedMessage.Message);
			TestEqual("Legacy(fixed) - channel", TestChannel, ReceivedMessage.Channel);
			TestEqual("Legacy(fixed) - user", TestUser, ReceivedMessage.UserID);
			TestEqual("Legacy(fixed) - type", EPubnubMessageType::PMT_Published, ReceivedMessage.MessageType);
		}
	});

	FOnPubnubSubscribeOperationResponseNative SubscribeCb;
	SubscribeCb.BindLambda([this, bSubscribed](const FPubnubOperationResult& Result)
	{
		*bSubscribed = true;
		TestFalse("Subscribe should not fail (Legacy fixed)", Result.Error);
		TestEqual("Subscribe status (Legacy fixed)", Result.Status, 200);
	});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SubscribeCb, Handshake, HandshakeBaseline]()
	{
		*HandshakeBaseline = *Handshake.Epoch;
		PubnubClient->SubscribeToChannelAsync(TestChannel, SubscribeCb);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribed]() { return *bSubscribed; }, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForPubnubHandshakeCommand(Handshake, HandshakeBaseline, { TestChannel }, {}, MAX_WAIT_TIME));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, Handshake, TestChannel]()
	{
		TestFalse(FString::Printf(TEXT("Legacy fixed handshake failed: %s"), **Handshake.FailureReason), *Handshake.bFailed);
		TestTrue("Legacy fixed handshake should include the channel", Handshake.Channels->Contains(TestChannel));
	}, 0.0f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		PubnubClient->PublishMessage(TestChannel, TestMessage);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bReceived]() { return *bReceived; }, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bReceived]()
	{
		if(!*bReceived) { AddError("Legacy(fixed): Message was not received"); }
	}, 0.1f));

	CleanUp();
	return true;
}

bool FPubnubFetchHistoryWithEncryptionTest::RunTest(const FString& Parameters)
{
    const FString TestUser = SDK_PREFIX + "crypto_user_hist_aes";
    const FString TestChannel = SDK_PREFIX + "crypto_channel_hist_aes";
    const FString AesKey = "test-aes-key-history";
    const FString MsgEnc1 = "\"Hist AES 1\"";
    const FString MsgEnc2 = "\"Hist AES 2\"";

    TSharedPtr<bool> bPub1Done = MakeShared<bool>(false);
    TSharedPtr<bool> bPub1Ok = MakeShared<bool>(false);
    TSharedPtr<bool> bPub2Done = MakeShared<bool>(false);
    TSharedPtr<bool> bPub2Ok = MakeShared<bool>(false);
    TSharedPtr<bool> bFetchDone = MakeShared<bool>(false);
    TSharedPtr<bool> bFetchOk = MakeShared<bool>(false);
    TSharedPtr<TArray<FPubnubHistoryMessageData>> History = MakeShared<TArray<FPubnubHistoryMessageData>>();

    if(!InitTest())
    {
        AddError("TestInitialization failed");
        return false;
    }

    // Configure AES module for encryption
    UPubnubAesCryptor* Aes = NewObject<UPubnubAesCryptor>(GameInstance);
    Aes->SetCipherKey(AesKey);
    TScriptInterface<IPubnubCryptorInterface> AesIntf; AesIntf.SetObject(Aes); AesIntf.SetInterface(Cast<IPubnubCryptorInterface>(Aes));

    UPubnubCryptoModule* Module = NewObject<UPubnubCryptoModule>(GameInstance);
    Module->InitCryptoModule(AesIntf, TArray<TScriptInterface<IPubnubCryptorInterface>>());
    if (!RecreateClient(Module))
    {
        AddError("RecreateClient with crypto module failed");
        CleanUp();
        return false;
    }

    PubnubClient->SetUserID(TestUser);

    // Timetoken window start
    TSharedPtr<FString> StartTT = MakeShared<FString>(UPubnubTimetokenUtilities::GetCurrentUnixTimetoken());

    // Publish 1
    FOnPubnubPublishMessageResponseNative Pub1Cb;
    Pub1Cb.BindLambda([this, bPub1Done, bPub1Ok](const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
    {
        *bPub1Done = true; *bPub1Ok = (!Result.Error && Result.Status == 200);
        if (!*bPub1Ok) AddError(FString::Printf(TEXT("Publish1 failed: %s"), *Result.ErrorMessage));
    });
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, MsgEnc1, Pub1Cb]()
    {
        PubnubClient->PublishMessageAsync(TestChannel, MsgEnc1, Pub1Cb);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bPub1Done]() { return *bPub1Done; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bPub1Ok]() { TestTrue("Publish 1 ok", *bPub1Ok); }, 0.1f));

    // Publish 2
    FOnPubnubPublishMessageResponseNative Pub2Cb;
    Pub2Cb.BindLambda([this, bPub2Done, bPub2Ok](const FPubnubOperationResult& Result, const FPubnubMessageData& PublishedMessage)
    {
        *bPub2Done = true; *bPub2Ok = (!Result.Error && Result.Status == 200);
        if (!*bPub2Ok) AddError(FString::Printf(TEXT("Publish2 failed: %s"), *Result.ErrorMessage));
    });
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, MsgEnc2, Pub2Cb]()
    {
        PubnubClient->PublishMessageAsync(TestChannel, MsgEnc2, Pub2Cb);
    }, 0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bPub2Done]() { return *bPub2Done; }, MAX_WAIT_TIME));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bPub2Ok]() { TestTrue("Publish 2 ok", *bPub2Ok); }, 0.1f));

    // Fetch history in the time window
    FOnPubnubFetchHistoryResponseNative FetchCb;
    FetchCb.BindLambda([this, bFetchDone, bFetchOk, History](const FPubnubOperationResult& Result, const TArray<FPubnubHistoryMessageData>& Messages)
    {
        *bFetchDone = true; *bFetchOk = (!Result.Error && Result.Status == 200);
        *History = Messages;
        if (!*bFetchOk) AddError(FString::Printf(TEXT("FetchHistory failed: %s"), *Result.ErrorMessage));
    });
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, FetchCb, StartTT]()
    {
        FPubnubFetchHistorySettings Settings; Settings.Start = UPubnubTimetokenUtilities::GetCurrentUnixTimetoken(); Settings.End = *StartTT; Settings.MaxPerChannel = 20;
        PubnubClient->FetchHistoryAsync(TestChannel, FetchCb, Settings);
    }, 0.2f));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bFetchDone]() { return *bFetchDone; }, MAX_WAIT_TIME));

    // Manually decrypt and verify
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, History, MsgEnc1, MsgEnc2]()
	{
		bool bFound1 = false, bFound2 = false;
		for (const auto& M : *History)
		{
			if (M.Message == MsgEnc1) bFound1 = true;
			if (M.Message == MsgEnc2) bFound2 = true;
		}

		TestTrue(TEXT("History manual decrypt AES: found msg1"), bFound1);
		TestTrue(TEXT("History manual decrypt AES: found msg2"), bFound2);
	}, 0.1f));

    CleanUp();
    return true;
}

bool FPubnubCryptoAesManualEncryptionTest::RunTest(const FString& Parameters)
{
	const FString AesKey = "test-aes-manual";
	const FString Plain = "\"AES manual roundtrip\""; // JSON string style, like other tests

	if(!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	UPubnubAesCryptor* Aes = NewObject<UPubnubAesCryptor>(GameInstance);
	Aes->SetCipherKey(AesKey);

	const FPubnubEncryptedData Encrypted = IPubnubCryptorInterface::Execute_Encrypt(Aes, Plain);
	TestFalse(TEXT("AES manual: Encrypt returned empty"), Encrypted.EncryptedData.IsEmpty());

	const FString Dec = IPubnubCryptorInterface::Execute_Decrypt(Aes, Encrypted);
	TestEqual(TEXT("AES manual: Decrypt equals original"), Dec, Plain);

	CleanUp();
	return true;
}

bool FPubnubCryptoLegacyRandomManualEncryptionTest::RunTest(const FString& Parameters)
{
	const FString LegacyKey = "test-legacy-manual-rand";
	const FString Plain = "\"Legacy manual roundtrip (random IV)\"";

	if(!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	UPubnubLegacyCryptor* Legacy = NewObject<UPubnubLegacyCryptor>(GameInstance);
	Legacy->UseRandomIV = true;
	Legacy->SetCipherKey(LegacyKey);

	const FPubnubEncryptedData Encrypted = IPubnubCryptorInterface::Execute_Encrypt(Legacy, Plain);
	TestFalse(TEXT("Legacy random: Encrypt returned empty"), Encrypted.EncryptedData.IsEmpty());

	const FString Dec = IPubnubCryptorInterface::Execute_Decrypt(Legacy, Encrypted);
	TestEqual(TEXT("Legacy random: Decrypt equals original"), Dec, Plain);

	CleanUp();
	return true;
}

bool FPubnubCryptoLegacyFixedManualEncryptionTest::RunTest(const FString& Parameters)
{
	const FString LegacyKey = "test-legacy-manual-fixed";
	const FString Plain = "\"Legacy manual roundtrip (fixed IV)\"";

	if(!InitTest())
	{
		AddError("InitTest failed");
		return false;
	}

	UPubnubLegacyCryptor* Legacy = NewObject<UPubnubLegacyCryptor>(GameInstance);
	Legacy->UseRandomIV = false;
	Legacy->SetCipherKey(LegacyKey);

	const FPubnubEncryptedData Encrypted = IPubnubCryptorInterface::Execute_Encrypt(Legacy, Plain);
	TestFalse(TEXT("Legacy fixed: Encrypt returned empty"), Encrypted.EncryptedData.IsEmpty());

	const FString Dec = IPubnubCryptorInterface::Execute_Decrypt(Legacy, Encrypted);
	TestEqual(TEXT("Legacy fixed: Decrypt equals original"), Dec, Plain);

	CleanUp();
	return true;
}
#endif // WITH_DEV_AUTOMATION_TESTS
