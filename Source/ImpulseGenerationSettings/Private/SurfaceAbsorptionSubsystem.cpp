// Fill out your copyright notice in the Description page of Project Settings.


#include "SurfaceAbsorptionSubsystem.h"
#include "SurfaceAbsorptionSettings.h"


void USurfaceAbsorptionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!IsEnabled())
	{
		return;
	}

	const USurfaceAbsorptionSettings* Settings = GetDefault<USurfaceAbsorptionSettings>();
	if (Settings && !Settings->SurfaceAbsorptionDataTable.IsNull())
	{
		SurfaceAbsorptionData = Cast<UDataTable>(Settings->SurfaceAbsorptionDataTable.TryLoad());
		if (!SurfaceAbsorptionData)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load Surface Floats Data Table"));
		}
	}
}

const TArray<float>& USurfaceAbsorptionSubsystem::GetSurfaceAbsorptionCoefficients(EPhysicalSurface PhysicalSurface)
{
	static const TArray<float> EmptyArray;

	if (!IsEnabled() || !SurfaceAbsorptionData)
	{
		return EmptyArray;
	}

	const UEnum* SurfaceEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPhysicalSurface"), true);
	if (!SurfaceEnum)
	{
		return EmptyArray;
	}

	const FName RowName = SurfaceEnum->GetNameByValue((int64)PhysicalSurface);
	const FSurfaceAbsorptionTableRow* Row = SurfaceAbsorptionData->FindRow<FSurfaceAbsorptionTableRow>(
		RowName, TEXT("USurfaceAbsorptionSubsystem::GetSurfaceAbsorptionCoefficients"));

	return Row ? Row->Values : EmptyArray;
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
	return Settings && !Settings->SurfaceAbsorptionDataTable.IsNull();
}
