// Fill out your copyright notice in the Description page of Project Settings.


#include "SonarFunctionLibrary.h"

void USonarFunctionLibrary::UpdateTexturePixels(UTexture2D* Texture, const TArray<FColor>& Colors, int32 Width,
	int32 Height)
{
	if (!Texture)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateTexturePixels: Texture is null."));
		return;
	}
	 
	if (Colors.Num() != Width * Height)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateTexturePixels: Color array size mismatch."));
		return;
	}
	 
	if (Texture->GetSizeX() != Width || Texture->GetSizeY() != Height)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateTexturePixels: Texture dimensions mismatch."));
		return;
	}
	 
	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Data, Colors.GetData(), Colors.Num() * sizeof(FColor));
	Mip.BulkData.Unlock();
	 
	Texture->UpdateResource();
}
