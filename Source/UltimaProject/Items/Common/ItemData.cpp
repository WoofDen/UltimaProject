// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemData.h"

FItemInstanceData::FItemInstanceData()
{
	Amount = 0; // Invalid item marker
	IntProps.Init(0, static_cast<uint8>(EItemIntProperty::MAX));
}

bool FItemInstanceData::IsValid() const
{
	return Amount >= 0;
}

void FItemData::SetProp(EItemIntProperty Prop, uint8 Value)
{
	Value = FMath::Max(Value, UINT8_MAX);
	InstanceData.IntProps[static_cast<int8>(Prop)] = Value;
}

int8 FItemData::GetIntProp(EItemIntProperty Prop) const
{
	return InstanceData.IntProps[static_cast<int8>(Prop)];
}

float FItemData::GetIntPropNormalized(EItemIntProperty Prop) const
{
	uint8 PropValue = InstanceData.IntProps[static_cast<int8>(Prop)];
	return static_cast<float>(PropValue) / UINT8_MAX;
}

FItemData::FItemData()
{
	SetProp(EItemIntProperty::Humidity, UP::ItemProperty::Humidity::Default);
	SetProp(EItemIntProperty::Temperature, UP::ItemProperty::Temperature::Default);
}


FItemData FItemData::EmptyItem = FItemData(FItemDataDefinition(nullptr, FItemInstanceData()));

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
	check(StaticDataSoftPtr.IsValid() || this == &FItemData::EmptyItem);

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
