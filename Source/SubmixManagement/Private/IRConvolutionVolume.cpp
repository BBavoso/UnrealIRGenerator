// Fill out your copyright notice in the Description page of Project Settings.

#include "IRConvolutionVolume.h"
#include "IRConvolutionSubsystem.h"

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
