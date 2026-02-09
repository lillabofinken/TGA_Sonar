#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialRenderProxy.h"

#include "DeformationCS/DeformationCS.h"
#include "DeformationCSLibrary.generated.h"
//This is a static blueprint library that can be used to invoke our compute shader from blueprints.
UCLASS()
class DEFORMATIONCOMPUTE_API UDeformationCSLibrary : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void ExecuteRTComputeShader(UTextureRenderTarget2D* RT, float SnowDepth);
	
	static void ExecuteRTComputeShader(FDeformationCSDispatchParams _params);
};