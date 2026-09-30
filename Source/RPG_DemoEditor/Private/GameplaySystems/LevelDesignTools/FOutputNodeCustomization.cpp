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
#include "Utilities/LoggingFunctionLibrary.h"

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

    // Changing TargetType or Target changes the set of available inputs, so options have to be rebuilt
    TargetTypeHandle->SetOnPropertyValueChanged(
        FSimpleDelegate::CreateSP(this, &FOutputNodeCustomization::RefreshOptions));
    TargetHandle->SetOnPropertyValueChanged(
        FSimpleDelegate::CreateSP(this, &FOutputNodeCustomization::RefreshOptions));

    // Initial fill of options, so the combo box has proper content as soon as the panel is displayed
    RefreshOptions();

    
    // Overriding CustomizeChildren means every member of FOutputNode has to be added manually - otherwise it wouldn't be displayed at all
    uint32 Num; Handle->GetNumChildren(Num);
    for (uint32 i = 0; i < Num; ++i)
    {
        TSharedRef<IPropertyHandle> Child = Handle->GetChildHandle(i).ToSharedRef();
        const FName Name = Child->GetProperty()->GetFName();

        // TargetInput gets a searchable combo box instead of a plain text field.
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
        // Remaining members are displayed with default widgets
        else
        {
            Builder.AddProperty(Child);
        }
    }
}

void FOutputNodeCustomization::RefreshOptions()
{
    Options.Reset();
    UObject* TargetObj = nullptr;
    uint8 RawTargetType = 0;
    TargetHandle->GetValue(TargetObj);

    // Without TargetType we can't tell where inputs come from (for example, when several nodes with different TargetType are selected)
    if (TargetTypeHandle->GetValue(RawTargetType) != FPropertyAccess::Result::Success) {
        return;
    }


    AActor* Actor = Cast<AActor>(TargetObj);

    EIOTargetType TargetType = static_cast<EIOTargetType>(RawTargetType);


    if (Actor && TargetType != EIOTargetType::Activator) {
        for (const FName& In : URPGInputOutputComponent::GetActorInputs(Actor)) {
            Options.Add(MakeShared<FString>(In.ToString()));
        }
    }
    
    // Activator - the actual target is known only at runtime, so inputs of all blueprints with InputOutputComponent are listed.
    // THIS IS VERY HEAVY! GetAllActorInputs scans asset registry and loads blueprint classes
    else if (TargetType == EIOTargetType::Activator) {
        for (const FName& In : URPGInputOutputComponent::GetAllActorInputs())
            Options.Add(MakeShared<FString>(In.ToString()));
    }


    if (TargetInputComboBox) {
        // Notify combo box that its OptionsSource has changed
        TargetInputComboBox->RefreshOptions();
        FString inpt = GetCurrentInputText().ToString();
        bool inptFound = false;
        for (TSharedPtr<FString> option : Options) {
            if (option->Equals(inpt)) {
                inptFound = true;
                break;
            }
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