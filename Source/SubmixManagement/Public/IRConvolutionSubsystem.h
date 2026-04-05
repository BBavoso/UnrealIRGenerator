// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "IRConvolutionSubsystem.generated.h"

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

	// Called by volume actors when player enters
	UFUNCTION()
	void NotifyVolumeEntered(AActor* Volume, AActor* OverlappingActor);

	// Called by volume actors when player exits
	UFUNCTION()
	void NotifyVolumeExited(AActor* Volume, AActor* OverlappingActor);

private:
	void UpdateActiveVolume();

	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActiveVolumes;

	UPROPERTY()
	TObjectPtr<AActor> CurrentActiveVolume;
};
