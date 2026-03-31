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
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubCryptoMixedModuleTest, FPubnubAutomationTestBase, "Pubnub.Integration.Crypto.MixedEncryption", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubCryptoResetModuleTest, FPubnubAutomationTestBase, "Pubnub.Integration.Crypto.ResetCryptoModuleToNull", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPubnubCryptoSetAndGetModuleTest, FPubnubAutomationTestBase, "Pubnub.Integration.Crypto.SetAndGetModule", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);
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

	UPubnubAesCryptor* Aes = NewObject<UPubnubAesCryptor>(PubnubClient);
	Aes->SetCipherKey(AesKey);
	TScriptInterface<IPubnubCryptorInterface> AesIntf; AesIntf.SetObject(Aes); AesIntf.SetInterface(Cast<IPubnubCryptorInterface>(Aes));

	UPubnubCryptoModule* Module = NewObject<UPubnubCryptoModule>(PubnubClient);
	Module->InitCryptoModule(AesIntf, TArray<TScriptInterface<IPubnubCryptorInterface>>());

	TScriptInterface<IPubnubCryptoProviderInterface> ModuleIntf; ModuleIntf.SetObject(Module); ModuleIntf.SetInterface(Cast<IPubnubCryptoProviderInterface>(Module));
	PubnubClient->SetCryptoModule(ModuleIntf);

	PubnubClient->SetUserID(TestUser);
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

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SubscribeCb]()
	{
		PubnubClient->SubscribeToChannelAsync(TestChannel, SubscribeCb);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribed]() { return *bSubscribed; }, MAX_WAIT_TIME));

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

bool FPubnubCryptoSetAndGetModuleTest::RunTest(const FString& Parameters)
{
    if(!InitTest())
    {
        AddError("InitTest failed");
        return false;
    }

    // Module A: AES
    UPubnubAesCryptor* Aes = NewObject<UPubnubAesCryptor>(PubnubClient);
    Aes->SetCipherKey("k1");
    TScriptInterface<IPubnubCryptorInterface> AesIntf; AesIntf.SetObject(Aes); AesIntf.SetInterface(Cast<IPubnubCryptorInterface>(Aes));
    UPubnubCryptoModule* ModuleA = NewObject<UPubnubCryptoModule>(PubnubClient);
    ModuleA->InitCryptoModule(AesIntf, TArray<TScriptInterface<IPubnubCryptorInterface>>());
    TScriptInterface<IPubnubCryptoProviderInterface> A; A.SetObject(ModuleA); A.SetInterface(Cast<IPubnubCryptoProviderInterface>(ModuleA));

    // Module B: Legacy fixed
    UPubnubLegacyCryptor* Legacy = NewObject<UPubnubLegacyCryptor>(PubnubClient);
    Legacy->UseRandomIV = false;
    Legacy->SetCipherKey("k2");
    TScriptInterface<IPubnubCryptorInterface> LegacyIntf; LegacyIntf.SetObject(Legacy); LegacyIntf.SetInterface(Cast<IPubnubCryptorInterface>(Legacy));
    UPubnubCryptoModule* ModuleB = NewObject<UPubnubCryptoModule>(PubnubClient);
    ModuleB->InitCryptoModule(LegacyIntf, TArray<TScriptInterface<IPubnubCryptorInterface>>());
    TScriptInterface<IPubnubCryptoProviderInterface> B; B.SetObject(ModuleB); B.SetInterface(Cast<IPubnubCryptoProviderInterface>(ModuleB));

    // Set A
    PubnubClient->SetCryptoModule(A);
    auto RB1 = PubnubClient->GetCryptoModule();
    TestTrue(TEXT("Set A → Get A valid"), RB1 && RB1.GetObject());
    TestTrue(TEXT("Set A → Get A same object"), RB1.GetObject() == ModuleA);

    // Switch to B
    PubnubClient->SetCryptoModule(B);
    auto RB2 = PubnubClient->GetCryptoModule();
    TestTrue(TEXT("Switch to B → Get B valid"), RB2 && RB2.GetObject());
    TestTrue(TEXT("Switch to B → Get B same object"), RB2.GetObject() == ModuleB);

    // Set null
    TScriptInterface<IPubnubCryptoProviderInterface> NullProv;
    PubnubClient->SetCryptoModule(NullProv);
    auto RB3 = PubnubClient->GetCryptoModule();
    TestFalse(TEXT("Set null → Get null"), RB3 && RB3.GetObject());

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

	UPubnubLegacyCryptor* Legacy = NewObject<UPubnubLegacyCryptor>(PubnubClient);
	Legacy->UseRandomIV = true;
	Legacy->SetCipherKey(LegacyKey);
	TScriptInterface<IPubnubCryptorInterface> LegacyIntf; LegacyIntf.SetObject(Legacy); LegacyIntf.SetInterface(Cast<IPubnubCryptorInterface>(Legacy));

	UPubnubCryptoModule* Module = NewObject<UPubnubCryptoModule>(PubnubClient);
	Module->InitCryptoModule(LegacyIntf, TArray<TScriptInterface<IPubnubCryptorInterface>>());

	TScriptInterface<IPubnubCryptoProviderInterface> ModuleIntf; ModuleIntf.SetObject(Module); ModuleIntf.SetInterface(Cast<IPubnubCryptoProviderInterface>(Module));
	PubnubClient->SetCryptoModule(ModuleIntf);

	PubnubClient->SetUserID(TestUser);
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

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SubscribeCb]()
	{
		PubnubClient->SubscribeToChannelAsync(TestChannel, SubscribeCb);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribed]() { return *bSubscribed; }, MAX_WAIT_TIME));

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

	UPubnubLegacyCryptor* Legacy = NewObject<UPubnubLegacyCryptor>(PubnubClient);
	Legacy->UseRandomIV = false;
	Legacy->SetCipherKey(LegacyKey);
	TScriptInterface<IPubnubCryptorInterface> LegacyIntf; LegacyIntf.SetObject(Legacy); LegacyIntf.SetInterface(Cast<IPubnubCryptorInterface>(Legacy));

	UPubnubCryptoModule* Module = NewObject<UPubnubCryptoModule>(PubnubClient);
	Module->InitCryptoModule(LegacyIntf, TArray<TScriptInterface<IPubnubCryptorInterface>>());

	TScriptInterface<IPubnubCryptoProviderInterface> ModuleIntf; ModuleIntf.SetObject(Module); ModuleIntf.SetInterface(Cast<IPubnubCryptoProviderInterface>(Module));
	PubnubClient->SetCryptoModule(ModuleIntf);

	PubnubClient->SetUserID(TestUser);
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

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SubscribeCb]()
	{
		PubnubClient->SubscribeToChannelAsync(TestChannel, SubscribeCb);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribed]() { return *bSubscribed; }, MAX_WAIT_TIME));

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

bool FPubnubCryptoMixedModuleTest::RunTest(const FString& Parameters)
{
	const FString TestMessage = "\"AES default (with Legacy additional)\"";
	const FString TestUser = SDK_PREFIX + "crypto_user_mixed";
	const FString TestChannel = SDK_PREFIX + "crypto_channel_mixed";
	const FString AesKey = "test-aes-key-mixed";
	const FString LegacyKey = "test-legacy-key-mixed";
	TSharedPtr<bool> bReceived = MakeShared<bool>(false);
	TSharedPtr<bool> bSubscribed = MakeShared<bool>(false);

	if(!InitTest())
	{
		AddError("TestInitialization failed");
		return false;
	}

	// Prepare cryptors and two modules:
	// - Mixed module: AES default + Legacy additional (for inbound decryption)
	// - Legacy-only module: Legacy default (for outbound encryption of a message we want to be decrypted by additional path)
	UPubnubAesCryptor* Aes = NewObject<UPubnubAesCryptor>(PubnubClient); Aes->SetCipherKey(AesKey);
	TScriptInterface<IPubnubCryptorInterface> AesIntf; AesIntf.SetObject(Aes); AesIntf.SetInterface(Cast<IPubnubCryptorInterface>(Aes));

	UPubnubLegacyCryptor* Legacy = NewObject<UPubnubLegacyCryptor>(PubnubClient); Legacy->UseRandomIV = true; Legacy->SetCipherKey(LegacyKey);
	TScriptInterface<IPubnubCryptorInterface> LegacyIntf; LegacyIntf.SetObject(Legacy); LegacyIntf.SetInterface(Cast<IPubnubCryptorInterface>(Legacy));

	UPubnubCryptoModule* MixedModule = NewObject<UPubnubCryptoModule>(PubnubClient);
	TArray<TScriptInterface<IPubnubCryptorInterface>> Additional; Additional.Add(LegacyIntf);
	MixedModule->InitCryptoModule(AesIntf, Additional);
	TScriptInterface<IPubnubCryptoProviderInterface> MixedModuleIntf; MixedModuleIntf.SetObject(MixedModule); MixedModuleIntf.SetInterface(Cast<IPubnubCryptoProviderInterface>(MixedModule));

	UPubnubCryptoModule* LegacyOnlyModule = NewObject<UPubnubCryptoModule>(PubnubClient);
	LegacyOnlyModule->InitCryptoModule(LegacyIntf, TArray<TScriptInterface<IPubnubCryptorInterface>>());
	TScriptInterface<IPubnubCryptoProviderInterface> LegacyOnlyModuleIntf; LegacyOnlyModuleIntf.SetObject(LegacyOnlyModule); LegacyOnlyModuleIntf.SetInterface(Cast<IPubnubCryptoProviderInterface>(LegacyOnlyModule));

	// Start with mixed module for subscription/decryption
	PubnubClient->SetCryptoModule(MixedModuleIntf);

	PubnubClient->SetUserID(TestUser);
	PubnubClient->OnMessageReceivedNative.AddLambda([this, TestMessage, TestChannel, TestUser, bReceived](FPubnubMessageData ReceivedMessage)
	{
		if(ReceivedMessage.Channel == TestChannel)
		{
			*bReceived = true;
			TestEqual("Mixed(AES default) - content", TestMessage, ReceivedMessage.Message);
			TestEqual("Mixed(AES default) - channel", TestChannel, ReceivedMessage.Channel);
			TestEqual("Mixed(AES default) - user", TestUser, ReceivedMessage.UserID);
			TestEqual("Mixed(AES default) - type", EPubnubMessageType::PMT_Published, ReceivedMessage.MessageType);
		}
	});

	FOnPubnubSubscribeOperationResponseNative SubscribeCb;
	SubscribeCb.BindLambda([this, bSubscribed](const FPubnubOperationResult& Result)
	{
		*bSubscribed = true;
		TestFalse("Subscribe should not fail (Mixed)", Result.Error);
		TestEqual("Subscribe status (Mixed)", Result.Status, 200);
	});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SubscribeCb]()
	{
		PubnubClient->SubscribeToChannelAsync(TestChannel, SubscribeCb);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribed]() { return *bSubscribed; }, MAX_WAIT_TIME));

	// Publish a message encrypted with Legacy-only module (to exercise additional-cryptor path on inbound)
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, LegacyOnlyModuleIntf]()
	{
		PubnubClient->SetCryptoModule(LegacyOnlyModuleIntf);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, TestMessage]()
	{
		PubnubClient->PublishMessage(TestChannel, TestMessage);
	}, 0.1f));

	// Switch back to mixed before the message arrives, so decryption uses additional cryptor
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, MixedModuleIntf]()
	{
		PubnubClient->SetCryptoModule(MixedModuleIntf);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bReceived]() { return *bReceived; }, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, bReceived]()
	{
		if(!*bReceived) { AddError("Mixed module(AES default): Message was not received"); }
	}, 0.1f));

	CleanUp();
	return true;
}

bool FPubnubCryptoResetModuleTest::RunTest(const FString& Parameters)
{
	const FString Msg1 = "\"Before reset (encrypted)\"";
	const FString Msg2 = "\"After reset (plaintext)\"";
	const FString TestUser = SDK_PREFIX + "crypto_user_reset";
	const FString TestChannel = SDK_PREFIX + "crypto_channel_reset";
	const FString AesKey = "test-aes-key-reset";
	TSharedPtr<int32> ReceivedCount = MakeShared<int32>(0);
	TSharedPtr<bool> bSubscribed = MakeShared<bool>(false);

	if(!InitTest())
	{
		AddError("TestInitialization failed");
		return false;
	}

	UPubnubAesCryptor* Aes = NewObject<UPubnubAesCryptor>(PubnubClient);
	Aes->SetCipherKey(AesKey);
	TScriptInterface<IPubnubCryptorInterface> AesIntf; AesIntf.SetObject(Aes); AesIntf.SetInterface(Cast<IPubnubCryptorInterface>(Aes));

	UPubnubCryptoModule* Module = NewObject<UPubnubCryptoModule>(PubnubClient);
	Module->InitCryptoModule(AesIntf, TArray<TScriptInterface<IPubnubCryptorInterface>>());

	TScriptInterface<IPubnubCryptoProviderInterface> ModuleIntf; ModuleIntf.SetObject(Module); ModuleIntf.SetInterface(Cast<IPubnubCryptoProviderInterface>(Module));
	PubnubClient->SetCryptoModule(ModuleIntf);

	PubnubClient->SetUserID(TestUser);
	PubnubClient->OnMessageReceivedNative.AddLambda([this, Msg1, Msg2, TestChannel, ReceivedCount](FPubnubMessageData ReceivedMessage)
	{
		if(ReceivedMessage.Channel == TestChannel)
		{
			(*ReceivedCount)++;
			if(*ReceivedCount == 1)
			{
				TestEqual("Before reset - content", Msg1, ReceivedMessage.Message);
			}
			else if(*ReceivedCount == 2)
			{
				TestEqual("After reset - content", Msg2, ReceivedMessage.Message);
			}
		}
	});

	FOnPubnubSubscribeOperationResponseNative SubscribeCb;
	SubscribeCb.BindLambda([this, bSubscribed](const FPubnubOperationResult& Result)
	{
		*bSubscribed = true;
		TestFalse("Subscribe should not fail (Reset)", Result.Error);
		TestEqual("Subscribe status (Reset)", Result.Status, 200);
	});

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, SubscribeCb]()
	{
		PubnubClient->SubscribeToChannelAsync(TestChannel, SubscribeCb);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([bSubscribed]() { return *bSubscribed; }, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, Msg1]()
	{
		PubnubClient->PublishMessage(TestChannel, Msg1);
	}, 0.1f));

	// Wait until first (encrypted) message is received before resetting crypto
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([ReceivedCount]() { return *ReceivedCount >= 1; }, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this]()
	{
		TScriptInterface<IPubnubCryptoProviderInterface> NullProv;
		PubnubClient->SetCryptoModule(NullProv);
	}, 0.1f));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, TestChannel, Msg2]()
	{
		PubnubClient->PublishMessage(TestChannel, Msg2);
	}, 0.2f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilLatentCommand([ReceivedCount]() { return *ReceivedCount >= 2; }, MAX_WAIT_TIME));

	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, ReceivedCount]()
	{
		if(*ReceivedCount < 2) { AddError("Reset crypto module: Did not receive both messages"); }
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
    UPubnubAesCryptor* Aes = NewObject<UPubnubAesCryptor>(PubnubClient);
    Aes->SetCipherKey(AesKey);
    TScriptInterface<IPubnubCryptorInterface> AesIntf; AesIntf.SetObject(Aes); AesIntf.SetInterface(Cast<IPubnubCryptorInterface>(Aes));

    UPubnubCryptoModule* Module = NewObject<UPubnubCryptoModule>(PubnubClient);
    Module->InitCryptoModule(AesIntf, TArray<TScriptInterface<IPubnubCryptorInterface>>());
    TScriptInterface<IPubnubCryptoProviderInterface> ModuleIntf; ModuleIntf.SetObject(Module); ModuleIntf.SetInterface(Cast<IPubnubCryptoProviderInterface>(Module));

    PubnubClient->SetUserID(TestUser);
    PubnubClient->SetCryptoModule(ModuleIntf);

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
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, History, Module, MsgEnc1, MsgEnc2]()
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

	UPubnubAesCryptor* Aes = NewObject<UPubnubAesCryptor>(PubnubClient);
	Aes->SetCipherKey(AesKey);
	TScriptInterface<IPubnubCryptorInterface> AesIntf; AesIntf.SetObject(Aes); AesIntf.SetInterface(Cast<IPubnubCryptorInterface>(Aes));

	UPubnubCryptoModule* Module = NewObject<UPubnubCryptoModule>(PubnubClient);
	Module->InitCryptoModule(AesIntf, TArray<TScriptInterface<IPubnubCryptorInterface>>());

	const FString EncB64 = IPubnubCryptoProviderInterface::Execute_ProviderEncrypt(Module, Plain);
	TestFalse(TEXT("AES manual: ProviderEncrypt returned empty"), EncB64.IsEmpty());

	const FString Dec = IPubnubCryptoProviderInterface::Execute_ProviderDecrypt(Module, EncB64);
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

	UPubnubLegacyCryptor* Legacy = NewObject<UPubnubLegacyCryptor>(PubnubClient);
	Legacy->UseRandomIV = true;
	Legacy->SetCipherKey(LegacyKey);
	TScriptInterface<IPubnubCryptorInterface> LegacyIntf; LegacyIntf.SetObject(Legacy); LegacyIntf.SetInterface(Cast<IPubnubCryptorInterface>(Legacy));

	UPubnubCryptoModule* Module = NewObject<UPubnubCryptoModule>(PubnubClient);
	Module->InitCryptoModule(LegacyIntf, TArray<TScriptInterface<IPubnubCryptorInterface>>());

	const FString EncB64 = IPubnubCryptoProviderInterface::Execute_ProviderEncrypt(Module, Plain);
	TestFalse(TEXT("Legacy random: ProviderEncrypt returned empty"), EncB64.IsEmpty());

	const FString Dec = IPubnubCryptoProviderInterface::Execute_ProviderDecrypt(Module, EncB64);
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

	UPubnubLegacyCryptor* Legacy = NewObject<UPubnubLegacyCryptor>(PubnubClient);
	Legacy->UseRandomIV = false;
	Legacy->SetCipherKey(LegacyKey);
	TScriptInterface<IPubnubCryptorInterface> LegacyIntf; LegacyIntf.SetObject(Legacy); LegacyIntf.SetInterface(Cast<IPubnubCryptorInterface>(Legacy));

	UPubnubCryptoModule* Module = NewObject<UPubnubCryptoModule>(PubnubClient);
	Module->InitCryptoModule(LegacyIntf, TArray<TScriptInterface<IPubnubCryptorInterface>>());

	const FString EncB64 = IPubnubCryptoProviderInterface::Execute_ProviderEncrypt(Module, Plain);
	TestFalse(TEXT("Legacy fixed: ProviderEncrypt returned empty"), EncB64.IsEmpty());

	const FString Dec = IPubnubCryptoProviderInterface::Execute_ProviderDecrypt(Module, EncB64);
	TestEqual(TEXT("Legacy fixed: Decrypt equals original"), Dec, Plain);

	CleanUp();
	return true;
}
#endif // WITH_DEV_AUTOMATION_TESTS