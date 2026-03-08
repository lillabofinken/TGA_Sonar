#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialRenderProxy.h"

#define NoiseEmitterMaxAmount 32
//#include "PassiveSonarCS.generated.h"
struct NoiseEmitterDataStruct
{
	FVector Position;
	float Range;
	float Sharpness;
	NoiseEmitterDataStruct()
	{
		Position  = FVector( 0 );
		Range     = 0;
		Sharpness = 0;	
	}
};

struct DEFORMATIONCOMPUTE_API FPassiveSonarCSDispatchParams
{
	int X;
	int Y;
	int Z;

	
	FRenderTarget* RenderTarget = nullptr;
	float time = 0;
	float UpdateAmount = 0;
	TArray<NoiseEmitterDataStruct> NoiseEmitters;
	int EmitterAmount = 0;

	FPassiveSonarCSDispatchParams(int x, int y, int z)
		: X(x)
		, Y(y)
		, Z(z)
	{
	}
};


// This is a public interface that we define so outside code can invoke our compute shader.
class DEFORMATIONCOMPUTE_API FPassiveSonarCSInterface {
public:
	// Executes this shader on the render thread
	static void DispatchRenderThread(
		FRHICommandListImmediate& RHICmdList,
		FPassiveSonarCSDispatchParams Params
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void DispatchGameThread(
		FPassiveSonarCSDispatchParams Params
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
		FPassiveSonarCSDispatchParams Params
	)
	{
		if (IsInRenderingThread()) {
			DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), Params);
		}else{
			DispatchGameThread(Params);
		}
	}
};