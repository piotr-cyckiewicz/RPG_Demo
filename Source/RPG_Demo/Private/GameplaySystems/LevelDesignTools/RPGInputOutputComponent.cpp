// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplaySystems/LevelDesignTools/RPGInputOutputComponent.h"
#include "GameplaySystems/LevelDesignTools/RPGInputOutputStructures.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#endif

// Sets default values for this component's properties
URPGInputOutputComponent::URPGInputOutputComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void URPGInputOutputComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

TArray<FName> URPGInputOutputComponent::GetActorInputs(AActor* Actor)
{
	TArray<FName> Inputs;

	for (const FImplementedInterface& Interface : Actor->GetClass()->Interfaces) {
		if (!Interface.bImplementedByK2) {
			continue;
		}
		
		FString Name = Interface.Class.GetName();
		if (!Name.StartsWith(IOInterfacesPrefix, ESearchCase::CaseSensitive)) {
			continue;
		}

		for (TFieldIterator<UFunction> It(Interface.Class); It; ++It)
		{
			UFunction* Function = *It;
			Inputs.Add(*Function->GetName());
		}
	}
	TArray<FName> Inputs;
}

#if WITH_EDITOR
TArray<FName> URPGInputOutputComponent::GetAllActorInputs()
{
	TArray<FName> Inputs;

	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;

	TArray<FAssetData> Blueprints;
	AssetRegistry.GetAssets(Filter, Blueprints);

	for (const FAssetData& Asset : Blueprints)
	{
		FString BlueprintType;
		if (Asset.GetTagValue(FBlueprintTags::BlueprintType, BlueprintType)
			&& BlueprintType == TEXT("BPTYPE_Interface") && Asset.AssetName.ToString() == IOInterfacesPrefix)
		{
			FString ClassPath;
			Asset.GetTagValue(FBlueprintTags::GeneratedClassPath, ClassPath);
			FString ObjectPath = FPackageName::ExportTextPathToObjectPath(ClassPath);
			UClass* InterfaceClass = LoadObject<UClass>(nullptr, *ObjectPath);
			for (TFieldIterator<UFunction> It(InterfaceClass); It; ++It)
			{
				UFunction* Function = *It;
				Inputs.Add(*Function->GetName());
			}
		}
	}

	//TODO: Get all function names from 
}
#endif

#if WITH_EDITOR
void URPGInputOutputComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.PropertyChain.GetActiveMemberNode() == nullptr
		|| PropertyChangedEvent.PropertyChain.GetActiveMemberNode()->GetValue() == nullptr)
		return;
	if (PropertyChangedEvent.Property == nullptr)
		return;

	FName MemberName = PropertyChangedEvent.PropertyChain.GetActiveMemberNode()->GetValue()->GetFName();
	FName PropertyName = PropertyChangedEvent.Property->GetFName();

	if (MemberName != GET_MEMBER_NAME_CHECKED(URPGInputOutputComponent, OutputNodes)
		|| PropertyName != GET_MEMBER_NAME_CHECKED(FOutputNode, TargetInput))
		return;

	int32 Index = PropertyChangedEvent.GetArrayIndex(TEXT("OutputNodes"));

	if (!OutputNodes.IsValidIndex(Index)) {
		UE_LOG(LogTemp, Error, TEXT("URPGInputOutputComponent::PostEditChangeChainProperty - Improper index detected"));
		return;
	}

	// We have to make sure that vaid output always has proper OutputActor set
	OutputNodes[Index].OutputActor = GetOwner();

	if (OutputNodes[Index].TargetType == EIOTargetType::Actor && !IsValid(OutputNodes[Index].Target)) {
		OutputNodes[Index].InputParameters.Reset();
		return;
	}

	const FString& TargetInput = OutputNodes[Index].TargetInput;
	if (OutputNodes[Index].TargetType != EIOTargetType::Activator && OutputNodes[Index].Target == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("URPGInputOutputComponent::PostEditChangeChainProperty - No Target set despite changing TargetInput"));
		return;
	}

	UFunction* Function = OutputNodes[Index].Target->FindFunction(FName(*TargetInput));
	if (!Function) {
		UE_LOG(LogTemp, Error, TEXT("URPGInputOutputComponent::PostEditChangeChainProperty - No function matching TargetInput was found"));
		return;
	}
	
	OutputNodes[Index].InputParameters.Reset();

	for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		FProperty* Param = *It;

		if (Param->HasAnyPropertyFlags(CPF_ReturnParm) || Param->HasAnyPropertyFlags(CPF_OutParm))
			continue;

		OutputNodes[Index].InputParameters.Add(FIOParameter(Param));
	
	}
}
#endif


TArray<FString> URPGInputOutputComponent::GetOutputOptions() const
{
	return OutputList;
}

TArray<FString> URPGInputOutputComponent::GetInputOptions() const
{
	return TArray<FString>();
}
