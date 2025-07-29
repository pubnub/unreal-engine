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

	void LockForSubscribeOperation();
	void UnlockAfterSubscriptionOperationFinished();

private:
	TArray<TFunction<void()>> PubnubAsyncFunctionsBuffer;
	TArray<TFunction<void()>> PubnubAsyncFunctionsQueue;
	
	void WaitForSubscriptionOperationEnd();
	
	bool IsLockedForSubscription = false;
	float QueueLoopDelay = 0.05f;
	float WaitForSubscriptionDelay = 0.05f;
	int WaitForSubscriptionDelayMaxCount = 6200; //310 seconds, 300 from c-core reconnection timeout and + additional few seconds
};

