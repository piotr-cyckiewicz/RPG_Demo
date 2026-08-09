// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplaySystems/LevelDesignTools/ListenIOInputK2Node.h"
#include "Editor/BlueprintGraph/Public/BlueprintActionDatabaseRegistrar.h"
#include "Editor/BlueprintGraph/Public/BlueprintNodeSpawner.h"
#include "Runtime/Engine/Classes/Engine/SCS_Node.h"
#include "GameplaySystems/LevelDesignTools/RPGInputOutputComponent.h"
#include "GameplaySystems/LevelDesignTools/RPGInputOutputStructures.h"
#include "K2Node_CustomEvent.h"
#include "KismetCompiler.h"
#include "Editor/BlueprintGraph/Classes/K2Node_CallFunction.h"
#include "GameplaySystems/LevelDesignTools/IOParamLibrary.h"

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
	Super::ExpandNode(C, Graph);

	const FInputNode* Node = FindInputNode(SelectedInputName);
	if (!Node) { BreakAllNodeLinks(); return; }

	// CustomEvent that FireInput should find
	UK2Node_CustomEvent* Event =
		C.SpawnIntermediateNode<UK2Node_CustomEvent>(this, Graph);
	Event->CustomFunctionName = SelectedInputName;
	Event->CreateUserDefinedPin(TEXT("Params"),
		ParamsArrayPinType(), EGPD_Output, false);
	Event->AllocateDefaultPins();

	UEdGraphPin* EvThen = Event->FindPinChecked(UEdGraphSchema_K2::PN_Then);
	UEdGraphPin* EvParams = Event->FindPinChecked(TEXT("Params"));

	// for every pin, we find parameter
	for (int32 i = 0; i < Node->Parameters.Num(); ++i) {
		const FIOParameter& P = Node->Parameters[i];
		UEdGraphPin* MyOut = FindPinChecked(P.ParamName, EGPD_Output);

		UK2Node_CallFunction* Getter =
			C.SpawnIntermediateNode<UK2Node_CallFunction>(this, Graph);
		Getter->FunctionReference.SetExternalMember(
			GetterNameForType(P.ParamType), UIOParamLibrary::StaticClass());
		Getter->AllocateDefaultPins();

		Getter->FindPinChecked(TEXT("P"))->MakeLinkTo(EvParams);
		Getter->FindPinChecked(TEXT("Index"))->DefaultValue = FString::FromInt(i);

		// make sure proper values are received from this pin
		C.MovePinLinksToIntermediate(*MyOut, *Getter->GetReturnValuePin());
	}

	// trigger exec pin when event is triggered
	C.MovePinLinksToIntermediate(
		*FindPinChecked(UEdGraphSchema_K2::PN_Then),
		*EvThen);

	BreakAllNodeLinks();
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

FEdGraphPinType UListenIOInputK2Node::ParamsArrayPinType() const
{
	FEdGraphPinType PinType;
	PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	PinType.PinSubCategoryObject = FIOParameter::StaticStruct();
	PinType.ContainerType = EPinContainerType::Array;
	return PinType;
}

FName UListenIOInputK2Node::GetterNameForType(EIOParamType ParamType)
{
	if(ParamType == EIOParamType::Bool)
		//TODO: Finish this func
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
