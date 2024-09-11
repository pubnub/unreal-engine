// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"

/**
 * 
 */
class PUBNUBLIBRARY_API FPubnubFunctionThread : FRunnable
{
public:
	
	FPubnubFunctionThread()
	{
		Thread = FRunnableThread::Create(this, TEXT("PubnubThread"));
	};
	~FPubnubFunctionThread()
	{
		if(Thread)
		{
			Thread->Kill();
			delete Thread;
		}
	};
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;
	
	FRunnableThread* Thread;
	bool bShutdown = false;

	//Add function to the queue buffer, it will be added to the queue after current queue is finished
	void AddFunctionToQueue(TFunction<void()> InFunction);

private:
	TArray<TFunction<void()>> PubnubAsyncFunctionsBuffer;
	TArray<TFunction<void()>> PubnubAsyncFunctionsQueue;
};

