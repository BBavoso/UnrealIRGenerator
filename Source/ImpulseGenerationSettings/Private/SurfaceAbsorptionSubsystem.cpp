// Fill out your copyright notice in the Description page of Project Settings.


#include "SurfaceAbsorptionSubsystem.h"
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

const TArray<float>& USurfaceAbsorptionSubsystem::GetSurfaceAbsorptionCoefficients(EPhysicalSurface PhysicalSurface)
{
	static const TArray<float> EmptyArray;

	if (!IsEnabled() || SurfaceAbsorptionData.IsEmpty())
	{
		return EmptyArray;
	}

	if (const UEnum* SurfaceEnum = StaticEnum<EPhysicalSurface>())
	{
		const int64 EnumValue = (int64)PhysicalSurface;
		const FText DisplayName = SurfaceEnum->GetDisplayNameTextByValue(EnumValue);
		const FName SurfaceName = FName(*DisplayName.ToString());
		const FSurfaceAbsorptionData* AbsorptionData = SurfaceAbsorptionData.Find(SurfaceName);
		return AbsorptionData ? AbsorptionData->Values : EmptyArray;
	}
	
	return EmptyArray;
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


