// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IOParamLibrary.generated.h"

struct FIOParameter;


UCLASS()
class RPG_DEMO_API UIOParamLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure) static bool	GetBoolParam(const TArray<FIOParameter>& P, int32 Index);
	UFUNCTION(BlueprintPure) static int32   GetIntParam(const TArray<FIOParameter>& P, int32 Index);
	UFUNCTION(BlueprintPure) static double  GetFloatParam(const TArray<FIOParameter>& P, int32 Index);
	UFUNCTION(BlueprintPure) static FString GetStringParam(const TArray<FIOParameter>& P, int32 Index);
	UFUNCTION(BlueprintPure) static FVector GetVectorParam(const TArray<FIOParameter>& P, int32 Index);
	UFUNCTION(BlueprintPure) static AActor* GetActorParam(const TArray<FIOParameter>& P, int32 Index);

};
