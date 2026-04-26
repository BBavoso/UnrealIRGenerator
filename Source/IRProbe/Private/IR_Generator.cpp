// Fill out your copyright notice in the Description page of Project Settings.


#include "IR_Generator.h"
#include "IR_Generator_DSP.h"
#include "Components/SphereComponent.h"
#include "Sampling/SphericalFibonacci.h"
#include "Sound/SampleBufferIO.h"
#include "Engine/World.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "EffectConvolutionReverb.h"

#if WITH_EDITOR
#include "SurfaceAbsorptionSubsystem.h"
#include "Factories/SoundFactory.h"
#include "AudioImpulseResponseAsset.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Sound/SoundWave.h"
#include "AssetToolsModule.h"
#include "EditorAssetLibrary.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetImportTask.h"
#endif


constexpr float ListenerSphereRadius = 15.0f;
constexpr float MaxRaycastDistance = 10000.0f;


// Sets default values
AIR_Generator::AIR_Generator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITORONLY_DATA
	ListenerSphere = CreateEditorOnlyDefaultSubobject<USphereComponent>(TEXT("EditorTraceSphere"));

	if (ListenerSphere)
	{
		ListenerSphere->SetupAttachment(RootComponent);
		ListenerSphere->InitSphereRadius(ListenerSphereRadius);

		ListenerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		// Sphere->SetCollisionObjectType(ECC_WorldDynamic);

		// Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		ListenerSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

		ListenerSphere->SetHiddenInGame(true);
	}
#endif
}

// Called when the game starts or when spawned
void AIR_Generator::BeginPlay()
{
	Super::BeginPlay();
}


float Impulse::GetDelaySeconds() const
{
	float DistanceMeters = DistanceTraveled / 100.0f;
	constexpr float SpeedOfSoundMetersPerSecond = 343.0f;
	return DistanceMeters / SpeedOfSoundMetersPerSecond;
}


MaterialCoefficients LookupCoefficients(const EPhysicalSurface& Material)
{
	USurfaceAbsorptionSubsystem* AbsorptionSubsystem = GEditor->GetEditorSubsystem<USurfaceAbsorptionSubsystem>();
	TArray<float> SurfaceCoefficients = AbsorptionSubsystem->GetSurfaceAbsorptionCoefficients(Material);
	MaterialCoefficients Result = MaterialCoefficients();
	for (int i = 0; i < NumFrequencyBands; ++i)
	{
		/**
		 * The Absorption coefficents mean that 0 is the most reflective (0 absorption) and 1 is the least
		 * We want to store the Reflectiveness not the Absorption
		 */

		Result[i] = 1 - SurfaceCoefficients[i];
	}

	return Result;
}

MaterialCoefficients AccumulateMaterial(
	const EPhysicalSurface& Material,
	const MaterialCoefficients& Current
)
{
	const auto& MaterialCoefficients = LookupCoefficients(Material);


	auto ReturnValue = Current;
	for (size_t i = 0; i < NumFrequencyBands; i++)
	{
		ReturnValue[i] *= MaterialCoefficients[i];
	}

	return ReturnValue;
}


struct Raycast
{
	Raycast() = default;

	Raycast(FVector Center, FVector Direction) :
		R{Center, Direction}
	{
	}

	FRay R;
	FHitResult Intersection;
	Impulse CurrentImpulse;
	bool Invalid = false;
};


TArray<Impulse> AIR_Generator::CastRays(const FVector Center)
{
	TArray<Impulse> ReturnValue;
	ReturnValue.Reserve(NumRaycasts);

	TArray<Raycast> WorkingVectors;

	UE::Geometry::TSphericalFibonacci<double> SphericalFib(NumRaycasts);
	WorkingVectors.Reserve(NumRaycasts);
	for (int i = 0; i < NumRaycasts; i++)
	{
		WorkingVectors.Emplace(Center, SphericalFib.Point(i));
	}

	for (unsigned int Bounce = 0; Bounce < NumBounces; Bounce++)
	{
		// Raycast from each working vector
		for (Raycast& WorkingVector : WorkingVectors)
		{
			FHitResult Intersection;
			FCollisionQueryParams CollisionParams;
			CollisionParams.bReturnPhysicalMaterial = true;
			if (Bounce == 0)
				CollisionParams.AddIgnoredComponent(ListenerSphere);
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				Intersection,
				WorkingVector.R.Origin,
				WorkingVector.R.Origin + WorkingVector.R.Direction * MaxRaycastDistance,
				ECC_Visibility,
				CollisionParams
			);

			if (!bHit)
			{
				WorkingVector.Invalid = true;
				continue;
			}

			WorkingVector.CurrentImpulse.DistanceTraveled += Intersection.Distance;

			if (Intersection.GetComponent() && Intersection.GetComponent() == ListenerSphere)
			{
				ReturnValue.Push(WorkingVector.CurrentImpulse);

				// If we hit the listener, cast again ignoring the listener
				WorkingVector.CurrentImpulse.DistanceTraveled -= Intersection.Distance;
				CollisionParams.bReturnPhysicalMaterial = true;
				CollisionParams.AddIgnoredComponent(ListenerSphere);
				bool bHitNoListener = GetWorld()->LineTraceSingleByChannel(
					Intersection,
					WorkingVector.R.Origin,
					WorkingVector.R.Origin + WorkingVector.R.Direction * MaxRaycastDistance,
					ECC_Visibility,
					CollisionParams
				);
				if (!bHitNoListener)
				{
					WorkingVector.Invalid = true;
					continue;
				}

				WorkingVector.CurrentImpulse.DistanceTraveled += Intersection.Distance;

				if (Intersection.PhysMaterial.IsValid())
				{
					WorkingVector.CurrentImpulse.DesiredGainValues = AccumulateMaterial(
						Intersection.PhysMaterial->SurfaceType, WorkingVector.CurrentImpulse.DesiredGainValues);
				}
				else
				{
					WorkingVector.CurrentImpulse.DesiredGainValues = AccumulateMaterial(
						SurfaceType_Default, WorkingVector.CurrentImpulse.DesiredGainValues);
				}

				WorkingVector.Intersection = Intersection;
			}
			else
			{
				WorkingVector.Intersection = Intersection;

				if (Intersection.PhysMaterial.IsValid())
				{
					WorkingVector.CurrentImpulse.DesiredGainValues = AccumulateMaterial(
						Intersection.PhysMaterial->SurfaceType, WorkingVector.CurrentImpulse.DesiredGainValues);
				}
				else
				{
					WorkingVector.CurrentImpulse.DesiredGainValues = AccumulateMaterial(
						SurfaceType_Default, WorkingVector.CurrentImpulse.DesiredGainValues);
				}
			}
		}

		WorkingVectors.RemoveAll(
			[&](const Raycast& R) { return R.Invalid; }
		);

		// Reflect each working vector left around hit normal
		for (Raycast& WorkingVector : WorkingVectors)
		{
			FVector Reflection =
				WorkingVector.R.Direction.MirrorByVector(WorkingVector.Intersection.Normal);
			WorkingVector.R = {
				WorkingVector.Intersection.ImpactPoint,
				Reflection
			};
		}
	}

	return ReturnValue;
}

void AIR_Generator::MergeImpulses(TArray<Impulse>& Impulses)
{
	Impulses.Sort();


	auto NumImpulses = Impulses.Num();

	size_t CurrentImpulseIndex = 0;

	for (size_t i = 1; i < NumImpulses; i++)
	{
		Impulse& CurrentImpulse = Impulses[CurrentImpulseIndex];
		Impulse& NextImpulse = Impulses[i];

		float DeltaTime = NextImpulse.GetDelaySeconds() - CurrentImpulse.GetDelaySeconds();

		if (DeltaTime < TimeStep)
		{
			CurrentImpulse.MergeWith(NextImpulse);
			NextImpulse.Invalid = true;
		}
		else
		{
			CurrentImpulseIndex = i;
		}
	}

	Impulses.RemoveAll(
		[](const Impulse& Impulse) { return Impulse.Invalid; }
	);
}

void AIR_Generator::CalculateImpulseStarts(TArray<Impulse>& Impulses)
{
	for (Impulse& Impulse : Impulses)
	{
		Impulse.StartSample = GetSamplesFromSeconds(Impulse.GetDelaySeconds());
	}
}


#if WITH_EDITOR
void AIR_Generator::CalculateAndRecordImpulseResponseToFile()
{
	// -- Impulse Generation --
	TArray<Impulse> Impulses = CastRays(ListenerSphere->GetComponentLocation());
	UE_LOG(LogTemp, Warning, TEXT("Found %i impulses"), Impulses.Num());
	MergeImpulses(Impulses);
	UE_LOG(LogTemp, Warning, TEXT("Merged, now %i impulses"), Impulses.Num());
	CalculateImpulseStarts(Impulses);

	// -- Impulse Filtering --

	UE_LOG(LogTemp, Warning, TEXT("Running DSP"));
	UE_LOG(LogTemp, Warning, TEXT("Should be %f seconds"), Impulses.Last().GetDelaySeconds() + LengthSeconds);
	Audio::TSampleBuffer SampleBuffer = IR_Generator_DSP::RunDSP(Impulses, AcousticAbsorption, AtmosphericAbsorption);
	UE_LOG(LogTemp, Warning, TEXT("Buffer finished with %i samples"), SampleBuffer.GetNumSamples());

	// -- Impulse Writing -- 

	UE_LOG(LogTemp, Warning, TEXT("Writing to wav"));
	Audio::FSoundWavePCMWriter WaveWriter = Audio::FSoundWavePCMWriter();
	const FString GeneratedDirectory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GeneratedIR"));
	IFileManager::Get().MakeDirectory(*GeneratedDirectory, true);

	const FString WavFilePath = FPaths::Combine(GeneratedDirectory, FileName + TEXT(".wav"));
	UE_LOG(LogTemp, Warning, TEXT("Checking for existing WAV at: %s"), *WavFilePath);
	if (IFileManager::Get().FileExists(*WavFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Found existing WAV file, deleting..."));
		bool bDeleteSuccess = IFileManager::Get().Delete(*WavFilePath);
		UE_LOG(LogTemp, Warning, TEXT("Delete result: %s"), bDeleteSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No existing WAV file found"));
	}

	WaveWriter.SynchronouslyWriteToWavFile(SampleBuffer, FileName, GeneratedDirectory);

	// Delete existing SoundWave asset if it exists
	FString ExistingAssetPath = FString::Printf(TEXT("/Game/GeneratedIR/%s"), *FileName);
	UE_LOG(LogTemp, Warning, TEXT("Checking for existing asset at: %s"), *ExistingAssetPath);
	if (UEditorAssetLibrary::DoesAssetExist(ExistingAssetPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Found existing asset, deleting..."));
		bool bDeleteSuccess = UEditorAssetLibrary::DeleteAsset(ExistingAssetPath);
		UE_LOG(LogTemp, Warning, TEXT("Asset delete result: %s"), bDeleteSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No existing asset found"));
	}

	// Deselect everything in content browser to prevent template dialog
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(
		"ContentBrowser");
	TArray<UObject*> EmptySelection;
	ContentBrowserModule.Get().SyncBrowserToAssets(EmptySelection, false, false);

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	// Create and configure the Factory
	USoundFactory* SoundFactory = NewObject<USoundFactory>();
	SoundFactory->bAutoCreateCue = false;
	// SoundFactory->bTextMode = false;
	SoundFactory->bEditorImport = true;

	// Set up the Import Task
	UAssetImportTask* ImportTask = NewObject<UAssetImportTask>();
	ImportTask->Filename = WavFilePath;
	ImportTask->DestinationPath = TEXT("/Game/GeneratedIR");
	ImportTask->DestinationName = FileName;
	ImportTask->Factory = SoundFactory;
	ImportTask->bAutomated = true;
	ImportTask->bReplaceExisting = true;
	ImportTask->bSave = true;

	// Execute via AssetTools
	AssetToolsModule.Get().ImportAssetTasks({ImportTask});

	// Load the imported SoundWave by path
	FString SoundWavePath = FString::Printf(TEXT("/Game/GeneratedIR/%s"), *FileName);
	USoundWave* ImportedWave = Cast<USoundWave>(StaticLoadObject(USoundWave::StaticClass(), nullptr, *SoundWavePath));

	if (!ImportedWave)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load imported SoundWave at: %s"), *SoundWavePath);
		return;
	}

	UAudioImpulseResponseFactory* IRFactory = NewObject<UAudioImpulseResponseFactory>();
	IRFactory->StagedSoundWave = ImportedWave;

	FString IRAssetPath;
	FString IRAssetName;
	AssetToolsModule.Get().CreateUniqueAssetName(
		FString::Printf(TEXT("/Game/GeneratedIR/%s_IR"), *FileName),
		TEXT(""),
		IRAssetPath,
		IRAssetName);

	UObject* GeneratedIR = AssetToolsModule.Get().CreateAsset(
		IRAssetName,
		FPackageName::GetLongPackagePath(IRAssetPath),
		UAudioImpulseResponse::StaticClass(),
		IRFactory);

	if (!GeneratedIR)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create IR asset"));
		return;
	}

	Modify();
	GeneratedImpulseResponse = Cast<UAudioImpulseResponse>(GeneratedIR);
}
#endif
