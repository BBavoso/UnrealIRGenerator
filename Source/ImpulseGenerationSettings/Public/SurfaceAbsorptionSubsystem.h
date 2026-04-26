// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SurfaceAbsorptionSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Engine/DataTable.h"
#include "EditorSubsystem.h"
#include "SurfaceAbsorptionSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class IMPULSEGENERATIONSETTINGS_API USurfaceAbsorptionSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Get the absorption coefficients associated with a surface
	 * @param PhysicalSurface The surface to look up
	 * @return Array of absorption coefficients for the given surface, empty if not found
	 */
	TArray<float> GetSurfaceAbsorptionCoefficients(EPhysicalSurface PhysicalSurface);
private:
	UPROPERTY()
	TMap<FName, FSurfaceAbsorptionData> SurfaceAbsorptionData;

	static bool IsEnabled();
};
