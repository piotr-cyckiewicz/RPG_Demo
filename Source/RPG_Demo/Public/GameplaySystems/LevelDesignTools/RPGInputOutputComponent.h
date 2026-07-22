// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RPGInputOutputComponent.generated.h"




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent, DisplayName = "Input Output Component") )
class RPG_DEMO_API URPGInputOutputComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URPGInputOutputComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	

	UFUNCTION()
	static TArray<FName> GetActorInputs(AActor* Actor);
#if WITH_EDITOR
	UFUNCTION()
	static TArray<FName> GetAllActorInputs();
#endif

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif


public:
	UPROPERTY(EditInstanceOnly)
	TArray<struct FOutputNode> OutputNodes;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "InputOutputConfiguration")
	TArray<FString> OutputList;

	inline static FString IOInterfacesPrefix = TEXT("BPI_IOSystem");

	UFUNCTION()
	TArray<FString> GetOutputOptions() const;
	UFUNCTION()
	TArray<FString> GetInputOptions() const;
};
