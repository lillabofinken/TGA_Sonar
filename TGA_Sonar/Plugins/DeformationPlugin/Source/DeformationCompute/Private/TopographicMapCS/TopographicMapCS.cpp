#include "TopographicMapCS.h"
#include "DeformationCompute/Public/TopographicMapCS/TopographicMapCS.h"
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

DECLARE_STATS_GROUP(TEXT("TopographicMapCS"), STATGROUP_TopographicMapCS, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("TopographicMapCS Execute"), STAT_TopographicMapCS_Execute, STATGROUP_TopographicMapCS);

// This class carries our parameter declarations and acts as the bridge between cpp and HLSL.
class DEFORMATIONCOMPUTE_API FTopographicMapCS: public FGlobalShader
{
public:
	
	DECLARE_GLOBAL_SHADER(FTopographicMapCS);
	SHADER_USE_PARAMETER_STRUCT(FTopographicMapCS, FGlobalShader);
	
	
	class FTopographicMapCS_Perm_TEST : SHADER_PERMUTATION_INT("TEST", 1);
	using FPermutationDomain = TShaderPermutationDomain<
		FTopographicMapCS_Perm_TEST
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

		
		SHADER_PARAMETER_RDG_TEXTURE_UAV( RWTexture2D, RenderTarget )
	    SHADER_PARAMETER_RDG_TEXTURE    ( Texture2D,   Heightmap    )
		SHADER_PARAMETER                ( FIntPoint,   MapResolution   )
		SHADER_PARAMETER                ( FIntPoint,   HeightmapResolution   )
	    SHADER_PARAMETER_SAMPLER        ( SamplerState, TextureSampler )
	    
	    SHADER_PARAMETER( float, ContourLineStep      )
	    SHADER_PARAMETER( int,   IndexLineStep        )
	    SHADER_PARAMETER( float, ContourLineThickness )
	    SHADER_PARAMETER( float, IndexLineThickness   )
		

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
		OutEnvironment.SetDefine(TEXT("THREADS_X"), NUM_THREADS_TopographicMapCS_X);
		OutEnvironment.SetDefine(TEXT("THREADS_Y"), NUM_THREADS_TopographicMapCS_Y);
		OutEnvironment.SetDefine(TEXT("THREADS_Z"), NUM_THREADS_TopographicMapCS_Z);

		// This shader must support typed UAV load and we are testing if it is supported at runtime using RHIIsTypedUAVLoadSupported
		//OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);

		// FForwardLightingParameters::ModifyCompilationEnvironment(Parameters.Platform, OutEnvironment);
	}
private:
};

// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FTopographicMapCS, "/DeformationComputeShaders/TopographicMapCS/TopographicMapCS.usf", "TopographicMapCS", SF_Compute);

void FTopographicMapCSInterface::DispatchRenderThread(FRHICommandListImmediate& RHICmdList, FTopographicMapCSDispatchParams Params) {
	FRDGBuilder GraphBuilder(RHICmdList);

	{
		SCOPE_CYCLE_COUNTER(STAT_TopographicMapCS_Execute);
		DECLARE_GPU_STAT(TopographicMapCS);
		RDG_EVENT_SCOPE(GraphBuilder, "TopographicMapCS");
		RDG_GPU_STAT_SCOPE(GraphBuilder, TopographicMapCS);
		
		typename FTopographicMapCS::FPermutationDomain PermutationVector;
		
		// Add any static permutation options here
		// PermutationVector.Set<FTopographicMapCS::FMyPermutationName>(12345);

		TShaderMapRef<FTopographicMapCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel), PermutationVector);
		

		bool bIsShaderValid = ComputeShader.IsValid();

		if (bIsShaderValid) {
			FTopographicMapCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FTopographicMapCS::FParameters>();

			FIntPoint MapResolution = Params.RenderTarget->GetSizeXY();
			FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(MapResolution, PF_B8G8R8A8, FClearValueBinding::White, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV));
			FRDGTextureRef TmpTexture = GraphBuilder.CreateTexture(Desc, TEXT("TopographicMapCS_TempTexture"));
			FRDGTextureRef TargetTexture = RegisterExternalTexture(GraphBuilder, Params.RenderTarget->GetRenderTargetTexture(), TEXT("TopographicMapCS_RT"));

			FIntPoint HeightmapResolution = FIntPoint( Params.Heightmap->GetSizeX(), Params.Heightmap->GetSizeY() );
			FRDGTextureRef Heightmap = RegisterExternalTexture(GraphBuilder,Params.Heightmap->GetResource()->GetTexture2DRHI(),TEXT( "DeformationCS_RT_Read" ) );

			
			{// PARAMS
				PassParameters->RenderTarget         = GraphBuilder.CreateUAV(TmpTexture);
				PassParameters->Heightmap            = Heightmap;
				PassParameters->MapResolution  = MapResolution;
				PassParameters->HeightmapResolution  = HeightmapResolution;
				PassParameters->TextureSampler       = TStaticSamplerState<ESamplerFilter::SF_Point>::GetRHI();

				PassParameters->ContourLineStep      = Params.ContourLineStep;
                PassParameters->IndexLineStep        = Params.IndexLineStep;
                PassParameters->ContourLineThickness = Params.ContourLineThickness;
                PassParameters->IndexLineThickness   = Params.IndexLineThickness;
			}// PARAMS
			

			auto GroupCount = FComputeShaderUtils::GetGroupCount(FIntVector(Params.X, Params.Y, Params.Z), FComputeShaderUtils::kGolden2DGroupSize);
			GraphBuilder.AddPass(
				RDG_EVENT_NAME("ExecuteTopographicMapCS"),
				PassParameters,
				ERDGPassFlags::AsyncCompute,
				[&PassParameters, ComputeShader, GroupCount](FRHIComputeCommandList& RHICmdList)
			{
				FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *PassParameters, GroupCount);
			});

			
			// The copy will fail if we don't have matching formats, let's check and make sure we do.
			if (TargetTexture->Desc.Format == PF_B8G8R8A8) {
				AddCopyTexturePass(GraphBuilder, TmpTexture, TargetTexture, FRHICopyTextureInfo());
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