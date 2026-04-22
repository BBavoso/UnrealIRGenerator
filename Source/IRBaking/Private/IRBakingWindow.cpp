#include "IRBakingWindow.h"

#include "Kismet/GameplayStatics.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "IRProbe/Public/IR_Generator.h"
#include "SubmixManagement/Public/IRConvolutionVolume.h"

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
		+ SVerticalBox::Slot()
		.Padding(20.0f)
		[
			SNew(SButton)
			.Text(FText::FromString("Bake All Probes"))
			.OnClicked(this, &SIRBakingWindow::BakeAllProbes)
		]
	];
}

FReply SIRBakingWindow::BakeAllProbes() const
{
	UE_LOG(LogTemp, Warning, TEXT("Generate button clicked!"));
	UWorld* WorldContext = GEditor->GetEditorWorldContext().World();
	TArray<AActor*> Probes;
	UGameplayStatics::GetAllActorsOfClass(WorldContext, AIR_Generator::StaticClass(), Probes);
	UE_LOG(LogTemp, Warning, TEXT("Found %i probes"), Probes.Num());

	for (AActor*& Probe : Probes)
	{
		AIR_Generator* generator = Cast<AIR_Generator>(Probe);
		if (generator->BakeProbe)
		{
			generator->CalculateAndRecordImpulseResponseToFile();
		}
	}

	TArray<AActor*> ConvolutionVolumes;
	UGameplayStatics::GetAllActorsOfClass(WorldContext, AIRConvolutionVolume::StaticClass(), ConvolutionVolumes);
	UE_LOG(LogTemp, Warning, TEXT("Found %i convolution volumes"), ConvolutionVolumes.Num());

	for (AActor*& ConvolutionVolume : ConvolutionVolumes)
	{
		AIRConvolutionVolume* volume = Cast<AIRConvolutionVolume>(ConvolutionVolume);
		if (volume->IrGenerator)
		{
			volume->ImpulseResponse = volume->IrGenerator->GeneratedImpulseResponse;
		}
	}

	return FReply::Handled();
}
