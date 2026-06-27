// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "MovementGridValidator.generated.h"

/**
 * 
 */

UCLASS(Abstract)
class RPG_DEMOEDITOR_API UMovementGridValidator : public UEditorValidatorBase
{
	GENERATED_BODY()
	
	virtual bool CanValidateAsset_Implementation(
		const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const override;

	virtual EDataValidationResult ValidateLoadedAsset_Implementation(
		const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;


protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AGameModeBase> TargetGameModeClass;
};
