#include "PubnubBaseLogger.h"

void UPubnubBaseLogger::Log_Implementation(const FPubnubLogMessage& LogMessage)
{
}

void UPubnubBaseLogger::LogTrace_Implementation(const FPubnubLogMessage& LogMessage)
{
	IPubnubLoggerInterface::Execute_Log(this, LogMessage);
}

void UPubnubBaseLogger::LogDebug_Implementation(const FPubnubLogMessage& LogMessage)
{
	IPubnubLoggerInterface::Execute_Log(this, LogMessage);
}

void UPubnubBaseLogger::LogInfo_Implementation(const FPubnubLogMessage& LogMessage)
{
	IPubnubLoggerInterface::Execute_Log(this, LogMessage);
}

void UPubnubBaseLogger::LogWarning_Implementation(const FPubnubLogMessage& LogMessage)
{
	IPubnubLoggerInterface::Execute_Log(this, LogMessage);
}

void UPubnubBaseLogger::LogError_Implementation(const FPubnubLogMessage& LogMessage)
{
	IPubnubLoggerInterface::Execute_Log(this, LogMessage);
}

void UPubnubBaseLogger::SetMinimumLogLevel_Implementation(EPubnubLogLevel InLevel)
{
	MinimumLogLevel = InLevel;
}

EPubnubLogLevel UPubnubBaseLogger::GetMinimumLogLevel_Implementation() const
{
	return MinimumLogLevel;
}

void UPubnubBaseLogger::SetMinimumCCoreLogLevel_Implementation(EPubnubLogLevel InLevel)
{
	MinimumCCoreLogLevel = InLevel;
}

EPubnubLogLevel UPubnubBaseLogger::GetMinimumCCoreLogLevel_Implementation() const
{
	return MinimumCCoreLogLevel;
}
