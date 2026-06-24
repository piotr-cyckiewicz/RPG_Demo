// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplaySystems/Navigation/RPGMovementGridStructures.h"
#include "RPGMovementGridManager.generated.h"

USTRUCT(BlueprintType)
struct FIntActorPair
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Num = 0;

	UPROPERTY()
	TObjectPtr<AActor> Actor;

	FIntActorPair() {
		Num = 0;
	}

	FIntActorPair(int32 Num, AActor* Actor) {
		this->Num = Num;
		this->Actor = Actor;
	}
};

UCLASS()
class RPG_DEMO_API ARPGMovementGridManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARPGMovementGridManager();

	UFUNCTION(BlueprintCallable)
	void AddCell(FMovementGridCellProperties CellProperties);

	// Should be called after adding all cells from the level. Goes through all cells and fills in neighbour information.
	UFUNCTION(BlueprintCallable)
	void BindCells();

	UFUNCTION(BlueprintCallable)
	void ClearCells();

	UFUNCTION()
	int32 GetNumberOfCells();


protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Grid", meta = (UIMin = "1.0", UIMax = "300.0"))
	float CellSize =  75;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Grid", meta = (UIMin = "1.0", UIMax = "2000.0"))
	float CellHeight = 200;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Grid", meta = (UIMin = "1.0", UIMax = "200.0"))
	float CellCapsuleRadius = 30;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Grid", meta = (UIMin = "1.0", UIMax = "300.0"))
	float CellMaxStep = 30;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Grid")
	TObjectPtr<UPhysicalMaterial> WalkablePhysMaterial;



	UPROPERTY(VisibleAnywhere)
	TArray<FMovementGridCellProperties> Cells;

	// X/Y is X/Y coordinate of the cell divided by cell size, Z is rounded Z coordnate of the cell 
	UPROPERTY(VisibleAnywhere)
	TMap<FIntVector, int32> CoordinatesToIndex;

	// Temporary value to store cells neighbouring a cell with this ID in Cells array. Will probably be empty during runtime.
	UPROPERTY(VisibleAnywhere)
	TArray<FIntActorPair> temporaryNeighbours;
};
