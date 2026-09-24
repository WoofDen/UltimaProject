// Fill out your copyright notice in the Description page of Project Settings.

#include "ContainerItem.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/UI/ContainerWidget.h"

UObject* UContainerItem::GetTemperatureIcon(uint8 Temperature) const
{
	if (Temperature <= UP::ItemProperty::Temperature::Frozen)
	{
		return IconStatusFrozen;
	}
	if (Temperature <= UP::ItemProperty::Temperature::Cold)
	{
		return IconStatusCold;
	}

	if (Temperature >= UP::ItemProperty::Temperature::Hot)
	{
		return IconStatusHot;
	}
	if (Temperature >= UP::ItemProperty::Temperature::Burning)
	{
		return IconStatusBurning;
	}

	return nullptr;
}

UObject* UContainerItem::GetHumidityIcon(uint8 Humidity) const
{
	if (Humidity >= UP::ItemProperty::Humidity::Wet)
	{
		return IconStatusHumid;
	}
	if (Humidity >= UP::ItemProperty::Humidity::Soaked)
	{
		return IconStatusWet;
	}

	return nullptr;
}

void UContainerItem::AddStatusIcon(UObject* ResourceObject)
{
	NULLCHECK(ResourceObject);
	NULLCHECK(StatusList);

	if (UImage* StatusIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass()))
	{
		StatusList->AddChild(StatusIcon);
		StatusIcon->SetBrush(StatusBrushSettings);
		StatusIcon->SetBrushResourceObject(ResourceObject);
	}
}

void UContainerItem::NativePreConstruct()
{
	Super::NativePreConstruct();

#if WITH_EDITOR
	if (!IsDesignTime())
	{
		ResetItem();
	}
	else
	{
		AddStatusIcon(IconStatusWet);
		AddStatusIcon(IconStatusCold);
		AddStatusIcon(IconStatusDamaged);
	}
#endif
}

void UContainerItem::SetItem(const FContainerItemData& Item)
{
	CurrentItem = Item.GetItemData().GetDataDefinition();
	ContainerItemHandle = Item.GetHandle();

	if (ItemIcon)
	{
		const FItemData& ItemData = Item.GetItemData();
		UObject* IconResourceObject = ItemData.GetViewIcon();

		ItemIcon->SetOpacity(1.f);
		ItemIcon->SetBrushResourceObject(IconResourceObject);
	}

	if (ItemAmount)
	{
		uint32 Amount = Item.GetItemData().GetAmount();
		if (Amount > 1)
		{
			const FText AmountText = FText::FromString(FString::FromInt(Amount));
			ItemAmount->SetText(AmountText);
			ItemAmount->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	if (StatusList)
	{
		StatusList->ClearChildren();

		const uint8 Temperature = Item.GetItemData().GetIntProp(EItemIntProperty::Temperature);
		AddStatusIcon(GetTemperatureIcon(Temperature));

		const uint8 Humidity = Item.GetItemData().GetIntProp(EItemIntProperty::Humidity);
		AddStatusIcon(GetHumidityIcon(Humidity));
	}
}

void UContainerItem::ResetItem()
{
	CurrentItem = FItemDataDefinition();
	ContainerItemHandle = FContainerItemData::InvalidHandle;

	if (ItemIcon)
	{
		ItemIcon->SetBrushResourceObject(nullptr);
		ItemIcon->SetOpacity(0.f);
	}

	if (ItemAmount)
	{
		ItemAmount->SetText(FText());
		ItemAmount->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (StatusList)
	{
		StatusList->ClearChildren();
	}
}
