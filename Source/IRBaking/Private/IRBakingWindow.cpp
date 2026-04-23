#include "IRBakingWindow.h"

#include "EditorAssetLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "IRProbe/Public/IR_Generator.h"
#include "SubmixManagement/Public/IRConvolutionVolume.h"
#include "FileHelpers.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EffectConvolutionReverb.h"

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
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20.0f)
		[
			SNew(SButton)
			.Text(FText::FromString("Delete Unreferenced IRs"))
			.OnClicked(this, &SIRBakingWindow::DeleteUnreferencedIRs)
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
			volume->UpdateVolumeWithoutGenerating();
		}
	}
	
	UEditorLoadingAndSavingUtils::SaveDirtyPackages(
		true, true
	);	
	
	UEditorLoadingAndSavingUtils::SaveCurrentLevel();

	return FReply::Handled();
}

FReply SIRBakingWindow::DeleteUnreferencedIRs() const
{
	UE_LOG(LogTemp, Warning, TEXT("Deleting unreferenced IRs..."));
	
	// Load all AudioImpulseResponse assets in GeneratedIR folder
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> IRAssets;
	AssetRegistryModule.Get().GetAssetsByPath(FName("/Game/GeneratedIR"), IRAssets, true);
	
	// Filter to only AudioImpulseResponse assets
	IRAssets.RemoveAll([](const FAssetData& Asset) 
	{ 
		return Asset.AssetClassPath.GetAssetName() != UAudioImpulseResponse::StaticClass()->GetFName();
	});
	
	UE_LOG(LogTemp, Warning, TEXT("Found %i IR assets total"), IRAssets.Num());
	
	int32 DeletedCount = 0;
	for (const FAssetData& Asset : IRAssets)
	{
		// Only delete IRs in the GeneratedIR folder
		if (!Asset.GetObjectPathString().Contains(TEXT("/Game/GeneratedIR/")))
		{
			continue;
		}
		
		// Check if the asset is referenced
		FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetIdentifier> Referencers;
		AssetRegistry.Get().GetReferencers(Asset.PackageName, Referencers);
		
		if (Referencers.IsEmpty())
		{
			if (UEditorAssetLibrary::DeleteAsset(Asset.GetObjectPathString()))
			{
				UE_LOG(LogTemp, Warning, TEXT("Deleted unreferenced IR: %s"), *Asset.GetObjectPathString());
				DeletedCount++;
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Deleted %i unreferenced IR assets"), DeletedCount);
	
	return FReply::Handled();
}
