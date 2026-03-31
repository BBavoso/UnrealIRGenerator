#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Docking/SDockTab.h"

class FIRBakingModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
    void PluginButtonClicked();

private:
    void RegisterMenus();
    
    TSharedRef<SDockTab> SpawnIRGeneratorTab(const FSpawnTabArgs& SpawnTabArgs);
    
    TSharedPtr<class FUICommandList> PluginCommands;
};