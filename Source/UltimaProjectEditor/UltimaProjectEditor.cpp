#include "UltimaProjectEditor.h"
#include "LayoutCustomization/ItemPropertiesCustomization.h"
#include "LayoutCustomization/LiquidDescriptorCustomization.h"
#include "UltimaProject/Items/Common/ItemData.h"
#include "UltimaProject/Items/Containers/ContainerTypes.h"

#define LOCTEXT_NAMESPACE "FUltimaProjectEditorModule"

void FUltimaProjectEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// FLiquidDescriptor
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FLiquidDescriptor::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLiquidDescriptorCustomization::MakeInstance)
	);

	// ItemInstanceData
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FItemProperties::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FItemPropertiesCustomization::MakeInstance)
	);
}

void FUltimaProjectEditorModule::ShutdownModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.UnregisterCustomPropertyTypeLayout(FLiquidDescriptor::StaticStruct()->GetFName());

	PropertyModule.UnregisterCustomPropertyTypeLayout(FItemProperties::StaticStruct()->GetFName());
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUltimaProjectEditorModule, UltimaProjectEditor)
