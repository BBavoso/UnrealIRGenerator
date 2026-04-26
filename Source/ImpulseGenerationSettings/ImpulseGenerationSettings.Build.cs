using UnrealBuildTool;

public class ImpulseGenerationSettings : ModuleRules
{
    public ImpulseGenerationSettings(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "DeveloperSettings",
                "Chaos",
                "PhysicsCore",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "EditorSubsystem",
            }
        );
        
        if (Target.bBuildEditor) {
            // PrivateDependencyModuleNames.Add("EditorSubsystem");
            // PublicDependencyModuleNames.Add("EditorSubsystem");
        }
    }
}