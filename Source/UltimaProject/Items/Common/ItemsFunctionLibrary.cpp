// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemsFunctionLibrary.h"
#include "UltimaProject/Items/Common/Item.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"

float UItemsFunctionLibrary::GetItemProp(const FItemData& ItemData, EItemNumericProperty Property)
{
	return ItemData.GetNumericProp(Property);
}

void UItemsFunctionLibrary::SetItemProp(FItemData& ItemData, EItemNumericProperty Property, float Value)
{
	ensureAlways(ItemData.IsValid());
	check(ItemData.IsSupportProp(Property));

	ItemData.SetNumericProp(Property, Value);
}

void UItemsFunctionLibrary::AddItemProp(FItemData& ItemData, EItemNumericProperty Property, float Value)
{
	ensureAlways(ItemData.IsValid());
	check(ItemData.IsSupportProp(Property));

	SetItemProp(ItemData, Property, GetItemProp(ItemData, Property) + Value);
}

