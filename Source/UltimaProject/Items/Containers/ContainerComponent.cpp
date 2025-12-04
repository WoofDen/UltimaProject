// Game includes
#include "ContainerComponent.h"
#include "UltimaProject/UI/ContainerWidget.h"
#include <UltimaProject/Items/Common/ItemFactoryHelper.h>

// Engine includes
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

DEFINE_LOG_CATEGORY(LogUPContainers)

bool FContainerItemData::operator==(const FContainerItemData& Other) const
{
	return ItemData == Other.ItemData;
}

FContainerItemData::FContainerItemData(UItemData* InitData, UContainerComponent* InitContainer, int32 InitSlotIndex)
{
	ItemData = InitData;
	ItemData = InitData;
	Container = InitContainer;
	SlotIndex = InitSlotIndex;
}

FContainerItemData::FContainerItemData()
{
}

void FContainerItems::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	// Propagate changes to client
	if (UContainerComponent* Container = ContainerComponent.Get())
	{
		for (int32 Index : AddedIndices)
		{
			Container->NotifyContainerItemChanged(Items[Index]);
		}

		Container->NotifyContainerItemsChanged();
	}
}

void FContainerItems::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	// Propagate changes to client
	if (UContainerComponent* Container = ContainerComponent.Get())
	{
		for (int32 Index : ChangedIndices)
		{
			Container->NotifyContainerItemChanged(Items[Index]);
		}

		Container->NotifyContainerItemsChanged();
	}
}

void FContainerItems::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	// Propagate changes to client
	if (UContainerComponent* Container = ContainerComponent.Get())
	{
		Container->NotifyContainerItemsChanged();
	}
}

int32 UContainerComponent::GetStoredSlotsCount() const
{
	// todo cache values?
	int32 Result = 0;
	for (const auto& Item : ContainerItems.Items)
	{
		Result += Item.ItemData->GetStaticData()->Slots;
	}

	return Result;
}

void UContainerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	{
		FDoRepLifetimeParams Params;
		Params.bIsPushBased = true;
		Params.Condition = COND_OwnerOnly;

		DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ContainerItems, Params);
	}
}

bool UContainerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// FContainerItemData::ItemData is not replicated by default as a struct member
	for (auto& Item : ContainerItems.Items)
	{
		WroteSomething |= Channel->ReplicateSubobject(Item.ItemData, *Bunch, *RepFlags);
	}

	return WroteSomething;
}

void UContainerComponent::BeginPlay()
{
	Super::BeginPlay();
	ContainerItems.ContainerComponent = this;
}

void UContainerComponent::InitializeContainerWidget()
{
	if (!IsValid(ContainerWidgetClass))
	{
		UE_LOG(LogUPContainers, Error, TEXT("ContainerWidgetClass is not set for %s/%s"), *GetNameSafe(this),
		       *GetNameSafe(GetOwner()));
		return;
	}

	if (IsValid(ContainerWidget))
	{
		ContainerWidget->RemoveFromParent();
	}

	ContainerWidget = CreateWidget<UContainerWidget>(GetWorld(), ContainerWidgetClass);

	if (ContainerWidget)
	{
		ContainerWidget->Initialize(this);
	}
}

bool UContainerComponent::FindDropTransform(const UItemData* ItemData, FTransform& Result) const
{
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		Result.SetLocation(Pawn->GetNavAgentLocation());
		return true;
	}

	// TODO for chests or etc traces will be needed
	return false;
}

void UContainerComponent::OnRep_Items()
{
	ensureAlways(GetOwner() && !GetOwner()->HasAuthority());
	NotifyContainerItemsChanged();
}

void UContainerComponent::NotifyContainerItemsChanged_Implementation()
{
	// Mark Items as dirty for replication
	if (AActor* Owner = GetOwner(); Owner && Owner->HasAuthority())
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ContainerItems, this);
		Owner->ForceNetUpdate();
	}

	OnContainerItemsChanged.Broadcast();
}

void UContainerComponent::NotifyContainerItemChanged_Implementation(const FContainerItemData& Item)
{
	OnContainerItemChanged.Broadcast(Item);
}

int32 UContainerComponent::GetItemsCapacity() const
{
	return ItemSlotsCapacity;
}

void UContainerComponent::SetItemsCapacity(const int32 NewValue)
{
	ensureAlways(ItemSlotsCapacity <= NewValue); // shrinking not implemented yet
	ItemSlotsCapacity = NewValue;
}

int32 UContainerComponent::GetItemCapacity() const
{
	return ItemSlotsCapacity;
}

void UContainerComponent::DisplayContainerWidget()
{
	// TODO global hud and displayed containers var
	if (!IsValid(ContainerWidget))
	{
		InitializeContainerWidget();

		if (!IsValid(ContainerWidget))
		{
			UE_LOG(LogUPContainers, Error, TEXT("Failed to create ContainerWidget for %s/%s"), *GetNameSafe(this),
			       *GetNameSafe(GetOwner()));
			return;
		}
	}

	ContainerWidget->AddToViewport();
}

bool UContainerComponent::HasItem(const FContainerItemData& ItemData) const
{
	return ContainerItems.Items.Contains(ItemData);
}

FItemTransactionResult UContainerComponent::AddItem(UItemData* ItemData)
{
	FContainerItemData AddedItem;
	return AddItem(ItemData, AddedItem);
}

FItemTransactionResult UContainerComponent::AddItem(UItemData* ItemData, FContainerItemData& AddedItem)
{
	ensureAlways(GetOwner() && GetOwner()->HasAuthority());

	// todo should we?
	// ItemData->GetStaticData().LoadSynchronous();

	const int32 SlotIndex = GetStoredSlotsCount();
	ensureAlways(SlotIndex < ItemSlotsCapacity);
	// todo ensure get item at slot == null

	FContainerItemData ContainerItemData(ItemData, this, SlotIndex);
	AddedItem = ContainerItems.Items.Add_GetRef(MoveTemp(ContainerItemData));

	ContainerItems.MarkItemDirty(AddedItem);

	return GItemTransactionResult_Success;
}

FItemTransactionResult UContainerComponent::SplitItem(FContainerItemData& Data, const int32 SplitAmount)
{
	if (!ensureAlways(HasItem(Data)))
	{
		return GItemTransactionResult_Error;
	}

	// we need at least one extra slot
	if (ContainerItems.Items.Num() >= ItemSlotsCapacity)
	{
		return GItemTransactionResult_Capacity;
	}

	UItemData* NewItemData = Data.ItemData->SplitItem(SplitAmount);
	if (!NewItemData)
	{
		return GItemTransactionResult_Error;
	}

	// The origin has been changed
	ContainerItems.MarkItemDirty(Data);

	FItemTransactionResult Result = AddItem(NewItemData);
	if (!Result.IsSuccess())
	{
		// todo
		UE_LOG(LogUPContainers, Error,
		       TEXT("Unable to add item to container during split operation - the origin item amount has been reduced"
		       ));
		NewItemData->MarkAsGarbage();
	}

	return Result;
}

FItemTransactionResult UContainerComponent::MoveItem(FContainerItemData& SourceItem)
{
	// Container->Container move
	check(GetOwner()->HasAuthority());

	// FContainerItemData should always have a container
	if (!ensureAlways(SourceItem.Container.IsValid()))
	{
		return GItemTransactionResult_Error;
	}

	if (SourceItem.Container == this)
	{
		ensure(false);
		return GItemTransactionResult_Success;
	}

	UItemData* SourceItemData = SourceItem.ItemData;
	if (!SourceItemData)
	{
		return GItemTransactionResult_Error;
	}

	// Try to stack into existing items
	int32 MovedAmount = 0;
	for (FContainerItemData& Item : ContainerItems.Items)
	{
		int32 StackableAmount = SourceItemData->GetStackableAmount(Item.ItemData);

		Item.ItemData->ModifyAmount(StackableAmount);
		SourceItemData->ModifyAmount(-StackableAmount);

		ContainerItems.MarkItemDirty(SourceItem);
		SourceItem.Container->ContainerItems.MarkItemDirty(SourceItem);

		NotifyContainerItemChanged(Item);

		MovedAmount += StackableAmount;

		if (SourceItemData->GetAmount() == 0)
		{
			break;
		}
	}

	// If the item amount is 0, remove the item
	if (SourceItemData->GetAmount() == 0)
	{
		UContainerComponent* SourceItemContainer = SourceItem.Container.Get();
		SourceItemContainer->RemoveItem(SourceItem);
	}

	FItemTransactionResult Result;
	if (MovedAmount > 0)
	{
		NotifyContainerItemsChanged();

		Result += SourceItemData->GetAmount() == 0
			          ? EItemTransactionResultCode::FullyMoved
			          : EItemTransactionResultCode::PartiallyMoved;
	}

	// some amount of item has left, check for the free slot
	if (ContainerItems.Items.Num() >= ItemSlotsCapacity)
	{
		Result += EItemTransactionResultCode::NotEnoughCapacity;
		return Result;
	}

	return AddItem(SourceItem.ItemData);
}

FItemTransactionResult UContainerComponent::MoveItem(FContainerItemData& Item, AItem* OutItem)
{
	// Container->World

	check(GetOwner()->HasAuthority());

	FTransform Transform;
	if (!FindDropTransform(Item.ItemData, Transform))
	{
		return GItemTransactionResult_Error;
	}

	if (RemoveItem(Item)) // remove item from the container
	{
		if (AItem* Result = UItemFactoryHelper::SpawnItem(GetWorld(), Item.ItemData, Transform))
		{
			NotifyContainerItemsChanged();

			OutItem = Result;
			return GItemTransactionResult_Success;
		}

		// If failed to spawn item, return UItemData back to container
		AddItem(Item.ItemData);
	}

	return GItemTransactionResult_Error;
}

FItemTransactionResult UContainerComponent::MoveItem(AItem* WorldItem)
{
	// World->Container move. 
	check(GetOwner()->HasAuthority());

	if (!ensureAlways(IsValid(WorldItem)))
	{
		return GItemTransactionResult_Error;
	}

	UItemData* WorldItemData = WorldItem->GetItemData();
	if (!ensureAlways(WorldItemData))
	{
		return GItemTransactionResult_Error;
	}

	// Iterate all items in the container and move as many items as possible
	int32 MovedAmount = 0;
	for (FContainerItemData& Item : ContainerItems.Items)
	{
		const int32 StackableAmount = WorldItemData->GetStackableAmount(Item.ItemData);

		WorldItemData->ModifyAmount(-StackableAmount);
		Item.ItemData->ModifyAmount(StackableAmount);

		ContainerItems.MarkItemDirty(Item);

		NotifyContainerItemChanged(Item);

		MovedAmount += StackableAmount;

		if (WorldItemData->GetAmount() == 0)
		{
			break;
		}
	}

	FItemTransactionResult Result;
	if (WorldItemData->GetAmount() == 0)
	{
		// Item is completely stacked into a new container
		WorldItem->RemoveFromWorld();
		Result += EItemTransactionResultCode::FullyMoved;
	}
	else
	{
		// Try to move the rest into the container as a new item
		if (MovedAmount > 0)
		{
			Result += EItemTransactionResultCode::PartiallyMoved;
		}

		// We need an extra slot in the container to place what left
		if (ContainerItems.Items.Num() == ItemSlotsCapacity)
		{
			Result += EItemTransactionResultCode::NotEnoughCapacity;
		}
		// Add a container item to the container
		else if (AddItem(WorldItem->GetItemData()).IsSuccess())
		{
			// Reference on UItemData will be stored in the container now
			// So, it's safe to destroy the actor from the world now
			WorldItem->RemoveFromWorld();
		}
	}

	if (MovedAmount > 0)
	{
		NotifyContainerItemsChanged();
	}

	return Result;
}

bool UContainerComponent::RemoveItem(FContainerItemData& ItemData)
{
	ensureAlways(GetOwner() && GetOwner()->HasAuthority());

	if (ContainerItems.Items.RemoveSingle(ItemData) > 0)
	{
		ItemData.Container = nullptr;

		ContainerItems.MarkArrayDirty();
	}

	return true;
}

TArray<FContainerItemData> UContainerComponent::GetItems()
{
	return ContainerItems.Items;
}
