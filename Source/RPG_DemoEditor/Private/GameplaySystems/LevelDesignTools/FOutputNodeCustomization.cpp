// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplaySystems/LevelDesignTools/RPGInputOutputStructures.h"
#include "GameplaySystems/LevelDesignTools/RPGInputOutputComponent.h"
#include "GameplaySystems/LevelDesignTools/FOutputNodeCustomization.h"
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
    TargetTypeHandle = Handle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOutputNode, TargetType));
    TargetHandle = Handle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOutputNode, Target));
    TargetInputHandle = Handle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOutputNode, TargetInput));
    OutputActorHandle = Handle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOutputNode, OutputActor));

    TargetTypeHandle->SetOnPropertyValueChanged(
        FSimpleDelegate::CreateSP(this, &FOutputNodeCustomization::RefreshTarget));
    RefreshTarget();

    TargetHandle->SetOnPropertyValueChanged(
        FSimpleDelegate::CreateSP(this, &FOutputNodeCustomization::RefreshOptions));
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
                    SNew(SSearchableComboBox)
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
    UObject* TargetTypeObj = nullptr;
    TargetTypeHandle->GetValue(TargetTypeObj);
    EIOTargetType TargetType = *CastChecked<EIOTargetType>(TargetTypeObj);
    if (TargetType == EIOTargetType::Activator) {
        TargetHandle->SetValue((AActor*)nullptr);
    }
    else if (TargetType == EIOTargetType::Self) {
        UObject* OutputActor; OutputActorHandle->GetValue(OutputActor);
        TargetHandle->SetValue(CastChecked<AActor>(OutputActor));
    }
}

void FOutputNodeCustomization::RefreshOptions()
{
    Options.Reset();
    UObject* TargetObj = nullptr;
    UObject* TargetTypeObj = nullptr;
    TargetHandle->GetValue(TargetObj);
    TargetTypeHandle->GetValue(TargetTypeObj);
    AActor* Actor = Cast<AActor>(TargetObj);
    EIOTargetType TargetType = *CastChecked<EIOTargetType>(TargetTypeObj);
    if (Actor && TargetType != EIOTargetType::Activator)
        for (const FName& In : URPGInputOutputComponent::GetActorInputs(Actor))
            Options.Add(MakeShared<FString>(In.ToString()));
    else if(TargetType == EIOTargetType::Activator)
        for (const FName& In : URPGInputOutputComponent::GetAllActorInputs())
            Options.Add(MakeShared<FString>(In.ToString()));
}

void FOutputNodeCustomization::OnInputPicked(TSharedPtr<FString> Item, ESelectInfo::Type)
{
    if (Item.IsValid())
        TargetInputHandle->SetValue(FName(**Item));
}

FText FOutputNodeCustomization::GetCurrentInputText() const
{
    FName Cur; TargetInputHandle->GetValue(Cur);
    return FText::FromString(Cur.IsNone() ? TEXT("<none>") : Cur.ToString());
}