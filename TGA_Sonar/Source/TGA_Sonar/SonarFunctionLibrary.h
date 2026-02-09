// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SonarFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TGA_SONAR_API USonarFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	
	UFUNCTION(BlueprintCallable, Category = "Texture")
	static void UpdateTexturePixels(UTexture2D* Texture, const TArray<FColor>& Colors, int32 Width, int32 Height);
	
};
