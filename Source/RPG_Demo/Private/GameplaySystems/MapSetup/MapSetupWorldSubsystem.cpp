// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplaySystems/MapSetup/MapSetupWorldSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"

void UMapSetupWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	auto* GameMode = UGameplayStatics::GetGameMode(this);
	if (auto* Func = GameMode->FindFunction(FName(TEXT("PreBeginPlay")))) {
		GameMode->ProcessEvent(Func, nullptr);
	}
}
