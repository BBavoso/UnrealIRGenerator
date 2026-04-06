// Fill out your copyright notice in the Description page of Project Settings.

#include "IRConvolutionSubsystem.h"
#include "IRConvolutionVolume.h"
#include "Sound/AudioSettings.h"
#include "Sound/SoundSubmix.h"
#include "EffectConvolutionReverb.h"
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
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CrossfadeTimerHandle);
	}
	
	ActiveVolumes.Empty();
	CurrentActiveVolume = nullptr;
	PendingVolume = nullptr;
	
	UE_LOG(LogTemp, Log, TEXT("IRConvolutionSubsystem deinitialized"));
}

void UIRConvolutionSubsystem::LoadPluginAssets()
{
	// Load submixes from plugin Content folder
	SubmixA = LoadObject<USoundSubmix>(nullptr, TEXT("/IRGenerator/SM_ConvolutionSubmixA"));
	SubmixB = LoadObject<USoundSubmix>(nullptr, TEXT("/IRGenerator/SM_ConvolutionSubmixB"));
	
	if (!SubmixA)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load SubmixA from /IRGenerator/SM_ConvolutionSubmixA"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded SubmixA: %s"), *SubmixA->GetName());
	}
	
	if (!SubmixB)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load SubmixB from /IRGenerator/SM_ConvolutionSubmixB"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded SubmixB: %s"), *SubmixB->GetName());
	}

	// Load convolution effect presets
	ConvolutionPresetA = LoadObject<USubmixEffectConvolutionReverbPreset>(nullptr, TEXT("/IRGenerator/SEP_ConvolutionA"));
	ConvolutionPresetB = LoadObject<USubmixEffectConvolutionReverbPreset>(nullptr, TEXT("/IRGenerator/SEP_ConvolutionB"));

	if (!ConvolutionPresetA)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load ConvolutionPresetA from /IRGenerator/SEP_ConvolutionA"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded ConvolutionPresetA: %s"), *ConvolutionPresetA->GetName());
	}

	if (!ConvolutionPresetB)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load ConvolutionPresetB from /IRGenerator/SEP_ConvolutionB"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded ConvolutionPresetB: %s"), *ConvolutionPresetB->GetName());
	}
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
	
	// Start with SubmixA as active (but at 0 volume)
	bSubmixAActive = true;
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
		AIRConvolutionVolume* TargetVolume = Cast<AIRConvolutionVolume>(NewActiveVolume);

		if (TargetVolume)
		{
			UE_LOG(LogTemp, Warning, TEXT(">>> ACTIVE VOLUME CHANGED: %s (Priority: %d)"), 
				*TargetVolume->GetName(), TargetVolume->Priority);

			// If currently crossfading, queue the new volume
			if (CrossfadeState == ECrossfadeState::Crossfading)
			{
				PendingVolume = NewActiveVolume;
				bPendingSilence = false; // Clear silence flag, we have a new volume queued
				UE_LOG(LogTemp, Warning, TEXT("Crossfade in progress, queueing volume: %s"), *TargetVolume->GetName());
			}
			else
			{
				// Start crossfade immediately
				CurrentActiveVolume = NewActiveVolume;
				StartCrossfade(TargetVolume);
			}
		}
		else
		{
			// No active volume (player exited all volumes)
			CurrentActiveVolume = nullptr;
			
			if (CrossfadeState == ECrossfadeState::Crossfading)
			{
				PendingVolume = nullptr;
				bPendingSilence = true;
				UE_LOG(LogTemp, Warning, TEXT("No active volume, queueing silence"));
			}
			else
			{
				// Fade out current submix to silence
				UE_LOG(LogTemp, Warning, TEXT(">>> ACTIVE VOLUME: None (fading to silence)"));
				StartFadeToSilence();
			}
		}
	}
}

void UIRConvolutionSubsystem::StartCrossfade(AIRConvolutionVolume* TargetVolume)
{
	if (!TargetVolume)
	{
		return;
	}

	if (!SubmixA || !SubmixB)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot start crossfade: Submixes not loaded!"));
		return;
	}

	// Determine which submix to use (the inactive one)
	USoundSubmix* TargetSubmix = bSubmixAActive ? SubmixB : SubmixA;
	USoundSubmix* CurrentSubmix = bSubmixAActive ? SubmixA : SubmixB;

	UE_LOG(LogTemp, Warning, TEXT("Starting crossfade to %s (Target: Submix%s, Current: Submix%s)"), 
		*TargetVolume->GetName(),
		bSubmixAActive ? TEXT("B") : TEXT("A"),
		bSubmixAActive ? TEXT("A") : TEXT("B"));

	SetSubmixVolume(TargetSubmix, 0.0f);
	
	ApplyIRToSubmix(TargetSubmix, TargetVolume->ImpulseResponse);

	CrossfadeState = ECrossfadeState::Crossfading;
	CrossfadeProgress = 0.0f;
	CrossfadeDuration = TargetVolume->CrossfadeDuration;

	// Start timer for crossfade ticking
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CrossfadeTimerHandle,
			[this]()
			{
				if (UWorld* World = GetWorld())
				{
					TickCrossfade(World->GetDeltaSeconds());
				}
			},
			0.016f, // ~60fps
			true
		);
	}
}

void UIRConvolutionSubsystem::StartFadeToSilence()
{
	if (!SubmixA || !SubmixB)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot fade to silence: Submixes not loaded!"));
		return;
	}

	if (!bPreviouslyInVolume)
	{
		// Already at silence, nothing to do
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Starting fade to silence from Submix%s"), 
		bSubmixAActive ? TEXT("A") : TEXT("B"));

	CrossfadeState = ECrossfadeState::Crossfading;
	CrossfadeProgress = 0.0f;
	// TODO: make this based on the volume crossfade duration
	CrossfadeDuration = 1.0f; 

	// Start timer for crossfade ticking
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CrossfadeTimerHandle,
			[this]()
			{
				if (UWorld* World = GetWorld())
				{
					// Tick fade to silence
					if (CrossfadeState != ECrossfadeState::Crossfading)
					{
						return;
					}

					CrossfadeProgress += World->GetDeltaSeconds();
					float Alpha = FMath::Clamp(CrossfadeProgress / CrossfadeDuration, 0.0f, 1.0f);

					// Fade out active submix
					float FadeOutVolume = FMath::Cos(Alpha * PI * 0.5f);
					USoundSubmix* CurrentSubmix = bSubmixAActive ? SubmixA : SubmixB;
					USoundSubmix* InactiveSubmix = bSubmixAActive ? SubmixB : SubmixA;

					SetSubmixVolume(CurrentSubmix, FadeOutVolume);
					SetSubmixVolume(InactiveSubmix, 0.0f); // Ensure inactive stays at 0

					// Check if fade complete
					if (Alpha >= 1.0f)
					{
						CrossfadeState = ECrossfadeState::Idle;
						bPreviouslyInVolume = false; // Now in silence

						World->GetTimerManager().ClearTimer(CrossfadeTimerHandle);

						UE_LOG(LogTemp, Warning, TEXT("Fade to silence complete"));

						// Process queued volume if exists
						if (PendingVolume)
						{
							AActor* NextVolume = PendingVolume;
							PendingVolume = nullptr;
							CurrentActiveVolume = NextVolume;
							
							if (AIRConvolutionVolume* ConvolutionVolume = Cast<AIRConvolutionVolume>(NextVolume))
							{
								UE_LOG(LogTemp, Warning, TEXT("Processing queued volume after silence: %s"), *NextVolume->GetName());
								StartCrossfade(ConvolutionVolume);
							}
						}
					}
				}
			},
			0.016f, // ~60fps
			true
		);
	}
}

void UIRConvolutionSubsystem::TickCrossfade(float DeltaTime)
{
	if (CrossfadeState != ECrossfadeState::Crossfading)
	{
		return;
	}

	CrossfadeProgress += DeltaTime;
	float Alpha = FMath::Clamp(CrossfadeProgress / CrossfadeDuration, 0.0f, 1.0f);

	USoundSubmix* CurrentSubmix = bSubmixAActive ? SubmixA : SubmixB;
	USoundSubmix* TargetSubmix = bSubmixAActive ? SubmixB : SubmixA;

	if (bPreviouslyInVolume)
	{
		// Normal crossfade: transitioning between two volumes
		// Equal power crossfade
		float FadeOutVolume = FMath::Cos(Alpha * PI * 0.5f);
		float FadeInVolume = FMath::Sin(Alpha * PI * 0.5f);

		SetSubmixVolume(CurrentSubmix, FadeOutVolume);
		SetSubmixVolume(TargetSubmix, FadeInVolume);
	}
	else
	{
		// Fading in from silence: only fade in target submix
		float FadeInVolume = FMath::Sin(Alpha * PI * 0.5f);
		
		SetSubmixVolume(CurrentSubmix, 0.0f); // Keep current at 0
		SetSubmixVolume(TargetSubmix, FadeInVolume); // Fade in target
	}

	// Check if crossfade complete
	if (Alpha >= 1.0f)
	{
		CrossfadeState = ECrossfadeState::Idle;
		bSubmixAActive = !bSubmixAActive; // Toggle active submix
		bPreviouslyInVolume = true; // Now we're in a volume

		// Clear timer
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(CrossfadeTimerHandle);
		}

		UE_LOG(LogTemp, Warning, TEXT("Crossfade complete. Active submix: %s"), 
			bSubmixAActive ? TEXT("A") : TEXT("B"));

		// Process queued volume or silence
		if (bPendingSilence)
		{
			bPendingSilence = false;
			UE_LOG(LogTemp, Warning, TEXT("Processing queued silence"));
			StartFadeToSilence();
		}
		else if (PendingVolume)
		{
			AActor* NextVolume = PendingVolume;
			PendingVolume = nullptr;
			CurrentActiveVolume = NextVolume;
			
			if (AIRConvolutionVolume* ConvolutionVolume = Cast<AIRConvolutionVolume>(NextVolume))
			{
				UE_LOG(LogTemp, Warning, TEXT("Processing queued volume: %s"), *NextVolume->GetName());
				StartCrossfade(ConvolutionVolume);
			}
		}
	}
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
		UE_LOG(LogTemp, Error, TEXT("Cannot apply IR: Convolution preset not found for submix '%s'"), *Submix->GetName());
		return;
	}

	TargetPreset->SetImpulseResponse(ImpulseResponse);
	
	UE_LOG(LogTemp, Log, TEXT("Applied IR '%s' to submix '%s' via preset '%s'"), 
		*ImpulseResponse->GetName(), 
		*Submix->GetName(),
		*TargetPreset->GetName());
}

