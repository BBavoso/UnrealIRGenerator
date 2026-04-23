// Fill out your copyright notice in the Description page of Project Settings.

#include "IRConvolutionVolume.h"
#include "IRConvolutionSubsystem.h"
#include "Engine/World.h"

AIRConvolutionVolume::AIRConvolutionVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);

	// Set default box size
	TriggerBox->SetBoxExtent(FVector(200.0f, 200.0f, 200.0f));
}

#if WITH_EDITOR
void AIRConvolutionVolume::UpdateVolumeWithoutGenerating()
{
	if (!IrGenerator)
	{
		UE_LOG(LogTemp, Error, TEXT("Provide an IR Generator to bake"));
		return;
	}	
	
	if (!IrGenerator->GeneratedImpulseResponse)
	{
		UE_LOG(LogTemp, Error, TEXT("Impulse response has not been generated yet, use 'Bake Impulse and Update Volume'"));
		return;
	}
	
	Modify();
	ImpulseResponse = IrGenerator->GeneratedImpulseResponse;
}

void AIRConvolutionVolume::BakeImpulseAndUpdateVolume()
{
	if (!IrGenerator)
	{
		UE_LOG(LogTemp, Error, TEXT("Provide an IR Generator to bake"));
		return;
	}	
	
	
	if (!IrGenerator->BakeProbe)
	{
		UE_LOG(LogTemp, Error, TEXT("Impulse Response Generator has baking disabled, enable it first to bake it"));
		return;
	}
	
	IrGenerator->CalculateAndRecordImpulseResponseToFile();
	
	ImpulseResponse = IrGenerator->GeneratedImpulseResponse;
}
#endif

void AIRConvolutionVolume::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AIRConvolutionVolume::OnBoxBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AIRConvolutionVolume::OnBoxEndOverlap);
}

void AIRConvolutionVolume::OnBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (UIRConvolutionSubsystem* Subsystem = GetWorld()->GetSubsystem<UIRConvolutionSubsystem>())
	{
		Subsystem->NotifyVolumeEntered(this, OtherActor);
	}
}

void AIRConvolutionVolume::OnBoxEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (UIRConvolutionSubsystem* Subsystem = GetWorld()->GetSubsystem<UIRConvolutionSubsystem>())
	{
		Subsystem->NotifyVolumeExited(this, OtherActor);
	}
}


