// Fill out your copyright notice in the Description page of Project Settings.


#include "Threads/CCoreLoopingThread.h"


bool FCCoreLoopingThread::Init()
{
	return true;
}

uint32 FCCoreLoopingThread::Run()
{
	
	while(!bShutdown)
	{
		FCriticalSection Mutex;
		Mutex.Lock();
		//Add all functions from buffer to the actual looping array
		CCoreAsyncLoopingFunctions.Append(CCoreAsyncLoopingFunctionsBuffer);
		CCoreAsyncLoopingFunctionsBuffer.Empty();
		Mutex.Unlock();
		
		//Run all looping functions

		if(!CCoreAsyncLoopingFunctions.IsEmpty())
		{
			//Run all functions from queue
			for(int i = 0; i <  CCoreAsyncLoopingFunctions.Num(); i++)
			{
				UE_LOG(LogTemp, Warning, TEXT("Run CCore Async looping function"));
				CCoreAsyncLoopingFunctions[i]();
			}
		}
		
		FPlatformProcess::Sleep(0.05f);
	}
	return 0;
}

void FCCoreLoopingThread::Exit()
{
}

void FCCoreLoopingThread::Stop()
{
	bShutdown = true;
}

void FCCoreLoopingThread::AddLoopingFunction(TFunction<void()> InFunction)
{
	FCriticalSection Mutex;
	Mutex.Lock();
	CCoreAsyncLoopingFunctionsBuffer.Add(InFunction);
	Mutex.Unlock();
}


void FCCoreLoopingThread::ClearLoopingFunctions()
{
	FCriticalSection Mutex;
	Mutex.Lock();
	CCoreAsyncLoopingFunctions.Empty();
	Mutex.Unlock();
}
