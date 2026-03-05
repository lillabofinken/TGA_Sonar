#include "PassiveSonarCS.h"
#include "DeformationCompute/Public/PassiveSonarCS/PassiveSonarCS.h"
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

DECLARE_STATS_GROUP(TEXT("PassiveSonarCS"), STATGROUP_PassiveSonarCS, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("PassiveSonarCS Execute"), STAT_PassiveSonarCS_Execute, STATGROUP_PassiveSonarCS);

#define RenderTextureFormat PF_R16F

// This class carries our parameter declarations and acts as the bridge between cpp and HLSL.
class DEFORMATIONCOMPUTE_API FPassiveSonarCS: public FGlobalShader
{
public:
	
	DECLARE_GLOBAL_SHADER(FPassiveSonarCS);
	SHADER_USE_PARAMETER_STRUCT(FPassiveSonarCS, FGlobalShader);
	
	
	class FPassiveSonarCS_Perm_TEST : SHADER_PERMUTATION_INT("TEST", 1);
	using FPermutationDomain = TShaderPermutationDomain<
		FPassiveSonarCS_Perm_TEST
	>;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		/*
		* Here's where you define one or more of the input parameters for your shader.
		* Some examples:
		*/
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

		SHADER_PARAMETER_STRUCT_ARRAY   ( NoiseEmitterDataStruct, EmitterData, [NoiseEmitterMaxAmount] )
		SHADER_PARAMETER                ( int,                    EmitterAmount     )
		SHADER_PARAMETER_RDG_TEXTURE_UAV( RWTexture2D,            RenderTargetWrite )
		SHADER_PARAMETER_RDG_TEXTURE    ( Texture2D,              RenderTargetRead  )
		SHADER_PARAMETER                ( float,                  Time              )
		SHADER_PARAMETER                ( float,                  UpdateAmount      )
		SHADER_PARAMETER                ( FIntPoint,              RtResolution      )


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
		OutEnvironment.SetDefine(TEXT("THREADS_X"), NUM_THREADS_PassiveSonarCS_X);
		OutEnvironment.SetDefine(TEXT("THREADS_Y"), NUM_THREADS_PassiveSonarCS_Y);
		OutEnvironment.SetDefine(TEXT("THREADS_Z"), NUM_THREADS_PassiveSonarCS_Z);

		// This shader must support typed UAV load and we are testing if it is supported at runtime using RHIIsTypedUAVLoadSupported
		//OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);

		// FForwardLightingParameters::ModifyCompilationEnvironment(Parameters.Platform, OutEnvironment);
	}
private:
};

// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FPassiveSonarCS, "/DeformationComputeShaders/PassiveSonarCS/PassiveSonarCS.usf", "PassiveSonarCS", SF_Compute);

void FPassiveSonarCSInterface::DispatchRenderThread(FRHICommandListImmediate& RHICmdList, FPassiveSonarCSDispatchParams Params) {
	FRDGBuilder GraphBuilder(RHICmdList);

	{
		SCOPE_CYCLE_COUNTER(STAT_PassiveSonarCS_Execute);
		DECLARE_GPU_STAT(PassiveSonarCS);
		RDG_EVENT_SCOPE(GraphBuilder, "PassiveSonarCS");
		RDG_GPU_STAT_SCOPE(GraphBuilder, PassiveSonarCS);
		
		typename FPassiveSonarCS::FPermutationDomain PermutationVector;
		
		// Add any static permutation options here
		// PermutationVector.Set<FPassiveSonarCS::FMyPermutationName>(12345);

		TShaderMapRef<FPassiveSonarCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel), PermutationVector);
		

		bool bIsShaderValid = ComputeShader.IsValid();

		if (bIsShaderValid) {
			FPassiveSonarCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FPassiveSonarCS::FParameters>();

			auto RtResolution = Params.RenderTarget->GetSizeXY();
			FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(RtResolution, RenderTextureFormat, FClearValueBinding::White, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV));
			FRDGTextureRef RtWrite = GraphBuilder.CreateTexture(Desc, TEXT("PassiveSonarCS_Write"));
			FRDGTextureRef RtRead = RegisterExternalTexture(GraphBuilder, Params.RenderTarget->GetRenderTargetTexture(), TEXT("PassiveSonarCS_Read"));
			FRDGTextureRef TargetTexture = RegisterExternalTexture(GraphBuilder, Params.RenderTarget->GetRenderTargetTexture(), TEXT("PassiveSonarCS_RT"));
			{//PARAMS
				
				for( int i = 0; i < /*Params.ObjectAmount*/ 64; i++ )
				{
					PassParameters->EmitterData[ i ] = Params.NoiseEmitters[ i ];				
				}
				PassParameters->EmitterAmount     = Params.EmitterAmount;				
				PassParameters->RenderTargetWrite = GraphBuilder.CreateUAV(RtWrite);
				PassParameters->RenderTargetWrite = GraphBuilder.CreateUAV(RtWrite);
				PassParameters->RenderTargetRead  = RtRead;
				PassParameters->Time              = Params.time;
				PassParameters->UpdateAmount      = Params.UpdateAmount;
				PassParameters->RtResolution      = RtResolution;
			}//PARAMS
			

			auto GroupCount = FComputeShaderUtils::GetGroupCount(FIntVector(Params.X, Params.Y, Params.Z), FComputeShaderUtils::kGolden2DGroupSize);
			GraphBuilder.AddPass(
				RDG_EVENT_NAME("ExecutePassiveSonarCS"),
				PassParameters,
				ERDGPassFlags::AsyncCompute,
				[&PassParameters, ComputeShader, GroupCount](FRHIComputeCommandList& RHICmdList)
			{
				FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *PassParameters, GroupCount);
			});

			
			// The copy will fail if we don't have matching formats, let's check and make sure we do.
			if (TargetTexture->Desc.Format == RenderTextureFormat) {
				AddCopyTexturePass(GraphBuilder, RtWrite, TargetTexture, FRHICopyTextureInfo());
			} else {
				#if WITH_EDITOR
					GEngine->AddOnScreenDebugMessage((uint64)42145125184, 6.f, FColor::Red, FString(TEXT("The provided render target has an incompatible format (Please change the RT format to: RGBA8).")));
				#endif
			}
			
		} else {
			#if WITH_EDITOR
				GEngine->AddOnScreenDebugMessage((uint64)42145125184, 6.f, FColor::Red, FString(TEXT("The compute shader has a problem.")));
			#endif

			// We exit here as we don't want to crash the game if the shader is not found or has an error.
			
		}
	}

	GraphBuilder.Execute();
}