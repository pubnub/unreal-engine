// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PubnubBaseLogger.h"
#include "PubnubDefaultLogger.generated.h"

UCLASS(BlueprintType, Blueprintable)
class PUBNUBLIBRARY_API UPubnubDefaultLogger : public UPubnubBaseLogger
{
	GENERATED_BODY()

public:
	virtual void Log_Implementation(const FPubnubLogMessage& LogMessage) override;
};
