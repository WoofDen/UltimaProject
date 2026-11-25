#if WITH_DEV_AUTOMATION_TESTS

#include "UltimaProject/Items/Common/ItemData.h"
#include "UltimaProject/Items/Common/Item.h"
#include "UltimaProject/Items/Common/ItemFactoryHelper.h"
#include "UltimaProject/Items/Chest/ChestBase.h"
#include "UltimaProject/Items/Common/ItemsFunctionLibrary.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "UltimaProject/Items/Containers/Components/ExternalContainerComponent.h"
#include "UltimaProject/Items/Containers/Components/ProxyContainerComponent.h"
#include "UltimaProject/Framework/UPPlayerController.h"
#include "Components/BoxComponent.h"
#include "EngineUtils.h"
#include "Materials/Material.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FItemNumericPropertyTest, "UltimaProject.Items.NumericProperties", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemNumericPropertyTest::RunTest(const FString& Parameters)
{
	FItemData Item;
	Item.SetNumericProp(EItemNumericProperty::Humidity, -2.5f);
	TestTrue(TEXT("Stores the raw float value"), FMath::IsNearlyEqual(Item.GetNumericProp(EItemNumericProperty::Humidity), -2.5f));
	TestTrue(TEXT("Returns the supplied default for a missing property"), FMath::IsNearlyEqual(Item.GetNumericProp(EItemNumericProperty::Temperature, 23.75f), 23.75f));

	Item.SetNumericProp(EItemNumericProperty::Humidity, 4.25f);
	TestTrue(TEXT("Replaces an existing numeric property"), FMath::IsNearlyEqual(Item.GetNumericProp(EItemNumericProperty::Humidity), 4.25f));
	TestEqual(TEXT("Replacement keeps one key"), Item.GetInstanceData().Properties.PropertyKeys.Num(), 1);
	TestEqual(TEXT("Replacement keeps one value"), Item.GetInstanceData().Properties.PropertyValues.Num(), 1);

	UItemDataAsset* ItemDataAsset = NewObject<UItemDataAsset>();
	ItemDataAsset->DefaultProperties.PropertyKeys = {EItemNumericProperty::Humidity, EItemNumericProperty::Temperature, EItemNumericProperty::Durability, EItemNumericProperty::Grow};
	ItemDataAsset->DefaultProperties.PropertyValues = {0.25f, 23.5f, 0.5f};
	ItemDataAsset->DefaultProperties.SolidSubstance.Type = ESubstance::Wart;
	Item.SetStaticData(ItemDataAsset);
	TestTrue(TEXT("The item has valid static data"), Item.IsValid());
	Item.InitializeDefaultProperties();
	TestTrue(TEXT("An existing value is not replaced by a default"), FMath::IsNearlyEqual(Item.GetNumericProp(EItemNumericProperty::Humidity), 4.25f));
	TestTrue(TEXT("A missing value receives its static default"), FMath::IsNearlyEqual(Item.GetNumericProp(EItemNumericProperty::Temperature), 23.5f));
	TestTrue(TEXT("Another missing value receives its static default"), FMath::IsNearlyEqual(Item.GetNumericProp(EItemNumericProperty::Durability), 0.5f));
	TestTrue(TEXT("A key without a value defaults to zero"), FMath::IsNearlyEqual(Item.GetNumericProp(EItemNumericProperty::Grow, -1.f), 0.f));
	TestTrue(TEXT("A default key marks a property as supported"), Item.IsSupportProp(EItemNumericProperty::Humidity));
	TestFalse(TEXT("An absent default key is unsupported"), Item.IsSupportProp(EItemNumericProperty::Decay));
	TestEqual(TEXT("An existing nonnumeric asset default is retained"), Item.GetInstanceData().Properties.SolidSubstance.Type, ESubstance::Wart);
	Item.InitializeDefaultProperties();
	TestTrue(TEXT("Applying defaults repeatedly is idempotent"), FMath::IsNearlyEqual(Item.GetNumericProp(EItemNumericProperty::Durability), 0.5f));

	FItemDataDefinition IncompleteDefinition = Item.GetDataDefinition();
	IncompleteDefinition.InstanceData.Properties.PropertyKeys = {EItemNumericProperty::Humidity, EItemNumericProperty::Temperature, EItemNumericProperty::Durability};
	IncompleteDefinition.InstanceData.Properties.PropertyValues = {0.f};
	FItemData IncompleteItem(IncompleteDefinition);
	IncompleteItem.InitializeDefaultProperties();
	TestEqual(TEXT("Explicit zero is preserved"), IncompleteItem.GetNumericProp(EItemNumericProperty::Humidity), 0.f);
	TestEqual(TEXT("A missing instance value receives its default"), IncompleteItem.GetNumericProp(EItemNumericProperty::Temperature), 23.5f);
	TestEqual(TEXT("Multiple missing instance values receive their defaults"), IncompleteItem.GetNumericProp(EItemNumericProperty::Durability), 0.5f);

	UItemsFunctionLibrary::SetItemProp(Item, EItemNumericProperty::Humidity, -3.5f);
	TestTrue(TEXT("Blueprint getter returns the numeric value"), FMath::IsNearlyEqual(UItemsFunctionLibrary::GetItemProp(Item, EItemNumericProperty::Humidity), -3.5f));
	UItemsFunctionLibrary::AddItemProp(Item, EItemNumericProperty::Humidity, 5.25f);
	TestTrue(TEXT("Blueprint addition preserves unrestricted float values"), FMath::IsNearlyEqual(UItemsFunctionLibrary::GetItemProp(Item, EItemNumericProperty::Humidity), 1.75f));

	return true;
}

#endif