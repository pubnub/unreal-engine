// Copyright 2026 PubNub Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class PubNubModule : ModuleRules
{
	public PubNubModule(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		string SDKPath = Path.Combine(ModuleDirectory, "..");

		// New C-Core public headers are included as "pubnub/..." (e.g. pubnub/pubnub.h).
		PublicSystemIncludePaths.Add(Path.Combine(SDKPath, "Include"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.Add(Path.Combine(SDKPath, "lib", "win64", "pubnub.lib"));

			// pubnub.lib is a static archive of C-Core OBJECT libraries only.
			// CMake usage requirements are not merged into the .lib, so consumers
			// must link the same deps the Windows C-Core build used:
			// socket + OpenSSL TLS, custom DNS, and proxy WPAD.
			// zlib is linked on every platform: compression middleware calls
			// deflate/inflate/compressBound, and those objects are not in the archive.
			PublicDependencyModuleNames.AddRange(new string[]
			{
				"OpenSSL",
				"zlib"
			});

			PublicSystemLibraries.AddRange(new string[]
			{
				"ws2_32.lib",
				"crypt32.lib",
				"bcrypt.lib",
				"iphlpapi.lib",
				"winhttp.lib"
			});
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PublicAdditionalLibraries.Add(Path.Combine(SDKPath, "lib", "MacOS", "libpubnub.a"));
			AddAppleCCoreSystemLibraries();
			AddZlibDependency();
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PublicAdditionalLibraries.Add(Path.Combine(SDKPath, "lib", "arm64", "libpubnub.a"));
			AddZlibDependency();
		}
		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PublicAdditionalLibraries.Add(Path.Combine(SDKPath, "lib", "ios", "libpubnub.a"));
			AddAppleCCoreSystemLibraries();
			AddZlibDependency();
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			PublicAdditionalLibraries.Add(Path.Combine(SDKPath, "lib", "linux", "libpubnub.a"));
			AddZlibDependency();
		}
		else
		{
			System.Console.WriteLine("Error - this target platform is not supported");
		}
	}

	// Compression middleware in every shipped archive calls zlib. Those objects
	// are not archived into libpubnub, so each platform must link zlib itself.
	private void AddZlibDependency()
	{
		PublicDependencyModuleNames.Add("zlib");
	}

	// libpubnub.a is a static archive of C-Core objects. CMake usage requirements
	// are not stored in the archive, so Apple consumers must link the same
	// system libraries the socket transport build used
	private void AddAppleCCoreSystemLibraries()
	{
		PublicSystemLibraries.Add("resolv");
		PublicFrameworks.Add("CoreFoundation");
		PublicFrameworks.Add("CFNetwork");
	}
}
