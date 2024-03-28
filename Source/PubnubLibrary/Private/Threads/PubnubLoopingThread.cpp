// Fill out your copyright notice in the Description page of Project Settings.


#include "Threads/PubnubLoopingThread.h"


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
				UE_LOG(LogTemp, Warning, TEXT("Run Pubnub Async looping function"));
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
