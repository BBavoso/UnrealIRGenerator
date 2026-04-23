#pragma once

#include "Widgets/SCompoundWidget.h"

class SIRBakingWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SIRBakingWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
private:
	FReply BakeAllProbes() const;
	FReply DeleteUnreferencedIRs() const;
};