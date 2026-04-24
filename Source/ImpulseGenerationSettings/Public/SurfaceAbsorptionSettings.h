// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SurfaceAbsorptionSettings.generated.h"

/**
 * Data Table Row for storing Absorption Coefficients per surface
 */
USTRUCT()
struct FSurfaceAbsorptionTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Surface Floats")
	TArray<float> Values;
};

/**
 * 
 */
UCLASS(Config="game", DefaultConfig)
class IMPULSEGENERATIONSETTINGS_API USurfaceAbsorptionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	USurfaceAbsorptionSettings(const FObjectInitializer& ObjectInitializer);

	/**
	* Data table for storing float values per surface @see FSurfaceFloatTableRow
	*/
	UPROPERTY(config, EditAnywhere, Category = GameplayTags,
		meta = (
			AllowedClasses = "/Script/Engine.DataTable",
			RowType = "/Script/ImpulseGenerationSettings.SurfaceAbsorptionTableRow"
		))
	FSoftObjectPath SurfaceAbsorptionDataTable;
};
