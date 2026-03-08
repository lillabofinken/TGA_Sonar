// Fill out your copyright notice in the Description page of Project Settings.


#include "PassiveSonarManager.h"
#include "PassiveSonarCS/PassiveSonarCS.h"

// Sets default values for this component's properties
UPassiveSonarManager::UPassiveSonarManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	DeformationManager = this;
	// ...
}


// Called when the game starts
void UPassiveSonarManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPassiveSonarManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	updatePassiveSonar(DeltaTime);
	
	// ...
}

void UPassiveSonarManager::AddTrackedObjects(TArray<USceneComponent*> _trackedComponents)
{
	for( auto comp : _trackedComponents )
	{
		TrackedObjects.Add( comp );
	}
}

void UPassiveSonarManager::AddTrackedObject(USceneComponent* _trackedComponent)
{
	TrackedObjects.Add( _trackedComponent );
}

void UPassiveSonarManager::RemoveTrackedObjects( TArray<USceneComponent*> _trackedComponents )
{
	for( auto comp : _trackedComponents )
	{
		TrackedObjects.Remove( comp );
	}
}

void UPassiveSonarManager::RemoveTrackedObject( USceneComponent* _trackedComponent )
{
	TrackedObjects.Remove( _trackedComponent );
}

void UPassiveSonarManager::updatePassiveSonar(float _deltaTime)
{
	if ( !RenderTarget )
		return;

	framerateTime += _deltaTime;
	const float frameStep = 1.0f / Framerate;
	const float SonarStepSize = frameStep / WaterfallSeconds;
	
	while ( framerateTime >= frameStep )
	{
		FPassiveSonarCSDispatchParams params(RenderTarget->SizeX, RenderTarget->SizeY,1);
		params.RenderTarget = RenderTarget->GameThread_GetRenderTargetResource();
		params.time = GetWorld()->GetTimeSeconds() + framerateTime;
		params.UpdateAmount = SonarStepSize;
		UDeformationCSLibrary::ExecutePassiveSonarComputeShader(params);

		framerateTime -= frameStep;
	}
}

UPassiveSonarManager* UPassiveSonarManager::GetDeformationManager()
{
	return DeformationManager;
}

