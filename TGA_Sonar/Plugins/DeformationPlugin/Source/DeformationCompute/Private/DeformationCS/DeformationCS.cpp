#include "DeformationCS.h"
#include "DeformationCompute/Public/DeformationCS/DeformationCS.h"
#include "PixelShaderUtils.h"
#include "MeshPassProcessor.inl"
#include "StaticMeshResources.h"
#include "DynamicMeshBuilder.h"
#include "RenderGraphResources.h"
#include "GlobalShader.h"
#include "UnifiedBuffer.h"
#include "CanvasTypes.h"
#include "MeshDrawShaderBindings.h"
#include "RHIGPUReadback.h"
#include "MeshPassUtils.h"
#include "MaterialShader.h"

DECLARE_STATS_GROUP(TEXT("DeformationCS"), STATGROUP_DeformationCS, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("DeformationCS Execute"), STAT_DeformationCS_Execute, STATGROUP_DeformationCS);

#define RenderTextureFormat PF_G16R16F
#define InputTextureFormat PF_R16F
// This class carries our parameter declarations and acts as the bridge between cpp and HLSL.
class DEFORMATIONCOMPUTE_API FDeformationCS: public FGlobalShader
{
public:
	
	DECLARE_GLOBAL_SHADER(FDeformationCS);
	SHADER_USE_PARAMETER_STRUCT(FDeformationCS, FGlobalShader);
	
	
	class FDeformationCS_Perm_TEST : SHADER_PERMUTATION_INT("TEST", 1);
	using FPermutationDomain = TShaderPermutationDomain<
		FDeformationCS_Perm_TEST
	>;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		/*
		* Here's where you define one or more of the input parameters for your shader.
		* Some examples:
		// SHADER_PARAMETER(uint32, MyUint32) // On the shader side: uint32 MyUint32;
		// SHADER_PARAMETER(FVector3f, MyVector) // On the shader side: float3 MyVector;

		// SHADER_PARAMETER_TEXTURE(Texture2D, MyTexture) // On the shader side: Texture2D<float4> MyTexture; (float4 should be whatever you expect each pixel in the texture to be, in this case float4(R,G,B,A) for 4 channels)
		// SHADER_PARAMETER_SAMPLER(SamplerState, MyTextureSampler) // On the shader side: SamplerState MySampler; // CPP side: TStaticSamplerState<ESamplerFilter::SF_Bilinear>::GetRHI();

		// SHADER_PARAMETER_ARRAY(float, MyFloatArray, [3]) // On the shader side: float MyFloatArray[3];

		// SHADER_PARAMETER_UAV(RWTexture2D<FVector4f>, MyTextureUAV) // On the shader side: RWTexture2D<float4> MyTextureUAV;
		// SHADER_PARAMETER_UAV(RWStructuredBuffer<FMyCustomStruct>, MyCustomStructs) // On the shader side: RWStructuredBuffer<FMyCustomStruct> MyCustomStructs;
		// SHADER_PARAMETER_UAV(RWBuffer<FMyCustomStruct>, MyCustomStructs) // On the shader side: RWBuffer<FMyCustomStruct> MyCustomStructs;

		// SHADER_PARAMETER_SRV(StructuredBuffer<FMyCustomStruct>, MyCustomStructs) // On the shader side: StructuredBuffer<FMyCustomStruct> MyCustomStructs;
		// SHADER_PARAMETER_SRV(Buffer<FMyCustomStruct>, MyCustomStructs) // On the shader side: Buffer<FMyCustomStruct> MyCustomStructs;
		// SHADER_PARAMETER_SRV(Texture2D<FVector4f>, MyReadOnlyTexture) // On the shader side: Texture2D<float4> MyReadOnlyTexture;

		// SHADER_PARAMETER_STRUCT_REF(FMyCustomStruct, MyCustomStruct)

		*/
		
       SHADER_PARAMETER_RDG_TEXTURE_UAV( RWTexture2D, RenderTargetWrite )
       SHADER_PARAMETER_RDG_TEXTURE    ( Texture2D,   RenderTargetRead  )
       SHADER_PARAMETER_RDG_TEXTURE    ( Texture2D,   Panorama          )
	   SHADER_PARAMETER_SAMPLER        ( SamplerState, TextureSampler   )

       SHADER_PARAMETER( float,     CurrentAngle       )
       SHADER_PARAMETER( float,     UpdateAngle        )
       SHADER_PARAMETER( float,     Range              )
       SHADER_PARAMETER( FIntPoint, RtResolution       )
       SHADER_PARAMETER( FIntPoint, PanoramaResolution )

		

	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		const FPermutationDomain PermutationVector(Parameters.PermutationId);
		
		return true;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		const FPermutationDomain PermutationVector(Parameters.PermutationId);

		/*
		* Here you define constants that can be used statically in the shader code.
		* Example:
		*/
		// OutEnvironment.SetDefine(TEXT("MY_CUSTOM_CONST"), TEXT("1"));

		/*
		* These defines are used in the thread count section of our shader
		*/
		OutEnvironment.SetDefine(TEXT("THREADS_X"), NUM_THREADS_DeformationCS_X);
		OutEnvironment.SetDefine(TEXT("THREADS_Y"), NUM_THREADS_DeformationCS_Y);
		OutEnvironment.SetDefine(TEXT("THREADS_Z"), NUM_THREADS_DeformationCS_Z);

		// This shader must support typed UAV load and we are testing if it is supported at runtime using RHIIsTypedUAVLoadSupported
		//OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);

		// FForwardLightingParameters::ModifyCompilationEnvironment(Parameters.Platform, OutEnvironment);
	}
private:
};

// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FDeformationCS, "/DeformationComputeShaders/DeformationCS/DeformationCS.usf", "DeformationCS", SF_Compute);

void FDeformationCSInterface::DispatchRenderThread(FRHICommandListImmediate& RHICmdList, FDeformationCSDispatchParams Params) {
	FRDGBuilder GraphBuilder(RHICmdList);

	{
		SCOPE_CYCLE_COUNTER(STAT_DeformationCS_Execute);
		DECLARE_GPU_STAT(DeformationCS);
		RDG_EVENT_SCOPE(GraphBuilder, "DeformationCS");
		RDG_GPU_STAT_SCOPE(GraphBuilder, DeformationCS);
		
		typename FDeformationCS::FPermutationDomain PermutationVector;
		
		// Add any static permutation options here
		// PermutationVector.Set<FDeformationCS::FMyPermutationName>(12345);

		TShaderMapRef<FDeformationCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel), PermutationVector);
		

		bool bIsShaderValid = ComputeShader.IsValid();

		if (bIsShaderValid) {
			FDeformationCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FDeformationCS::FParameters>();

			auto PanoramaResolution = Params.Panorama->GetSizeXY();
			FRDGTextureRef PanoramaTexture  = RegisterExternalTexture(GraphBuilder,Params.Panorama->GetRenderTargetTexture(),TEXT( "PanoramaTexture" ) );
			
			auto RtResolution = Params.RenderTarget->GetSizeXY();
			FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(RtResolution, RenderTextureFormat, FClearValueBinding::White, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV));
			FRDGTextureRef RtTextureWrite = GraphBuilder.CreateTexture(Desc, TEXT("RtWrite"));
			FRDGTextureRef RtTextureRead  = RegisterExternalTexture(GraphBuilder,Params.RenderTarget->GetRenderTargetTexture(),TEXT( "RtRead" ) );
			FRDGTextureRef TargetTexture  = RegisterExternalTexture(GraphBuilder, Params.RenderTarget->GetRenderTargetTexture(), TEXT("RtOutput"));


			{// Pass Values to shader, EPIC VERY COOL
				
				PassParameters->RenderTargetWrite  = GraphBuilder.CreateUAV(RtTextureWrite);
				PassParameters->RenderTargetRead   = RtTextureRead;
				PassParameters->Panorama           = PanoramaTexture;
				PassParameters->CurrentAngle       = Params.CurrentAngle;
				PassParameters->UpdateAngle        = Params.UpdateAngle;
				PassParameters->Range              = Params.Range;
				PassParameters->RtResolution       = RtResolution;
				PassParameters->PanoramaResolution = PanoramaResolution;

				PassParameters->TextureSampler = TStaticSamplerState<ESamplerFilter::SF_Bilinear>::GetRHI();
			}// Pass Values to shader, EPIC VERY COOL

			auto GroupCount = FComputeShaderUtils::GetGroupCount(FIntVector(Params.X, Params.Y, Params.Z), FComputeShaderUtils::kGolden2DGroupSize);

			AddCopyTexturePass(GraphBuilder, RtTextureRead, RtTextureWrite, FRHICopyTextureInfo());
			GraphBuilder.AddPass(
				RDG_EVENT_NAME("ExecuteDeformationCS"),
				PassParameters,
				ERDGPassFlags::AsyncCompute,
				[&PassParameters, ComputeShader, GroupCount](FRHIComputeCommandList& RHICmdList)
			{
				FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *PassParameters, GroupCount);
			});
			
			
			if (TargetTexture->Desc.Format == RenderTextureFormat) {
				AddCopyTexturePass(GraphBuilder, RtTextureWrite, TargetTexture, FRHICopyTextureInfo());
			} else {
				#if WITH_EDITOR
					GEngine->AddOnScreenDebugMessage((uint64)42145125184, 6.f, FColor::Red, FString(TEXT("The provided render target has an incompatible format (Please change the RT format to: RGBA8).")));
				#endif
			}
			
		} else {
			#if WITH_EDITOR
				GEngine->AddOnScreenDebugMessage((uint64)42145125184, 6.f, FColor::Red, FString(TEXT("The compute shader has a problem.")));
			#endif
		}
	}

	GraphBuilder.Execute();
}