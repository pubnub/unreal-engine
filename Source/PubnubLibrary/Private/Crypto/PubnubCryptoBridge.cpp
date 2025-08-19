// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "Crypto/PubnubCryptoBridge.h"
#include "PubnubSubsystem.h"
#include "FunctionLibraries/PubnubCryptoUtilities.h"


void UPubnubCryptoBridge::InitCryptoBridge(const TScriptInterface<IPubnubCryptoProviderInterface>& InCryptoModule)
{
    CryptoModule = InCryptoModule;
    if (!CryptoModule || !CryptoModule.GetObject()) {
        UE_LOG(PubnubLog, Error, TEXT("Crypto module is invalid. Aborting."));
        return;
    }

    // Build a custom C-Core provider that calls our UE module
    CCoreCryptoProvider = static_cast<pubnub_crypto_provider_t*>(malloc(sizeof(pubnub_crypto_provider_t)));
    if (!CCoreCryptoProvider) {
        UE_LOG(PubnubLog, Error, TEXT("Failed to allocate crypto provider."));
        return;
    }
    CCoreCryptoProvider->user_data = this;
    CCoreCryptoProvider->encrypt = &UPubnubCryptoBridge::CCoreProviderEncrypt;
    CCoreCryptoProvider->decrypt = &UPubnubCryptoBridge::CCoreProviderDecrypt;
}

void UPubnubCryptoBridge::CleanUpCryptoBridge()
{
    if (CCoreCryptoProvider) {
        free(CCoreCryptoProvider);
        CCoreCryptoProvider = nullptr;
    }
}

pubnub_bymebl_t UPubnubCryptoBridge::CCoreProviderEncrypt(const pubnub_crypto_provider_t* provider, pubnub_bymebl_t to_encrypt)
{
    pubnub_bymebl_t out{nullptr,0};

    //Get self from user_data
    auto* Self = static_cast<UPubnubCryptoBridge*>(provider ? provider->user_data : nullptr);
    if (!Self || !Self->CryptoModule || !Self->CryptoModule.GetObject())
    {
        UE_LOG(PubnubLog, Error, TEXT("[CCoreProviderEncrypt]: Failed to get self from user_data."));
        return out;
    }

    // Convert to FString
    const FString PlainText = UPubnubCryptoUtilities::ConvertBytesToString(to_encrypt.ptr, to_encrypt.size);

    //Call ProviderEncrypt from IPubnubCryptoProviderInterface
    const FString EncryptedText = IPubnubCryptoProviderInterface::Execute_ProviderEncrypt(Self->CryptoModule.GetObject(), PlainText);

    // Back to bytes
    uint8* bytes = nullptr;
    size_t sz = 0;
    if (!UPubnubCryptoUtilities::Base64Decode(EncryptedText, bytes, sz))
    {
        UE_LOG(PubnubLog, Error, TEXT("[CCoreProviderEncrypt]: Decode to Base64 failed"));
        return out;
    }
    
    out.ptr = bytes;
    out.size = sz;
    return out;
}

pubnub_bymebl_t UPubnubCryptoBridge::CCoreProviderDecrypt(const pubnub_crypto_provider_t* provider, pubnub_bymebl_t to_decrypt)
{
    pubnub_bymebl_t out{nullptr, 0};

    // Get self from user_data
    auto* Self = static_cast<UPubnubCryptoBridge*>(provider ? provider->user_data : nullptr);
    if (!Self || !Self->CryptoModule || !Self->CryptoModule.GetObject())
    {
        UE_LOG(PubnubLog, Error, TEXT("[CCoreProviderDecrypt]: Failed to get self from user_data."));
        return out;
    }

    // Validate input buffer
    if (!to_decrypt.ptr || to_decrypt.size == 0)
    {
        UE_LOG(PubnubLog, Error, TEXT("[CCoreProviderDecrypt]: Invalid input buffer (null or empty)."));
        return out;
    }

    // Convert bytes to Base64 (header + ciphertext)
    const FString B64 = UPubnubCryptoUtilities::Base64Encode(to_decrypt.ptr, to_decrypt.size);
    if (B64.IsEmpty())
    {
        UE_LOG(PubnubLog, Error, TEXT("[CCoreProviderDecrypt]: Base64 encode failed."));
        return out;
    }

    // Call ProviderDecrypt from IPubnubCryptoProviderInterface
    const FString Plain = IPubnubCryptoProviderInterface::Execute_ProviderDecrypt(Self->CryptoModule.GetObject(), B64);
    if (Plain.IsEmpty())
    {
        UE_LOG(PubnubLog, Error, TEXT("[CCoreProviderDecrypt]: ProviderDecrypt returned empty plaintext."));
        return out;
    }

    // Convert plaintext (UTF-8) back to bytes
    uint8* bytes = nullptr;
    size_t sz = 0;
    if (!UPubnubCryptoUtilities::ConvertStringToBytes(Plain, bytes, sz))
    {
        UE_LOG(PubnubLog, Error, TEXT("[CCoreProviderDecrypt]: ConvertStringToBytes failed."));
        return out;
    }

    out.ptr = bytes;
    out.size = sz;
    return out;
}
