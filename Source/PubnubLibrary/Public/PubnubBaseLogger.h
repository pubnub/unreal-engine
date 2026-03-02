#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Interfaces/PubnubLoggerInterface.h"
#include "PubnubBaseLogger.generated.h"

UCLASS(BlueprintType, Blueprintable)
class PUBNUBLIBRARY_API UPubnubBaseLogger : public UObject, public IPubnubLoggerInterface
{
	GENERATED_BODY()

public:
	virtual void Log_Implementation(const FPubnubLogMessage& LogMessage) override;
	virtual void LogTrace_Implementation(const FPubnubLogMessage& LogMessage) override;
	virtual void LogDebug_Implementation(const FPubnubLogMessage& LogMessage) override;
	virtual void LogInfo_Implementation(const FPubnubLogMessage& LogMessage) override;
	virtual void LogWarning_Implementation(const FPubnubLogMessage& LogMessage) override;
	virtual void LogError_Implementation(const FPubnubLogMessage& LogMessage) override;

	virtual void SetMinimumLogLevel_Implementation(EPubnubLogLevel InLevel) override;
	virtual EPubnubLogLevel GetMinimumLogLevel_Implementation() const override;

	virtual void SetMinimumCCoreLogLevel_Implementation(EPubnubLogLevel InLevel) override;
	virtual EPubnubLogLevel GetMinimumCCoreLogLevel_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pubnub|Logger")
	EPubnubLogLevel MinimumLogLevel = EPubnubLogLevel::PLL_Trace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pubnub|Logger")
	EPubnubLogLevel MinimumCCoreLogLevel = EPubnubLogLevel::PLL_Trace;
};
