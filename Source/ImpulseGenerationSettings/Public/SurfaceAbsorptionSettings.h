// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SurfaceAbsorptionSettings.generated.h"

/**
 * Data structure for storing Absorption Coefficients per surface
 */
USTRUCT()
struct FSurfaceAbsorptionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Surface Floats")
	TArray<float> Values;

	FSurfaceAbsorptionData()
	{
		// Initialize with 6 frequency bands (default)
		Values.SetNum(6);
		for (int32 i = 0; i < 6; ++i)
		{
			Values[i] = 0.5f;
		}
	}
};

/**
 * 
 */
UCLASS(config = Engine, defaultconfig)
class IMPULSEGENERATIONSETTINGS_API USurfaceAbsorptionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	USurfaceAbsorptionSettings(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION()
	TArray<FName> GetSurfaceOptions() const;
	
	UPROPERTY(EditAnywhere)
	double AirTemperatureFahrenheit = 50;
	
	UPROPERTY(EditAnywhere)
	double HumidityPercent = 50;

	/**
	* Map of surface types to their absorption coefficients
	*/
	UPROPERTY(config, EditAnywhere, Category = "Surface Absorption",
		meta = (GetKeyOptions = "GetSurfaceOptions"))
	TMap<FName, FSurfaceAbsorptionData> SurfaceAbsorptionMap;
};
