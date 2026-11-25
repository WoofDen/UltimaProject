// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemData.h"
#include "Item.h"

FItemInstanceData::FItemInstanceData()
{
	Amount = 1;
}

FItemInstanceData::FItemInstanceData(uint32 InAmount)
	: FItemInstanceData()
{
	Amount = InAmount;
}

bool FItemInstanceData::IsValid() const
{
	return Amount >= 0;
}

UItemDataAsset::UItemDataAsset()
{
	DefaultProperties.PropertyKeys.Add(EItemNumericProperty::Durability);
	DefaultProperties.PropertyValues.Add(100.f);
}

void FItemData::SetNumericProp(EItemNumericProperty Prop, float Value)
{
	InstanceData.Properties.PropertyValues.SetNum(InstanceData.Properties.PropertyKeys.Num());
	if (int32 Idx = InstanceData.Properties.PropertyKeys.Find(Prop); Idx != INDEX_NONE)
	{
		InstanceData.Properties.PropertyValues[Idx] = Value;
	}
	else
	{
		InstanceData.Properties.PropertyKeys.Add(Prop);
		InstanceData.Properties.PropertyValues.Add(Value);
	}
}

float FItemData::GetNumericProp(EItemNumericProperty Prop, float DefaultValue) const
{
	if (int32 Idx = InstanceData.Properties.PropertyKeys.Find(Prop); Idx != INDEX_NONE)
	{
		return InstanceData.Properties.PropertyValues.IsValidIndex(Idx) ? InstanceData.Properties.PropertyValues[Idx] : DefaultValue;
	}

	return DefaultValue;
}

bool FItemData::IsSupportProp(EItemNumericProperty Prop) const
{
	if (const UItemDataAsset* ItemDataAsset = GetStaticData())
	{
		return ItemDataAsset->DefaultProperties.PropertyKeys.Contains(Prop);
	}

	return false;
}

void FItemData::InitializeDefaultProperties()
{
	const UItemDataAsset* ItemDataAsset = GetStaticData();
	if (!ItemDataAsset)
	{
		return;
	}

	const FItemProperties& StaticDefaults = ItemDataAsset->DefaultProperties;
	
	FItemProperties& CurrentProperties = InstanceData.Properties;
	
	// Ensure values num == keys num
	const int32 ExistingPropertyKeysNum = CurrentProperties.PropertyKeys.Num();
	CurrentProperties.PropertyValues.SetNum(ExistingPropertyKeysNum);

	// Merge default properties into item data if its empty
	for (int32 Index = 0; Index < StaticDefaults.PropertyKeys.Num(); ++Index)
	{
		const EItemNumericProperty Property = StaticDefaults.PropertyKeys[Index];
		const int32 ExistingIndex = CurrentProperties.PropertyKeys.Find(Property);

		if (ExistingIndex == INDEX_NONE || ExistingIndex >= ExistingPropertyKeysNum)
		{
			const float DefaultValue = StaticDefaults.PropertyValues.IsValidIndex(Index) ? StaticDefaults.PropertyValues[Index] : 0.f;
			SetNumericProp(Property, DefaultValue);
		}
	}

	if (CurrentProperties.SolidSubstance == FLiquidDescriptor{})
	{
		CurrentProperties.SolidSubstance = StaticDefaults.SolidSubstance;
	}
}

FItemData::FItemData()
{
	InstanceData.Amount = 1;
}

FItemData FItemData::InvalidItemData = FItemData(FItemDataDefinition(nullptr, FItemInstanceData(0)));

bool FItemData::PreInitialize(FItemData* Source /* = nullptr */)
{
	check(StaticDataSoftPtr.IsValid());

	if (Source)
	{
		StaticDataSoftPtr = Source->StaticDataSoftPtr;
		InstanceData = Source->InstanceData;
	}

	StaticDataSoftPtr->Icon.LoadSynchronous();
	StaticDataSoftPtr->WorldMesh.LoadSynchronous();
	return true;
}

bool FItemData::PreInitialize(const FItemDataDefinition& Definition)
{
	StaticDataSoftPtr = Definition.StaticDataSoftPtr;
	InstanceData = Definition.InstanceData;

	// Do not allow invalid UItemData
	check(StaticDataSoftPtr.IsValid() || this == &FItemData::InvalidItemData);

	return true;
}

FItemDataDefinition FItemData::GetDataDefinition() const
{
	const FItemData* This = this;
	return FItemDataDefinition(*This);
}

const UItemDataAsset* FItemData::GetStaticData() const
{
	if (!StaticData)
	{
		StaticData = StaticDataSoftPtr.LoadSynchronous();
	}

	return StaticData;
}

bool FItemData::Is(const TSoftObjectPtr<UItemDataAsset>& DataAssetSoftPtr) const
{
	return DataAssetSoftPtr == StaticDataSoftPtr;
}

void FItemData::SetStaticData(const UItemDataAsset* InStaticData)
{
	ensureAlways(::IsValid(InStaticData));

	StaticDataSoftPtr = InStaticData;
	LoadStaticData();
}

void FItemData::LoadStaticData()
{
	if (!::IsValid(StaticData))
	{
		StaticData = StaticDataSoftPtr.LoadSynchronous();
	}
}

const FItemInstanceData& FItemData::GetInstanceData() const
{
	return InstanceData;
}

TSubclassOf<AItem> FItemData::GetActorClass() const
{
	return GetStaticData()->ActorClass;
}

bool FItemData::IsValid() const
{
	return StaticDataSoftPtr.IsValid() && InstanceData.IsValid();
}

int32 FItemData::GetStackableAmount(const FItemData& TargetItem) const
{
	ensureAlways(TargetItem.IsValid());
	if (StaticDataSoftPtr != TargetItem.StaticDataSoftPtr)
	{
		return 0;
	}

	const uint32 FreeItemCount = TargetItem.GetMaxAmountPerStack() - TargetItem.GetAmount();
	if (FreeItemCount == 0)
	{
		return 0;
	}

	return FMath::Min(GetAmount(), FreeItemCount);
}

FItemDataDefinition::FItemDataDefinition()
{
}

FItemDataDefinition::FItemDataDefinition(const FItemData& Item)
	: FItemDataDefinition(Item.GetStaticData(), Item.GetInstanceData())

{
}

FItemDataDefinition::FItemDataDefinition(TSoftObjectPtr<const UItemDataAsset> StaticDataIn,
                                         FItemInstanceData InstanceDataIn)
{
	StaticDataSoftPtr = StaticDataIn;
	InstanceData = InstanceDataIn;
}


FItemData::FItemData(const FItemDataDefinition& Definition)
{
	PreInitialize(Definition);
}

FText FItemData::GetDisplayName() const
{
	static FText Unnamed = FText::FromString(TEXT("Unnamed"));
	if (StaticDataSoftPtr.IsValid())
	{
		return GetStaticData()->Name;
	}

	return Unnamed;
}

UObject* FItemData::GetViewIcon() const
{
	if (!ensureAlways(StaticDataSoftPtr.IsValid()))
	{
		return nullptr;
	}

	return GetStaticData()->Icon.LoadSynchronous();
}

uint32 FItemData::GetAmount() const
{
	return StaticCast<uint32>(InstanceData.Amount);
}

uint32 FItemData::GetMaxAmountPerStack() const
{
	return StaticCast<uint32>(GetStaticData()->MaxAmountPerStack);
}

uint32 FItemData::SetAmount(const uint32 Value)
{
	if (Value != InstanceData.Amount)
	{
		const int32 Value32 = StaticCast<int32>(Value);
		InstanceData.Amount = FMath::Min(GetStaticData()->MaxAmountPerStack, Value32);
		// MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InstanceData, this);
	}

	return InstanceData.Amount;
}

uint32 FItemData::ModifyAmount(const int32 Value)
{
	if (Value != 0)
	{
		const int32 NewAmount = InstanceData.Amount + Value;
		InstanceData.Amount = FMath::Min(GetStaticData()->MaxAmountPerStack, FMath::Max(0, NewAmount));

		// MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InstanceData, this);
	}

	return StaticCast<uint32>(InstanceData.Amount);
}

TSoftObjectPtr<UStaticMesh> FItemData::GetStaticMesh() const
{
	return GetStaticData()->WorldMesh;
}
