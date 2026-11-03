// Copyright 2026 PubNub Inc. All Rights Reserved.

using UnrealBuildTool;

public class PubnubLibraryTests : ModuleRules
{
	public PubnubLibraryTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bDisableStaticAnalysis = true;

		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"PubnubLibrary",
				"Projects"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Json",
				"JsonUtilities"
			}
			);
	}
}
