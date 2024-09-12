// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemData.h"

FItemInstanceData::FItemInstanceData()
{
	Amount = 1;
}

UItemData::UItemData()
{
}

bool UItemData::Initialize(UItemData* Source /* = nullptr */)
{
	check(GetClass());
	
	Source = Source ? Source : GetClass()->GetDefaultObject<UItemData>();
	
	StaticData = Source->StaticData;
	InstanceData = Source->InstanceData;
	
	StaticData->Icon.LoadSynchronous();
	StaticData->WorldMesh.LoadSynchronous();
	return true;
}

TObjectPtr<const UItemDataAsset> UItemData::GetStaticData() const
{
	return StaticData;
}

UTexture2D* UItemData::GetViewIcon() const
{
	return StaticData->Icon.Get();
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
	check(Value >= 0);
	InstanceData.Amount = FMath::Max(StaticData->MaxAmountPerStack, Value);
	return InstanceData.Amount;
}
