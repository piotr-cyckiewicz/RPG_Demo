// Fill out your copyright notice in the Description page of Project Settings.


#include "Validation/PlayerStartValidator.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/GameModeBase.h"

bool UPlayerStartValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
	return InAssetData.AssetClassPath == UWorld::StaticClass()->GetClassPathName();
}

EDataValidationResult UPlayerStartValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
    UWorld* World = Cast<UWorld>(InAsset);
    if (!World)
    {
        AssetPasses(InAsset);
        return EDataValidationResult::NotValidated;
    }
    if (!TargetGameModeClass) {
        AssetFails(InAsset, FText::FromString("Properties not set in PlayerStartValidator"));
        return EDataValidationResult::Invalid;
    }

    const AWorldSettings* Settings = World->GetWorldSettings();
    const TSubclassOf<AGameModeBase> GM = Settings ? Settings->DefaultGameMode : nullptr;

    if (GM != TargetGameModeClass && GM) //We assume that maps without GM override are explorable maps, as default gamemode is the exploration one
    {
        AssetPasses(InAsset);
        return EDataValidationResult::Valid;
    }

    int32 Count = 0;
    for (TActorIterator<APlayerStart> It(World); It; ++It)
    {
        ++Count;
    }

    if (Count != 1) {
        AssetFails(InAsset, FText::Format(NSLOCTEXT("Validation", "WrongActorCount",
            "Map has {0} actors of type PlayerStart, expected 1."),
            FText::AsNumber(Count)));
        return EDataValidationResult::Invalid;
    }

    AssetPasses(InAsset);
    return EDataValidationResult::Valid;


}
