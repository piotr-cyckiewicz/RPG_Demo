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

	// Used to trigger outputs (such as "OnTriggerEnter") in Blueprint for easier set up
	UFUNCTION(BlueprintCallable)
	void FireOutput(UPARAM(meta = (GetOptions = "GetOutputOptions")) FString OutputName, AActor* Activator = nullptr);

	// Removes every output node that is waiting in this component's queue at the moment of the call
	// Nodes added after the call are processed normally.
	// Available to other IO components as built-in input "CancelPending" (no IO_ event needed on the owner).
	UFUNCTION(BlueprintCallable)
	void CancelPendingOutputs();

	// Used to trigger input (such as "Teleport") with Parameters
	void FireInput(AActor* OutputActor, AActor* Activator, FString InputName, TArray<struct FIOParameter> IOParamaters);

protected:
	// Processes Output node if it's delay is zero - triggers FireInput and deletes the node from Processing Queue
	bool ProcessOutputNode(int32 index);

#if WITH_EDITOR
	// Handles updates to variables of modified FOutputNode
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif


public:
	// Configurable Output Nodes describing what and in what way is triggered after certain output (such as OnTriggerEnter) is triggered
	UPROPERTY(EditInstanceOnly)
	TArray<struct FOutputNode> OutputNodes;

	// Arrays containing information about otput nodes that are currently in the queue
	UPROPERTY()
	TArray<int32> OutputNodesToProcess;
	UPROPERTY()
	TArray<float> OutputNodesToProcessDelay;
	UPROPERTY()
	TArray<AActor*> OutputNodesToProcessActivators;
protected:
	// Contains list of all possible outputs (for example TriggerBox may fire outputs such as OnTriggerEnter, or OnTriggerExit)
	UPROPERTY(EditDefaultsOnly, Category = "InputOutputConfiguration")
	TArray<FString> OutputList;

	// Incremented by every CancelPendingOutputs. Helps with recognizing if queue was cleared in queue loops
	int32 PendingGeneration = 0;

	// Name of the built-in input handled by CancelPendingOutputs.
	static constexpr const TCHAR* CancelPendingInputName = TEXT("CancelPending");


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
	static TMap<FName, TArray<struct FIOParameter>> GetAllActorInputs();
#endif

private:
#if !UE_BUILD_SHIPPING
	FString OutputNodeToString(FOutputNode& Node) const;
#endif
};
