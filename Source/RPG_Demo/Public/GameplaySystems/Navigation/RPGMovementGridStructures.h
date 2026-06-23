// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplaySystems/Combat/RPGCombatStructures.h"
#include "RPGMovementGridStructures.generated.h"

USTRUCT(BlueprintType)
struct RPG_DEMO_API FMovementGridCellProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WorldPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseMovementCost = 1.f;

    UPROPERTY()
    TMap<ECombatTeams, int32> TeamDependentMovementCost;

    // This assumes that the whole struct is used in an array, and those are indexes for neighbours in that array
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> Neighbors;

    
    
    FMovementGridCellProperties() {
        this->WorldPosition = FVector::Zero();
        this->BaseMovementCost = 1.0f;
        this->TeamDependentMovementCost = TMap<ECombatTeams, int32>();
    }

    FMovementGridCellProperties(FVector WorldPosition, float BaseMovementCost = 1.f, TMap<ECombatTeams, int32> TeamDependentMovementCost = TMap<ECombatTeams, int32>()) {
        this->WorldPosition = WorldPosition;
        this->BaseMovementCost = BaseMovementCost;
        this->TeamDependentMovementCost = TeamDependentMovementCost;
    }
};