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

	    string extention = null;
        string PlatformLib = null;
        string binary = null;
        string buildLocation = null;
        if(Target.Platform == UnrealTargetPlatform.Win64)
        {
	        extention = StaticLink ? "lib" : "dll";
	        PlatformLib = OpenSsl ? "openssl" : "windows";
	        binary = $"pubnub.{extention}";
            BuildLocation = "Lib/win64";
        }
        else if(Target.Platform == UnrealTargetPlatform.Mac)
        {
	        extention = StaticLink ? "a" : "dylib";
	        PlatformLib = OpenSsl ? "openssl" : "posix";
	        binary = $"libpubnub.{extention}";
            BuildLocation = "Lib/MacOS";
        }
        else
        {
	        extention = StaticLink ? "a" : "so";
	        PlatformLib = OpenSsl ? "openssl" : "posix";
	        binary = $"libpubnub.{extention}";
            BuildLocation = "Lib/Linux";
        }

        if (OpenSsl) {
            PublicDependencyModuleNames.AddRange(new string[] { "OpenSSL" });
        }

		var SDKPath = Path.Combine(new string[] { ModuleDirectory, ".." });
		
		PublicAdditionalLibraries.Add(Path.Combine(SDKPath, BuildLocation, binary));
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
