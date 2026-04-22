// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SurfaceAbsorptionSettings.generated.h"

/**
 * 
 */
UCLASS(Config="game", DefaultConfig)
class IMPULSEGENERATIONSETTINGS_API USurfaceAbsorptionSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	USurfaceAbsorptionSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Config, EditAnywhere, Category = "Surface Absorption")
	bool TestBool = false;
};
