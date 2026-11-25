// Fill out your copyright notice in the Description page of Project Settings.

#include "ContainerItem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/UI/ContainerWidget.h"

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

void UContainerItem::BuildPropertyIcons(const FItemData& ItemData)
{
	const FItemProperties& Properties = ItemData.GetInstanceData().Properties;
	for (const EItemNumericProperty Property : Properties.PropertyKeys)
	{
		if (!PropertyIcons.Contains(Property))
		{
			continue;
		}

		const float PropertyValue = ItemData.GetNumericProp(Property);

		// Find index within the ranges array
		const FItemPropIconSettings& StatusIconSettings = PropertyIcons[Property];

		for (int32 i = 0; i < StatusIconSettings.PropertyRanges.Num(); i++)
		{
			const FVector2f& PropertyRange = StatusIconSettings.PropertyRanges[i];

			bool bMatches;
			if (i == StatusIconSettings.PropertyRanges.Num() - 1)
			{
				bMatches = PropertyValue >= PropertyRange.GetMin() && PropertyValue < PropertyRange.GetMax();
			}
			else
			{
				// Include Max. value if this is the last element
				bMatches = PropertyValue >= PropertyRange.GetMin() && PropertyValue <= PropertyRange.GetMax();
			}

			if (bMatches)
			{
				if (StatusIconSettings.Icons.IsValidIndex(i))
				{
					AddStatusIcon(StatusIconSettings.Icons[i]);
				}
			}
		}
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

		const FItemData& ItemData = Item.GetItemData();
		BuildPropertyIcons(ItemData);
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
