#include "FunctionLibraries/PubnubLogUtilities.h"

FString UPubnubLogUtilities::FormatIso8601UtcWithMilliseconds(const FDateTime& InUtcTime)
{
	return FString::Printf(
		TEXT("%04d-%02d-%02dT%02d:%02d:%02d.%03dZ"),
		InUtcTime.GetYear(),
		InUtcTime.GetMonth(),
		InUtcTime.GetDay(),
		InUtcTime.GetHour(),
		InUtcTime.GetMinute(),
		InUtcTime.GetSecond(),
		InUtcTime.GetMillisecond()
	);
}
