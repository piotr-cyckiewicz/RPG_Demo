// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

/**
 * 
 */
class RPG_DEMOEDITOR_API FOutputNodeCustomization : public IPropertyTypeCustomization
{

public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<FOutputNodeCustomization>(); }

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> Handle,
		FDetailWidgetRow& Row, IPropertyTypeCustomizationUtils&) override;

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> Handle,
		IDetailChildrenBuilder& Builder, IPropertyTypeCustomizationUtils&) override;


private:
	void RefreshTarget();
	void RefreshOptions();
	void OnInputPicked(TSharedPtr<FString> Item, ESelectInfo::Type);
	FText GetCurrentInputText() const;

	TSharedPtr<IPropertyHandle> OutputActorHandle, TargetTypeHandle, TargetHandle, TargetInputHandle;
	TArray<TSharedPtr<FString>> Options;
};
