// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "IRConvolutionVolume.generated.h"

UCLASS()
class SUBMIXMANAGEMENT_API AIRConvolutionVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	AIRConvolutionVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IR Convolution")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IR Convolution")
	float CrossfadeDuration = 1.0f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
	                       bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
