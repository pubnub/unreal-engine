// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class PubNubModule : ModuleRules
{
    private bool OpenSsl = true;
    private bool StaticLink = true;

    public PubNubModule(ReadOnlyTargetRules Target) : base(Target)
    {
	    Type = ModuleType.External;

	    if (OpenSsl) {
            PublicDependencyModuleNames.AddRange(new string[] { "OpenSSL" });
        }

		var SDKPath = Path.Combine(new string[] { ModuleDirectory, ".." });

		
		
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string BuildLocation = "Lib/win64";
			PublicAdditionalLibraries.Add(Path.Combine(SDKPath, BuildLocation, "pubnub.lib"));
			
		}
		else if(Target.Platform == UnrealTargetPlatform.Mac)
		{
			PublicDelayLoadDLLs.Add(Path.Combine(SDKPath, "lib", "macos", "libpubnub.dylib"));
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/sdk/lib/macos/libpubnub.dylib");
		}
		
		string PlatformLib = "openssl";
		
		
		PublicIncludePaths.AddRange(
			new string[] {
				SDKPath,
				Path.Combine(SDKPath, "Include"),
				Path.Combine(SDKPath, "Include/core"),
				Path.Combine(SDKPath, "Include/Lib"),
				Path.Combine(SDKPath, "Include", PlatformLib)
			}
		);
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        
    }
}
