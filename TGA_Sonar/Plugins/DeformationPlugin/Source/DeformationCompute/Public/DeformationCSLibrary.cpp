#include "DeformationCSLibrary.h"

#include "Landscape.h"
#include "Engine/Texture2D.h"

void UDeformationCSLibrary::ExecuteRTComputeShader(UTextureRenderTarget2D* RT, UTextureRenderTarget2D* InputTexture, float CurrentAngle, float UpdateAngle, float Range)
{
	//Create a dispatch parameters struct and fill it the input array with our args
	FDeformationCSDispatchParams Params(RT->SizeX, RT->SizeY, 1);
	const int WidthToUpdate = RT->SizeX * (UpdateAngle / 360.0f);
	Params.X = 16;

	
	Params.Panorama = InputTexture->GameThread_GetRenderTargetResource();
	Params.RenderTarget = RT->GameThread_GetRenderTargetResource();
	Params.CurrentAngle = CurrentAngle;
	Params.UpdateAngle  = UpdateAngle;
	Params.Range        = Range;
	
	FDeformationCSInterface::Dispatch(Params);
}

void UDeformationCSLibrary::ExecuteRTComputeShader(FDeformationCSDispatchParams _params)
{
	FDeformationCSInterface::Dispatch(_params);
}

void UDeformationCSLibrary::ExecutePassiveSonarComputeShader(FPassiveSonarCSDispatchParams _params)
{
	FPassiveSonarCSInterface::Dispatch(_params);
}

void UDeformationCSLibrary::ExecuteTopographicMapComputeShader( UTextureRenderTarget2D* RT, ALandscape* LandscapeActor,
	float ContourLineStep, int IndexLineStep, float ContourLineThickness, float IndexLineThickness )
{





	
	FTopographicMapCSDispatchParams Params(RT->SizeX, RT->SizeY, 1);
	Params.RenderTarget = RT->GameThread_GetRenderTargetResource();
	Params.ContourLineStep = ContourLineStep;
	Params.IndexLineStep = IndexLineStep;
	Params.ContourLineThickness = ContourLineThickness;
	Params.IndexLineThickness = IndexLineThickness;
	
}

void UDeformationCSLibrary::ExecuteTopographicMapComputeShader( FTopographicMapCSDispatchParams _params )
{
	FTopographicMapCSInterface::Dispatch( _params );
}

UTexture2D* UDeformationCSLibrary::CreateLandscapeHeightmapTexture( ALandscape* LandscapeActor )
{
	if (LandscapeActor->LandscapeComponents.Num() > 0)
	{
		ULandscapeComponent* FirstComponent = LandscapeActor->LandscapeComponents[0];
		if (FirstComponent)
		{
			return FirstComponent->GetHeightmap(); // UTexture2D* with height data
		}
	}
	return nullptr;
}
