// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "FunctionLibraries/PubnubTimetokenUtilities.h"
#include "Misc/DateTime.h"


FString UPubnubTimetokenUtilities::GetCurrentUnixTimetoken()
{
	FDateTime Now = FDateTime::UtcNow();

	// Unix timestamp in seconds
	int64 UnixSeconds = Now.ToUnixTimestamp();

	// Convert to 100ns units: seconds * 10^7 + fraction
	int64 Timetoken = UnixSeconds * 10000000LL;

	// Add the fraction (ticks) — FDateTime::GetTicks() is in 100ns units
	Timetoken += (Now.GetTicks() % ETimespan::TicksPerSecond) % 10000000LL;

	return FString::Printf(TEXT("%lld"), Timetoken);
}