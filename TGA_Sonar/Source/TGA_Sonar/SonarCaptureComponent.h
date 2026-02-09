// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "SonarCaptureComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TGA_SONAR_API USonarCaptureComponent : public USceneCaptureComponent2D
{
	GENERATED_BODY()

public:
	UPROPERTY(interp, EditAnywhere, BlueprintReadWrite, Category=Projection )
	float HorizontalCustomFOVAngle = 90;
	UPROPERTY(interp, EditAnywhere, BlueprintReadWrite, Category=Projection )
	float VerticalCustomFOVAngle = 90;
	
	UPROPERTY(interp, EditAnywhere, BlueprintReadWrite, Category=Projection )
	float FarClip = 10000;
	UPROPERTY(interp, EditAnywhere, BlueprintReadWrite, Category=Projection )
	float NearClip = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projection")
	bool bUseCustomProjection = false;

	virtual void UpdateSceneCaptureContents(FSceneInterface* Scene, class ISceneRenderBuilder& SceneRenderBuilder) override;

private:

	const FMatrix GetCustomProjectionMatrix();
};
