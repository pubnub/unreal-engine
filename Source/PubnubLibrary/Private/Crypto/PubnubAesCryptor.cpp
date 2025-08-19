// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "Crypto/PubnubAesCryptor.h"
#include "PubnubSubsystem.h"
#include "FunctionLibraries/PubnubCryptoUtilities.h"

// OpenSSL includes
#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/err.h>
THIRD_PARTY_INCLUDES_END


TArray<uint8_t> UPubnubAesCryptor::GetIdentifier_Implementation()
{
    return {'A', 'C', 'R', 'H'};
}

FPubnubEncryptedData UPubnubAesCryptor::Encrypt_Implementation(const FString& Data)
{
    if(CipherKey.IsEmpty())
    {
        UE_LOG(PubnubLog, Warning, TEXT("CipherKey is empty, can't encrypt data. Use SetCipherKey before encrypting/decrypting data"));
        return FPubnubEncryptedData();
    }
    
    // Convert FString to TArray<uint8>
    FTCHARToUTF8 UTF8Data(*Data);
    TArray<uint8> DataBytes;
    DataBytes.Append(reinterpret_cast<const uint8*>(UTF8Data.Get()), UTF8Data.Length());
    
    // Call internal encryption function
    FPubnubEncryptedDataInternal InternalResult;
    if (!EncryptData(CipherKey, DataBytes, InternalResult))
    {
        UE_LOG(PubnubLog, Error, TEXT("Internal encryption failed"));
        return FPubnubEncryptedData();
    }
    
    // Convert internal result to interface format (Base64)
    FPubnubEncryptedData Result = ConvertToInterface(InternalResult);
    
    return Result;
}

FString UPubnubAesCryptor::Decrypt_Implementation(const FPubnubEncryptedData& Data)
{
    if(CipherKey.IsEmpty())
    {
        UE_LOG(PubnubLog, Warning, TEXT("CipherKey is empty, can't decrypt data. Use SetCipherKey before encrypting/decrypting data"));
        return "";
    }
    
    // Validate input
    if (Data.EncryptedData.IsEmpty() || Data.Metadata.IsEmpty())
    {
        UE_LOG(PubnubLog, Error, TEXT("Incorrect data to decrypt. Empty encrypted data or metadata"));
        return FString();
    }
    
    // Convert interface format to internal format
    FPubnubEncryptedDataInternal InternalData = ConvertFromInterface(Data);
    
    // Call internal decryption function
    TArray<uint8> DecryptedBytes;
    if (!DecryptData(CipherKey, InternalData, DecryptedBytes))
    {
        UE_LOG(PubnubLog, Error, TEXT("Internal decryption failed"));
        return FString();
    }
    
    // Convert TArray<uint8> back to FString
    // Add null terminator for safety
    DecryptedBytes.Add(0);
    FString Result = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(DecryptedBytes.GetData())));
    
    return Result;
}



bool UPubnubAesCryptor::EncryptData(const FString& CipherKey, const TArray<uint8>& DataToEncrypt, FPubnubEncryptedDataInternal& OutResult)
{
    // Clear output
    OutResult.Data.Empty();
    OutResult.Metadata.Empty();
    
    if (DataToEncrypt.Num() == 0)
    {
        UE_LOG(PubnubLog, Error, TEXT("PubNubAESCrypto: No data to encrypt"));
        return false;
    }
    
    // 1. Hash the cipher key with SHA256 (same as pbsha256_digest_str)
    uint8 KeyHash[Sha256Len + 1]; // +1 for null terminator like in original
    if (!HashKeySHA256(CipherKey, KeyHash))
    {
        UE_LOG(PubnubLog, Error, TEXT("PubNubAESCrypto: Failed to hash cipher key"));
        return false;
    }
    KeyHash[Sha256Len] = '\0'; // Null terminate like in original
    
    // 2. Generate random IV (same as generate_init_vector)
    uint8 IV[AesIvSize];
    GenerateRandomIV(IV, AesIvSize);
    
    // 3. Calculate buffer size (same as estimated_enc_buffer_size)
    int32 EstimatedSize = DataToEncrypt.Num() + (AesBlockSize - (DataToEncrypt.Num() % AesBlockSize)) + AesBlockSize;
    OutResult.Data.SetNum(EstimatedSize);
    
    // 4. Perform AES-256-CBC encryption (same as pbaes256_encrypt)
    EVP_CIPHER_CTX* Context = EVP_CIPHER_CTX_new();
    if (!Context)
    {
        UE_LOG(PubnubLog, Error, TEXT("PubNubAESCrypto: Failed to create encryption context"));
        return false;
    }
    
    bool bSuccess = false;
    int32 Len = 0;
    int32 TotalLen = 0;
    
    do
    {
        // Initialize encryption with AES-256-CBC
        if (EVP_EncryptInit_ex(Context, EVP_aes_256_cbc(), nullptr, KeyHash, IV) != 1)
        {
            UE_LOG(PubnubLog, Error, TEXT("PubNubAESCrypto: Failed to initialize AES-256 encryption"));
            break;
        }
        
        // Encrypt the data
        if (EVP_EncryptUpdate(Context, OutResult.Data.GetData(), &Len, DataToEncrypt.GetData(), DataToEncrypt.Num()) != 1)
        {
            UE_LOG(PubnubLog, Error, TEXT("PubNubAESCrypto: Failed to encrypt data"));
            break;
        }
        TotalLen = Len;
        
        // Finalize encryption (adds PKCS padding)
        if (EVP_EncryptFinal_ex(Context, OutResult.Data.GetData() + Len, &Len) != 1)
        {
            UE_LOG(PubnubLog, Error, TEXT("PubNubAESCrypto: Failed to finalize encryption"));
            break;
        }
        TotalLen += Len;
        
        // Resize to actual encrypted size
        OutResult.Data.SetNum(TotalLen);
        
        // Store IV as metadata (same as original)
        OutResult.Metadata.SetNum(AesIvSize);
        FMemory::Memcpy(OutResult.Metadata.GetData(), IV, AesIvSize);
        
        bSuccess = true;
        
    } while (false);
    
    EVP_CIPHER_CTX_free(Context);
    
    if (!bSuccess)
    {
        OutResult.Data.Empty();
        OutResult.Metadata.Empty();
    }
    
    return bSuccess;
}

bool UPubnubAesCryptor::DecryptData(const FString& CipherKey, const FPubnubEncryptedDataInternal& EncryptedData, TArray<uint8>& OutResult)
{
    OutResult.Empty();
    
    if (EncryptedData.Data.Num() == 0 || EncryptedData.Metadata.Num() != AesIvSize)
    {
        UE_LOG(PubnubLog, Error, TEXT("PubNubAESCrypto: Invalid encrypted data or metadata"));
        return false;
    }
    
    // Hash the cipher key
    uint8 KeyHash[Sha256Len + 1];
    if (!HashKeySHA256(CipherKey, KeyHash))
    {
        UE_LOG(PubnubLog, Error, TEXT("PubNubAESCrypto: Failed to hash cipher key for decryption"));
        return false;
    }
    KeyHash[Sha256Len] = '\0';
    
    // Prepare output buffer
    int32 EstimatedSize = EncryptedData.Data.Num() + AesBlockSize + 1;
    OutResult.SetNum(EstimatedSize);
    
    EVP_CIPHER_CTX* Context = EVP_CIPHER_CTX_new();
    if (!Context)
    {
        UE_LOG(PubnubLog, Error, TEXT("PubNubAESCrypto: Failed to create decryption context"));
        return false;
    }
    
    bool bSuccess = false;
    int32 Len = 0;
    int32 TotalLen = 0;
    
    do
    {
        // Initialize decryption
        if (EVP_DecryptInit_ex(Context, EVP_aes_256_cbc(), nullptr, KeyHash, EncryptedData.Metadata.GetData()) != 1)
        {
            UE_LOG(PubnubLog, Error, TEXT("PubNubAESCrypto: Failed to initialize AES-256 decryption"));
            break;
        }
        
        // Decrypt the data
        if (EVP_DecryptUpdate(Context, OutResult.GetData(), &Len, EncryptedData.Data.GetData(), EncryptedData.Data.Num()) != 1)
        {
            UE_LOG(PubnubLog, Error, TEXT("PubNubAESCrypto: Failed to decrypt data"));
            break;
        }
        TotalLen = Len;
        
        // Finalize decryption (removes PKCS padding)
        if (EVP_DecryptFinal_ex(Context, OutResult.GetData() + Len, &Len) != 1)
        {
            UE_LOG(PubnubLog, Error, TEXT("PubNubAESCrypto: Failed to finalize decryption"));
            break;
        }
        TotalLen += Len;
        
        // Resize to actual decrypted size
        OutResult.SetNum(TotalLen);
        bSuccess = true;
        
    } while (false);
    
    EVP_CIPHER_CTX_free(Context);
    
    if (!bSuccess)
    {
        OutResult.Empty();
    }
    
    return bSuccess;
}

void UPubnubAesCryptor::GenerateRandomIV(uint8* IV, int32 Size)
{
    // Use OpenSSL's secure random number generator (same as original uses rand())
    // This is more secure than the original's rand() function
    if (RAND_bytes(IV, Size) != 1)
    {
        // Fallback to less secure method if RAND_bytes fails
        for (int32 i = 0; i < Size; i++)
        {
            IV[i] = FMath::RandRange(0, 255);
        }
    }
}

bool UPubnubAesCryptor::HashKeySHA256(const FString& Key, uint8* OutHash)
{
    // Convert FString to UTF8
    FTCHARToUTF8 UTF8Key(*Key);
    
    // Use SHA256 (same as pbsha256_digest_str macro)
    if (SHA256(reinterpret_cast<const unsigned char*>(UTF8Key.Get()), UTF8Key.Length(), OutHash) == nullptr)
    {
        return false;
    }
    
    return true;
}

FPubnubEncryptedData UPubnubAesCryptor::ConvertToInterface(const FPubnubEncryptedDataInternal& InternalData)
{
    FPubnubEncryptedData Result;
    Result.EncryptedData = UPubnubCryptoUtilities::Base64Encode(InternalData.Data);
    Result.Metadata = UPubnubCryptoUtilities::Base64Encode(InternalData.Metadata);
    return Result;
}

FPubnubEncryptedDataInternal UPubnubAesCryptor::ConvertFromInterface(const FPubnubEncryptedData& InterfaceData)
{
    FPubnubEncryptedDataInternal Result;
    UPubnubCryptoUtilities::Base64Decode(InterfaceData.EncryptedData, Result.Data);
    UPubnubCryptoUtilities::Base64Decode(InterfaceData.Metadata, Result.Metadata);
    return Result;
}