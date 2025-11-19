// Copyright 2025 PubNub Inc. All Rights Reserved.


#include "Threads/PubnubFunctionThread.h"
#include "PubnubSubsystem.h"
#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformProcess.h"
#elif PLATFORM_MAC
#include "Mac/MacPlatformProcess.h"
#elif PLATFORM_ANDROID
#include "Android/AndroidPlatformProcess.h"
#elif PLATFORM_IOS
#include "Ios/IosPlatformProcess.h"
#elif PLATFORM_LINUX
#include "Linux/LinuxPlatformProcess.h"
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
				
				//If last called operation is subscribe operation, we need to wait until it's finished as it's not blocking
				if(IsLockedForSubscription)
				{
					WaitForSubscriptionOperationEnd();
				}

				//Stop executing functions if thread was set to Shutdown
				if(bShutdown)
				{
					break;
				}
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
		
		FPlatformProcess::Sleep(QueueLoopDelay);
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

void FPubnubFunctionThread::LockForSubscribeOperation()
{
	IsLockedForSubscription = true;
}

void FPubnubFunctionThread::UnlockAfterSubscriptionOperationFinished()
{
	IsLockedForSubscription = false;
}

void FPubnubFunctionThread::WaitForSubscriptionOperationEnd()
{
	int LoopCount = 0;
	while(IsLockedForSubscription)
	{
		if(bShutdown)
		{
			break;
		}
		
		FPlatformProcess::Sleep(WaitForSubscriptionDelay);
		LoopCount++;
		if(LoopCount > WaitForSubscriptionDelayMaxCount)
		{
			UE_LOG(PubnubLog, Error, TEXT("WaitForSubscriptionOperationEnd exausted all tries. Skipping waiting for subscription result."))
			break;
		}

	}
}
