// Fill out your copyright notice in the Description page of Project Settings.


#include "SurfaceAbsorptionSettings.h"

USurfaceAbsorptionSettings::USurfaceAbsorptionSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CategoryName = "Project";
	
	SectionName = "Surface Absorption";
}

TArray<FName> USurfaceAbsorptionSettings::GetSurfaceOptions() const
{
	TArray<FName> SurfaceNames;
	
	const UEnum* SurfaceEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPhysicalSurface"), true);
	if (!SurfaceEnum)
	{
		return SurfaceNames;
	}
	
	// Get all surface type names from the EPhysicalSurface enum
	for (int32 i = 0; i < SurfaceEnum->NumEnums() - 1; ++i)
	{
		SurfaceNames.Add(SurfaceEnum->GetNameByIndex(i));
	}
	
	return SurfaceNames;
}
