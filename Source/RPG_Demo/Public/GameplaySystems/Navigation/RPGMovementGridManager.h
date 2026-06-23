// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplaySystems/Navigation/RPGMovementGridStructures.h"
#include "RPGMovementGridManager.generated.h"

UCLASS()
class RPG_DEMO_API ARPGMovementGridManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARPGMovementGridManager();

	UFUNCTION()
	void AddCell(FMovementGridCellProperties CellProperties);

	UFUNCTION()
	void ClearCells(bool PreserveMemoryAllocations = true);



protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;



protected:
	UPROPERTY()
	TArray<FMovementGridCellProperties> Cells;

	// X/Y is X/Y coordinate of the cell divided by cell size, Z is rounded Z coordnate of the cell 
	UPROPERTY()
	TMap<FIntVector, int32> CoordToIndex;
};
