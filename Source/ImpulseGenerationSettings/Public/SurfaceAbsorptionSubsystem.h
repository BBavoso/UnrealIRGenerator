// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Engine/DataTable.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SurfaceAbsorptionSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class IMPULSEGENERATIONSETTINGS_API USurfaceAbsorptionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Get the absorption coefficients associated with a surface
	 * @param PhysicalSurface The surface to look up
	 * @return Array of absorption coefficients for the given surface, empty if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "Surface Absorption")
	const TArray<float>& GetSurfaceAbsorptionCoefficients(EPhysicalSurface PhysicalSurface);

private:
	UPROPERTY()
	TObjectPtr<UDataTable> SurfaceAbsorptionData;

	static bool IsEnabled();
};
