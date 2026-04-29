// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

using UnrealBuildTool;

public class PromptGameGenerator : ModuleRules
{
	public PromptGameGenerator(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Landscape",
			"Foliage",
			"ProceduralMeshComponent"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"EditorStyle",
			"UnrealEd",
			"LevelEditor",
			"Projects",
			"ToolMenus",
			"HTTP",
			"Json",
			"JsonUtilities",
			"EditorScriptingUtilities",
			"PropertyEditor",
			"EditorWidgets",
			"Blutility",
			"UMG",
			"AssetTools",
			"ContentBrowser",
			"MaterialEditor",
			"PhysicsCore",
			"Niagara",
			"AssetRegistry",
			"DesktopPlatform"
		});
	}
}
