// Fill out your copyright notice in the Description page of Project Settings.


#include "IR_Generator.h"

#include "IR_Generator_DSP.h"
#include "Components/SphereComponent.h"
#include "DSP/Noise.h"
#include "Sampling/SphericalFibonacci.h"
#include "Sound/SampleBufferIO.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "EffectConvolutionReverb.h"

#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "Factories/SoundFactory.h"
#include "AudioImpulseResponseAsset.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Sound/SoundWave.h"
#endif

constexpr float ListenerSphereRadius = 15.0f;
constexpr float MaxRaycastDistance = 10000.0f;


// Sets default values
AIR_Generator::AIR_Generator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

#if WITH_EDITORONLY_DATA
	Sphere = CreateEditorOnlyDefaultSubobject<USphereComponent>(TEXT("EditorTraceSphere"));

	if (Sphere)
	{
		Sphere->SetupAttachment(RootComponent);
		Sphere->InitSphereRadius(ListenerSphereRadius);

		Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		// Sphere->SetCollisionObjectType(ECC_WorldDynamic);

		// Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		Sphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

		Sphere->SetHiddenInGame(true);
	}
#endif
}

// Called when the game starts or when spawned
void AIR_Generator::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AIR_Generator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AIR_Generator::LogImpulseValues()
{
	UE_LOG(LogTemp, Warning, TEXT("Running ray casting"));
	TArray<Impulse> Impulses = CastRays(Sphere->GetComponentLocation());
	UE_LOG(LogTemp, Warning, TEXT("Found %i impulses"), Impulses.Num())
	MergeImpulses(Impulses);
	UE_LOG(LogTemp, Warning, TEXT("Merged, now %i impulses"), Impulses.Num())
	for (auto& Impulse : Impulses)
	{
		UE_LOG(LogTemp, Warning, TEXT("Delay - %.3f"), Impulse.GetDelaySeconds());
	}
}

void AIR_Generator::TestFilter()
{
	// 0-0
	TArray<float> buf;
	buf.Reserve(48000);

	Audio::FWhiteNoise noise;

	for (int32 i = 0; i < 48000; i++)
	{
		buf.Add(noise.Generate());
	}

	Audio::FBiquadFilter Filter;
	Filter.Init(48000, 1, Audio::EBiquadFilter::Lowpass, 500);
	Filter.SetEnabled(true);

	Filter.ProcessAudio(buf.GetData(), 48000, buf.GetData());

	Audio::TSampleBuffer<> sbuf(buf.GetData(), 48000, 1, 48000);

	Audio::FSoundWavePCMWriter WaveWriter = Audio::FSoundWavePCMWriter();
	WaveWriter.SynchronouslyWriteToWavFile(sbuf, FileName, FString("."));
}


float Impulse::GetDelaySeconds() const
{
	float DistanceMeters = DistanceTraveled / 100.0f;
	constexpr float SpeedOfSoundMetersPerSecond = 434.0f;
	return DistanceMeters / SpeedOfSoundMetersPerSecond;
}


MaterialCoefficients LookupCoefficients(const EPhysicalSurface& Material)
{
	switch (Material)
	{
	// Wood
	case SurfaceType1:
		return {0.19f, 0.23f, 0.25f, 0.30f, 0.37f, 0.42f};
	// Concrete
	case SurfaceType2:
		return {0.02f, 0.03f, 0.03f, 0.03f, 0.04f, 0.07f};
	default:
		// return {0.05f, 0.04f, 0.02f, 0.04f, 0.05f, 0.05f};
		return {0.8f, 0.7f, 0.7f, 0.7f, 0.7f, 0.8f};
	}
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
			if (Bounce == 0)
				CollisionParams.AddIgnoredComponent(Sphere);
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				Intersection,
				WorkingVector.R.Origin,
				WorkingVector.R.Origin + WorkingVector.R.Direction * MaxRaycastDistance,
				ECC_Visibility,
				CollisionParams
			);
			if (ShowDebugRaycasts)
			{
				DrawDebugLine(
					GetWorld(),
					WorkingVector.R.Origin,
					bHit
						? Intersection.ImpactPoint
						: WorkingVector.R.Origin + WorkingVector.R.Direction * MaxRaycastDistance,
					bHit ? FColor::Red : FColor::Green,
					false,
					5.0f,
					0,
					1.5f
				);
			}
			if (!bHit)
			{
				WorkingVector.Invalid = true;
				continue;
			}

			WorkingVector.CurrentImpulse.DistanceTraveled += Intersection.Distance;

			if (Intersection.GetComponent() && Intersection.GetComponent() == Sphere)
			{
				ReturnValue.Push(WorkingVector.CurrentImpulse);

				// If we hit the listener, cast again ignoring the listener
				WorkingVector.CurrentImpulse.DistanceTraveled -= Intersection.Distance;
				CollisionParams.AddIgnoredComponent(Sphere);
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
		for (auto& WorkingVector : WorkingVectors)
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
		auto& CurrentImpulse = Impulses[CurrentImpulseIndex];
		Impulse& NextImpulse = Impulses[i];

		auto DeltaTime = NextImpulse.GetDelaySeconds() - CurrentImpulse.GetDelaySeconds();

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
	TArray<Impulse> Impulses = CastRays(Sphere->GetComponentLocation());
	UE_LOG(LogTemp, Warning, TEXT("Found %i impulses"), Impulses.Num());
	MergeImpulses(Impulses);
	UE_LOG(LogTemp, Warning, TEXT("Merged, now %i impulses"), Impulses.Num());
	CalculateImpulseStarts(Impulses);


	UE_LOG(LogTemp, Warning, TEXT("Running DSP"));
	UE_LOG(LogTemp, Warning, TEXT("Should be %f seconds"), Impulses.Last().GetDelaySeconds() + LengthSeconds);
	Audio::TSampleBuffer SampleBuffer = IR_Generator_DSP::RunDSP(Impulses, AcousticAbsorption, AtmosphericAbsorption);
	UE_LOG(LogTemp, Warning, TEXT("Buffer finished with %i samples"), SampleBuffer.GetNumSamples());

	UE_LOG(LogTemp, Warning, TEXT("Writing to wav"));
	Audio::FSoundWavePCMWriter WaveWriter = Audio::FSoundWavePCMWriter();
	const FString GeneratedDirectory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GeneratedIR"));
	IFileManager::Get().MakeDirectory(*GeneratedDirectory, true);
	WaveWriter.SynchronouslyWriteToWavFile(SampleBuffer, FileName, GeneratedDirectory);

	const FString WavFilePath = FPaths::Combine(GeneratedDirectory, FileName + TEXT(".wav"));

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	USoundFactory* SoundFactory = NewObject<USoundFactory>();
	TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssets(
		{WavFilePath},
		TEXT("/Game/GeneratedIR"),
		SoundFactory,
		false);

	USoundWave* ImportedWave = nullptr;
	for (UObject* ImportedAsset : ImportedAssets)
	{
		ImportedWave = Cast<USoundWave>(ImportedAsset);
		if (ImportedWave)
		{
			break;
		}
	}

	if (!ImportedWave)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to import generated wav as a SoundWave"));
		return;
	}

	UAudioImpulseResponseFactory* Factory = NewObject<UAudioImpulseResponseFactory>();
	Factory->StagedSoundWave = ImportedWave;

	FString PackagePath;
	FString AssetName;
	AssetToolsModule.Get().CreateUniqueAssetName(ImportedWave->GetOutermost()->GetName(), TEXT("_IR"), PackagePath, AssetName);
	AssetToolsModule.Get().CreateAsset(
		AssetName,
		FPackageName::GetLongPackagePath(PackagePath),
		UAudioImpulseResponse::StaticClass(),
		Factory);
}
#endif
