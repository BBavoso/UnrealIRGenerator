// Fill out your copyright notice in the Description page of Project Settings.


#include "SurfaceAbsorptionSubsystem.h"
#include "SurfaceAbsorptionSettings.h"
#include "PhysicsCore.h"


void USurfaceAbsorptionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!IsEnabled())
	{
		return;
	}

	if (const USurfaceAbsorptionSettings* Settings = GetDefault<USurfaceAbsorptionSettings>())
	{
		SurfaceAbsorptionData = Settings->SurfaceAbsorptionMap;
	}
}

TArray<float> USurfaceAbsorptionSubsystem::GetSurfaceAbsorptionCoefficients(
	EPhysicalSurface PhysicalSurface)
{
	static const TArray<float> DefaultCoefficients = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

	if (!IsEnabled() || SurfaceAbsorptionData.IsEmpty())
	{
		return DefaultCoefficients;
	}

	if (const UEnum* SurfaceEnum = StaticEnum<EPhysicalSurface>())
	{
		const int64 EnumValue = (int64)PhysicalSurface;
		const FText DisplayName = SurfaceEnum->GetDisplayNameTextByValue(EnumValue);
		const FName SurfaceName = FName(*DisplayName.ToString());
		const FSurfaceAbsorptionData* AbsorptionData = SurfaceAbsorptionData.Find(SurfaceName);
		return AbsorptionData ? AbsorptionData->Values : DefaultCoefficients;
	}

	return DefaultCoefficients;
}


bool USurfaceAbsorptionSubsystem::IsEnabled()
{
	/**
	 * impulse generation only works in the editor and cannot be done at runtime
	 */
#if !WITH_EDITOR
	return false;
#endif

	const USurfaceAbsorptionSettings* Settings = GetDefault<USurfaceAbsorptionSettings>();
	return Settings->IsValidLowLevel();
}
