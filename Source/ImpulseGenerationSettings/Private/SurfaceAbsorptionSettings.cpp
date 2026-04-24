// Fill out your copyright notice in the Description page of Project Settings.


#include "SurfaceAbsorptionSettings.h"
#include "PhysicsCore.h"

USurfaceAbsorptionSettings::USurfaceAbsorptionSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CategoryName = "Project";
	
	SectionName = "Surface Absorption";
}

TArray<FName> USurfaceAbsorptionSettings::GetSurfaceOptions() const
{
	TArray<FName> SurfaceNames;
	
	if (const UEnum* SurfaceEnum = StaticEnum<EPhysicalSurface>())
	{
		for (int32 i = 0; i < SurfaceEnum->NumEnums() - 1; ++i)
		{
			SurfaceNames.Add(SurfaceEnum->GetNameByIndex(i));
		}
	}
	
	return SurfaceNames;
}
