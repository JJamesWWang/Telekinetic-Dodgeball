// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TD : ModuleRules
{
	public TD(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
		PrivateDependencyModuleNames.AddRange(new string[] { "UMG", "OnlineSubsystem", "OnlineSubsystemNull", "OnlineSubsystemSteam", "PhysicsCore", "GameplayAbilities", "GameplayTags", "GameplayTasks" });

		PrivateIncludePaths.AddRange(new string[] { "TD", "TD/System" });
	}
}
