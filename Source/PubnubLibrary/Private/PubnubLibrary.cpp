// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "PubnubLibrary.h"
#include "Core.h"
#include "Kismet/KismetSystemLibrary.h"

#define LOCTEXT_NAMESPACE "FPubnubLibraryModule"

void FPubnubLibraryModule::StartupModule()
{
}

void FPubnubLibraryModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPubnubLibraryModule, PubnubLibrary)