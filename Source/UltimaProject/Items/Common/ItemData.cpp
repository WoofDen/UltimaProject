// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemData.h"

#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

FItemInstanceData::FItemInstanceData()
{
	Amount = 0; // Invalid item marker
}

bool FItemInstanceData::IsValid() const
{
	return Amount >= 0;
}

FItemData::FItemData()
{
}


FItemData FItemData::EmptyItem = FItemData(FItemDataDefinition(nullptr, FItemInstanceData()));

bool FItemData::PreInitialize(FItemData* Source /* = nullptr */)
{
	check(StaticData.IsValid());

	if (Source)
	{
		StaticData = Source->StaticData;
		InstanceData = Source->InstanceData;
	}

	// Do not allow invalid UItemData
	check(StaticData.IsValid());

	StaticData->Icon.LoadSynchronous();
	StaticData->WorldMesh.LoadSynchronous();
	return true;
}

bool FItemData::PreInitialize(const FItemDataDefinition& Definition)
{
	StaticData = Definition.StaticData;
	InstanceData = Definition.InstanceData;

	// Do not allow invalid UItemData
	check(StaticData.IsValid() || this == &FItemData::EmptyItem);

	return true;
}

FItemDataDefinition FItemData::GetDataDefinition() const
{
	const FItemData* This = this;
	return FItemDataDefinition(*This);
}

TSoftObjectPtr<const UItemDataAsset> FItemData::GetStaticData() const
{
	return StaticData;
}

void FItemData::SetStaticData(const UItemDataAsset* InStaticData)
{
	check(!StaticData.IsValid());
	StaticData = InStaticData;
}

const FItemInstanceData& FItemData::GetInstanceData() const
{
	return InstanceData;
}

TSubclassOf<AItem> FItemData::GetActorClass() const
{
	return StaticData->ActorClass;
}

bool FItemData::IsValid() const
{
	return StaticData && InstanceData.IsValid();
}

int32 FItemData::GetStackableAmount(const FItemData& TargetItem) const
{
	ensureAlways(TargetItem.IsValid());
	if (StaticData != TargetItem.StaticData)
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
	checkNoEntry();
}

FItemDataDefinition::FItemDataDefinition(const FItemData& Item)
	: FItemDataDefinition(Item.GetStaticData(), Item.GetInstanceData())

{
}

FItemDataDefinition::FItemDataDefinition(TSoftObjectPtr<const UItemDataAsset> StaticDataIn, FItemInstanceData InstanceDataIn)
{
	StaticData = StaticDataIn;
	InstanceData = InstanceDataIn;
}


FItemData::FItemData(const FItemDataDefinition& Definition)
{
	PreInitialize(Definition);
}

FText FItemData::GetDisplayName() const
{
	static FText Unnamed = FText::FromString(TEXT("Unnamed"));
	if (StaticData.IsValid())
	{
		return StaticData->Name;
	}

	return Unnamed;
}

UTexture2D* FItemData::GetViewIcon() const
{
	if (!ensureAlways(StaticData.IsValid()))
	{
		return nullptr;
	}

	return StaticData->Icon.LoadSynchronous();
}

uint32 FItemData::GetAmount() const
{
	return StaticCast<uint32>(InstanceData.Amount);
}

uint32 FItemData::GetMaxAmountPerStack() const
{
	return StaticCast<uint32>(StaticData->MaxAmountPerStack);
}

uint32 FItemData::SetAmount(const uint32 Value)
{
	if (Value != InstanceData.Amount)
	{
		const int32 Value32 = StaticCast<int32>(Value);
		InstanceData.Amount = FMath::Min(StaticData->MaxAmountPerStack, Value32);
		// MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InstanceData, this);
	}

	return InstanceData.Amount;
}

uint32 FItemData::ModifyAmount(const int32 Value)
{
	if (Value != 0)
	{
		const int32 NewAmount = InstanceData.Amount + Value;
		InstanceData.Amount = FMath::Min(StaticData->MaxAmountPerStack, FMath::Max(0, NewAmount));

		// MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InstanceData, this);
	}

	return StaticCast<uint32>(InstanceData.Amount);
}

TSoftObjectPtr<UStaticMesh> FItemData::GetStaticMesh() const
{
	if (StaticData)
	{
		return StaticData->WorldMesh;
	}

	return nullptr;
}
