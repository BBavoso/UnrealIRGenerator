using UnrealBuildTool;

public class IRProbe : ModuleRules {
	public IRProbe(ReadOnlyTargetRules Target) : base(Target) {
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"Synthesis",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"SignalProcessing",
				"Synthesis",
				"ImpulseGenerationSettings",
			}
		);

		if (Target.bBuildEditor) {
			PrivateDependencyModuleNames.Add("AssetTools");
			PrivateDependencyModuleNames.Add("AudioEditor");
			PrivateDependencyModuleNames.Add("SynthesisEditor");
			PrivateDependencyModuleNames.Add("UnrealEd");
			PrivateDependencyModuleNames.Add("EditorScriptingUtilities");
			PrivateDependencyModuleNames.Add("ContentBrowser");
		}
	}
}