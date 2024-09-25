// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "Threads/PubnubFunctionThread.h"
#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformProcess.h"
#elif PLATFORM_MAC
#include "Mac/MacPlatformProcess.h"
#else
#error "Platform not supported"
#endif


bool FPubnubFunctionThread::Init()
{
	return true;
}

uint32 FPubnubFunctionThread::Run()
{
	
	while(!bShutdown)
	{
		if(!PubnubAsyncFunctionsQueue.IsEmpty())
		{
			//Run all functions from the queue
			for(int i = 0; i <  PubnubAsyncFunctionsQueue.Num(); i++)
			{
				PubnubAsyncFunctionsQueue[i]();
			}
			
			//Clear queue
			PubnubAsyncFunctionsQueue.Empty();
		}

		//Add functions from buffer to queue. Lock Mutex until everything is finished.
		FCriticalSection Mutex;
		Mutex.Lock();
		PubnubAsyncFunctionsQueue = PubnubAsyncFunctionsBuffer;
		PubnubAsyncFunctionsBuffer.Empty();
		Mutex.Unlock();
		
		FPlatformProcess::Sleep(0.05f);
	}
	return 0;
}

void FPubnubFunctionThread::Exit()
{
}

void FPubnubFunctionThread::Stop()
{
	bShutdown = true;
}

void FPubnubFunctionThread::AddFunctionToQueue(TFunction<void()> InFunction)
{
	//Add function to buffer firstly, it will be added to queue after current queue is done
	//Lock this array for other threads, so it can be added safely
	FCriticalSection Mutex;
	Mutex.Lock();
	PubnubAsyncFunctionsBuffer.Add(InFunction);
	Mutex.Unlock();
}
