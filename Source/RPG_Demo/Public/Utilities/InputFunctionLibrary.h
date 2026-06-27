// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InputFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class RPG_DEMO_API UInputFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	

    UFUNCTION(BlueprintCallable, Category = "RPGUtilties|Input", meta = (WorldContext = "WorldContextObject"))
    static void SimulateKeyUse(const UObject* WorldContextObject, FKey Key, float Delta = 1, int32 PlayerIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "RPGUtilties|Input", meta = (WorldContext = "WorldContextObject"))
    static void InjectInputAction(const UObject* WorldContextObject, UInputAction* Action, float Value, int32 PlayerIndex = 0);
};
