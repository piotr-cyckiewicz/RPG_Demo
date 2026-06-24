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
	if (!IsValid(CellProperties.CellActor)) {
		UE_LOG(LogTemp, Error, TEXT("CellActor is not valid"));
		return;
	}
	

	FIntVector Coords;
	Coords.Z = FMath::RoundToInt(CellProperties.CellActor->GetActorLocation().Z); // this is intentional, may be changed to Z/CellHeight later
	Coords.X = FMath::RoundToInt(CellProperties.CellActor->GetActorLocation().X / CellSize);
	Coords.Y = FMath::RoundToInt(CellProperties.CellActor->GetActorLocation().Y / CellSize);
	
	if (CoordinatesToIndex.Contains(Coords)) {
		UE_LOG(LogTemp, Error, TEXT("Duplicate cell at coords %s"), *Coords.ToString());
	}
	CoordinatesToIndex.Add(Coords, Cells.Num());

	for (AActor* CellActor : CellProperties.temporaryNeighbours) {
		temporaryNeighbours.Add(FIntActorPair(Cells.Num(), CellActor));
	}
	CellProperties.temporaryNeighbours.Empty();


	Cells.Add(CellProperties);
}



void ARPGMovementGridManager::BindCells()
{
	// This is very slow, but it's fine for editor work. Binds 10k cellls without issue in dozen of seconds.
	for (const FIntActorPair& Connection : temporaryNeighbours) {
		if (Connection.Num >= Cells.Num()) {
			UE_LOG(LogTemp, Error, TEXT("Index in tempoaryNeighbours is greater/equal to size of Cells array"));
			continue;
		}
		if (!IsValid(Connection.Actor)) {
			UE_LOG(LogTemp, Error, TEXT("Actor referenced in tempoaryNeighbours is not valid"));
			continue;
		}

		int32 CellID = -1;
		for (int32 i = 0; i < Cells.Num(); i++) {
			if (Cells[i].CellActor == Connection.Actor) {
				CellID = i;
				break;
			}
		}

		if (CellID == -1) {
			UE_LOG(LogTemp, Error, TEXT("Actor listed in temporaryNeighbours array not found"));
			continue;
		}
		if (Connection.Num == CellID) {
			UE_LOG(LogTemp, Error, TEXT("Neighbour of the cell in temporaryNeighbours refers to the cell itself"));
			continue;
		}

		Cells[Connection.Num].Neighbors.Add(CellID);
	}
	temporaryNeighbours.Empty();
}

void ARPGMovementGridManager::ClearCells()
{
	Cells.Empty();
	CoordinatesToIndex.Empty();
	temporaryNeighbours.Empty();
}

int32 ARPGMovementGridManager::GetNumberOfCells()
{
	return Cells.Num();
}
