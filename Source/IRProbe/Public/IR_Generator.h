// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	// Sets default values for this actor's properties
	AIR_Generator();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* Sphere;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	TArray<Impulse> CastRays(const FVector Center);

	UFUNCTION(CallInEditor, Category = "Impulse Response Generation")
	void CalculateAndRecordImpulseResponseToFile();

	UFUNCTION(CallInEditor, Category = "Impulse Response Generation")
	void LogImpulseValues();

	UFUNCTION(CallInEditor, Category = "Impulse Response Generation")
	void TestFilter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse Response Generation")
	int NumRaycasts = 32000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse Response Generation")
	int NumBounces = 256;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse Response Generation")
	bool ShowDebugRaycasts = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse Response Generation")
	FString FileName = "GeneratedImpulseResponse";

private:
	static void MergeImpulses(TArray<Impulse>& Impulses);
	static void CalculateImpulseStarts(TArray<Impulse>& Impulses);
};
