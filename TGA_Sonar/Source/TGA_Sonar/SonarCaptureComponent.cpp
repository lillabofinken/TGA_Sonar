// Fill out your copyright notice in the Description page of Project Settings.


#include "SonarCaptureComponent.h"

void USonarCaptureComponent::UpdateSceneCaptureContents(FSceneInterface* Scene,
	class ISceneRenderBuilder& SceneRenderBuilder)
{
	if (bUseCustomProjection)
	{
		ProjectionType = ECameraProjectionMode::Perspective;
		CustomProjectionMatrix = GetCustomProjectionMatrix();
		bUseCustomProjectionMatrix = true;
	}
	
	Super::UpdateSceneCaptureContents(Scene, SceneRenderBuilder);
}

const FMatrix USonarCaptureComponent::GetCustomProjectionMatrix()
{
	const float TanHalfHorFOV = FMath::Tan(FMath::DegreesToRadians(HorizontalCustomFOVAngle / 2.0f));
	const float TanHalfVerFOV = FMath::Tan(FMath::DegreesToRadians(VerticalCustomFOVAngle / 2.0f));

	const float XScale = 1.0f / TanHalfHorFOV;
	const float YScale = 1.0f / TanHalfVerFOV;

	return FMatrix(
		FPlane(XScale, 0.0f,   0.0f,    0.0f),
		FPlane(0.0f,   YScale, 0.0f,    0.0f),
		FPlane(0.0f,   0.0f,   0.0f,    1.0),
		FPlane(0.0f,   0.0f,   NearClip, 0.0f)
	);
}
