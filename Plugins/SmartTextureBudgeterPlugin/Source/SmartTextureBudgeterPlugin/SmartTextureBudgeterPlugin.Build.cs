// SmartTextureBudgeterPlugin.Build.cs
using UnrealBuildTool;

public class SmartTextureBudgeterPlugin : ModuleRules
{
    public SmartTextureBudgeterPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        /* ── módulos necessários em runtime ── */
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "AssetRegistry",
            "InputCore"
        });

        /* ── módulos extras quando compilar o Editor ── */
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "ToolMenus",
                "ContentBrowser"     // thumbnails
            });
        }
    }
}
