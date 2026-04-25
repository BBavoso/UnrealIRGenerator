// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EffectConvolutionReverb.h"
#include "IR_Generator_DSP.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "IR_Generator.generated.h"


// Minimum
constexpr float TimeStep = 0.001f;

UCLASS()
class IRPROBE_API AIR_Generator : public AActor
{
	GENERATED_BODY()

public:
	AIR_Generator();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Impulse Response Generation")
	USphereComponent* ListenerSphere;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Impulse Response Generation")
	USceneComponent* Root;

public:
	TArray<Impulse> CastRays(const FVector Center);

#if WITH_EDITOR
	UFUNCTION(CallInEditor, Category = "Impulse Response Generation")
	void CalculateAndRecordImpulseResponseToFile();
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse Response Generation")
	int NumRaycasts = 32000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse Response Generation")
	int NumBounces = 256;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse Response Generation")
	FString FileName = "GeneratedImpulseResponse";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse Response Generation")
	bool AcousticAbsorption = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse Response Generation")
	bool AtmosphericAbsorption = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse Response Generation")
	bool BakeProbe = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse Response Generation")
	TObjectPtr<UAudioImpulseResponse> GeneratedImpulseResponse;

private:
	static void MergeImpulses(TArray<Impulse>& Impulses);
	static void CalculateImpulseStarts(TArray<Impulse>& Impulses);
};
