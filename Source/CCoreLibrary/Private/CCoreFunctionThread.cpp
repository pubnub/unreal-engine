// Fill out your copyright notice in the Description page of Project Settings.


#include "CCoreFunctionThread.h"


bool FCCoreFunctionThread::Init()
{
	return true;
}

uint32 FCCoreFunctionThread::Run()
{
	
	while(!bShutdown)
	{
		if(!CCoreAsyncFunctionsQueue.IsEmpty())
		{
			//Run all functions from the queue
			for(int i = 0; i <  CCoreAsyncFunctionsQueue.Num(); i++)
			{
				CCoreAsyncFunctionsQueue[i]();
			}
			
			//Clear queue
			CCoreAsyncFunctionsQueue.Empty();
		}

		//Add functions from buffer to queue. Lock Mutex until everything is finished.
		FCriticalSection Mutex;
		Mutex.Lock();
		CCoreAsyncFunctionsQueue = CCoreAsyncFunctionsBuffer;
		CCoreAsyncFunctionsBuffer.Empty();
		Mutex.Unlock();
		
		FPlatformProcess::Sleep(0.05f);
	}
	return 0;
}

void FCCoreFunctionThread::Exit()
{
}

void FCCoreFunctionThread::Stop()
{
	bShutdown = true;
}

void FCCoreFunctionThread::AddFunctionToQueue(TFunction<void()> InFunction)
{
	//Add function to buffer firstly, it will be added to queue after current queue is done
	//Lock this array for other threads, so it can be added safely
	FCriticalSection Mutex;
	Mutex.Lock();
	CCoreAsyncFunctionsBuffer.Add(InFunction);
	Mutex.Unlock();
}