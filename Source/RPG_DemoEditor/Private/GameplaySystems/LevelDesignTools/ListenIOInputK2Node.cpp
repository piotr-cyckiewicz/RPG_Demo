// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplaySystems/LevelDesignTools/ListenIOInputK2Node.h"
#include "Editor/BlueprintGraph/Public/BlueprintActionDatabaseRegistrar.h"
#include "Editor/BlueprintGraph/Public/BlueprintNodeSpawner.h"
#include "Runtime/Engine/Classes/Engine/SCS_Node.h"
#include "GameplaySystems/LevelDesignTools/RPGInputOutputComponent.h"
#include "GameplaySystems/LevelDesignTools/RPGInputOutputStructures.h"

void UListenIOInputK2Node::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	if (const FInputNode* Row = FindInputNode(SelectedInputName))
		for (const FIOParameter& P : Row->Parameters) {
			const FEdGraphPinType& pinType = ParamTypeToPinType(P.ParamType);
			CreatePin(EGPD_Output, pinType, FName(*P.ParamName));
		}
			
	

	Super::AllocateDefaultPins();
}

void UListenIOInputK2Node::ExpandNode(FKismetCompilerContext& C, UEdGraph* Graph)
{

}

void UListenIOInputK2Node::GetMenuActions(FBlueprintActionDatabaseRegistrar& R) const
{
	UClass* Key = GetClass();
	if (R.IsOpenForRegistration(Key))
		R.AddBlueprintAction(Key, UBlueprintNodeSpawner::Create(Key));
}

FText UListenIOInputK2Node::GetNodeTitle(ENodeTitleType::Type) const
{
	return FText();
}

FText UListenIOInputK2Node::GetMenuCategory() const
{
	return FText();
}

void UListenIOInputK2Node::PostEditChangeProperty(FPropertyChangedEvent& E)
{
	Super::PostEditChangeProperty(E);
	if (E.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UListenIOInputK2Node, SelectedInputName)) {
		ReconstructNode();
	}
}

const URPGInputOutputComponent* UListenIOInputK2Node::GetTemplateComponent() const
{
	UBlueprint* BP = GetBlueprint();
	if (!BP) return nullptr;

	if (BP->SimpleConstructionScript)
		for (USCS_Node* N : BP->SimpleConstructionScript->GetAllNodes())
			if (auto* Comp = Cast<URPGInputOutputComponent>(N->ComponentTemplate))
				return Comp;

	return nullptr;
}

FInputNode* UListenIOInputK2Node::FindInputNode(FName Name)
{
	const URPGInputOutputComponent* Comp = GetTemplateComponent();
	if (!Comp)
		return nullptr;

	auto Inputs = Comp->GetInputNodes();
	for (int32 i = 0; i < Inputs.Num(); i++) {
		if (Inputs[i].InputName.Equals(Name.ToString())) {
			return &Inputs[i];
		}
	}

	return nullptr;
}

FEdGraphPinType UListenIOInputK2Node::ParamTypeToPinType(EIOParamType Type) const
{
	FEdGraphPinType result;
	if(Type == EIOParamType::Actor) { result.PinCategory = UEdGraphSchema_K2::PC_Object; result.PinSubCategoryObject = AActor::StaticClass(); }
	else if(Type == EIOParamType::Bool) { result.PinCategory = UEdGraphSchema_K2::PC_Boolean ; }
	else if(Type == EIOParamType::Float) { result.PinCategory = UEdGraphSchema_K2::PC_Real; result.PinSubCategory = UEdGraphSchema_K2::PC_Double; }
	else if (Type == EIOParamType::Int) { result.PinCategory = UEdGraphSchema_K2::PC_Int; }
	else if (Type == EIOParamType::String) { result.PinCategory = UEdGraphSchema_K2::PC_String; }
	else if (Type == EIOParamType::Vector) { result.PinCategory = UEdGraphSchema_K2::PC_Struct; result.PinSubCategoryObject = TBaseStructure<FVector>::Get(); }

	return result;
}
