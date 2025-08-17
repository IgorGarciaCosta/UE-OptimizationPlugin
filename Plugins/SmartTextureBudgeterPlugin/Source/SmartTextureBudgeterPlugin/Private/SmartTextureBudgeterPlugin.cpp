#include "SmartTextureBudgeterPlugin.h"

#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "STextureBudgetWindow.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
static const FName TextureBudgetTabName(TEXT("TextureBudgetTab"));
#endif

#define LOCTEXT_NAMESPACE "FSmartTextureBudgeterPluginModule"

/*──────────────────────────────
	Startup / Shutdown
──────────────────────────────*/
void FSmartTextureBudgeterPluginModule::StartupModule()
{
#if WITH_EDITOR
	RegisterTabSpawner();
	RegisterMenus();
#endif
}

void FSmartTextureBudgeterPluginModule::ShutdownModule()
{
#if WITH_EDITOR
	UnregisterMenus();
	UnregisterTabSpawner();
#endif
}

/*──────────────────────────────
	Editor helpers
──────────────────────────────*/
#if WITH_EDITOR

void FSmartTextureBudgeterPluginModule::RegisterTabSpawner()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		TextureBudgetTabName,
		FOnSpawnTab::CreateRaw(this, &FSmartTextureBudgeterPluginModule::SpawnTextureBudgetTab))
		.SetDisplayName(LOCTEXT("TextureBudgetTabTitle", "Texture Budget"))
		.SetTooltipText(LOCTEXT("TextureBudgetTabTooltip", "Open the Smart Texture Budgeter window"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FSmartTextureBudgeterPluginModule::UnregisterTabSpawner()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TextureBudgetTabName);
}

TSharedRef<SDockTab> FSmartTextureBudgeterPluginModule::SpawnTextureBudgetTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(STextureBudgetWindow)
		];
}

/*----  Menu na barra Window ----*/
void FSmartTextureBudgeterPluginModule::RegisterMenus()
{
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateLambda([]
			{
				UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
				FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
				Section.AddMenuEntry(
					"OpenTextureBudgeter",
					LOCTEXT("MenuEntryTitle", "Smart Texture Budgeter"),
					LOCTEXT("MenuEntryTooltip", "Open the Smart Texture Budgeter window"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([]()
						{
							FGlobalTabmanager::Get()->TryInvokeTab(TextureBudgetTabName);
						}))
				);
			}));
}

void FSmartTextureBudgeterPluginModule::UnregisterMenus()
{
	/* UE 5.4 – não existe mais IsToolMenusEnabled().
	   Verificamos apenas se o sistema já está inicializado. */
	if (UToolMenus::Get())
	{
		UToolMenus::UnregisterOwner(this);
	}
}
#endif	// WITH_EDITOR

#undef LOCTEXT_NAMESPACE
IMPLEMENT_MODULE(FSmartTextureBudgeterPluginModule, SmartTextureBudgeterPlugin)
