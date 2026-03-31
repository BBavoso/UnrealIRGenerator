#include "IRBakingWindow.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SIRBakingWindow::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString("IR Generator Plugin"))
			.TextStyle(FAppStyle::Get(), "NormalText.Important")
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Window is working!"))
		]
	];
}