// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/SCS_Node.h"
#include "RPGInputOutputStructures.generated.h"

struct FInputParameter;

UENUM(BlueprintType)
enum class EIOParamType : uint8 { None, Bool, Int, Float, String, Vector, Actor };

USTRUCT(BlueprintType)
struct FIOParameter
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly)
	FString ParamName = FString(TEXT(""));

	UPROPERTY(VisibleInstanceOnly)
	EIOParamType ParamType = EIOParamType::None;

	UPROPERTY(EditInstanceOnly,
		meta = (EditCondition = "ParamType == EIOParamType::Bool", EditConditionHides))
	bool BoolValue = false;

	UPROPERTY(EditInstanceOnly,
		meta = (EditCondition = "ParamType == EIOParamType::Int", EditConditionHides))
	int32 IntValue = 0;

	UPROPERTY(EditInstanceOnly,
		meta = (EditCondition = "ParamType == EIOParamType::Float", EditConditionHides))
	double FloatValue = 0.f;

	UPROPERTY(EditInstanceOnly,
		meta = (EditCondition = "ParamType == EIOParamType::String", EditConditionHides))
	FString StringValue = FString(TEXT(""));

	UPROPERTY(EditInstanceOnly,
		meta = (EditCondition = "ParamType == EIOParamType::Vector", EditConditionHides))
	FVector VectorValue = FVector::ZeroVector;

	UPROPERTY(EditInstanceOnly,
		meta = (EditCondition = "ParamType == EIOParamType::Actor", EditConditionHides))
	TObjectPtr<AActor> ActorValue = nullptr;

	FIOParameter() {}
	FIOParameter(FString ParamName, bool BoolValue) { this->ParamName = ParamName; ParamType = EIOParamType::Bool; this->BoolValue = BoolValue; }
	FIOParameter(FString ParamName, int32 IntValue) { this->ParamName = ParamName; ParamType = EIOParamType::Int; this->IntValue = IntValue; }
	FIOParameter(FString ParamName, double FloatValue) { this->ParamName = ParamName; ParamType = EIOParamType::Float; this->FloatValue = FloatValue; }
	FIOParameter(FString ParamName, FString StringValue) { this->ParamName = ParamName; ParamType = EIOParamType::String; this->StringValue = StringValue; }
	FIOParameter(FString ParamName, FVector VectorValue) { this->ParamName = ParamName; ParamType = EIOParamType::Vector; this->VectorValue = VectorValue; }
	FIOParameter(FString ParamName, AActor* ActorValue) { this->ParamName = ParamName; ParamType = EIOParamType::Actor; this->ActorValue = ActorValue; }

	FIOParameter(FInputParameter Parameter);

	FIOParameter(FProperty* Property) {
		this->ParamName = Property->GetName();
		if (Property->GetCPPType() == FString(TEXT("bool"))) ParamType = EIOParamType::Bool;
		else if (Property->GetCPPType() == FString(TEXT("int32"))) ParamType = EIOParamType::Int;
		else if (Property->GetCPPType() == FString(TEXT("double"))) ParamType = EIOParamType::Float;
		else if (Property->GetCPPType() == FString(TEXT("FString"))) ParamType = EIOParamType::String;
		else if (Property->GetCPPType() == FString(TEXT("FVector"))) ParamType = EIOParamType::Vector;
		else if (Property->GetCPPType() == FString(TEXT("AActor*"))) ParamType = EIOParamType::Actor;
		else UE_LOG(LogTemp, Error, TEXT("RPGInputOutputStructures - FIOParameter(FProperty* Property) - Unsupported type of property"));
	}
};


USTRUCT(BlueprintType)
struct FInputParameter
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	FString ParamName = FString();

	UPROPERTY(EditDefaultsOnly)
	EIOParamType ParamType = ParamType = EIOParamType::None;

	UPROPERTY(EditDefaultsOnly,
		meta = (EditCondition = "ParamType == EIOParamType::Bool", EditConditionHides))
	bool BoolValue = false;

	UPROPERTY(EditDefaultsOnly,
		meta = (EditCondition = "ParamType == EIOParamType::Int", EditConditionHides))
	int32 IntValue = 0;

	UPROPERTY(EditDefaultsOnly,
		meta = (EditCondition = "ParamType == EIOParamType::Float", EditConditionHides))
	double FloatValue = 0;

	UPROPERTY(EditDefaultsOnly,
		meta = (EditCondition = "ParamType == EIOParamType::String", EditConditionHides))
	FString StringValue = FString();

	UPROPERTY(EditDefaultsOnly,
		meta = (EditCondition = "ParamType == EIOParamType::Vector", EditConditionHides))
	FVector VectorValue = FVector();

	UPROPERTY(EditDefaultsOnly,
		meta = (EditCondition = "ParamType == EIOParamType::Actor", EditConditionHides))
	TObjectPtr<AActor> ActorValue = nullptr;

	FInputParameter(FIOParameter Parameter) {
		if (Parameter.ParamType == EIOParamType::Bool)			BoolValue = Parameter.BoolValue;
		else if (Parameter.ParamType == EIOParamType::Int)		IntValue = Parameter.IntValue;
		else if (Parameter.ParamType == EIOParamType::Float)	FloatValue = Parameter.FloatValue;
		else if (Parameter.ParamType == EIOParamType::String)	StringValue = Parameter.StringValue;
		else if (Parameter.ParamType == EIOParamType::Vector)	VectorValue = Parameter.VectorValue;
		else if (Parameter.ParamType == EIOParamType::Actor)	ActorValue = Parameter.ActorValue;
		else { UE_LOG(LogTemp, Error, TEXT("No supported property type in FIOParameter detected")); }
		ParamName = Parameter.ParamName;
		ParamType = Parameter.ParamType;
	}
	FInputParameter() {
		ParamType = EIOParamType::None;
	}
};

UENUM(BlueprintType)
enum class EIOTargetType : uint8
{
	Actor			UMETA(DisplayName = "Actor"),
	Activator		UMETA(DisplayName = "Activator"),
	Self			UMETA(DisplayName = "Self")
};

USTRUCT(BlueprintType)
struct RPG_DEMO_API FOutputNode
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> OutputActor;

	UPROPERTY(EditInstanceOnly, meta = (GetOptions = "GetOutputOptionsWithNoneOption"))
	FString OutputType;

	UPROPERTY(EditInstanceOnly)
	EIOTargetType TargetType = EIOTargetType::Actor;

	UPROPERTY(EditInstanceOnly, meta = (EditCondition = "TargetType == EIOTargetType::Actor", EditConditionHides))
	TObjectPtr<AActor> Target;

	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<class URPGInputOutputComponent> TargetIOComp;

	UPROPERTY(EditInstanceOnly)
	FString TargetInput;
	UPROPERTY(EditInstanceOnly, EditFixedSize)
	TArray<FIOParameter> InputParameters;
	UPROPERTY(EditInstanceOnly, meta = (ClampMin = 0))
	float Delay = 0;
	UPROPERTY()
	float DelayTimer = 0;
	UPROPERTY(EditInstanceOnly, meta = (ToolTip = "Details how many times can this output node be triggered. Use -1 for unlimited triggers"))
	int32 MaxFireCount = -1;
	UPROPERTY()
	int32 FireCount = 0;

	FOutputNode() {

	}
};

USTRUCT(BlueprintType)
struct RPG_DEMO_API FInputNode
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	FString InputName;

	UPROPERTY(EditDefaultsOnly)
	TArray<FInputParameter> Parameters;
};

inline FIOParameter::FIOParameter(FInputParameter Parameter) {
	if (Parameter.ParamType == EIOParamType::Bool)			BoolValue = Parameter.BoolValue;
	else if (Parameter.ParamType == EIOParamType::Int)		IntValue = Parameter.IntValue;
	else if (Parameter.ParamType == EIOParamType::Float)	FloatValue = Parameter.FloatValue;
	else if (Parameter.ParamType == EIOParamType::String)	StringValue = Parameter.StringValue;
	else if (Parameter.ParamType == EIOParamType::Vector)	VectorValue = Parameter.VectorValue;
	else if (Parameter.ParamType == EIOParamType::Actor)	ActorValue = Parameter.ActorValue;
	else { UE_LOG(LogTemp, Error, TEXT("No supported property type in FIOParameter detected")); }
	ParamName = Parameter.ParamName;
	ParamType = Parameter.ParamType;
}