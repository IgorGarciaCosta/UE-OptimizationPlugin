// SmartTextureBudgeterPlugin.Build.cs
using UnrealBuildTool;

public class SmartTextureBudgeterPlugin : ModuleRules
{
    public SmartTextureBudgeterPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        /* ---- módulos necessários em runtime ---- */
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "AssetRegistry",
            "InputCore"
        });

        /* ---- módulos extras para o Editor ---- */
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "ToolMenus",
                "ContentBrowser",
                "TextureUtilitiesCommon",
                "TextureEditor"        //  << aqui agora
            });

            /* expõe headers da pasta Private do módulo ---------------- */
            PrivateIncludePathModuleNames.AddRange(new string[]
            {
                "TextureEditor"        //  << caminho p/ TextureEditorSubsystem.h
            });
        }
    }
}
