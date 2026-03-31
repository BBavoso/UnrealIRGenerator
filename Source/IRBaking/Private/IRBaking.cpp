#include "IRBaking.h"

#include "IRBakingCommands.h"
#include "IRBakingWindow.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Application/SlateApplication.h"

#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FIRGeneratorModule"

static const FName IRGeneratorTabName = FName(TEXT("IR"));

void FIRBakingModule::StartupModule()
{
	FIRBakingCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FIRBakingCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FIRBakingModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FIRBakingModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
								IRGeneratorTabName,
								FOnSpawnTab::CreateRaw(this, &FIRBakingModule::SpawnIRGeneratorTab))
		.SetDisplayName(LOCTEXT("TabTitle", "IR Generator"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FIRBakingModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FIRBakingCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(IRGeneratorTabName);
}

void FIRBakingModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(IRGeneratorTabName);
}

void FIRBakingModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu *Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection &Section = Menu->FindOrAddSection("IR Generator Button?");
			Section.AddMenuEntryWithCommandList(FIRBakingCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	// {
	// 	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
	// 	{
	// 		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
	// 		{
	// 			FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FIRGeneratorCommands::Get().OpenPluginWindow));
	// 			Entry.SetCommandList(PluginCommands);
	// 		}
	// 	}
	// }
}

TSharedRef<SDockTab> FIRBakingModule::SpawnIRGeneratorTab(const FSpawnTabArgs &SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
			[SNew(SIRBakingWindow)];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FIRBakingModule, IRBaking)