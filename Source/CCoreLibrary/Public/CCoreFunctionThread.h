// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UCCoreSubsystemAsync;

/**
 * 
 */
class CCORELIBRARY_API FCCoreFunctionThread : FRunnable
{
public:
	
	FCCoreFunctionThread()
	{
		Thread = FRunnableThread::Create(this, TEXT("CCoreThread"));
	};
	~FCCoreFunctionThread()
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
	TArray<TFunction<void()>> CCoreAsyncFunctionsBuffer;
	TArray<TFunction<void()>> CCoreAsyncFunctionsQueue;
};

