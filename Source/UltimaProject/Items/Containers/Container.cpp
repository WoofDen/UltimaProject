#include "Container.h"

#include "UltimaProject/Items/Common/ItemFactoryHelper.h"

bool FContainerItemData::operator==(const FContainerItemData& Other) const
{
	return ItemData == Other.ItemData;
}

bool UContainer::FindDropTransform(FContainerItemData& Item, FTransform& Result) const
{
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		Result.SetLocation(Pawn->GetNavAgentLocation());
		return true;
	}

	// TODO for chests or etc traces will be needed
	return false;;
}

void UContainer::SetItemsCapacity(const int64 NewValue)
{
	ItemsCapacity = NewValue;
}

void UContainer::SetWeightCapacity(const int64 NewValue)
{
	WeightCapacity = NewValue;
}

bool UContainer::AddItem(FContainerItemData& ItemData, FItemTransactionResult& Result)
{
	if (ensureAlways(ItemData.Container == nullptr))
	{
		ItemData.Container = this;

		Items.Add(ItemData);
		return true;
	}

	return false;
}

bool UContainer::AddItem(FContainerItemData& ItemData)
{
	FItemTransactionResult Result;
	return AddItem(ItemData, Result);
}

bool UContainer::MoveItem(FContainerItemData& ItemData, FItemTransactionResult& Result)
{
	if (ItemData.Container.IsValid())
	{
		if (ItemData.Container == this)
		{
			return true;
		}

		ItemData.Container->RemoveItem(ItemData);
	}

	return AddItem(ItemData, Result);
}

bool UContainer::RemoveItem(FContainerItemData& ItemData)
{
	if (Items.RemoveSingle(ItemData) > 0)
	{
		ItemData.Container = nullptr;
	}

	return true;
}

bool UContainer::GetRandomItem(FContainerItemData& Item) const
{
	if (Items.IsEmpty())
	{
		return false;
	}
	Item = Items[FMath::RandRange(0, Items.Num() - 1)];

	return true;
}

AItem* UContainer::DropItem(FContainerItemData& Item)
{
	FTransform Transform;
	if (!FindDropTransform(Item, Transform))
	{
		return nullptr;
	}

	if (RemoveItem(Item))
	{
		if (AItem* Result = UItemFactoryHelper::SpawnItem(GetWorld(), Item.ItemData, Transform))
		{
			return Result;
		}
		
		// If failed to spawn item, return UItemData back to container
		AddItem(Item);
	}

	return nullptr;
}

TArray<FContainerItemData> UContainer::GetItems()
{
	return Items;
}
