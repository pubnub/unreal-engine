#include "PubnubLibrary.h"
#include "Core.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FPubnubLibraryModule"

void FPubnubLibraryModule::StartupModule()
{
#if PLATFORM_MAC
	FString BaseDir = IPluginManager::Get().FindPlugin("PubnubChat")->GetBaseDir();
	FString LibraryPath;
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/sdk/lib/macos/libpubnub-chat.dylib"));

	
	PubnubLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (!PubnubLibraryHandle)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ChatSDKModuleHandle", "Failed to load pubnub third party library"));
	}
#endif
}

void FPubnubLibraryModule::ShutdownModule()
{
#if PLATFORM_MAC
	FPlatformProcess::FreeDllHandle(PubnubLibraryHandle);
	PubnubLibraryHandle = nullptr;
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPubnubLibraryModule, PubnubLibrary)
