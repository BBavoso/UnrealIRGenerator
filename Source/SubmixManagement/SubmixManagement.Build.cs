using UnrealBuildTool;

public class SubmixManagement : ModuleRules
{
    public SubmixManagement(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", 
                "Synthesis",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "AudioMixer",
                "Synthesis",
            }
        );

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("IRProbe");
        }
    }
}