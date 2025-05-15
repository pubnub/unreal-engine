// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "Misc/DateTime.h"
#include "FunctionLibraries/PubnubTimetokenUtilities.h"


FString UPubnubTimetokenUtilities::GetCurrentUnixTimetoken()
{
	FDateTime Now = FDateTime::UtcNow();
	int64 UnixTimeMicroseconds = Now.ToUnixTimestamp() * 1000000LL + Now.GetMillisecond() * 1000;
	return FString::Printf(TEXT("%lld"), UnixTimeMicroseconds);
}