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
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:	
	// Sets OutputActor in specified OutputNode to owner of this component
	void UpdateOutputActor(int32 OutputNodeIndex);

	// Used to trigger outputs (such as "OnTriggerEnter") in Blueprint for easier set up
	UFUNCTION(BlueprintCallable)
	void FireOutput(UPARAM(meta = (GetOptions = "GetOutputOptions")) FString OutputName, AActor* Activator = nullptr);

	// Used to trigger input (such as "Teleport) with Parameters
	void FireInput(AActor* OutputActor, FString InputName, TArray<struct FIOParameter> IOParamaters);

protected:
	// Processes output node if it's delay is zero - triggers FireInput and deleted the node from Processing Queue
	bool ProcessOutputNode(int32 index);

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
	UPROPERTY()
	TArray<AActor*> OutputNodesToProcessActivators;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "InputOutputConfiguration")
	TArray<FString> OutputList;


public:
	UFUNCTION()
	TArray<FString> GetOutputOptions() const;
	UFUNCTION()
	TArray<FString> GetOutputOptionsWithNoneOption() const;
	UFUNCTION()
	TArray<FString> GetInputOptions() const;
	// Class based input finding - works on a SCS template where GetOwner() is null. Needed for Activator TargetType
	static TArray<FString> GetInputOptionsForClass(const UClass* Class);
	UFUNCTION()
	TArray<FString> GetInputOptionsWithNoneOption() const;
	UFUNCTION()
	static TArray<FName> GetActorInputs(AActor* Actor);
#if WITH_EDITOR
	// THIS IS VERY HEAVY! It uses asset registry to find all the outputs.
	UFUNCTION()
	static TArray<FName> GetAllActorInputs();
#endif

private:
#if !UE_BUILD_SHIPPING
	FString OutputNodeToString(FOutputNode& Node) const;
#endif
};
