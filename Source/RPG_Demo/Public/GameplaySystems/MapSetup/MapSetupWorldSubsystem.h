// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MapSetupWorldSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class RPG_DEMO_API UMapSetupWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;


};
