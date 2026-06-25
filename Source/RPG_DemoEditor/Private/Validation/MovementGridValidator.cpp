// Fill out your copyright notice in the Description page of Project Settings.


#include "Validation/MovementGridValidator.h"


bool UMovementGridValidator::CanValidateAsset_Implementation(
    const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
    return InAssetData.AssetClassPath == UWorld::StaticClass()->GetClassPathName();
}

EDataValidationResult UMovementGridValidator::ValidateLoadedAsset_Implementation(
    const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
    UWorld* World = Cast<UWorld>(InAsset);
    if (!World)
    {
        AssetPasses(InAsset);
        return EDataValidationResult::NotValidated;
    }
    if (!TargetGameModeClass) {
        AssetFails(InAsset, FText::FromString("Properties not set in MovementGridValidator"));
        return EDataValidationResult::Invalid;
    }

    const AWorldSettings* Settings = World->GetWorldSettings();
    const TSubclassOf<AGameModeBase> GM = Settings ? Settings->DefaultGameMode : nullptr;

    if (GM != TargetGameModeClass && GM) //We assume that maps without GM override are explorable maps, as default gamemode is the exploration one
    {
        AssetPasses(InAsset);
        return EDataValidationResult::Valid;
    }

    ARPGMovementGridManager* Manager = nullptr;
    int32 Count = 0;
    for (TActorIterator<ARPGMovementGridManager> It(World); It; ++It)
    {
        Manager = *It;
        ++Count;
    }

    if (Count != 1) {
        AssetFails(InAsset, FText::Format(NSLOCTEXT("Validation", "WrongActorCount",
            "Map has {0} actors of type MovementGridManager, expected 1."),
            FText::AsNumber(Count)));
        return EDataValidationResult::Invalid;
    }

    if (Manager->GetNumberOfCells() < 10) {
        AssetFails(InAsset, FText::Format(NSLOCTEXT("Validation", "WrongCellCount",
            "Map has only {0} Movement Grid Cells. That's not very playable, is it? ;) At least 10 is required"),
            FText::AsNumber(Manager->GetNumberOfCells())));
        return EDataValidationResult::Invalid;
    }

    AssetPasses(InAsset);
    return EDataValidationResult::Valid;

    
}
