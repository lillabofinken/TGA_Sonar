#include "DeformationCSLibrary.h"

void UDeformationCSLibrary::ExecuteRTComputeShader(UTextureRenderTarget2D* RT, UTextureRenderTarget2D* InputTexture, float CurrentAngle, float UpdateAngle, float Range)
{
	//Create a dispatch parameters struct and fill it the input array with our args
	FDeformationCSDispatchParams Params(RT->SizeX, RT->SizeY, 1);
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