#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialRenderProxy.h"


struct DEFORMATIONCOMPUTE_API FDeformationCSDispatchParams
{
	int X;
	int Y;
	int Z;

	
	FRenderTarget* RenderTarget;
	FRenderTarget* Panorama;
	
	float CurrentAngle = 0;
	float UpdateAngle  = 0;
	float Range        = 0;
	

	FDeformationCSDispatchParams(int x, int y, int z)
		: X(x)
		, Y(y)
		, Z(z)
	{
	}
};

// This is a public interface that we define so outside code can invoke our compute shader.
class DEFORMATIONCOMPUTE_API FDeformationCSInterface {
public:
	// Executes this shader on the render thread
	static void DispatchRenderThread(
		FRHICommandListImmediate& RHICmdList,
		FDeformationCSDispatchParams Params
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void DispatchGameThread(
		FDeformationCSDispatchParams Params
	)
	{
		ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
		[Params](FRHICommandListImmediate& RHICmdList)
		{
			DispatchRenderThread(RHICmdList, Params);
		});
	}

	// Dispatches this shader. Can be called from any thread
	static void Dispatch(
		FDeformationCSDispatchParams Params
	)
	{
		if (IsInRenderingThread()) {
			DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), Params);
		}else{
			DispatchGameThread(Params);
		}
	}
};

// This is a static blueprint library that can be used to invoke our compute shader from blueprints.
//UCLASS()
//class DEFORMATIONCOMPUTE_API UDeformationCSLibrary : public UObject
//{
//	GENERATED_BODY()
//	
//public:
//	UFUNCTION(BlueprintCallable)
//	static void ExecuteRTComputeShader(UTextureRenderTarget2D* RT)
//	{
//		// Create a dispatch parameters struct and fill it the input array with our args
//		FDeformationCSDispatchParams Params(RT->SizeX, RT->SizeY, 1);
//		Params.RenderTarget = RT->GameThread_GetRenderTargetResource();
//
//		FDeformationCSInterface::Dispatch(Params);
//	}
//};
