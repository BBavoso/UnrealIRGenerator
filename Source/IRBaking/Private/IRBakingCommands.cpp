#include "IRBakingCommands.h"

#define LOCTEXT_NAMESPACE "FIRGeneratorModule"

FIRBakingCommands::FIRBakingCommands()
	: TCommands<FIRBakingCommands>(
		TEXT("IRBaking"),
		NSLOCTEXT("Contexts", "IRBaking", "IR Generator Plugin"),
		NAME_None,
		FAppStyle::GetAppStyleSetName())
{
}

void FIRBakingCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "IR Generator", "Open the IR Generator window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE