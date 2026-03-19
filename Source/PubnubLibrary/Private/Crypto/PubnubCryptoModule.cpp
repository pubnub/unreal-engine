// Copyright 2026 PubNub Inc. All Rights Reserved.


#include "Crypto/PubnubCryptoModule.h"
#include "PubnubSubsystem.h"
#include "FunctionLibraries/PubnubCryptoUtilities.h"


void UPubnubCryptoModule::InitCryptoModule(const TScriptInterface<IPubnubCryptorInterface>& InDefaultCryptor, const TArray<TScriptInterface<IPubnubCryptorInterface>>& InAdditionalCryptors)
{
    if (!InDefaultCryptor || !InDefaultCryptor.GetObject())
    {
        UE_LOG(PubnubLog, Error, TEXT("[InitCryptoModule]: Default Cryptor is invalid. Aborting init."));
        return;
    }
    DefaultCryptor = InDefaultCryptor;

    for(int i = 0; i < InAdditionalCryptors.Num(); i++)
    {
        if (!InAdditionalCryptors[i] || !InAdditionalCryptors[i].GetObject())
        {
            UE_LOG(PubnubLog, Warning, TEXT("[InitCryptoModule]: Additional Cryptor from index %d is invalid. Skipping this cryptor."), i);
            continue;
        }
        AdditionalCryptors.Add(InAdditionalCryptors[i]);
    }
}

FString UPubnubCryptoModule::ProviderEncrypt_Implementation(const FString& Data)
{
    // Convert to UTF-8 bytes
    TArray<uint8> PlainBytes;
    {
        uint8* tmp=nullptr; size_t sz=0;
        if (!UPubnubCryptoUtilities::ConvertStringToBytes(Data, tmp, sz))
        {
            UE_LOG(PubnubLog, Error, TEXT("[ProviderEncrypt_Implementation]: Conversion to bytes failed."));
            return FString();
        }
        PlainBytes.Append(tmp, sz); free(tmp);
    }

    // Call actual encryption logic
    TArray<uint8> Encoded;
    if (!ProviderEncrypt_Internal(this, PlainBytes, Encoded))
    {
        UE_LOG(PubnubLog, Error, TEXT("[ProviderEncrypt_Implementation]: Internal encryption failed."));
        return FString();
    }
    
    return UPubnubCryptoUtilities::Base64Encode(Encoded.GetData(), Encoded.Num());
}

FString UPubnubCryptoModule::ProviderDecrypt_Implementation(const FString& Data)
{
    // Decode Base64 to bytes
    TArray<uint8> InBytes;
    {
        uint8* tmp=nullptr; size_t sz=0;
        if (!UPubnubCryptoUtilities::Base64Decode(Data, tmp, sz))
        {
            UE_LOG(PubnubLog, Error, TEXT("[ProviderDecrypt_Implementation]: Base64 Decoding failed"));
            return FString();
        }
        InBytes.Append(tmp, sz); free(tmp);
    }

    // Call actual decryption logic
    TArray<uint8> PlainBytes;
    if (!ProviderDecrypt_Internal(this, InBytes, PlainBytes))
    {
        UE_LOG(PubnubLog, Error, TEXT("[ProviderDecrypt_Implementation]: Internal Decryption failed"));
        return FString();
    }
    
    // Convert to FString
    return UPubnubCryptoUtilities::ConvertBytesToString(PlainBytes.GetData(), PlainBytes.Num());
}

bool UPubnubCryptoModule::IdEquals(const uint8 a[4], const uint8 b[4]) {
    return 0 == FMemory::Memcmp(a, b, IdentLen);
}

// Select UE cryptor by identifier
TScriptInterface<IPubnubCryptorInterface> UPubnubCryptoModule::FindUECryptorById(const uint8 ident[4]) const
{
    if (DefaultCryptor && DefaultCryptor.GetObject()) {
        const TArray<uint8> id = IPubnubCryptorInterface::Execute_GetIdentifier(DefaultCryptor.GetObject());
        if (id.Num() == 4 && 0 == FMemory::Memcmp(id.GetData(), ident, 4)) {
            return DefaultCryptor;
        }
    }
    for (const auto& C : AdditionalCryptors) {
        if (!C || !C.GetObject()) continue;
        const TArray<uint8> id = IPubnubCryptorInterface::Execute_GetIdentifier(C.GetObject());
        if (id.Num() == 4 && 0 == FMemory::Memcmp(id.GetData(), ident, 4)) {
            return C;
        }
    }
    return TScriptInterface<IPubnubCryptorInterface>();
}

// Build header size from metadata size
size_t UPubnubCryptoModule::ComputeHeaderSize(size_t metadataSize)
{
    if (metadataSize == 0) return 0; // legacy or no metadata
    const size_t base = SentinelLen + 1 /*ver*/ + IdentLen;
    if (metadataSize < 255) return base + 1 /*len*/ + metadataSize;
    return base + 3 /*255 + hi + lo*/ + metadataSize;
}

// Serialize header into dst (size must be exactly headerSize). Returns offset where metadata should be placed (end of header - metadataSize).
size_t UPubnubCryptoModule::WriteHeader(uint8* dst, size_t headerSize, const uint8 ident[4], size_t metadataSize)
{
    if (headerSize == 0) return 0;
    size_t o = 0;
    FMemory::Memcpy(dst + o, Sentinel, SentinelLen); o += SentinelLen;
    dst[o++] = HeaderVer;
    FMemory::Memcpy(dst + o, ident, IdentLen); o += IdentLen;

    if (metadataSize < 255) {
        dst[o++] = static_cast<uint8>(metadataSize);
    } else {
        dst[o++] = 255;
        const uint16 hdr16 = static_cast<uint16>(headerSize);
        dst[o++] = static_cast<uint8>((hdr16 >> 8) & 0xFF); // high
        dst[o++] = static_cast<uint8>(hdr16 & 0xFF);        // low
    }
    // Metadata goes at the end of header
    return headerSize - metadataSize;
}

// Parse header. Returns true if header present/valid. Outputs:
// - outIdent: 4-byte identifier (legacy zeros if no header)
// - outHeaderSize: total header size in bytes
// - outMetaSize: metadata length in bytes
// - outMetaOffset: where metadata starts (within the full buffer)
bool UPubnubCryptoModule::ParseHeader(const uint8* buf, size_t bufSize, uint8 outIdent[4], size_t& outHeaderSize, size_t& outMetaSize, size_t& outMetaOffset)
{
    outHeaderSize = 0; outMetaSize = 0; outMetaOffset = 0;
    FMemory::Memcpy(outIdent, LegacyId, IdentLen); // default to legacy

    if (bufSize < SentinelLen + 1 + IdentLen + 1) {
        // too small to hold a header; treat as legacy
        return false;
    }

    if (0 != FMemory::Memcmp(buf, Sentinel, SentinelLen)) {
        // no sentinel -> legacy
        return false;
    }

    size_t o = SentinelLen;
    const uint8 ver = buf[o++]; if (ver != HeaderVer) return false;

    FMemory::Memcpy(outIdent, buf + o, IdentLen); o += IdentLen;

    const uint8 lenByte = buf[o++];
    size_t headerSize = 0;
    size_t metaSize   = 0;

    if (lenByte != 255) {
        metaSize   = lenByte;
        headerSize = SentinelLen + 1 + IdentLen + 1 + metaSize;
    } else {
        if (o + 2 > bufSize) return false;
        const uint16 hdr16 = static_cast<uint16>((buf[o] << 8) | buf[o+1]);
        headerSize = hdr16;
        // headerSize = base + 3 + metaSize  => metaSize = headerSize - (base + 3)
        const size_t base = SentinelLen + 1 + IdentLen;
        if (headerSize < base + 3) return false;
        metaSize = headerSize - (base + 3);
    }

    if (headerSize > bufSize) return false;

    outHeaderSize = headerSize;
    outMetaSize   = metaSize;
    outMetaOffset = headerSize - metaSize;
    return true;
}

bool UPubnubCryptoModule::ProviderEncrypt_Internal(UPubnubCryptoModule* Self, const TArray<uint8>& PlainUTF8, TArray<uint8>& OutHeaderPlusCipher)
{
    OutHeaderPlusCipher.Reset();
    if (!Self || !Self->DefaultCryptor || !Self->DefaultCryptor.GetObject()) return false;

    // 1) Call UE cryptor Encrypt
    const FString Plain = UPubnubCryptoUtilities::ConvertBytesToString(PlainUTF8.GetData(), PlainUTF8.Num());
    const FPubnubEncryptedData UEEnc = IPubnubCryptorInterface::Execute_Encrypt(Self->DefaultCryptor.GetObject(), Plain);
    if (UEEnc.EncryptedData.IsEmpty()) return false;

    // 2) Determine identifier and metadata
    const TArray<uint8> identArr = IPubnubCryptorInterface::Execute_GetIdentifier(Self->DefaultCryptor.GetObject());
    uint8 ident[4] = {0,0,0,0};
    if (identArr.Num() >= 4) { FMemory::Memcpy(ident, identArr.GetData(), 4); }
    const bool isLegacy = IdEquals(ident, LegacyId);

    // Decode metadata and ciphertext
    TArray<uint8> Meta; TArray<uint8> Cipher;
    if (!isLegacy && !UEEnc.Metadata.IsEmpty()) {
        uint8* m=nullptr; size_t ms=0; if (UPubnubCryptoUtilities::Base64Decode(UEEnc.Metadata, m, ms)) { Meta.Append(m, ms); free(m);} }
    {
        uint8* c=nullptr; size_t cs=0; if (!UPubnubCryptoUtilities::Base64Decode(UEEnc.EncryptedData, c, cs)) return false; Cipher.Append(c, cs); free(c);
    }

    // 3) Build header + ciphertext
    const size_t headerSize = isLegacy ? 0 : ComputeHeaderSize(Meta.Num());
    OutHeaderPlusCipher.SetNum(headerSize + Cipher.Num());
    if (headerSize > 0) {
        const size_t metaOffset = WriteHeader(OutHeaderPlusCipher.GetData(), headerSize, ident, Meta.Num());
        if (Meta.Num() > 0) { FMemory::Memcpy(OutHeaderPlusCipher.GetData() + metaOffset, Meta.GetData(), Meta.Num()); }
    }
    FMemory::Memcpy(OutHeaderPlusCipher.GetData() + headerSize, Cipher.GetData(), Cipher.Num());
    return true;
}

bool UPubnubCryptoModule::ProviderDecrypt_Internal(UPubnubCryptoModule* Self, const TArray<uint8>& InHeaderPlusCipher, TArray<uint8>& OutPlainUTF8)
{
    OutPlainUTF8.Reset();
    if (!Self || InHeaderPlusCipher.Num() == 0) return false;

    // 1) Parse header (or legacy)
    uint8 ident[4]; size_t headerSize=0, metaSize=0, metaOffset=0;
    const bool hasHeader = ParseHeader(InHeaderPlusCipher.GetData(), InHeaderPlusCipher.Num(), ident, headerSize, metaSize, metaOffset);
    if (!hasHeader) { FMemory::Memcpy(ident, LegacyId, IdentLen); }

    // 2) Select UE cryptor by identifier
    const TScriptInterface<IPubnubCryptorInterface> Cryptor = Self->FindUECryptorById(ident);
    if (!Cryptor || !Cryptor.GetObject()) {
        return false;
    }

    // 3) Build UE data struct and call UE Decrypt
    FPubnubEncryptedData UEData;
    UEData.EncryptedData = UPubnubCryptoUtilities::Base64Encode(InHeaderPlusCipher.GetData()+headerSize, InHeaderPlusCipher.Num()-headerSize);
    if (hasHeader && metaSize > 0) {
        UEData.Metadata = UPubnubCryptoUtilities::Base64Encode(InHeaderPlusCipher.GetData()+metaOffset, metaSize);
    }
    const FString Plain = IPubnubCryptorInterface::Execute_Decrypt(Cryptor.GetObject(), UEData);
    if (Plain.IsEmpty()) return false;

    // UTF-8 back to bytes
    uint8* tmp=nullptr; size_t sz=0; if (!UPubnubCryptoUtilities::ConvertStringToBytes(Plain, tmp, sz)) return false;
    OutPlainUTF8.Append(tmp, sz); free(tmp); return true;
}
