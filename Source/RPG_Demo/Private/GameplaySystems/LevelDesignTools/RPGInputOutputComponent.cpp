// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplaySystems/LevelDesignTools/RPGInputOutputComponent.h"
#include "GameplaySystems/LevelDesignTools/RPGInputOutputStructures.h"
#include "Engine/SCS_Node.h"
#include "Utilities/LoggingFunctionLibrary.h"

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
	PrimaryComponentTick.bCanEverTick = true;
}


void URPGInputOutputComponent::BeginPlay()
{
	Super::BeginPlay();
	for (int i = 0; i < OutputNodes.Num(); i++) {
		if (OutputNodes[i].TargetType == EIOTargetType::Actor) {
			if (!IsValid(OutputNodes[i].Target)) {
				LoggingFunctionLibrary::PrintError(this, FString(TEXT("URPGInputOutputComponent - BeginPlay - Target at index %d is invalid"), i));
				continue;
			}
			URPGInputOutputComponent* IOComp = OutputNodes[i].Target->GetComponentByClass<URPGInputOutputComponent>();
			if (!IsValid(IOComp)) {
				LoggingFunctionLibrary::PrintError(this, FString(TEXT("URPGInputOutputComponent - BeginPlay - TargetIOComp at index %d is invalid"), i));
				continue;
			}
			OutputNodes[i].TargetIOComp = IOComp;
		}
	}

#if !UE_BUILD_SHIPPING
	// CancelPending is a built-in input, so IO_CancelPending event on the owner should throw an error
	FString ReservedName = FString(TEXT("IO_")) + CancelPendingInputName;
	if (GetOwner()->FindFunction(FName(*ReservedName))) {
		LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("URPGInputOutputComponent - BeginPlay - %s defines IO_%s, but %s is a reserved built-in input - the event will never be called"),
			*GetOwner()->GetActorNameOrLabel(), CancelPendingInputName, CancelPendingInputName));
	}
#endif
}

void URPGInputOutputComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	for (int i = 0; i < OutputNodesToProcessDelay.Num(); i++) {
		OutputNodesToProcessDelay[i] -= DeltaTime;
	}

	int32 StartGeneration = PendingGeneration;
	for (int i = 0; i < OutputNodesToProcessDelay.Num(); i++) {
		if (OutputNodesToProcessDelay[i] <= 0) {
			bool bRemoved = ProcessOutputNode(i);
			// CancelPending was fired during ProcessOutputNode() - queue got cleared
			if (PendingGeneration != StartGeneration)
				break;
			if (bRemoved)
				i--; // ProcessOutputNode removes nodes from OutputNodesToProcess and OutputNodesToProcessDelay, so index shouldn't be increased
		}
	}
}


TArray<FName> URPGInputOutputComponent::GetActorInputs(AActor* Actor)
{
	TArray<FName> Inputs;
	
	if (!IsValid(Actor)) {
		LoggingFunctionLibrary::PrintError(Actor, FString(TEXT("RPGInputOutputComponent - GetActorInputs - No valid actor")));
		return Inputs;
	}

	auto* TargetIO = Actor->GetComponentByClass<URPGInputOutputComponent>();
	if (!TargetIO) {
		LoggingFunctionLibrary::PrintError(Actor, FString(TEXT("RPGInputOutputComponent - GetActorInputs - No InputOutputComponent found")));
		return Inputs;
	}

	auto InputsFString = TargetIO->GetInputOptions();

	for (auto InputRow : InputsFString) {
		Inputs.Add(FName(*InputRow));
	}
	return Inputs;
}

#if WITH_EDITOR
TMap<FName, TArray<struct FIOParameter>> URPGInputOutputComponent::GetAllActorInputs()
{
	TMap<FName, TArray<FIOParameter>> Inputs;

	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	Filter.PackagePaths.Add(TEXT("/Game")); // Skip engine content
	Filter.bRecursivePaths = true;

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
					if (Cast<URPGInputOutputComponent>(Node->ComponentTemplate))
					{
						for (FString& Option : GetInputOptionsForClass(BPGC)) {
							FName InputName = FName(*Option);
							if (Inputs.Contains(InputName)) continue; // Skip if we already found this input
							if (Option.ToLower().Contains(FString(TEXT("DebugOnlyInput")))) continue; // Skip debug inputs - designers don't need tos ee it in Activator case

							TArray<FIOParameter>& Params = Inputs.Add(InputName);
							if (Option.Equals(CancelPendingInputName, ESearchCase::IgnoreCase)) continue; // CancelPending has no parameters

							if (UFunction* Func = BPGC->FindFunctionByName(FName(*(TEXT("IO_") + Option)))) {
								for (TFieldIterator<FProperty> It(Func); It && It->HasAnyPropertyFlags(CPF_Parm); ++It) {
									if (It->HasAnyPropertyFlags(CPF_ReturnParm)) continue;
									Params.Add(FIOParameter(*It));
								}
							}
							else {
								LoggingFunctionLibrary::PrintError(nullptr, FString::Printf(TEXT("URPGInputOutputComponent - GetAllActorInputs() - IO_%s should be on %s, but couldn't be found"), *Option, *BPGC->GetName()));
							}
						}
						break;
					}
				}
			}
		}
	}

	return Inputs;
}
#endif

#if !UE_BUILD_SHIPPING
FString URPGInputOutputComponent::OutputNodeToString(FOutputNode& Node) const
{
	if(Node.TargetType == EIOTargetType::Activator ) {
		LoggingFunctionLibrary::PrintError(this, FString(TEXT("URPGInputOutputComponent - OutputNodeToString - String conversation for FOutputNode with TargetType==Activator not implemented")));
		return FString(TEXT("ERROR - String conversation for FOutputNode with TargetType==Activator not implemented"));
	}

	FString result = FString(TEXT(""));
	result.Append(Node.OutputActor->GetActorNameOrLabel()); result.Append(TEXT("->"));
	result.Append(Node.OutputType); result.Append(TEXT("->"));
	result.Append(Node.Target->GetActorNameOrLabel()); result.Append(TEXT("->"));
	result.Append(Node.TargetInput); result.Append(TEXT("->"));
	result.Append(FString::FromInt(Node.DelayTimer)); result.Append(TEXT("->"));
	result.Append(FString::FromInt(Node.Delay));
	
	return result;
}
#endif

void URPGInputOutputComponent::FireOutput(FString OutputName, AActor* Activator)
{
#if !UE_BUILD_SHIPPING
	if (!OutputList.Contains(OutputName)) {
		LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("URPGInputOutputComponent - FireOutput - %s output not found"), *OutputName));
		return;
	}
#endif
	if (!IsValid(Activator)) {
		LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("URPGInputOutputComponent - FireOutput - %s - Activator not specified"), *OutputName));
		return;
	}

	int32 OldQueueSize = OutputNodesToProcess.Num(); // We want to trigger new output nodes in queue with Delay = 0 in this frame so we need to know which ones are new

	for (int32 i = 0; i < OutputNodes.Num(); i++) {
		if (OutputNodes[i].OutputType == OutputName) {
			// Skip node if FireCount exceeds MaxFireCount
			if (OutputNodes[i].MaxFireCount == -1 || OutputNodes[i].FireCount < OutputNodes[i].MaxFireCount) {
				OutputNodes[i].OutputActor = GetOwner();
				OutputNodes[i].Activator = Activator;
				OutputNodesToProcess.Add(i);
				OutputNodesToProcessDelay.Add(OutputNodes[i].Delay);
				OutputNodesToProcessActivators.Add(Activator);
			}
			#if !UE_BUILD_SHIPPING
			// If FireCount exceeds MaxFireCount and logging of discarded nodes is enabled, we need to log it
			else if(CVarIOSystemLogDiscardedEvents.GetValueOnGameThread() > 0) {
				LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("URPGInputOutputComponent - FireOutput - Output Node %s discarded due to Fire Count (%d) exceeding Max Fire Count (%d)"),
					*OutputNodeToString(OutputNodes[i]), OutputNodes[i].FireCount, OutputNodes[i].MaxFireCount));
			}
			#endif
		}
	}

	// instantly trigger new OutputNodes in the queue with Delay = 0
	int32 StartGeneration = PendingGeneration;
	for (int32 i = OldQueueSize; i < OutputNodesToProcess.Num(); i++) {
		if (OutputNodesToProcessDelay[i] <= 0) {
			bool bRemoved = ProcessOutputNode(i);
			// CancelPending was fired during ProcessOutputNode() - queue got cleared
			if (PendingGeneration != StartGeneration)
				break;
			if(bRemoved)
				i--;  // ProcessOutputNode removes nodes from OutputNodesToProcess and OutputNodesToProcessDelay, so index shouldn't be increased
		}
	}
}

void URPGInputOutputComponent::CancelPendingOutputs()
{
#if !UE_BUILD_SHIPPING
	if (CVarIOSystemLogProcessingEvents.GetValueOnGameThread() > 0) {
		UE_LOG(LogTemp, Log, TEXT("URPGInputOutputComponent - CancelPendingOutputs - %s cancelled %d pending output nodes"),
			*GetOwner()->GetActorNameOrLabel(), OutputNodesToProcess.Num());
	}
#endif

	OutputNodesToProcess.Reset();
	OutputNodesToProcessDelay.Reset();
	OutputNodesToProcessActivators.Reset();

	// Lets every loop currently iterating over the queue (tick, FireOutput) know it has to stop
	++PendingGeneration;
}

// Determines the type of property FProperty stores and updates it to the value of FIOParameter. Returns false if types mismatch.
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

// Used to trigger input (such as "Teleport") with Parameters. Fires custom event named IO_InputName on the owner. If there are parameters, it arranges them into the buffer and calls the event with said buffer
void URPGInputOutputComponent::FireInput(AActor* OutputActor, AActor* Activator, FString InputName, TArray<FIOParameter> IOParamaters)
{
	if (!IsValid(OutputActor)) {
		LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("RPGInputOutputComponent - FireInput - OutputActor is invalid")));
		return;
	}

	// CancelPendingOutputs is handled by component itself, we can do that instantly, without looking for IO_event on the owner
	if (InputName.Equals(CancelPendingInputName, ESearchCase::IgnoreCase)) {
		CancelPendingOutputs();
		return;
	}

	// Find event generated by blueprint to try to process it
	FString FuncName = FString(TEXT("IO_")); FuncName.Append(InputName);
	UFunction* Func = GetOwner()->FindFunction(FName(*FuncName));
	if (!Func) {
		LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("RPGInputOutputComponent - FireInput - Function with name %s not found"), *FuncName));
		return;
	}

	// If event has no parameters, we skip creating param buffer and instantly trigger the function
	if (Func->ParmsSize == 0) {
		GetOwner()->ProcessEvent(Func, nullptr);
		return;
	}

	// We allocate and zero the memory for parameters
	uint8* Parms = (uint8*)FMemory_Alloca_Aligned(Func->ParmsSize, Func->GetMinAlignment());
	FMemory::Memzero(Parms, Func->ParmsSize);
	
	// We initialize values for all params excluding return param (there shouldn't be any, but just to be sure)
	for (TFieldIterator<FProperty> It(Func); It && It->HasAnyPropertyFlags(CPF_Parm); ++It) {
		It->InitializeValue_InContainer(Parms);
	}

	bool bParamsMatch = true;
	int32 IOIndex = 0;

	// Fill param buffer with values of parameters from FIOParameters
	for (TFieldIterator<FProperty> It(Func); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		FProperty* Prop = *It;
		if (Prop->HasAnyPropertyFlags(CPF_ReturnParm)) continue;

		if (!IOParamaters.IsValidIndex(IOIndex)) {
			LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("URPGInputOutputComponent - FireInput - %s expects more parameters than configured (%d)"),
				*FuncName, IOParamaters.Num()));
			bParamsMatch = false;
			break;
		}

		FIOParameter& Param = IOParamaters[IOIndex];
		if (Param.ParamName != Prop->GetName()) {
			LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("URPGInputOutputComponent - FireInput - %s - param %d name mismatch ('%s' vs '%s') - signature changed after configuration?"),
				*FuncName, IOIndex, *Param.ParamName, *Prop->GetName()));
		}

		// Update Activator parameter with value passed by ProcessOutputNode()
		if (CastField<FObjectProperty>(Prop) && Prop->GetName().ToLower() == FString(TEXT("activator")) && Param.ParamType == EIOParamType::Actor) {
			Param.ActorValue = Activator;
		}

		if (!WriteIOParamToParams(Prop, Parms, Param)) {
			LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("URPGInputOutputComponent - FireInput - %s: type mismatch on parameter '%s'"),
				*FuncName, *Prop->GetName()));
			bParamsMatch = false;
			break;
		}
		IOIndex++;
	}

	// Trigger custom event with parameters
	if (bParamsMatch) {
		GetOwner()->ProcessEvent(Func, Parms);
	}

	// Need to destroy values in the buffer — FString in parameters may leak
	for (TFieldIterator<FProperty> It(Func); It && It->HasAnyPropertyFlags(CPF_Parm); ++It) {
		It->DestroyValue_InContainer(Parms);
	}
}

// Processes Output node - triggers FireInput and deletes the node from Processing Queue. Returns false if OutputNode couldn't be processed due to delay
bool URPGInputOutputComponent::ProcessOutputNode(int32 index)
{
	// Output nodes that have Delay > 0 shouldn't be here, and definitely shouldn't be processed
	if (OutputNodesToProcessDelay[index] > 0) {
		LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("RPGInputOutputStructures - ProcessOutputNode - Node Delay is above 0")));
		return false;
	}
	int32 indexNode = OutputNodesToProcess[index];

	if (OutputNodes[indexNode].MaxFireCount != -1 && OutputNodes[indexNode].FireCount >= OutputNodes[indexNode].MaxFireCount) {
#if !UE_BUILD_SHIPPING
		if (CVarIOSystemLogDiscardedEvents.GetValueOnGameThread() > 0)
			LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("URPGInputOutputComponent - ProcessOutputNode - Output Node %s discarded due to Fire Count (%d) exceeding Max Fire Count (%d)"),
				*OutputNodeToString(OutputNodes[indexNode]), OutputNodes[indexNode].FireCount, OutputNodes[indexNode].MaxFireCount));
#endif
		OutputNodesToProcess.RemoveAt(index);
		OutputNodesToProcessDelay.RemoveAt(index);
		OutputNodesToProcessActivators.RemoveAt(index);
		return true;
	}
	if (!IsValid(OutputNodes[indexNode].OutputActor)) {
		LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("RPGInputOutputStructures - ProcessOutputNode - OutputActor is invalid")));
		OutputNodesToProcess.RemoveAt(index);
		OutputNodesToProcessDelay.RemoveAt(index);
		OutputNodesToProcessActivators.RemoveAt(index);
		return true;
	}

	// Properly change Target and TargetIOComp if TargetType is Activator
	if (OutputNodes[indexNode].TargetType == EIOTargetType::Activator) {
		OutputNodes[indexNode].Target = OutputNodesToProcessActivators[index];
		if (!IsValid(OutputNodes[indexNode].Target)) {
			LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("RPGInputOutputStructures - ProcessOutputNode - Target is not valid")));
			OutputNodesToProcess.RemoveAt(index);
			OutputNodesToProcessDelay.RemoveAt(index);
			OutputNodesToProcessActivators.RemoveAt(index);
			return true;
		}
		OutputNodes[indexNode].TargetIOComp = OutputNodes[indexNode].Target->GetComponentByClass<URPGInputOutputComponent>();
	}

	if (OutputNodes[indexNode].TargetType == EIOTargetType::Actor && !IsValid(OutputNodes[indexNode].Target)) {
		LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("RPGInputOutputStructures - ProcessOutputNode - Target is not valid")));
		OutputNodesToProcess.RemoveAt(index);
		OutputNodesToProcessDelay.RemoveAt(index);
		OutputNodesToProcessActivators.RemoveAt(index);
		return true;
	}
	if (OutputNodes[indexNode].TargetInput.IsEmpty() || OutputNodes[indexNode].TargetInput.ToLower().Equals(TEXT("<none>"))) {
		LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("RPGInputOutputStructures - ProcessOutputNode - TargetInput is not set")));
		OutputNodesToProcess.RemoveAt(index);
		OutputNodesToProcessDelay.RemoveAt(index);
		OutputNodesToProcessActivators.RemoveAt(index);
		return true;
	}
	if (OutputNodes[indexNode].TargetType != EIOTargetType::Self && !IsValid(OutputNodes[indexNode].TargetIOComp)) {
		LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("RPGInputOutputStructures - ProcessOutputNode - TargetIOComp is invalid")));
		OutputNodesToProcess.RemoveAt(index);
		OutputNodesToProcessDelay.RemoveAt(index);
		OutputNodesToProcessActivators.RemoveAt(index);
		return true;
	}

	// Cache activator before it gets removed
	AActor* Activator = OutputNodesToProcessActivators[index];

	// Remove the nodes before firing input, as it might conflict with save system
	// (for example we may fire Save Input that will save the state of OutputNodesToProcess, and we'll have unnecessary Save after loading the game)
	OutputNodesToProcess.RemoveAt(index);
	OutputNodesToProcessDelay.RemoveAt(index);
	OutputNodesToProcessActivators.RemoveAt(index);
	OutputNodes[indexNode].FireCount++;

	// Trigger FireInput on proper InputOutputComponent
	// Nothing after FireInput may touch OutputNodesToProcess or use index - CancelPendingOutputs may have been called on this component
	if (OutputNodes[indexNode].TargetType == EIOTargetType::Self) {
		FireInput(OutputNodes[indexNode].OutputActor, Activator,
			OutputNodes[indexNode].TargetInput, OutputNodes[indexNode].InputParameters);
	}
	else {
		OutputNodes[indexNode].TargetIOComp->FireInput(OutputNodes[indexNode].OutputActor, Activator,
			OutputNodes[indexNode].TargetInput, OutputNodes[indexNode].InputParameters);
	}
	

	return true;
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

	// Updates Target, TargetIOComp, TargetInput and InputParameters after changing Target Type
	if (MemberName == GET_MEMBER_NAME_CHECKED(URPGInputOutputComponent, OutputNodes)
		&& PropertyName == GET_MEMBER_NAME_CHECKED(FOutputNode, TargetType))
	{
		int32 Id = PropertyChangedEvent.GetArrayIndex(TEXT("OutputNodes"));
		if (OutputNodes.IsValidIndex(Id))
		{
			FOutputNode& Node = OutputNodes[Id];
			if (Node.TargetType == EIOTargetType::Self) {
				Node.Target = GetOwner();
				Node.TargetIOComp = Node.Target->GetComponentByClass<URPGInputOutputComponent>();
			}
			else if (Node.TargetType == EIOTargetType::Activator) {
				Node.Target = nullptr;
				Node.TargetIOComp = nullptr;

			}
			Node.TargetInput = TEXT("<none>");
			Node.InputParameters.Reset();
		}
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	// At this point, we're not interested in reacting to changes to any properties other than TargetInput
	if (MemberName != GET_MEMBER_NAME_CHECKED(URPGInputOutputComponent, OutputNodes)
		|| PropertyName != GET_MEMBER_NAME_CHECKED(FOutputNode, TargetInput)) {
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	int32 Index = PropertyChangedEvent.GetArrayIndex(TEXT("OutputNodes"));

	if (!OutputNodes.IsValidIndex(Index)) {
		LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("URPGInputOutputComponent::PostEditChangeChainProperty - Improper index detected")));
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	// We have to make sure that vaid output always has proper OutputActor set
	OutputNodes[Index].OutputActor = GetOwner();

	// If no actor is set, we reset input apramaters and return early
	if (OutputNodes[Index].TargetType == EIOTargetType::Actor && !IsValid(OutputNodes[Index].Target)) {
		OutputNodes[Index].InputParameters.Reset();
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	// If no TargetInput is set, we reset input paramaters and return early
	const FString& TargetInput = OutputNodes[Index].TargetInput;
	if (TargetInput.IsEmpty() || TargetInput.Equals(TEXT("<none>"), ESearchCase::IgnoreCase)) {
		OutputNodes[Index].InputParameters.Reset();
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	// CancelPending has no IO_ event on the target
	if (TargetInput.Equals(CancelPendingInputName, ESearchCase::IgnoreCase)) {
		OutputNodes[Index].InputParameters.Reset();
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	if (OutputNodes[Index].TargetType != EIOTargetType::Activator && OutputNodes[Index].Target == nullptr) {
		LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("URPGInputOutputComponent::PostEditChangeChainProperty - No Target set despite changing TargetInput")));
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	if (OutputNodes[Index].TargetType != EIOTargetType::Activator) {
		auto* IOComp = OutputNodes[Index].Target->GetComponentByClass<URPGInputOutputComponent>();
		if (!IsValid(IOComp)) {
			LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("URPGInputOutputComponent::PostEditChangeChainProperty - No InputOutputComponent found despite Target being valid")));
			Super::PostEditChangeChainProperty(PropertyChangedEvent);
			return;
		}
	}

	if (OutputNodes[Index].TargetType == EIOTargetType::Activator) {
		OutputNodes[Index].InputParameters.Reset();
		auto ActorInputs = GetAllActorInputs();

		if(auto* Params = ActorInputs.Find(FName(*TargetInput))) {
			OutputNodes[Index].InputParameters = *Params;
		}

		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	auto TargetInputWithPrefix = FString(TEXT("IO_")).Append(TargetInput);
	UFunction* Func = OutputNodes[Index].Target->FindFunction(FName(*TargetInputWithPrefix));

	if (Func == nullptr) {
		LoggingFunctionLibrary::PrintError(this, FString::Printf(TEXT("URPGInputOutputComponent::PostEditChangeChainProperty - No input matching TargetInput was found")));
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
		return;
	}

	// We configure InputParameters based on found custom event
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
	if (GetOwner()) {
		return GetInputOptionsForClass(GetOwner()->GetClass());
	}
	return GetInputOptionsForClass(GetTypedOuter<UClass>());
}

TArray<FString> URPGInputOutputComponent::GetInputOptionsForClass(const UClass* Class)
{
	TArray<FString> Result;
	if (!Class) return Result;

	for (TFieldIterator<UFunction> It(Class); It; ++It)
	{
		if (It->GetName().StartsWith(TEXT("IO_")))
		{
			FString InputName = It->GetName().RightChop(3);
			// IO_CancelPending should be shadowed by bult-in function (BeginPlay reports it) - don't list it twice
			if (InputName.Equals(CancelPendingInputName, ESearchCase::IgnoreCase)) continue;
			Result.Add(InputName);
		}
	}

	// Add built-in CancelPending function to the inputs - it should be available on every IO component
	Result.Add(FString(CancelPendingInputName));
	return Result;
}

TArray<FString> URPGInputOutputComponent::GetInputOptionsWithNoneOption() const
{
	TArray<FString> result = GetInputOptions();
	result.Insert(FString(TEXT("<none>")), 0);
	return result;
}
