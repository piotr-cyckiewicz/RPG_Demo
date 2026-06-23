// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplaySystems/Navigation/RPGMovementGridManager.h"

// Sets default values
ARPGMovementGridManager::ARPGMovementGridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ARPGMovementGridManager::AddCell(FMovementGridCellProperties CellProperties)
{
	Cells.Add(CellProperties);
	
	FIntVector Coords;
	Coords.Z = FMath::RoundToInt(CellProperties.WorldPosition.Z);
	//Coords.X = FMath::RoundToInt(CellProperties.WorldPosition/)
}

void ARPGMovementGridManager::ClearCells(bool PreserveMemoryAllocations)
{
	// Reseting the array with size 0 is probably unoptimized as hell due to rellocating the whole array later as we add elements to it,
	// but it runs in the editor, so it's probably fine.
	if (PreserveMemoryAllocations) {
		Cells.Reset(0);
		CoordToIndex.Reset();
	}
	else {
		Cells.Empty();
		CoordToIndex.Empty();
	}
}

// Called when the game starts or when spawned
//void ARPGMovementGridManager::BeginPlay()
//{
	//Super::BeginPlay();	
//}

