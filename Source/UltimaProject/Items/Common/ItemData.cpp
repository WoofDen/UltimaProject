// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemData.h"

#include "Net/UnrealNetwork.h"

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

	DOREPLIFETIME_CONDITION(UItemData, StaticData, COND_OwnerOnly);
}

bool UItemData::IsSupportedForNetworking() const
{
	return true;
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

TSoftObjectPtr<const UItemDataAsset> UItemData::GetStaticData() const
{
	return StaticData;
}

const FItemInstanceData& UItemData::GetInstanceData() const
{
	return InstanceData;
}

TSubclassOf<AItem> UItemData::GetActorClass() const
{
	return ActorClass;
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
	InstanceData.Amount = FMath::Max(StaticData->MaxAmountPerStack, Value);
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
