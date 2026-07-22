// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RPGInputOutputStructures.generated.h"

UENUM(BlueprintType)
enum class EIOParamType : uint8 { None, Bool, Int, Float, String, Vector, Actor };

USTRUCT(BlueprintType)
struct FIOParameter
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly)
	FString ParamName = FString(TEXT(""));

	UPROPERTY()
	EIOParamType ParamType = EIOParamType::None;

	UPROPERTY(EditInstanceOnly,
		meta = (EditCondition = "ParamType == EIOParamType::Bool", EditConditionHides))
	bool BoolValue = false;

	UPROPERTY(EditInstanceOnly,
		meta = (EditCondition = "ParamType == EIOParamType::Int", EditConditionHides))
	int32 IntValue = 0;

	UPROPERTY(EditInstanceOnly,
		meta = (EditCondition = "ParamType == EIOParamType::Float", EditConditionHides))
	float FloatValue = 0.f;

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
	FIOParameter(FString ParamName, float FloatValue) { this->ParamName = ParamName; ParamType = EIOParamType::Float; this->FloatValue = FloatValue; }
	FIOParameter(FString ParamName, FString StringValue) { this->ParamName = ParamName; ParamType = EIOParamType::String; this->StringValue = StringValue; }
	FIOParameter(FString ParamName, FVector VectorValue) { this->ParamName = ParamName; ParamType = EIOParamType::Vector; this->VectorValue = VectorValue; }
	FIOParameter(FString ParamName, AActor* ActorValue) { this->ParamName = ParamName; ParamType = EIOParamType::Actor; this->ActorValue = ActorValue; }

	FIOParameter(FProperty* Parameter) {
		if (Parameter->GetCPPType().Equals(TEXT("bool"))) { ParamName = Parameter->GetName(); ParamType = EIOParamType::Bool; }
		else if (Parameter->GetCPPType().Equals(TEXT("int32"))) { ParamName = Parameter->GetName(); ParamType = EIOParamType::Int; }
		else if (Parameter->GetCPPType().Equals(TEXT("float"))) { ParamName = Parameter->GetName(); ParamType = EIOParamType::Float; }
		else if (Parameter->GetCPPType().Equals(TEXT("FString"))) { ParamName = Parameter->GetName(); ParamType = EIOParamType::String; }
		else if (Parameter->GetCPPType().Equals(TEXT("FVector"))) { ParamName = Parameter->GetName(); ParamType = EIOParamType::Vector; }
		else if (Parameter->GetCPPType().Equals(TEXT("AActor*"))) { ParamName = Parameter->GetName(); ParamType = EIOParamType::Actor; }
		else { UE_LOG(LogTemp, Error, TEXT("No supported property type in FIOParameter detected")); }
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

	UPROPERTY(EditInstanceOnly, meta = (GetOptions = "GetOutputOptions"))
	FString OutputType;

	UPROPERTY(EditInstanceOnly)
	EIOTargetType TargetType = EIOTargetType::Actor;

	UPROPERTY(EditInstanceOnly, meta = (EditCondition = "TargetType == EIOTargetType::Actor", EditConditionHides))
	TObjectPtr<AActor> Target;

	UPROPERTY(EditInstanceOnly)
	FString TargetInput;
	UPROPERTY(EditInstanceOnly)
	TArray<FIOParameter> InputParameters;
	UPROPERTY(EditInstanceOnly, meta = (ClampMin = 0))
	float Delay;
	UPROPERTY()
	float DelayTimer;
	UPROPERTY(EditInstanceOnly, meta = (ToolTip = "Use -1 for infinite fireCounts"))
	int32 MaxFireCount = -1;
	UPROPERTY()
	int32 FireCount = 0;

	FOutputNode() {

	}
};