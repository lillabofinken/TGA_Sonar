// Fill out your copyright notice in the Description page of Project Settings.


#include "PassiveSonarManager.h"

// Sets default values for this component's properties
UPassiveSonarManager::UPassiveSonarManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

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

	// ...
}

void UPassiveSonarManager::AddTrackedObjects(TArray<USceneComponent*> _trackedComponent)
{
}

void UPassiveSonarManager::AddTrackedObject(USceneComponent* _trackedComponent)
{
}

UPassiveSonarManager* UPassiveSonarManager::GetDeformationManager()
{
	return DeformationManager;
}

