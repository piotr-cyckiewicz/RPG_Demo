// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplaySystems/Navigation/RPGMovementGridManager.h"
#include "Algo/Reverse.h"

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
	
	CellProperties.WorldPosition = CellProperties.CellActor->GetActorLocation();

	FIntVector Coords = GetLogicalCoordinates(CellProperties.WorldPosition);
	CellProperties.Coordinates = Coords;
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
	// This is very slow, but it's fine for editor work. Binds 10k cellls without issues in couple of seconds.
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

void ARPGMovementGridManager::BeginPlay()
{
	Super::BeginPlay();

	if (GCost.Num() != Cells.Num())
		GCost.SetNum(Cells.Num());
	if (CameFrom.Num() != Cells.Num())
		CameFrom.SetNum(Cells.Num());
	if (SearchStamp.Num() != Cells.Num())
		SearchStamp.SetNum(Cells.Num());
}

FIntVector ARPGMovementGridManager::GetLogicalCoordinates(FVector WorldPosition)
{
	FIntVector Result;
	Result.Z = FMath::RoundToInt(WorldPosition.Z / CellHeight);
	Result.X = FMath::RoundToInt(WorldPosition.X / CellSize);
	Result.Y = FMath::RoundToInt(WorldPosition.Y / CellSize);
	return Result;
}

FVector ARPGMovementGridManager::GetWorldPosition(int32 CellID)
{
	return Cells[CellID].WorldPosition;
}

int32 ARPGMovementGridManager::GetCellIndex(FVector WorldPosition)
{
	int32* Result = CoordinatesToIndex.Find(GetLogicalCoordinates(WorldPosition));
	
	if (!Result)
		return -1;
	return *Result;
}

int32 ARPGMovementGridManager::GetMoveCost(int32 Cell, ECombatTeams CharacterTeam)
{
	if (int32* Cost = Cells[Cell].TeamDependentMovementCost.Find(CharacterTeam))
		return *Cost;
	return Cells[Cell].BaseMovementCost;
}

TArray<int32> ARPGMovementGridManager::GetPath(FVector Start, FVector End, ECombatTeams Team, float MaxCost)
{
	TArray<int32> Result;
	int32 StartIndex = GetCellIndex(Start);
	int32 EndIndex = GetCellIndex(End);

	if (StartIndex == -1 || EndIndex == -1)
		return Result;
	
	if (StartIndex == EndIndex)
		return Result;

	if(GetRawDistanceBetweenCells(StartIndex, EndIndex) / CellSize > MaxCost)
		return Result;

	if(MaxCost > 300)
		UE_LOG(LogTemp, Warning, TEXT("Too big cost of a path may result in inproper cost calculation"));


	CurrentSearch++;
	GCost[StartIndex] = 0;

	auto Predicate = [](const TPair<int32, float>& A, const TPair<int32, float>& B)
	{
		return A.Value < B.Value; //lambda for comparing which cell is more promising
	};
	TArray<TPair<int32, float>> SearchHeap; // Heap of cells and its FCosts
	SearchHeap.Add(TPair<int32, float>(StartIndex,
		GetRawDistanceBetweenCells(StartIndex, EndIndex) / CellSize)); //Start cell is the first to investigate

	


	bool PathFound = false;

	while (SearchHeap.Num() > 0) {
		TPair<int32, float> Node;
		SearchHeap.HeapPop(Node, Predicate); //We choose cell with lowest FCost and delete it from the heap (won't need it later)
		if (Node.Key == EndIndex) { PathFound = true; break; } //if this cell is our end cell, we stop the search
		if (Node.Value > MaxCost) break;
		if (Node.Value > GCost[Node.Key] +
			GetRawDistanceBetweenCells(Node.Key, EndIndex) / CellSize) continue; //node with oudated cost, we ignore it as has improper data

		for (int32 NeighbourID : Cells[Node.Key].Neighbors) {
			float NewG = GCost[Node.Key] + GetMoveCost(Node.Key, Team); //"Move from here" cost rather than "Move here" cost
			if (Cells[Node.Key].Coordinates.X != Cells[NeighbourID].Coordinates.X &&
				Cells[Node.Key].Coordinates.Y != Cells[NeighbourID].Coordinates.Y) {
				NewG += GetMoveCost(Node.Key, Team) - 0.001f; //Moving diagonally costs 2x more, but we need to favor one diagonal move over two simple ones
			}
			if (SearchStamp[NeighbourID] != CurrentSearch || NewG < GCost[NeighbourID]) { // We ignore GCost of the cell, if it wasn't touched this search yet
				SearchStamp[NeighbourID] = CurrentSearch;
				GCost[NeighbourID] = NewG;
				CameFrom[NeighbourID] = Node.Key;
				SearchHeap.HeapPush(TPair<int32, float>(NeighbourID,
					NewG + GetRawDistanceBetweenCells(NeighbourID, EndIndex) / CellSize), Predicate);
			}
		}
	}
	




	if(!PathFound) return Result;

	Result.Add(EndIndex);
	
	int32 CellPath = EndIndex;
	while (CameFrom[CellPath] != StartIndex) {
		Result.Add(CameFrom[CellPath]);
		CellPath = CameFrom[CellPath];
	}
	Result.Add(StartIndex);

	Algo::Reverse(Result);
	return Result;
}

float ARPGMovementGridManager::GetRawDistanceBetweenCells(int32 StartCell, int32 EndCell)
{
	FVector Difference = Cells[StartCell].WorldPosition - Cells[EndCell].WorldPosition;
	return Difference.Length();
}

int32 ARPGMovementGridManager::GetNumberOfCells()
{
	return Cells.Num();
}
