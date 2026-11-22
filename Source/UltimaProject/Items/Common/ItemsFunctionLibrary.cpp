// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemsFunctionLibrary.h"

float UItemsFunctionLibrary::GetItemProp(const FItemData& ItemData, EItemIntProperty Property)
{
	return ItemData.GetIntPropNormalized(Property);
}

void UItemsFunctionLibrary::SetItemProp(FItemData& ItemData, EItemIntProperty Property, float Value)
{
	const uint8 IntValue = FMath::Lerp<uint8>(0, UINT8_MAX, Value);
	ItemData.SetProp(Property, IntValue);
}

void UItemsFunctionLibrary::AddItemProp(FItemData& ItemData, EItemIntProperty Property, float Value)
{
	SetItemProp(ItemData, Property, GetItemProp(ItemData, Property) + Value);
}
