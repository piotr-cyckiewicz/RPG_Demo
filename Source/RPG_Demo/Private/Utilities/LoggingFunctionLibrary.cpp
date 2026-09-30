// Fill out your copyright notice in the Description page of Project Settings.


#include "Utilities/LoggingFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

void LoggingFunctionLibrary::PrintError(const UObject* WorldContextObject, const FString& InString)
{
	UKismetSystemLibrary::PrintString(WorldContextObject, InString, true, false, FLinearColor::Red, 5.0f);
	UE_LOG(LogTemp, Error, TEXT("%s"), *InString);
}
