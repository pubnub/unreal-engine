// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class PUBNUBLIBRARY_API FPubnubLoopingThread : FRunnable
{
public:
	
	FPubnubLoopingThread()
	{
		Thread = FRunnableThread::Create(this, TEXT("PubnubThread"));
	};
	~FPubnubLoopingThread()
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
	
	//Add looping function which will be called ones for every queue loop
	void AddLoopingFunction(TFunction<void()> InFunction);
	//Remove all looping functions
	void ClearLoopingFunctions();

private:
	TArray<TFunction<void()>> PubnubAsyncLoopingFunctionsBuffer;
	TArray<TFunction<void()>> PubnubAsyncLoopingFunctions;
};

