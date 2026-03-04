// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NoiseEmitterComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TGA_SONAR_API UNoiseEmitterComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UNoiseEmitterComponent();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NoiseEmitter") float Range;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NoiseEmitter") float FalloffCurve;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NoiseEmitter") float Sharpness;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
