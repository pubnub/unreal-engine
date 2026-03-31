// Copyright 2026 PubNub Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class PubNubModule : ModuleRules
{
    public PubNubModule(ReadOnlyTargetRules Target) : base(Target)
    {
	    Type = ModuleType.External;
	    
        string binary = null;
        string BuildLocation = null;
        
        if(Target.Platform == UnrealTargetPlatform.Win64)
        {
	        binary = $"pubnub.lib";
		    BuildLocation = "lib/win64";
        }
        else if(Target.Platform == UnrealTargetPlatform.Mac)
        {
	        binary = $"libpubnub.a";
            BuildLocation = "lib/MacOS";
        }
        else if(Target.Platform == UnrealTargetPlatform.Android)
        {
	        binary = $"libpubnub.a";
	        BuildLocation = "lib/arm64";
        }
        else if(Target.Platform == UnrealTargetPlatform.IOS)
        {
	        binary = $"libpubnub.a";
	        BuildLocation = "lib/ios";
        }
        else if(Target.Platform == UnrealTargetPlatform.Linux)
        {
	        binary = $"libpubnub.a";
	        BuildLocation = "lib/linux";
        }
        else
        {
	        System.Console.WriteLine("Error - this target platform is not supported");
        }

		var SDKPath = Path.Combine(new string[] { ModuleDirectory, ".." });

		PublicAdditionalLibraries.Add(Path.Combine(SDKPath, BuildLocation, binary));
		PublicIncludePaths.AddRange(
			new string[] {
				SDKPath,
				Path.Combine(SDKPath, "Include"),
				Path.Combine(SDKPath, "Include/core"),
				Path.Combine(SDKPath, "Include/Lib"),
				Path.Combine(SDKPath, "Include/openssl")
			}
		);
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        
    }
}
