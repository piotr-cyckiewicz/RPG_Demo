// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RPGInputOutputComponent.generated.h"




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent, DisplayName = "Input Output Component"),
	HideCategories = (Cooking, AssetUserData, Navigation))
class RPG_DEMO_API URPGInputOutputComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URPGInputOutputComponent();

protected:
	virtual void BeginPlay() override;

public:	
	// Sets OutputActor in specified OutputNode to owner of this component
	UFUNCTION()
	void UpdateOutputActor(int32 OutputNodeIndex);

	// Used to trigger outputs (such as "OnTriggerEnter") in Blueprint for easier set up
	UFUNCTION(BlueprintCallable)
	void FireOutput(UPARAM(meta = (GetOptions = "GetOutputOptions")) FString OutputName);

	// Used to trigger input (such as "Teleport) with Parameters
	void FireInput(AActor* OutputActor, FString InputName, TArray<struct FIOParameter> IOParamaters);

protected:
	// Processes output node if it's delay is zero - triggers FireInput and deleted the node from Processing Queue
	void ProcessOutputNode(int32 index);

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif


public:
	UPROPERTY(EditInstanceOnly)
	TArray<struct FOutputNode> OutputNodes;
	UPROPERTY()
	TArray<int32> OutputNodesToProcess;
	UPROPERTY()
	TArray<float> OutputNodesToProcessDelay;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "InputOutputConfiguration")
	TArray<FString> OutputList;
	UPROPERTY(EditDefaultsOnly, Category = "InputOutputConfiguration")
	TArray<struct FInputNode> InputList;


	inline static FString IOInterfacesPrefix = TEXT("BPI_IOSystem");

public:
	UFUNCTION()
	TArray<FString> GetOutputOptions() const;
	UFUNCTION()
	TArray<FString> GetOutputOptionsWithNoneOption() const;
	UFUNCTION()
	TArray<FString> GetInputOptions() const;
	UFUNCTION()
	TArray<FInputNode> GetInputNodes() const;
	UFUNCTION()
	TArray<FString> GetInputOptionsWithNoneOption() const;
	UFUNCTION()
	static TArray<FName> GetActorInputs(AActor* Actor);
#if WITH_EDITOR
	// THIS IS VERY HEAVY! It uses asset registry to find all the outputs.
	UFUNCTION()
	static TArray<FName> GetAllActorInputs();
#endif
};
