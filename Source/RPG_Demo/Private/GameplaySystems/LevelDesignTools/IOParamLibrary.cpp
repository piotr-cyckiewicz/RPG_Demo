// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplaySystems/LevelDesignTools/IOParamLibrary.h"
#include "GameplaySystems/LevelDesignTools/RPGInputOutputStructures.h"

bool UIOParamLibrary::GetBoolParam(const TArray<FIOParameter>& P, int32 Index)
{
	return P[Index].BoolValue;
}

int32 UIOParamLibrary::GetIntParam(const TArray<FIOParameter>& P, int32 Index)
{
	return P[Index].IntValue;
}

double UIOParamLibrary::GetFloatParam(const TArray<FIOParameter>& P, int32 Index)
{
	return P[Index].FloatValue;
}

FString UIOParamLibrary::GetStringParam(const TArray<FIOParameter>& P, int32 Index)
{
	return P[Index].StringValue;
}

FVector UIOParamLibrary::GetVectorParam(const TArray<FIOParameter>& P, int32 Index)
{
	return P[Index].VectorValue;
}

AActor* UIOParamLibrary::GetActorParam(const TArray<FIOParameter>& P, int32 Index)
{
	return P[Index].ActorValue;
}
