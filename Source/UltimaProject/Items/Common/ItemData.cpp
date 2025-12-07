// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemData.h"

#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

FItemInstanceData::FItemInstanceData()
{
	Amount = 1;
}

UItemData::UItemData()
{
}

UItemData::UItemData(FObjectInitializer& Initializer)
	: Super(Initializer)
{
}

void UItemData::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UItemData, StaticData, COND_InitialOnly);

	{
		FDoRepLifetimeParams Params;
		Params.bIsPushBased = true;
		DOREPLIFETIME_WITH_PARAMS_FAST(UItemData, InstanceData, Params);
	}
}

bool UItemData::IsSupportedForNetworking() const
{
	return true;
}

bool UItemData::Initialize(UItemData* Source /* = nullptr */)
{
	check(GetClass());
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

TSoftObjectPtr<const UItemDataAsset> UItemData::GetStaticData() const
{
	return StaticData;
}

void UItemData::SetStaticData(const UItemDataAsset* InStaticData)
{
	check(!StaticData.IsValid());
	StaticData = InStaticData;
}

const FItemInstanceData& UItemData::GetInstanceData() const
{
	return InstanceData;
}

TSubclassOf<AItem> UItemData::GetActorClass() const
{
	return StaticData->ActorClass;
}

int32 UItemData::GetStackableAmount(const UItemData* TargetItem) const
{
	if (!TargetItem || StaticData != TargetItem->StaticData)
	{
		return 0;
	}

	const int32 FreeItemCount = TargetItem->GetMaxAmountPerStack() - TargetItem->GetAmount();
	if (FreeItemCount == 0)
	{
		return 0;
	}

	return FMath::Min(GetAmount(), FreeItemCount);
}

UItemData* UItemData::SplitItem(const int64 SplitAmount)
{
	if (SplitAmount <= 0 || SplitAmount >= GetAmount())
	{
		return nullptr;
	}

	UItemData* NewItemData = DuplicateObject<UItemData>(this, GetOuter());
	if (!ensureAlways(NewItemData))
	{
		return nullptr;
	}

	const int64 NewAmount = GetAmount() - SplitAmount;
	SetAmount(NewAmount);
	NewItemData->SetAmount(SplitAmount);
	return NewItemData;
}

FText UItemData::GetDisplayName() const
{
	static FText Unnamed = FText::FromString(TEXT("Unnamed"));
	if (StaticData.IsValid())
	{
		return StaticData->Name;
	}

	return Unnamed;
}

UTexture2D* UItemData::GetViewIcon() const
{
	if (!ensureAlways(StaticData.IsValid()))
	{
		return nullptr;
	}

	return StaticData->Icon.LoadSynchronous();
}

int64 UItemData::GetAmount() const
{
	return InstanceData.Amount;
}

int64 UItemData::GetMaxAmountPerStack() const
{
	return StaticData->MaxAmountPerStack;
}

int64 UItemData::SetAmount(const int64 Value)
{
	ensureAlways(Value >= 0);
	if (Value != InstanceData.Amount)
	{
		InstanceData.Amount = FMath::Min(StaticData->MaxAmountPerStack, Value);
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InstanceData, this);
	}

	return InstanceData.Amount;
}

int64 UItemData::ModifyAmount(const int64 Value)
{
	if (Value != 0)
	{
		const int32 NewAmount = InstanceData.Amount + Value;
		InstanceData.Amount = FMath::Min(StaticData->MaxAmountPerStack, FMath::Max(0, NewAmount));

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InstanceData, this);
	}

	return InstanceData.Amount;
}

TSoftObjectPtr<UStaticMesh> UItemData::GetStaticMesh() const
{
	if (StaticData)
	{
		return StaticData->WorldMesh;
	}

	return nullptr;
}
