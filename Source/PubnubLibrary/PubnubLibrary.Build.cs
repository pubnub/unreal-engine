// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PubnubLibrary : ModuleRules
{
	public PubnubLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		

		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"PubNubModule",
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
				"JsonUtilities",
				"DeveloperSettings"
			}
			);
	}
}
