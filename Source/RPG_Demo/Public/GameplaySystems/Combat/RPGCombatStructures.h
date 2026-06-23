// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RPGCombatStructures.generated.h"

UENUM(BlueprintType)
enum class ECombatTeams : uint8
{
    // List of possible combat teams.
    // -- Enemy1, Enemy2, Enemy3 and Enemy4 fight with each other.
    // -- AI prioritizes attacking Enemy4 more than other teams, including Player's Party

    PlayerTeam      UMETA(DisplayName = "Player Team"),
    Ally1           UMETA(DisplayName = "Ally 1"),
    Ally2           UMETA(DisplayName = "Ally 2"),
    Ally3           UMETA(DisplayName = "Ally 3"),
    Enemy1          UMETA(DisplayName = "Enemy 1"),
    Enemy2          UMETA(DisplayName = "Enemy 2"),
    Enemy3          UMETA(DisplayName = "Enemy 3"),
    Enemy4          UMETA(DisplayName = "Enemy 4")
};