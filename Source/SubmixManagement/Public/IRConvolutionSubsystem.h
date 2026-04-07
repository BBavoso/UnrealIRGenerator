// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Sound/SoundSubmix.h"
#include "IRConvolutionSubsystem.generated.h"

/*
 * ZZZZZZZZZZZZZ
 * TODO:
 * Clean up this to no longer need so many bool variables.
 * The System should be able to operate just based on the current volume and pending volume
 * if the current volume in null then we know we are coming from silence
 * if the pending is null then we want to fade to silence
 * if that doesn't work the logic should be moved into the crossfade state
*/

UENUM()
enum class ECrossfadeState : uint8
{
	Idle,
	Crossfading
};

/**
 * Manages dynamic IR convolution reverb based on player position in volumes
 */
UCLASS()
class SUBMIXMANAGEMENT_API UIRConvolutionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// Called by volume actors when player enters
	UFUNCTION()
	void NotifyVolumeEntered(AActor* Volume, AActor* OverlappingActor);

	// Called by volume actors when player exits
	UFUNCTION()
	void NotifyVolumeExited(AActor* Volume, AActor* OverlappingActor);

private:
	void LoadPluginAssets();
	void UpdateActiveVolume();
	void StartCrossfade(class AIRConvolutionVolume* TargetVolume);
	void StartFadeToSilence();
	void TickCrossfade(float DeltaTime);
	void ApplyIRToSubmix(USoundSubmix* Submix, class UAudioImpulseResponse* ImpulseResponse);
	void SetSubmixVolume(USoundSubmix* Submix, float Volume);

	// Plugin submixes (loaded automatically from Content/)
	UPROPERTY()
	TObjectPtr<USoundSubmix> SubmixA;

	UPROPERTY()
	TObjectPtr<USoundSubmix> SubmixB;

	// Convolution effect presets (loaded automatically from Content/)
	UPROPERTY()
	TObjectPtr<class USubmixEffectConvolutionReverbPreset> ConvolutionPresetA;

	UPROPERTY()
	TObjectPtr<class USubmixEffectConvolutionReverbPreset> ConvolutionPresetB;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActiveVolumes;

	UPROPERTY()
	TObjectPtr<AActor> CurrentActiveVolume;

	UPROPERTY()
	TObjectPtr<AActor> PendingVolume;

	// Flag to track if silence is queued (exited all volumes during crossfade)
	bool bPendingSilence = false;

	// Which submix is currently active (true = A, false = B)
	bool bSubmixAActive = true;

	// Track if we're currently in a volume (both submixes at 0 if false)
	bool bPreviouslyInVolume = false;

	// Crossfade state
	ECrossfadeState CrossfadeState = ECrossfadeState::Idle;
	float CrossfadeProgress = 0.0f;
	float CrossfadeDuration = 1.0f;

	FTimerHandle CrossfadeTimerHandle;
};
