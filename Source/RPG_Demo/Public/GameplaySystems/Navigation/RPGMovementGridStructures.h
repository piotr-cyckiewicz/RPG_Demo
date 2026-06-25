// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplaySystems/Combat/RPGCombatStructures.h"
#include "RPGMovementGridStructures.generated.h"

USTRUCT(BlueprintType)
struct RPG_DEMO_API FMovementGridCellProperties
{
    GENERATED_BODY()

    // We need world position of this cell, as cell actor is editor only actor, and we cannot get location from it in cooked builds.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector WorldPosition;

    // Coordinates normalized to the grid
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FIntVector Coordinates;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    int32 BaseMovementCost = 1;

    UPROPERTY()
    TMap<ECombatTeams, int32> TeamDependentMovementCost;
    
    // This assumes that the whole struct is used in an array, and those are indexes for neighbours in that array
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<int32> Neighbors;

    // Note that this info should only be accessed in the editor, as cell actors are editor only
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    TObjectPtr<AActor> CellActor;

    // TODO: Add pointer to character occupying this cell, once it's in the project



    // This is temporal array used to fill proper Neighbours array and will be empty after the process is done
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    TArray<TObjectPtr<AActor>> temporaryNeighbours;

    
    
    FMovementGridCellProperties() {
        this->BaseMovementCost = 1.0f;
        this->TeamDependentMovementCost = TMap<ECombatTeams, int32>();
        this->TeamDependentMovementCost.Add(ECombatTeams::PlayerTeam, 1);
        this->TeamDependentMovementCost.Add(ECombatTeams::Ally1, 1);
    }

    FMovementGridCellProperties(AActor* CellActor, TArray<TObjectPtr<AActor>> temporaryNeighboursm, float BaseMovementCost = 1.f, TMap<ECombatTeams, int32> TeamDependentMovementCost = TMap<ECombatTeams, int32>()) {
        this->CellActor = CellActor;
        this->temporaryNeighbours = temporaryNeighboursm;
        this->BaseMovementCost = BaseMovementCost;
        this->TeamDependentMovementCost = TeamDependentMovementCost;
        this->TeamDependentMovementCost.Add(ECombatTeams::PlayerTeam, 1);
        this->TeamDependentMovementCost.Add(ECombatTeams::Ally1, 1);
    }
};