using UnrealBuildTool;
public class UltimaProjectEditor : ModuleRules
{
    public UltimaProjectEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
				"UnrealEd",
                "PropertyEditor",
                "UltimaProject"
            }
        );
    }
}