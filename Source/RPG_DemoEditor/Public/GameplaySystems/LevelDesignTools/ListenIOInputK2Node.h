// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "ListenIOInputK2Node.generated.h"


class URPGInputOutputComponent;
enum class EIOParamType : uint8;
struct FInputNode;


UCLASS()
class RPG_DEMOEDITOR_API UListenIOInputK2Node : public UK2Node
{
	GENERATED_BODY()
	
public:
    UPROPERTY()
    FName SelectedInputName;

    virtual void AllocateDefaultPins() override; // Allocation of exec pin and parameter pins
    virtual void ExpandNode(FKismetCompilerContext& C, UEdGraph* Graph) override;
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& R) const override; // Registration in Blueprint palette
    virtual FText GetNodeTitle(ENodeTitleType::Type) const override;
    virtual FText GetMenuCategory() const override;
    virtual bool IsNodePure() const override { return false; } // We need out exec pin
    FEdGraphPinType ParamsArrayPinType() const;
    FName GetterNameForType(EIOParamType ParamType);

    virtual void PostEditChangeProperty(FPropertyChangedEvent& E) override; // Node reconstruciton after changing SelectedInputName

private:
    const URPGInputOutputComponent* GetTemplateComponent() const; // Gets template of IOComp of from this blueprint

    FInputNode* FindInputNode(FName Name); //Finds input node with given name
    
    FEdGraphPinType ParamTypeToPinType(EIOParamType Type) const; // returns proper pin types based on parameter type from InputParameter
};
