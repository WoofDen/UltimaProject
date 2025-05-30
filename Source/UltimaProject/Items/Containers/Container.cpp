﻿#include "Container.h"

#include "DerivedMeshDataTaskUtils.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "UltimaProject/Items/Common/ItemFactoryHelper.h"

DEFINE_LOG_CATEGORY(LogContainers)

bool FContainerItemData::operator==(const FContainerItemData& Other) const
{
	return ItemData == Other.ItemData;
}

FContainerItemData::FContainerItemData(UItemData* InitData, const UContainer* InitContainer, int64 InitSlotIndex)
{
	ItemData = InitData;
	Container = InitContainer;
	SlotIndex = InitSlotIndex;
}

FContainerItemData::FContainerItemData()
{
}

int64 UContainer::GetStoredSlotsCount() const
{
	// todo cache values?
	int64 Result = 0;
	for (const auto& Item : Items)
	{
		Result += Item.ItemData->GetStaticData()->Slots;
	}

	return Result;
}

void UContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool UContainer::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (auto& Item : Items)
		WroteSomething |= Channel->ReplicateSubobject(Item.ItemData, *Bunch, *RepFlags);

	return WroteSomething;
}

bool UContainer::FindDropTransform(const UItemData* ItemData, FTransform& Result) const
{
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		Result.SetLocation(Pawn->GetNavAgentLocation());
		return true;
	}

	// TODO for chests or etc traces will be needed
	return false;
}

void UContainer::OnRep_Items()
{
	ensureAlways(GetOwner() && !GetOwner()->HasAuthority());
	OnContainerItemsChanged.Broadcast();
}

void UContainer::SetItemsCapacity(const int64 NewValue)
{
	ensureAlways(ItemSlotsCapacity <= NewValue); // shrinking not implemented yet
	ItemSlotsCapacity = NewValue;
}

FItemTransactionResult UContainer::AddItem(UItemData* ItemData)
{
	ensureAlways(GetOwner() && GetOwner()->HasAuthority());

	// todo should we?
	// ItemData->GetStaticData().LoadSynchronous();

	const int64 SlotIndex = GetStoredSlotsCount();
	ensureAlways(SlotIndex < ItemSlotsCapacity);
	// todo ensure get item at slot == null

	FContainerItemData ContainerItemData(ItemData, this, SlotIndex);
	Items.Add(ContainerItemData);

	return GItemTransactionResult_Success;
}

FItemTransactionResult UContainer::SplitItem(const FContainerItemData& Data, const int64 SplitAmount)
{
	if (!ensureAlways(HasItem(Data)))
	{
		return GItemTransactionResult_Error;
	}

	// we need at least one extra slot
	if (Items.Num() >= ItemSlotsCapacity)
	{
		return GItemTransactionResult_Capacity;
	}

	UItemData* NewItemData = Data.ItemData->SplitItem(SplitAmount);
	if (!NewItemData)
	{
		return GItemTransactionResult_Error;
	}

	FItemTransactionResult Result = AddItem(NewItemData);
	if(!Result.IsSuccess())
	{
		// todo
		UE_LOG(LogContainers, Error, TEXT("Unable to add item to container during split operation - the origin item amount has been reduced"));
		NewItemData->MarkAsGarbage();
	}

	return Result;
}

FItemTransactionResult UContainer::MoveItem(const FContainerItemData& Item)
{
	if (Items.Num() >= ItemSlotsCapacity)
	{
		return GItemTransactionResult_Capacity;
	}

	check(GetOwner()->HasAuthority());

	ensureAlways(GetOwner() && GetOwner()->HasAuthority());

	// FContainerItemData should always have a container
	if (!ensureAlways(Item.Container.IsValid()))
	{
		return GItemTransactionResult_Error;
	}

	if (Item.Container == this)
	{
		ensure(false);
		return GItemTransactionResult_Success;
	}

	return AddItem(Item.ItemData);
}

FItemTransactionResult UContainer::MoveItem(FContainerItemData& Item, AItem* OutItem)
{
	check(GetOwner()->HasAuthority());

	FTransform Transform;
	if (!FindDropTransform(Item.ItemData, Transform))
	{
		return GItemTransactionResult_Error;
	}

	if (RemoveItem(Item)) // remove item from container
	{
		if (AItem* Result = UItemFactoryHelper::SpawnItem(GetWorld(), Item.ItemData, Transform))
		{
			OutItem = Result;
			//OnRep_Items();
			return GItemTransactionResult_Success;
		}

		// If failed to spawn item, return UItemData back to container
		AddItem(Item.ItemData);
	}

	return GItemTransactionResult_Error;
}

FItemTransactionResult UContainer::MoveItem(AItem* WorldItem)
{
	if (Items.Num() == ItemSlotsCapacity)
	{
		return GItemTransactionResult_Capacity;
	}

	check(GetOwner()->HasAuthority());
	if (AddItem(WorldItem->GetItemData()).IsSuccess())
	{
		// Reference on UItemData will be stored in the container now
		// So it's safe to destroy actor from the world now
		WorldItem->RemoveFromWorld();

		return GItemTransactionResult_Success;
	}

	return GItemTransactionResult_Success;
}

bool UContainer::RemoveItem(FContainerItemData& ItemData)
{
	ensureAlways(GetOwner() && GetOwner()->HasAuthority());

	if (Items.RemoveSingle(ItemData) > 0)
	{
		ItemData.Container = nullptr;
	}

	return true;
}

TArray<FContainerItemData> UContainer::GetItems()
{
	return Items;
}
