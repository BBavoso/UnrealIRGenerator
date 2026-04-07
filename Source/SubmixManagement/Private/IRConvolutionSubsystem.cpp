// Fill out your copyright notice in the Description page of Project Settings.

#include "IRConvolutionSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Sound/AudioSettings.h"
#include "EffectConvolutionReverb.h"
#include "Sound/SoundSubmix.h"
#include "SubmixEffects/SubmixEffectConvolutionReverb.h"

void UIRConvolutionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("IRConvolutionSubsystem initialized"));

	LoadPluginAssets();
}

void UIRConvolutionSubsystem::Deinitialize()
{
	Super::Deinitialize();

	ActiveVolumes.Empty();
	CurrentActiveVolume = nullptr;
	QueuedActiveVolume = nullptr;

	UE_LOG(LogTemp, Log, TEXT("IRConvolutionSubsystem deinitialized"));
}

void UIRConvolutionSubsystem::Tick(float DeltaTime)
{
	// Only advance crossfade progress when actually in a crossfade state
	bool bIsInCrossfadeState = (CrossfadeState != ECrossfadeState::Silent &&
		CrossfadeState != ECrossfadeState::SubmixAActive &&
		CrossfadeState != ECrossfadeState::SubmixBActive);

	if (bIsInCrossfadeState)
	{
		CrossfadeProgress += DeltaTime;
	}

	float CrossfadeProgressNormalized = FMath::Clamp(CrossfadeProgress / CrossfadeDuration, 0.0f, 1.0f);
	bool bCrossfadeFinished = bIsInCrossfadeState && (CrossfadeProgress >= CrossfadeDuration);

	switch (CrossfadeState)
	{
	case ECrossfadeState::FromSilence:
		SetSubmixVolume(SubmixA, FMath::Sin(CrossfadeProgressNormalized * PI * 0.5f));
		if (bCrossfadeFinished)
		{
			CrossfadeState = ECrossfadeState::SubmixAActive;
			SetSubmixVolume(SubmixA, 1.0f);
		}
		break;
	case ECrossfadeState::CrossfadeAToB:
		SetSubmixVolume(SubmixA, FMath::Cos(CrossfadeProgressNormalized * PI * 0.5f));
		SetSubmixVolume(SubmixB, FMath::Sin(CrossfadeProgressNormalized * PI * 0.5f));
		if (bCrossfadeFinished)
		{
			CrossfadeState = ECrossfadeState::SubmixBActive;
			SetSubmixVolume(SubmixA, 0.0f);
			SetSubmixVolume(SubmixB, 1.0f);
		}
		break;
	case ECrossfadeState::CrossfadeBToA:
		SetSubmixVolume(SubmixB, FMath::Cos(CrossfadeProgressNormalized * PI * 0.5f));
		SetSubmixVolume(SubmixA, FMath::Sin(CrossfadeProgressNormalized * PI * 0.5f));
		if (bCrossfadeFinished)
		{
			CrossfadeState = ECrossfadeState::SubmixAActive;
			SetSubmixVolume(SubmixB, 0.0f);
			SetSubmixVolume(SubmixA, 1.0f);
		}
		break;
	case ECrossfadeState::CrossfadeAToSilence:
		SetSubmixVolume(SubmixA, FMath::Cos(CrossfadeProgressNormalized * PI * 0.5f));
		if (bCrossfadeFinished)
		{
			CrossfadeState = ECrossfadeState::Silent;
			SetSubmixVolume(SubmixA, 0.0f);
		}
		break;
	case ECrossfadeState::CrossfadeBToSilence:
		SetSubmixVolume(SubmixB, FMath::Cos(CrossfadeProgressNormalized * PI * 0.5f));
		if (bCrossfadeFinished)
		{
			CrossfadeState = ECrossfadeState::Silent;
			SetSubmixVolume(SubmixB, 0.0f);
		}
		break;
	case ECrossfadeState::Silent:
		if (QueuedActiveVolume != CurrentActiveVolume)
		{
			StartCrossfade(QueuedActiveVolume, ECrossfadeState::FromSilence, SubmixA);
		}
		break;
	case ECrossfadeState::SubmixAActive:
		if (QueuedActiveVolume == CurrentActiveVolume) { break; }
		if (!QueuedActiveVolume && ActiveVolumes.Num() == 0)
		{
			StartCrossfade(nullptr, ECrossfadeState::CrossfadeAToSilence, SubmixA);
		}
		else if (QueuedActiveVolume)
		{
			StartCrossfade(QueuedActiveVolume, ECrossfadeState::CrossfadeAToB, SubmixB);
		}
		break;
	case ECrossfadeState::SubmixBActive:
		if (QueuedActiveVolume == CurrentActiveVolume) { break; }
		if (!QueuedActiveVolume && ActiveVolumes.Num() == 0)
		{
			StartCrossfade(nullptr, ECrossfadeState::CrossfadeBToSilence, SubmixB);
		}
		else if (QueuedActiveVolume)
		{
			StartCrossfade(QueuedActiveVolume, ECrossfadeState::CrossfadeBToA, SubmixA);
		}
		break;
	}
}

void UIRConvolutionSubsystem::LoadPluginAssets()
{
	// Load submixes
	SubmixA = LoadObject<USoundSubmix>(nullptr, TEXT("/IRGenerator/SM_ConvolutionSubmixA"));
	SubmixB = LoadObject<USoundSubmix>(nullptr, TEXT("/IRGenerator/SM_ConvolutionSubmixB"));

	// Load Convolution Reverb Effects
	ConvolutionPresetA = LoadObject<USubmixEffectConvolutionReverbPreset>(
		nullptr, TEXT("/IRGenerator/SEP_ConvolutionA"));
	ConvolutionPresetB = LoadObject<USubmixEffectConvolutionReverbPreset>(
		nullptr, TEXT("/IRGenerator/SEP_ConvolutionB"));

	if (!SubmixA || !SubmixB || !ConvolutionPresetA || !ConvolutionPresetB)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load plugin assets"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Loaded SubmixA: %s"), *SubmixA->GetName());
	UE_LOG(LogTemp, Log, TEXT("Loaded SubmixB: %s"), *SubmixB->GetName());
	UE_LOG(LogTemp, Log, TEXT("Loaded ConvolutionPresetA: %s"), *ConvolutionPresetA->GetName());
	UE_LOG(LogTemp, Log, TEXT("Loaded ConvolutionPresetB: %s"), *ConvolutionPresetB->GetName());
}

void UIRConvolutionSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Initialize both submixes to 0 volume
	if (SubmixA)
	{
		SetSubmixVolume(SubmixA, 0.0f);
		UE_LOG(LogTemp, Log, TEXT("SubmixA initialized to 0 volume"));
	}

	if (SubmixB)
	{
		SetSubmixVolume(SubmixB, 0.0f);
		UE_LOG(LogTemp, Log, TEXT("SubmixB initialized to 0 volume"));
	}

	CrossfadeState = ECrossfadeState::Silent;
}

TStatId UIRConvolutionSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(YouClassName, STATGROUP_Tickables);
}

void UIRConvolutionSubsystem::NotifyVolumeEntered(TObjectPtr<AIRConvolutionVolume> Volume, AActor* OverlappingActor)
{
	AActor* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn().Get();
	if (!OverlappingActor || OverlappingActor != PlayerPawn)
	{
		return;
	}

	// Add to active volumes if not already present
	if (!ActiveVolumes.Contains(Volume))
	{
		ActiveVolumes.Add(Volume);
		UE_LOG(LogTemp, Log, TEXT("Volume entered: %s"), *Volume->GetName());
		UpdateActiveVolume();
	}
}

void UIRConvolutionSubsystem::NotifyVolumeExited(TObjectPtr<AIRConvolutionVolume> Volume, AActor* OverlappingActor)
{
	if (!Volume || !OverlappingActor)
	{
		return;
	}

	// Remove from active volumes
	if (ActiveVolumes.Remove(Volume) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Volume exited: %s"), *Volume->GetName());
		UpdateActiveVolume();
	}
}

void UIRConvolutionSubsystem::UpdateActiveVolume()
{
	AIRConvolutionVolume* TargetVolume = nullptr;
	int32 HighestPriority = INT32_MIN;

	// Find highest priority volume
	for (const TObjectPtr<AIRConvolutionVolume> Volume : ActiveVolumes)
	{
		if (Volume->Priority > HighestPriority)
		{
			HighestPriority = Volume->Priority;
			TargetVolume = Volume;
		}
	}

	// Check if active volume changed
	if (TargetVolume == CurrentActiveVolume)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Active volume changing: %s -> %s"),
	       CurrentActiveVolume ? *CurrentActiveVolume->GetName() : TEXT("None"),
	       TargetVolume ? *TargetVolume->GetName() : TEXT("None"));

	switch (CrossfadeState)
	{
		using enum ECrossfadeState;

	case FromSilence:
	case CrossfadeAToB:
	case CrossfadeBToA:
	case CrossfadeAToSilence:
	case CrossfadeBToSilence:
		// Crossfade in progress - queue the new volume
		QueuedActiveVolume = TargetVolume;
		break;

	case Silent:
		QueuedActiveVolume = TargetVolume;
		StartCrossfade(TargetVolume, FromSilence, SubmixA);
		break;
	case SubmixAActive:
		QueuedActiveVolume = TargetVolume;
		if (TargetVolume)
		{
			StartCrossfade(TargetVolume, CrossfadeAToB, SubmixB);
		}
		else
		{
			StartCrossfade(nullptr, CrossfadeAToSilence, SubmixA);
		}
		break;
	case SubmixBActive:
		QueuedActiveVolume = TargetVolume;
		if (TargetVolume)
		{
			StartCrossfade(TargetVolume, CrossfadeBToA, SubmixA);
		}
		else
		{
			StartCrossfade(nullptr, CrossfadeBToSilence, SubmixB);
		}
		break;
	}
}

void UIRConvolutionSubsystem::StartCrossfade(
	AIRConvolutionVolume* TargetVolume,
	ECrossfadeState NewCrossfadeState,
	TObjectPtr<USoundSubmix> SubmixToUse)
{
	if (!SubmixA || !SubmixB)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot start crossfade: Submixes not loaded!"));
		return;
	}

	// Set crossfade duration and initial volume based on transition type
	if (CurrentActiveVolume && TargetVolume)
	{
		// Volume to volume
		CrossfadeDuration = (CurrentActiveVolume->CrossfadeDuration + TargetVolume->CrossfadeDuration) * 0.5f;
		SetSubmixVolume(SubmixToUse, 0.0f);
	}
	else if (!CurrentActiveVolume && TargetVolume)
	{
		// Silence to volume
		CrossfadeDuration = TargetVolume->CrossfadeDuration;
		SetSubmixVolume(SubmixToUse, 0.0f);
	}
	else if (CurrentActiveVolume && !TargetVolume)
	{
		// Volume to silence
		CrossfadeDuration = CurrentActiveVolume->CrossfadeDuration;
		SetSubmixVolume(SubmixToUse, 1.0f);
	}

	// Apply IR if transitioning to a volume with an impulse response
	if (TargetVolume && TargetVolume->ImpulseResponse)
	{
		ApplyIRToSubmix(SubmixToUse, TargetVolume->ImpulseResponse);
	}
	else if (TargetVolume && !TargetVolume->ImpulseResponse)
	{
		UE_LOG(LogTemp, Warning, TEXT("Volume '%s' has no impulse response assigned"), *TargetVolume->GetName());
	}

	CrossfadeProgress = 0.0f;
	CrossfadeState = NewCrossfadeState;
	CurrentActiveVolume = TargetVolume;
}

void UIRConvolutionSubsystem::SetSubmixVolume(USoundSubmix* Submix, float Volume)
{
	if (!Submix)
	{
		return;
	}

	Submix->SetSubmixOutputVolume(GetWorld(), Volume);
}

void UIRConvolutionSubsystem::ApplyIRToSubmix(USoundSubmix* Submix, UAudioImpulseResponse* ImpulseResponse)
{
	if (!Submix)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot apply IR: Submix is null"));
		return;
	}

	if (!ImpulseResponse)
	{
		UE_LOG(LogTemp, Warning, TEXT("No impulse response assigned to volume, skipping IR application"));
		return;
	}

	USubmixEffectConvolutionReverbPreset* TargetPreset = nullptr;
	if (Submix == SubmixA)
	{
		TargetPreset = ConvolutionPresetA;
	}
	else if (Submix == SubmixB)
	{
		TargetPreset = ConvolutionPresetB;
	}

	if (!TargetPreset)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot apply IR: Convolution preset not found for submix '%s'"),
		       *Submix->GetName());
		return;
	}

	TargetPreset->SetImpulseResponse(ImpulseResponse);

	UE_LOG(LogTemp, Log, TEXT("Applied IR '%s' to submix '%s' via preset '%s'"),
	       *ImpulseResponse->GetName(),
	       *Submix->GetName(),
	       *TargetPreset->GetName());
}
