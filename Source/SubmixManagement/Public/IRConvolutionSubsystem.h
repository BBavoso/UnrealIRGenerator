// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Sound/SoundSubmix.h"
#include "SubmixEffects/SubmixEffectConvolutionReverb.h"
#include "IRConvolutionVolume.h"
#include "IRConvolutionSubsystem.generated.h"

UENUM()
enum class ECrossfadeState : uint8
{
	Silent,
	FromSilence,
	SubmixAActive,
	CrossfadeAToB,
	CrossfadeAToSilence,
	SubmixBActive,
	CrossfadeBToA,
	CrossfadeBToSilence,
};

/**
 * Manages dynamic IR convolution reverb based on player position in volumes
 */
UCLASS()
class SUBMIXMANAGEMENT_API UIRConvolutionSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual TStatId GetStatId() const override;

	// Called by volume actors when player enters
	void NotifyVolumeEntered(TObjectPtr<AIRConvolutionVolume> Volume, AActor* OverlappingActor);

	// Called by volume actors when player exits
	void NotifyVolumeExited(TObjectPtr<AIRConvolutionVolume> Volume, AActor* OverlappingActor);

private:
	void LoadPluginAssets();
	void UpdateActiveVolume();
	void StartCrossfade(AIRConvolutionVolume* TargetVolume, ECrossfadeState NewCrossfadeState
	                    , TObjectPtr<USoundSubmix> SubmixToUse);
	void ApplyIRToSubmix(USoundSubmix* Submix, class UAudioImpulseResponse* ImpulseResponse);
	void SetSubmixVolume(USoundSubmix* Submix, float Volume);

	// Plugin submixes
	// These are loaded from the plugin content folder and should always exist
	UPROPERTY()
	TObjectPtr<USoundSubmix> SubmixA;

	UPROPERTY()
	TObjectPtr<USoundSubmix> SubmixB;

	// Convolution effect presets
	// These are loaded from the plugin content folder and should always exist
	// These effects should be already in the submixes
	UPROPERTY()
	TObjectPtr<USubmixEffectConvolutionReverbPreset> ConvolutionPresetA;

	UPROPERTY()
	TObjectPtr<USubmixEffectConvolutionReverbPreset> ConvolutionPresetB;

	UPROPERTY()
	TArray<TObjectPtr<AIRConvolutionVolume>> ActiveVolumes;

	// Either the current volume, or the volume that we are actively crossfading to
	UPROPERTY()
	TObjectPtr<AIRConvolutionVolume> CurrentActiveVolume;

	// The Volume we eventually want to get to
	// This will be equal to CurrentActiveVolume unless we enter/exit a volume during a crossfade
	UPROPERTY()
	TObjectPtr<AIRConvolutionVolume> QueuedActiveVolume;

	// Crossfade state
	ECrossfadeState CrossfadeState = ECrossfadeState::Silent;

	float CrossfadeProgress = 0.0f;
	float CrossfadeDuration = 1.0f;
};
