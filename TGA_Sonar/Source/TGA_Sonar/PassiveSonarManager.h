// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeformationCSLibrary.h"
#include "NoiseEmitterComponent.h"
#include "PassiveSonarManager.generated.h"


struct NoiseEmitterDataStruct
{
	FVector Position;
	float Range;
	float Sharpness;
};

struct NoiseEmitterStruct
{
	NoiseEmitterDataStruct data;
	UNoiseEmitterComponent* noiseEmitter;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )


class TGA_SONAR_API UPassiveSonarManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPassiveSonarManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY( EditAnywhere, Category = "ImportantVariables" ) UTextureRenderTarget2D* RenderTarget;
	UPROPERTY( EditAnywhere, Category = "ImportantVariables" ) float Framerate = 24;
	UPROPERTY( EditAnywhere, Category = "ImportantVariables" ) float WaterfallSeconds = 10;
	UPROPERTY()	TArray<USceneComponent*> TrackedObjects;

	UFUNCTION( BlueprintCallable ) void AddTrackedObjects  ( TArray<USceneComponent*> _trackedComponent);
	UFUNCTION( BlueprintCallable ) void AddTrackedObject   ( USceneComponent*         _trackedComponent);

private:
	void updatePassiveSonar(float _deltaTime);
	float framerateTime = 0;
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static UPassiveSonarManager* GetDeformationManager();
private:
	inline static UPassiveSonarManager* DeformationManager = nullptr;
};
