using UnrealBuildTool;

public class IRProbe : ModuleRules
{
    public IRProbe(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "SignalProcessing"
            }
        );

        PrivateDependencyModuleNames.Add("Synthesis");

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("AssetTools");
            PrivateDependencyModuleNames.Add("AudioEditor");
            PrivateDependencyModuleNames.Add("SynthesisEditor");
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }
}
