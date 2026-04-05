// Fill out your copyright notice in the Description page of Project Settings.

#include "IRConvolutionSubsystem.h"
#include "IRConvolutionVolume.h"

void UIRConvolutionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("IRConvolutionSubsystem initialized"));
}

void UIRConvolutionSubsystem::Deinitialize()
{
	Super::Deinitialize();
	ActiveVolumes.Empty();
	CurrentActiveVolume = nullptr;
	UE_LOG(LogTemp, Log, TEXT("IRConvolutionSubsystem deinitialized"));
}

void UIRConvolutionSubsystem::NotifyVolumeEntered(AActor* Volume, AActor* OverlappingActor)
{
	if (!Volume || !OverlappingActor)
	{
		return;
	}

	// Add to active volumes if not already present
	if (!ActiveVolumes.Contains(Volume))
	{
		ActiveVolumes.Add(Volume);
		UE_LOG(LogTemp, Warning, TEXT("Volume entered: %s by %s"), *Volume->GetName(), *OverlappingActor->GetName());
		UpdateActiveVolume();
	}
}

void UIRConvolutionSubsystem::NotifyVolumeExited(AActor* Volume, AActor* OverlappingActor)
{
	if (!Volume || !OverlappingActor)
	{
		return;
	}

	// Remove from active volumes
	if (ActiveVolumes.Remove(Volume) > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Volume exited: %s by %s"), *Volume->GetName(), *OverlappingActor->GetName());
		UpdateActiveVolume();
	}
}

void UIRConvolutionSubsystem::UpdateActiveVolume()
{
	AActor* NewActiveVolume = nullptr;
	int32 HighestPriority = INT32_MIN;

	// Find highest priority volume
	for (AActor* Volume : ActiveVolumes)
	{
		if (AIRConvolutionVolume* ConvolutionVolume = Cast<AIRConvolutionVolume>(Volume))
		{
			if (ConvolutionVolume->Priority > HighestPriority)
			{
				HighestPriority = ConvolutionVolume->Priority;
				NewActiveVolume = Volume;
			}
		}
	}

	// Check if active volume changed
	if (NewActiveVolume != CurrentActiveVolume)
	{
		CurrentActiveVolume = NewActiveVolume;

		if (CurrentActiveVolume)
		{
			UE_LOG(LogTemp, Warning, TEXT(">>> ACTIVE VOLUME: %s (Priority: %d)"), 
				*CurrentActiveVolume->GetName(), 
				Cast<AIRConvolutionVolume>(CurrentActiveVolume)->Priority);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT(">>> ACTIVE VOLUME: None (all volumes exited)"));
		}
	}
}
