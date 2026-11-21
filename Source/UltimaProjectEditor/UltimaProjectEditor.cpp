#include "UltimaProjectEditor.h"
#include "LayoutCustomization/LiquidDescriptorCustomization.h"
#include "UltimaProject/Items/Containers/ContainerTypes.h"

#define LOCTEXT_NAMESPACE "FUltimaProjectEditorModule"

void FUltimaProjectEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomPropertyTypeLayout(FLiquidDescriptor::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLiquidDescriptorCustomization::MakeInstance));
}

void FUltimaProjectEditorModule::ShutdownModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	PropertyModule.UnregisterCustomPropertyTypeLayout(FLiquidDescriptor::StaticStruct()->GetFName());
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUltimaProjectEditorModule, UltimaProjectEditor)
