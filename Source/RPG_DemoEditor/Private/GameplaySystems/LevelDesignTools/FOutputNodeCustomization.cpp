// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplaySystems/LevelDesignTools/FOutputNodeCustomization.h"
#include "GameplaySystems/LevelDesignTools/RPGInputOutputStructures.h"
#include "GameplaySystems/LevelDesignTools/RPGInputOutputComponent.h"
#include "Editor/PropertyEditor/Public/DetailWidgetRow.h"
#include "Editor/PropertyEditor/Public/PropertyHandle.h"
#include "Editor/PropertyEditor/Public/IDetailChildrenBuilder.h"
#include "Runtime/Core/Public/Misc/AssertionMacros.h"
#include "Runtime/SlateCore/Public/Widgets/DeclarativeSyntaxSupport.h"
#include "Developer/ToolWidgets/Public/SSearchableComboBox.h"

void FOutputNodeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> Handle, FDetailWidgetRow& Row, IPropertyTypeCustomizationUtils&)
{
	Row.NameContent()[ Handle->CreatePropertyNameWidget() ];
	
}

void FOutputNodeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> Handle, IDetailChildrenBuilder& Builder, IPropertyTypeCustomizationUtils&)
{
    MainHandle = Handle;
    TargetTypeHandle = Handle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOutputNode, TargetType));
    TargetHandle = Handle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOutputNode, Target));
    TargetInputHandle = Handle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOutputNode, TargetInput));

    TargetTypeHandle->SetOnPropertyValueChanged(
        FSimpleDelegate::CreateSP(this, &FOutputNodeCustomization::RefreshTarget));
    RefreshTarget();

    TargetHandle->SetOnPropertyValueChanged(
        FSimpleDelegate::CreateSP(this, &FOutputNodeCustomization::CacheTargetIOComp));
    TargetHandle->SetOnPropertyValueChanged(
        FSimpleDelegate::CreateSP(this, &FOutputNodeCustomization::RefreshOptions));
    CacheTargetIOComp();
    RefreshOptions();

    

    uint32 Num; Handle->GetNumChildren(Num);
    for (uint32 i = 0; i < Num; ++i)
    {
        TSharedRef<IPropertyHandle> Child = Handle->GetChildHandle(i).ToSharedRef();
        const FName Name = Child->GetProperty()->GetFName();

        if (Name == GET_MEMBER_NAME_CHECKED(FOutputNode, TargetInput))
        {
            Builder.AddProperty(Child).CustomWidget()
                .NameContent()[Child->CreatePropertyNameWidget()]
                .ValueContent().MinDesiredWidth(200.f)
                [
                    SAssignNew(TargetInputComboBox, SSearchableComboBox)
                        .OptionsSource(&Options)
                        .OnGenerateWidget_Lambda([](TSharedPtr<FString> In)
                            { return SNew(STextBlock).Text(FText::FromString(*In)); })
                        .OnSelectionChanged(this, &FOutputNodeCustomization::OnInputPicked)
                        .Content()
                        [
                            SNew(STextBlock).Text(this, &FOutputNodeCustomization::GetCurrentInputText)
                        ]
                ];
        }
        else
        {
            Builder.AddProperty(Child);
        }
    }
}

void FOutputNodeCustomization::RefreshTarget()
{
    uint8 RawTargetType = 0;
    if (TargetTypeHandle->GetValue(RawTargetType) != FPropertyAccess::Result::Success) {
        return;
    }
    EIOTargetType TargetType = static_cast<EIOTargetType>(RawTargetType);
    if (TargetType == EIOTargetType::Activator) {
        TargetHandle->SetValue((AActor*)nullptr);
    }
    else if (TargetType == EIOTargetType::Self) {
        TArray<UObject*> Objects;
        TargetHandle->GetOuterObjects(Objects);
        for (UObject* Obj : Objects) {
            URPGInputOutputComponent* IOComp = Cast<URPGInputOutputComponent>(Obj);
            if (IOComp) {
                int32 index = MainHandle->GetIndexInArray();
                IOComp->UpdateOutputActor(index);
                break;
            }
        }
    }
}

void FOutputNodeCustomization::CacheTargetIOComp()
{
    UObject* TargetObj = nullptr;
    TargetHandle->GetValue(TargetObj);
    AActor* Actor = Cast<AActor>(TargetObj);

    if (IsValid(Actor)) {
        URPGInputOutputComponent* TargetIOComp = Actor->GetComponentByClass<URPGInputOutputComponent>();
        if (TargetIOComp) {
            TArray<UObject*> Objects;
            TargetHandle->GetOuterObjects(Objects);
            for (UObject* Obj : Objects) {
                URPGInputOutputComponent* IOComp = Cast<URPGInputOutputComponent>(Obj);
                if (IOComp) {
                    int32 index = MainHandle->GetIndexInArray();
                    IOComp->OutputNodes[index].TargetIOComp = TargetIOComp;
                    break;
                }
            }
        }
    }
}

void FOutputNodeCustomization::RefreshOptions()
{
    Options.Reset();
    UObject* TargetObj = nullptr;
    uint8 RawTargetType = 0;
    TargetHandle->GetValue(TargetObj);
    if (TargetTypeHandle->GetValue(RawTargetType) != FPropertyAccess::Result::Success) {
        return;
    }


    AActor* Actor = Cast<AActor>(TargetObj);

    EIOTargetType TargetType = static_cast<EIOTargetType>(RawTargetType);
    if (Actor && TargetType != EIOTargetType::Activator) {
        UE_LOG(LogTemp, Display, TEXT("RefreshOptions Check 3"));
        for (const FName& In : URPGInputOutputComponent::GetActorInputs(Actor)) {
            Options.Add(MakeShared<FString>(In.ToString()));
        }
    }
        
    else if (TargetType == EIOTargetType::Activator) {
        for (const FName& In : URPGInputOutputComponent::GetAllActorInputs())
            Options.Add(MakeShared<FString>(In.ToString()));
    }


    if (TargetInputComboBox) {
        TargetInputComboBox->RefreshOptions();
        FString inpt = GetCurrentInputText().ToString();
        bool inptFound = false;
        for (TSharedPtr<FString> option : Options) {
            if (option->Equals(inpt)) {
                inptFound = true;
                break;
            }
        }
        if (!inptFound) {
            TargetInputHandle->SetValue(FName("<none>"));
        }
    }
}

void FOutputNodeCustomization::OnInputPicked(TSharedPtr<FString> Item, ESelectInfo::Type)
{
    if (Item.IsValid())
        TargetInputHandle->SetValue(FName(**Item));
}

FText FOutputNodeCustomization::GetCurrentInputText() const
{
    FString Cur; TargetInputHandle->GetValue(Cur);
    FName CurName = FName(Cur);
    return FText::FromString(CurName.IsNone() ? TEXT("<none>") : CurName.ToString());
}