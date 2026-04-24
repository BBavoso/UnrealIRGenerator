// Fill out your copyright notice in the Description page of Project Settings.


#include "SurfaceAbsorptionSubsystem.h"


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

	const UEnum* SurfaceEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPhysicalSurface"), true);
	if (!SurfaceEnum)
	{
		return EmptyArray;
	}

	const FName SurfaceName = SurfaceEnum->GetNameByValue((int64)PhysicalSurface);
	const FSurfaceAbsorptionData* AbsorptionData = SurfaceAbsorptionData.Find(SurfaceName);
	
	return AbsorptionData ? AbsorptionData->Values : EmptyArray;
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


