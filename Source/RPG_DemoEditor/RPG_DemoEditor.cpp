// Fill out your copyright notice in the Description page of Project Settings.

#include "RPG_DemoEditor.h"
#include "GameplaySystems/LevelDesignTools/RPGInputOutputStructures.h"
#include "GameplaySystems/LevelDesignTools/FOutputNodeCustomization.h"

#define LOCTEXT_NAMESPACE "FRPG_DemoEditorModule"

void FRPG_DemoEditorModule::StartupModule() {
	auto& PM = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PM.RegisterCustomPropertyTypeLayout(FOutputNode::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FOutputNodeCustomization::MakeInstance));
	PM.NotifyCustomizationModuleChanged();
}

void FRPG_DemoEditorModule::ShutdownModule() {

}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FRPG_DemoEditorModule, RPG_DemoEditor);
