// Copyright 2024 PubNub Inc. All Rights Reserved.

using UnrealBuildTool;

public class PubnubLibraryTests : ModuleRules
{
	public PubnubLibraryTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		

		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"PubNubLibrary",
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
