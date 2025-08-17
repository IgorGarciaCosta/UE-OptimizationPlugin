#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "Widgets/Docking/SDockTab.h"
#endif

/**
 * Módulo único (Runtime + Editor-Only em #if WITH_EDITOR)
 */
class FSmartTextureBudgeterPluginModule : public IModuleInterface
{
public:
	// IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

#if WITH_EDITOR
private:
	/*--- Helpers para Editor ---*/
	void RegisterTabSpawner();
	void UnregisterTabSpawner();

	void RegisterMenus();
	void UnregisterMenus();

	TSharedRef<SDockTab> SpawnTextureBudgetTab(const FSpawnTabArgs& Args);
#endif
};
