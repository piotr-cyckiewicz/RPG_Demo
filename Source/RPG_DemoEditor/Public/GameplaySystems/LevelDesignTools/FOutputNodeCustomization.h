// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

/**
 * Details panel customization for FOutputNode (registered in FRPG_DemoEditorModule::StartupModule).
 * Replaces the plain TargetInput text field with a searchable combo box listing inputs (IO_ events)
 * available on the current Target, so inputs are picked from a list instead of being typed by hand.
 * All other FOutputNode properties are displayed with their default widgets.
 * A separate instance is created for every displayed FOutputNode (e.g. every element of OutputNodes array).
 */

class RPG_DEMOEDITOR_API FOutputNodeCustomization : public IPropertyTypeCustomization
{

public:
	// Factory passed to PropertyEditor module when registering the customization
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<FOutputNodeCustomization>(); }

	// Builds the header row of the node - only the property name is displayed (value part is left empty)
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> Handle,
		FDetailWidgetRow& Row, IPropertyTypeCustomizationUtils&) override;

	// Builds rows for all members of the node - TargetInput gets a searchable combo box, the rest use default widgets
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> Handle,
		IDetailChildrenBuilder& Builder, IPropertyTypeCustomizationUtils&) override;


private:
	// Rebuilds the list of available inputs based on current TargetType and Target. Called whenever either of them changes
	void RefreshOptions();

	// Called when an input is picked in the combo box - writes it to TargetInput
	void OnInputPicked(TSharedPtr<FString> Item, ESelectInfo::Type);

	// Returns text displayed in the closed combo box - current TargetInput value, or "<none>" if it's not set
	FText GetCurrentInputText() const;



	// Handles to the customized FOutputNode and to the members that the customization reads or writes
	TSharedPtr<IPropertyHandle> MainHandle, TargetTypeHandle, TargetHandle, TargetInputHandle;

	// Combo box replacing the default TargetInput widget - stored so its options can be refreshed after Target changes
	TSharedPtr<class SSearchableComboBox> TargetInputComboBox;

	// Options source for TargetInputComboBox. The combo box keeps a pointer to this array, so it has to live as long as the widget
	TArray<TSharedPtr<FString>> Options;
};
