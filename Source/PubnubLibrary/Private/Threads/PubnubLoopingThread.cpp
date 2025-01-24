// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "Threads/PubnubLoopingThread.h"
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



bool FPubnubLoopingThread::Init()
{
	return true;
}

uint32 FPubnubLoopingThread::Run()
{
	
	while(!bShutdown)
	{
		FCriticalSection Mutex;
		Mutex.Lock();
		//Add all functions from buffer to the actual looping array
		PubnubAsyncLoopingFunctions.Append(PubnubAsyncLoopingFunctionsBuffer);
		PubnubAsyncLoopingFunctionsBuffer.Empty();
		Mutex.Unlock();
		
		//Run all looping functions

		if(!PubnubAsyncLoopingFunctions.IsEmpty())
		{
			//Run all functions from queue
			for(int i = 0; i <  PubnubAsyncLoopingFunctions.Num(); i++)
			{
				if(bShutdown)
				{
					return 0;
				}
				PubnubAsyncLoopingFunctions[i]();
			}
		}
		
		FPlatformProcess::Sleep(0.05f);
	}
	return 0;
}

void FPubnubLoopingThread::Exit()
{
}

void FPubnubLoopingThread::Stop()
{
	bShutdown = true;
}

void FPubnubLoopingThread::AddLoopingFunction(TFunction<void()> InFunction)
{
	FCriticalSection Mutex;
	Mutex.Lock();
	PubnubAsyncLoopingFunctionsBuffer.Add(InFunction);
	Mutex.Unlock();
}


void FPubnubLoopingThread::ClearLoopingFunctions()
{
	FCriticalSection Mutex;
	Mutex.Lock();
	PubnubAsyncLoopingFunctions.Empty();
	Mutex.Unlock();
}
