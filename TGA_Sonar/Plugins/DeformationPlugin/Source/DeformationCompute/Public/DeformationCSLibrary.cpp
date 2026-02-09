#include "DeformationCSLibrary.h"

void UDeformationCSLibrary::ExecuteRTComputeShader(UTextureRenderTarget2D* RT, float SnowDepth)
{
	//Create a dispatch parameters struct and fill it the input array with our args
	FDeformationCSDispatchParams Params(RT->SizeX, RT->SizeY, 1);
	Params.RenderTarget = RT->GameThread_GetRenderTargetResource();
	Params.testValue = SnowDepth;
	
	FDeformationCSInterface::Dispatch(Params);
}

void UDeformationCSLibrary::ExecuteRTComputeShader(FDeformationCSDispatchParams _params)
{
}