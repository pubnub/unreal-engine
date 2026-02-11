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
		TArray<TFunction<void()>> FunctionsToExecute;
		{
			FScopeLock QueueLock(&QueueMutex);
			if(PubnubAsyncFunctionsQueue.IsEmpty() && !PubnubAsyncFunctionsBuffer.IsEmpty())
			{
				PubnubAsyncFunctionsQueue = MoveTemp(PubnubAsyncFunctionsBuffer);
			}

			if(!PubnubAsyncFunctionsQueue.IsEmpty())
			{
				FunctionsToExecute = MoveTemp(PubnubAsyncFunctionsQueue);
			}
		}

		if(!FunctionsToExecute.IsEmpty())
		{
			//Run all functions from the queue
			for(int i = 0; i < FunctionsToExecute.Num(); i++)
			{
				FunctionsToExecute[i]();

				//Stop executing functions if thread was set to Shutdown
				if(bShutdown)
				{
					break;
				}
			}
		}
		
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
	FScopeLock QueueLock(&QueueMutex);
	PubnubAsyncFunctionsBuffer.Add(InFunction);
}
