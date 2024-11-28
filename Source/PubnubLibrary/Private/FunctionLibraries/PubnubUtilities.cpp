// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "FunctionLibraries/PubnubUtilities.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Json.h"


FString UPubnubUtilities::AddQuotesToString(const FString InString, bool SkipIfHasQuotes)
{
	if(InString.Left(1) != "\"" || InString.Right(1) != "\"" || !SkipIfHasQuotes)
	{
		return "\"" + InString + "\"";
	}

	return InString;
}

FString UPubnubUtilities::PubnubCharMemBlockToString(const pubnub_char_mem_block PnChar)
{
	if(!PnChar.ptr)
	{
		return "";
	}

#if ENGINE_MINOR_VERSION <= 3
	//This constructor is deprecated since 5.4
	return FString(PnChar.size, PnChar.ptr);
#else
	return FString::ConstructFromPtrSize(PnChar.ptr, PnChar.size);
#endif
}