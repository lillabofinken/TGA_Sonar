#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialRenderProxy.h"

//#include "TopographicMapCS.generated.h"

struct DEFORMATIONCOMPUTE_API FTopographicMapCSDispatchParams
{
	int X;
	int Y;
	int Z;

	
	FRenderTarget* RenderTarget;
	UTexture2D*    Heightmap;

	float ContourLineStep;
	int   IndexLineStep;

	int ContourLineThickness;
	int IndexLineThickness;
	
	

	FTopographicMapCSDispatchParams(int x, int y, int z)
		: X(x)
		, Y(y)
		, Z(z)
	{
	}
};

// This is a public interface that we define so outside code can invoke our compute shader.
class DEFORMATIONCOMPUTE_API FTopographicMapCSInterface {
public:
	// Executes this shader on the render thread
	static void DispatchRenderThread(
		FRHICommandListImmediate& RHICmdList,
		FTopographicMapCSDispatchParams Params
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void DispatchGameThread(
		FTopographicMapCSDispatchParams Params
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
		FTopographicMapCSDispatchParams Params
	)
	{
		if (IsInRenderingThread()) {
			DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), Params);
		}else{
			DispatchGameThread(Params);
		}
	}
};