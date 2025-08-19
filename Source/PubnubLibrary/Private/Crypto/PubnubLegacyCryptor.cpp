// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "Crypto/PubnubLegacyCryptor.h"
#include "PubnubSubsystem.h"
#include "FunctionLibraries/PubnubCryptoUtilities.h"

// OpenSSL includes
#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/err.h>
#include <openssl/rand.h>
THIRD_PARTY_INCLUDES_END

// Identifier is legacy: zeros (matches PUBNUB_LEGACY_CRYPTO_IDENTIFIER)
TArray<uint8> UPubnubLegacyCryptor::GetIdentifier_Implementation()
{
    return {0, 0, 0, 0};
}

FPubnubEncryptedData UPubnubLegacyCryptor::Encrypt_Implementation(const FString& Data)
{
    if(CipherKey.IsEmpty())
    {
        UE_LOG(PubnubLog, Warning, TEXT("CipherKey is empty, can't encrypt data. Use SetCipherKey before encrypting/decrypting data"));
        return FPubnubEncryptedData();
    }
    
    // Convert input text to UTF-8 bytes
    FTCHARToUTF8 UTF8Data(*Data);
    TArray<uint8> PlainDataBytes;
    PlainDataBytes.Append(reinterpret_cast<const uint8*>(UTF8Data.Get()), UTF8Data.Length());

    // Encrypt
    TArray<uint8> EncryptedDataBytes;
    FPubnubEncryptedData Out;
    if (!EncryptDataLegacy(PlainDataBytes, EncryptedDataBytes)) {
        return Out;
    }
    
    Out.EncryptedData = UPubnubCryptoUtilities::Base64Encode(EncryptedDataBytes);
    Out.Metadata = TEXT("");
    return Out;
}

FString UPubnubLegacyCryptor::Decrypt_Implementation(const FPubnubEncryptedData& Data)
{
    if(CipherKey.IsEmpty())
    {
        UE_LOG(PubnubLog, Warning, TEXT("CipherKey is empty, can't decrypt data. Use SetCipherKey before encrypting/decrypting data"));
        return "";
    }
    
    // Decode Base64
    TArray<uint8> Cipher;
    if (!UPubnubCryptoUtilities::Base64Decode(Data.EncryptedData, Cipher)) {
        return FString();
    }

    // Decrypt
    TArray<uint8> Plain;
    if (!DecryptDataLegacy(Cipher, Plain)) {
        return FString();
    }

    // Ensure null-terminated to safely construct FString
    Plain.Add(0);
    return FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(Plain.GetData())));
}

bool UPubnubLegacyCryptor::EncryptDataLegacy(const TArray<uint8>& Plain, TArray<uint8>& OutCipher)
{
    // Key (ASCII-hex 32 from SHA256 first 16 bytes)
    TArray<uint8_t> KeyAsciiHex; MakeLegacyKeyAsciiHex32(KeyAsciiHex);

    uint8 IV[16];
    if (UseRandomIV) {
        RAND_bytes(IV, 16);
    } else {
        GetFixedIV16(IV);
    }

    // Encrypt with IV
    TArray<uint8> Cipher;
    if (!Aes256CbcEncrypt(KeyAsciiHex, IV, Plain, Cipher)) return false;

    // If random IV is used, prefix IV
    if (UseRandomIV) {
        OutCipher.Reset();
        OutCipher.Append(IV, 16);
        OutCipher.Append(Cipher);
    } else {
        OutCipher = MoveTemp(Cipher);
    }
    return true;
}

bool UPubnubLegacyCryptor::DecryptDataLegacy(const TArray<uint8>& Cipher, TArray<uint8>& OutPlain)
{
    TArray<uint8_t> KeyAsciiHex; MakeLegacyKeyAsciiHex32(KeyAsciiHex);

    if (UseRandomIV) {
        if (Cipher.Num() < 16) return false;
        const uint8* IV   = Cipher.GetData();
        const uint8* Data = Cipher.GetData() + 16;
        const int32  Len  = Cipher.Num() - 16;
        return Aes256CbcDecrypt(KeyAsciiHex, IV, Data, Len, OutPlain);
    } else {
        uint8 FixedIV[16]; GetFixedIV16(FixedIV);
        return Aes256CbcDecrypt(KeyAsciiHex, FixedIV, Cipher.GetData(), Cipher.Num(), OutPlain);
    }
}

void UPubnubLegacyCryptor::MakeLegacyKeyAsciiHex32(TArray<uint8_t>& OutKeyAsciiHex32) const
{
    // pbcc_cipher_key_hash: takes SHA256 of cipher_key string, then formats first 16 bytes as 32 ascii hex chars
    FTCHARToUTF8 UTF8Key(*CipherKey);

    uint8_t digest[32];
    SHA256(reinterpret_cast<const unsigned char*>(UTF8Key.Get()), UTF8Key.Length(), digest);

    OutKeyAsciiHex32.SetNum(32);
    static const char* hex = "0123456789abcdef";
    for (int i = 0; i < 16; ++i) {
        OutKeyAsciiHex32[i*2 + 0] = static_cast<uint8_t>(hex[(digest[i] >> 4) & 0xF]);
        OutKeyAsciiHex32[i*2 + 1] = static_cast<uint8_t>(hex[digest[i] & 0xF]);
    }
}

void UPubnubLegacyCryptor::GetFixedIV16(uint8 OutIV[16])
{
    const char* kIV = "0123456789012345"; // exactly 16 bytes
    FMemory::Memcpy(OutIV, kIV, 16);
}

// AES-256-CBC encrypt: Key must be 32 bytes (ASCII-hex from legacy hash), IV must be 16 bytes
bool UPubnubLegacyCryptor::Aes256CbcEncrypt(const TArray<uint8>& Key, const uint8* IV, const TArray<uint8>& In, TArray<uint8>& Out)
{
    Out.SetNum(In.Num() + EVP_MAX_BLOCK_LENGTH);

    EVP_CIPHER_CTX* Ctx = EVP_CIPHER_CTX_new();
    if (!Ctx) {
        Out.Empty();
        return false;
    }

    int len = 0;
    bool ok = false;

    do {
        if (EVP_EncryptInit_ex(Ctx, EVP_aes_256_cbc(), nullptr, Key.GetData(), IV) != 1) break;

        if (EVP_EncryptUpdate(Ctx, Out.GetData(), &len, In.GetData(), In.Num()) != 1) break;

        int written = len;
        if (EVP_EncryptFinal_ex(Ctx, Out.GetData() + written, &len) != 1) break;

        Out.SetNum(written + len);
        ok = true;
    } while (false);

    EVP_CIPHER_CTX_free(Ctx);
    if (!ok) { Out.Empty(); }
    return ok;
}

// AES-256-CBC decrypt: Key must be 32 bytes (ASCII-hex from legacy hash), IV must be 16 bytes
bool UPubnubLegacyCryptor::Aes256CbcDecrypt(const TArray<uint8>& Key, const uint8* IV, const uint8* Data, int32 DataLen, TArray<uint8>& Out)
{
    Out.SetNum(DataLen + EVP_MAX_BLOCK_LENGTH);

    EVP_CIPHER_CTX* Ctx = EVP_CIPHER_CTX_new();
    if (!Ctx) {
        Out.Empty();
        return false;
    }

    int len = 0;
    bool ok = false;

    Out.SetNumUninitialized(DataLen);

    do {
        if (EVP_DecryptInit_ex(Ctx, EVP_aes_256_cbc(), nullptr, Key.GetData(), IV) != 1) break;

        if (EVP_DecryptUpdate(Ctx, Out.GetData(), &len, Data, DataLen) != 1) break;
        int written = len;

        if (EVP_DecryptFinal_ex(Ctx, Out.GetData() + written, &len) != 1) break;

        Out.SetNum(written + len);
        ok = true;
    } while (false);

    EVP_CIPHER_CTX_free(Ctx);
    if (!ok) { Out.Empty(); }
    return ok;
}