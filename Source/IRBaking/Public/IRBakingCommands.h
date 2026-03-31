#pragma once

#include "Framework/Commands/Commands.h"

class FIRBakingCommands : public TCommands<FIRBakingCommands>
{
public:
	FIRBakingCommands();
	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> OpenPluginWindow;
};