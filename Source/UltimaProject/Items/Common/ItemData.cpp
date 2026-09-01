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
	return FItemDataDefinition(StaticData, InstanceData);
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

	const int32 FreeItemCount = TargetItem.GetMaxAmountPerStack() - TargetItem.GetAmount();
	if (FreeItemCount == 0)
	{
		return 0;
	}

	return FMath::Min(GetAmount(), FreeItemCount);
}

bool FItemData::SplitItem(const int64 SplitAmount, FItemData& ResultItem)
{
	if (SplitAmount <= 0 || GetAmount() - SplitAmount <= 0)
	{
		// TODO 
		return false;
	}

	ResultItem.StaticData = StaticData;
	ResultItem.InstanceData = InstanceData;

	const int64 NewAmount = GetAmount() - SplitAmount;
	SetAmount(NewAmount);
	ResultItem.SetAmount(SplitAmount);
	return true;
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

int64 FItemData::GetAmount() const
{
	return InstanceData.Amount;
}

int64 FItemData::GetMaxAmountPerStack() const
{
	return StaticData->MaxAmountPerStack;
}

int64 FItemData::SetAmount(const int64 Value)
{
	ensureAlways(Value >= 0);
	if (Value != InstanceData.Amount)
	{
		InstanceData.Amount = FMath::Min(StaticData->MaxAmountPerStack, Value);
		// MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InstanceData, this);
	}

	return InstanceData.Amount;
}

int64 FItemData::ModifyAmount(const int64 Value)
{
	if (Value != 0)
	{
		const int32 NewAmount = InstanceData.Amount + Value;
		InstanceData.Amount = FMath::Min(StaticData->MaxAmountPerStack, FMath::Max(0, NewAmount));

		// MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InstanceData, this);
	}

	return InstanceData.Amount;
}

TSoftObjectPtr<UStaticMesh> FItemData::GetStaticMesh() const
{
	if (StaticData)
	{
		return StaticData->WorldMesh;
	}

	return nullptr;
}
