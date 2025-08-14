// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "FunctionLibraries/PubnubCryptoUtilities.h"


bool UPubnubCryptoUtilities::Base64Decode(const FString& In, TArray<uint8>& Out)
{
	return FBase64::Decode(In, Out);
}

bool UPubnubCryptoUtilities::Base64Decode(const FString& B64, uint8*& OutData, size_t& OutSize)
{
	OutData=nullptr; OutSize=0; if (B64.IsEmpty()) return false;
	TArray<uint8> Dec; if (!FBase64::Decode(B64, Dec)) return false;
	OutSize = Dec.Num(); OutData = static_cast<uint8*>(malloc(OutSize)); if (!OutData){OutSize=0; return false;}
	FMemory::Memcpy(OutData, Dec.GetData(), OutSize); return true;
}

FString UPubnubCryptoUtilities::Base64Encode(const TArray<uint8>& In)
{
	return FBase64::Encode(In.GetData(), In.Num());
}

FString UPubnubCryptoUtilities::Base64Encode(const uint8* Data, size_t Size)
{
	if (!Data || Size==0) return FString();
	return FBase64::Encode(Data, Size);
}

FString UPubnubCryptoUtilities::ConvertBytesToString(const uint8* Data, size_t Size)
{
	if (!Data || Size == 0)
	{
		return FString();
	}

	// Create a temporary buffer with null terminator
	TArray<uint8> TempBuffer;
	TempBuffer.Append(Data, Size);
	TempBuffer.Add(0); // Null terminator

	// Convert to FString assuming UTF-8 encoding
	return FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(TempBuffer.GetData())));
}

FString UPubnubCryptoUtilities::ConvertBytesToBase64(const uint8* Data, size_t Size)
{
	if (!Data || Size == 0)
	{
		return FString();
	}

	return FBase64::Encode(Data, Size);
}

bool UPubnubCryptoUtilities::ConvertBase64ToBytes(const FString& Base64String, uint8*& OutData, size_t& OutSize)
{
	if (Base64String.IsEmpty())
	{
		OutData = nullptr;
		OutSize = 0;
		return false;
	}

	TArray<uint8> DecodedData;
	if (!FBase64::Decode(Base64String, DecodedData))
	{
		OutData = nullptr;
		OutSize = 0;
		return false;
	}

	// Allocate memory using malloc (C-Core expects this)
	OutSize = DecodedData.Num();
	OutData = static_cast<uint8*>(malloc(OutSize));
	if (!OutData)
	{
		OutSize = 0;
		return false;
	}

	// Copy data
	FMemory::Memcpy(OutData, DecodedData.GetData(), OutSize);
	return true;
}

bool UPubnubCryptoUtilities::ConvertStringToBytes(const FString& String, uint8*& OutData, size_t& OutSize)
{
	if (String.IsEmpty()) { OutData = nullptr; OutSize = 0; return false; }

	FTCHARToUTF8 UTF8String(*String);
	const int32 BytesLen = UTF8String.Length(); // no terminator included

	OutData = static_cast<uint8*>(malloc(BytesLen + 1)); // +1 for '\0'
	if (!OutData) { OutSize = 0; return false; }

	FMemory::Memcpy(OutData, UTF8String.Get(), BytesLen);
	OutData[BytesLen] = '\0';                // ensure ASCIIZ
	OutSize = static_cast<size_t>(BytesLen); // report logical size without terminator
	return true;
}