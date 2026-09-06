// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplaySystems/LevelDesignTools/RPGInputOutputComponent.h"
#include "GameplaySystems/LevelDesignTools/RPGInputOutputStructures.h"
#include "Engine/SCS_Node.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#endif

static TAutoConsoleVariable<int32> CVarIOSystemLogProcessingEvents(
	TEXT("IOSystem.LogProcessingEvents"), 0,
	TEXT("Controls logging of processing events from Input Output System.\n")
	TEXT("  0: disabled\n")
	TEXT("  1: enabled"),
	ECVF_Default
);

static TAutoConsoleVariable<int32> CVarIOSystemLogDiscardedEvents(
	TEXT("IOSystem.LogDiscardedEvents"), 0,
	TEXT("Controls logging of discarded events from Input Output System (for example, events that got discarded due to reaching max fire count).\n")
	TEXT("  0: disabled\n")
	TEXT("  1: enabled"),
	ECVF_Default
);

URPGInputOutputComponent::URPGInputOutputComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void URPGInputOutputComponent::BeginPlay()
{
	Super::BeginPlay();
	for (int i = 0; i < OutputNodes.Num(); i++) {
		if (!IsValid(OutputNodes[i].Target)) {
			UE_LOG(LogTemp, Display, TEXT("URPGInputOutputComponent - BeginPlay - Target at index %d is invalid"), i);
			continue;
		}
		URPGInputOutputComponent* IOComp = OutputNodes[i].Target->GetComponentByClass<URPGInputOutputComponent>();
		if (!IsValid(IOComp)) {
			UE_LOG(LogTemp, Display, TEXT("URPGInputOutputComponent - BeginPlay - TargetIOComp at index %d is invalid"), i);
			continue;
		}
		OutputNodes[i].TargetIOComp = IOComp;
	}
}

void URPGInputOutputComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	for (int i = 0; i < OutputNodesToProcessDelay.Num(); i++) {
		OutputNodesToProcessDelay[i] -= DeltaTime;
	}

	for (int i = 0; i < OutputNodesToProcessDelay.Num(); i++) {
		if (OutputNodesToProcessDelay[i] <= 0) {
			ProcessOutputNode(i);
		}
	}
}

TArray<FName> URPGInputOutputComponent::GetActorInputs(AActor* Actor)
{
	TArray<FName> Inputs;
	
	if (!IsValid(Actor)) {
		UE_LOG(LogTemp, Error, TEXT("RPGInputOutputComponent - GetActorInputs - No valid actor"));
		return Inputs;
	}

	auto* TargetIO = Actor->GetComponentByClass<URPGInputOutputComponent>();
	if (!TargetIO) {
		UE_LOG(LogTemp, Error, TEXT("RPGInputOutputComponent - GetActorInputs - No InputOutputComponent found"));
		return Inputs;
	}

	auto InputsFString = TargetIO->GetInputOptions();

	for (auto InputRow : InputsFString) {
		Inputs.Add(FName(*InputRow));
	}
	return Inputs;
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
			&& (BlueprintType == TEXT("BPTYPE_NORMAL") || BlueprintType == TEXT("BPTYPE_Normal")))
		{
			FString ClassPath;
			Asset.GetTagValue(FBlueprintTags::GeneratedClassPath, ClassPath);
			FString ObjectPath = FPackageName::ExportTextPathToObjectPath(ClassPath);
			UClass* ObjectClass = LoadObject<UClass>(nullptr, *ObjectPath);
			if (!IsValid(ObjectClass)) continue;
			UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(ObjectClass);
			if (BPGC && BPGC->SimpleConstructionScript)
			{
				for (USCS_Node* Node : BPGC->SimpleConstructionScript->GetAllNodes())
				{
					if (auto* IOComp = Cast<URPGInputOutputComponent>(Node->ComponentTemplate))
					{
						for (auto InputRow : IOComp->GetInputOptions()) {
							Inputs.AddUnique(FName(*InputRow));
						}
					}
				}
			}
		}
	}

	return Inputs;
}
FString URPGInputOutputComponent::OutputNodeToString(FOutputNode& Node) const
{
	FString result = FString(TEXT(""));
	result.Append(Node.OutputActor->GetActorLabel()); result.Append(TEXT("->"));
	result.Append(Node.OutputType); result.Append(TEXT("->"));
	result.Append(Node.Target->GetActorLabel()); result.Append(TEXT("->"));
	result.Append(Node.TargetInput); result.Append(TEXT("->"));
	result.Append(FString::FromInt(Node.DelayTimer)); result.Append(TEXT("->"));
	result.Append(FString::FromInt(Node.Delay));
	
	return result;
}
void URPGInputOutputComponent::UpdateOutputActor(int32 OutputNodeIndex)
{
	OutputNodes[OutputNodeIndex].OutputActor = GetOwner();
}
#endif
void URPGInputOutputComponent::FireOutput(FString OutputName)
{
	if (!OutputList.Contains(OutputName)) {
		UE_LOG(LogTemp, Error, TEXT("URPGInputOutputComponent - FireOutput - %s output not found"), *OutputName);
	}

	int32 OldSize = OutputNodesToProcess.Num();
	for (int32 i = 0; i < OutputNodes.Num(); i++) {
		if (OutputNodes[i].OutputType == OutputName) {
			if (OutputNodes[i].MaxFireCount == -1 || OutputNodes[i].FireCount < OutputNodes[i].MaxFireCount) {
				OutputNodes[i].OutputActor = GetOwner();
				OutputNodesToProcess.Add(i);
				OutputNodesToProcessDelay.Add(OutputNodes[i].Delay);
			}
			else if(CVarIOSystemLogDiscardedEvents.GetValueOnGameThread() > 0) {
				UE_LOG(LogTemp, Display, TEXT("URPGInputOutputComponent - FireOutput - Output Node %s discarded due to Fire Count (%d) exceeding Max Fire Count (%d)"),
					*OutputNodeToString(OutputNodes[i]), OutputNodes[i].FireCount, OutputNodes[i].MaxFireCount);
			}
		}
	}
	for (int32 i = OldSize; i < OutputNodesToProcess.Num(); i++) {
		if (OutputNodesToProcessDelay[i] <= 0) {
			ProcessOutputNode(i);
		}
	}
}

bool WriteIOParamToParams(FProperty* Prop, void* Parms, FIOParameter& Param)
{
	if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop)) {
		if (Param.ParamType != EIOParamType::Bool) return false;
		BoolProp->SetPropertyValue_InContainer(Parms, Param.BoolValue);
		return true;
	}
	if (FIntProperty* IntProp = CastField<FIntProperty>(Prop)) {
		if (Param.ParamType != EIOParamType::Int) return false;
		IntProp->SetPropertyValue_InContainer(Parms, Param.IntValue);
		return true;
	}
	// 'Blueprinty' float may be either double or float
	if (FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Prop)) {
		if (Param.ParamType != EIOParamType::Float) return false;
		DoubleProp->SetPropertyValue_InContainer(Parms, Param.FloatValue);
		return true;
	}
	if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop)) {
		if (Param.ParamType != EIOParamType::Float) return false;
		FloatProp->SetPropertyValue_InContainer(Parms, static_cast<float>(Param.FloatValue));
		return true;
	}
	if (FStrProperty* StrProp = CastField<FStrProperty>(Prop)) {
		if (Param.ParamType != EIOParamType::String) return false;
		StrProp->SetPropertyValue_InContainer(Parms, Param.StringValue);
		return true;
	}
	if (FStructProperty* StructProp = CastField<FStructProperty>(Prop)) {
		if (Param.ParamType != EIOParamType::Vector) return false;
		if (StructProp->Struct != TBaseStructure<FVector>::Get()) return false;
		*StructProp->ContainerPtrToValuePtr<FVector>(Parms) = Param.VectorValue;
		return true;
	}
	if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop)) {
		if (Param.ParamType != EIOParamType::Actor) return false;
		AActor* Actor = Param.ActorValue;
		if (Actor && !Actor->IsA(ObjProp->PropertyClass)) return false;
		ObjProp->SetObjectPropertyValue_InContainer(Parms, Actor);
		return true;
	}
	return false;
}

void URPGInputOutputComponent::FireInput(AActor* OutputActor, FString InputName, TArray<FIOParameter> IOParamaters)
{
	if (!IsValid(OutputActor)) {
		UE_LOG(LogTemp, Error, TEXT("RPGInputOutputComponent - FireInput - OutputActor is invalid"));
		return;
	}

	// find event generated by custom K2 node and try to process it
	FString FuncName = FString(TEXT("IO_")); FuncName.Append(InputName);
	UFunction* Func = GetOwner()->FindFunction(FName(*FuncName));
	if (!Func) {
		UE_LOG(LogTemp, Error, TEXT("RPGInputOutputComponent - FireInput - Function with name %s not found"), *FuncName);
		return;
	}

	if (Func->ParmsSize == 0) {
		GetOwner()->ProcessEvent(Func, nullptr);
		return;
	}

	uint8* Parms = (uint8*)FMemory_Alloca_Aligned(Func->ParmsSize, Func->GetMinAlignment());
	FMemory::Memzero(Parms, Func->ParmsSize);
	
	for (TFieldIterator<FProperty> It(Func); It && It->HasAnyPropertyFlags(CPF_Parm); ++It) {
		It->InitializeValue_InContainer(Parms);
	}

	bool bParamsMatch = true;
	int32 IOIndex = 0;
	for (TFieldIterator<FProperty> It(Func); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		FProperty* Prop = *It;
		if (Prop->HasAnyPropertyFlags(CPF_ReturnParm)) continue;

		if (!IOParamaters.IsValidIndex(IOIndex)) {
			UE_LOG(LogTemp, Error, TEXT("URPGInputOutputComponent - FireInput - %s expects more parameters than configured (%d)"),
				*FuncName, IOParamaters.Num());
			bParamsMatch = false;
			break;
		}

		FIOParameter& Param = IOParamaters[IOIndex];
		if (Param.ParamName != Prop->GetName()) {
			UE_LOG(LogTemp, Warning, TEXT("URPGInputOutputComponent - FireInput - %s - param %d name mismatch ('%s' vs '%s') - signature changed after configuration?"),
				*FuncName, IOIndex, *Param.ParamName, *Prop->GetName());
		}
		if (!WriteIOParamToParams(Prop, Parms, Param)) {
			UE_LOG(LogTemp, Error, TEXT("URPGInputOutputComponent - FireInput - %s: type mismatch on parameter '%s'"),
				*FuncName, *Prop->GetName());
			bParamsMatch = false;
			break;
		}
		IOIndex++;
	}

	if (bParamsMatch) {
		GetOwner()->ProcessEvent(Func, Parms);
	}

	// Need to destroy values in the buffer — FString in parameters may leak
	for (TFieldIterator<FProperty> It(Func); It && It->HasAnyPropertyFlags(CPF_Parm); ++It) {
		It->DestroyValue_InContainer(Parms);
	}
}

void URPGInputOutputComponent::ProcessOutputNode(int32 index)
{
	if (OutputNodesToProcessDelay[index] > 0) {
		UE_LOG(LogTemp, Error, TEXT("RPGInputOutputStructures - ProcessOutputNode - Node Delay is above 0"));
		return;
	}
	int32 indexNode = OutputNodesToProcess[index];

	if (OutputNodes[indexNode].MaxFireCount != -1 && OutputNodes[indexNode].FireCount >= OutputNodes[indexNode].MaxFireCount) {
		if (CVarIOSystemLogDiscardedEvents.GetValueOnGameThread() > 0)
			UE_LOG(LogTemp, Display, TEXT("URPGInputOutputComponent - ProcessOutputNode - Output Node %s discarded due to Fire Count (%d) exceeding Max Fire Count (%d)"),
				*OutputNodeToString(OutputNodes[indexNode]), OutputNodes[indexNode].FireCount, OutputNodes[indexNode].MaxFireCount);
		return;
	}
	if (!IsValid(OutputNodes[indexNode].OutputActor)) {
		UE_LOG(LogTemp, Error, TEXT("RPGInputOutputStructures - ProcessOutputNode - OutputActor is invalid"));
		return;
	}
	if (!IsValid(OutputNodes[indexNode].Target)) {
		UE_LOG(LogTemp, Error, TEXT("RPGInputOutputStructures - ProcessOutputNode - Target is not valid"));
		return;
	}
	if (OutputNodes[indexNode].TargetInput.IsEmpty() || OutputNodes[indexNode].TargetInput.ToLower().Equals(TEXT("<none>"))) {
		UE_LOG(LogTemp, Error, TEXT("RPGInputOutputStructures - ProcessOutputNode - TargetInput is not set"));
		return;
	}
	if (!IsValid(OutputNodes[indexNode].TargetIOComp)) {
		UE_LOG(LogTemp, Error, TEXT("RPGInputOutputStructures - ProcessOutputNode - TargetIOComp is invalid"));
		return;
	}

	// Remove the nodes before firing input, as it might conflict with save system
	// (for example we may fire Save Input that will save the state of OutputNodesToProcess, and we'll have unnecessary Save after loading the game)

	OutputNodesToProcess.RemoveAt(index);
	OutputNodesToProcessDelay.RemoveAt(index);
	OutputNodes[indexNode].FireCount++;

	OutputNodes[indexNode].TargetIOComp->FireInput(OutputNodes[indexNode].OutputActor, OutputNodes[indexNode].TargetInput, OutputNodes[indexNode].InputParameters);

}

#if WITH_EDITOR
void URPGInputOutputComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.PropertyChain.GetActiveMemberNode() == nullptr
		|| PropertyChangedEvent.PropertyChain.GetActiveMemberNode()->GetValue() == nullptr) {
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}
		
	if (PropertyChangedEvent.Property == nullptr) {
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	FName MemberName = PropertyChangedEvent.PropertyChain.GetActiveMemberNode()->GetValue()->GetFName();
	FName PropertyName = PropertyChangedEvent.Property->GetFName();

	if (MemberName != GET_MEMBER_NAME_CHECKED(URPGInputOutputComponent, OutputNodes)
		|| PropertyName != GET_MEMBER_NAME_CHECKED(FOutputNode, TargetInput)) {
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	int32 Index = PropertyChangedEvent.GetArrayIndex(TEXT("OutputNodes"));

	if (!OutputNodes.IsValidIndex(Index)) {
		UE_LOG(LogTemp, Error, TEXT("URPGInputOutputComponent::PostEditChangeChainProperty - Improper index detected"));
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	// We have to make sure that vaid output always has proper OutputActor set
	OutputNodes[Index].OutputActor = GetOwner();

	// If no actor is set, we rteset input apramaters and return early
	if (OutputNodes[Index].TargetType == EIOTargetType::Actor && !IsValid(OutputNodes[Index].Target)) {
		OutputNodes[Index].InputParameters.Reset();
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	const FString& TargetInput = OutputNodes[Index].TargetInput;
	if (TargetInput.IsEmpty() || TargetInput.Equals(TEXT("<none>"), ESearchCase::IgnoreCase)) {
		OutputNodes[Index].InputParameters.Reset();
		return;
	}

	if (OutputNodes[Index].TargetType != EIOTargetType::Activator && OutputNodes[Index].Target == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("URPGInputOutputComponent::PostEditChangeChainProperty - No Target set despite changing TargetInput"));
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	auto* IOComp = OutputNodes[Index].Target->GetComponentByClass<URPGInputOutputComponent>();
	if (!IsValid(IOComp)) {
		UE_LOG(LogTemp, Error, TEXT("URPGInputOutputComponent::PostEditChangeChainProperty - No InputOutputComponent found despite Target being valid"));
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	auto TargetInputWithPrefix = FString(TEXT("IO_")).Append(TargetInput);
	UFunction* Func = OutputNodes[Index].Target->FindFunction(FName(*TargetInputWithPrefix));

	if (Func == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("URPGInputOutputComponent::PostEditChangeChainProperty - No input matching TargetInput was found"));
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	OutputNodes[Index].InputParameters.Reset();
	for (TFieldIterator<FProperty> It(Func); It; ++It)
	{
		FProperty* Property = *It;
		if (!Property->HasAnyPropertyFlags(CPF_Parm)) continue;
		if (Property->HasAnyPropertyFlags(CPF_ReturnParm)) continue;

		OutputNodes[Index].InputParameters.Add(FIOParameter(Property));
	}

	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif


TArray<FString> URPGInputOutputComponent::GetOutputOptions() const
{
	return OutputList;
}

TArray<FString> URPGInputOutputComponent::GetOutputOptionsWithNoneOption() const
{
	TArray<FString> result = GetOutputOptions();
	result.Insert(FString(TEXT("<none>")), 0);
	return result;
}

TArray<FString> URPGInputOutputComponent::GetInputOptions() const
{
	TArray<FString> Result;
	AActor* Actor = GetOwner();

	for (TFieldIterator<UFunction> It(Actor->GetClass()); It; ++It)
	{
		UFunction* Func = *It;
		if (Func->GetName().StartsWith(TEXT("IO_")))
		{
			Result.Add(Func->GetName().RightChop(3));
		}
	}
	return Result;
}

TArray<FString> URPGInputOutputComponent::GetInputOptionsWithNoneOption() const
{
	TArray<FString> result = GetInputOptions();
	result.Insert(FString(TEXT("<none>")), 0);
	return result;
}
