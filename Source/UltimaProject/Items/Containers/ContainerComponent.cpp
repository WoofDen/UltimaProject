// Game includes
#include "ContainerComponent.h"
#include "UltimaProject/UI/ContainerWidget.h"
#include <UltimaProject/Items/Common/ItemFactoryHelper.h>

// Engine includes
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "UltimaProject/Common/Macro.h"

DEFINE_LOG_CATEGORY(LogUPContainers)

bool FContainerItemData::operator==(const FContainerItemData& Other) const
{
	return Handle == Other.Handle;
}

FContainerItemData::FContainerItemData(FItemData&& InitData, UContainerComponent* InitContainer, int32 InitSlotIndex)
{
	ItemData = InitData;
	Container = InitContainer;
	SlotIndex = InitSlotIndex;
	Handle = Container->GenerateItemHandle();
}

FContainerItemData::FContainerItemData()
{
}

void FContainerItems::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	NULLCHECK_SP(ContainerComponent);

	if (AddedIndices.IsEmpty())
	{
		return;
	}

	// Propagate changes to client
	if (UContainerComponent* ListenContainer = ContainerComponent->GetListenContainer())
	{
		for (int32 Index : AddedIndices)
		{
			ListenContainer->NotifyContainerItemChanged(Items[Index]);
		}

		ListenContainer->NotifyContainerItemsChanged();
	}
}

void FContainerItems::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	NULLCHECK_SP(ContainerComponent);

	if (ChangedIndices.IsEmpty())
	{
		return;
	}

	// Propagate changes to client
	if (UContainerComponent* ListenContainer = ContainerComponent->GetListenContainer())
	{
		for (int32 Index : ChangedIndices)
		{
			ListenContainer->NotifyContainerItemChanged(Items[Index]);
		}

		ListenContainer->NotifyContainerItemsChanged();
	}
}

void FContainerItems::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	NULLCHECK_SP(ContainerComponent);
	if (RemovedIndices.IsEmpty())
	{
		return;
	}

	// Propagate changes to the client
	// The replicated prop. ( ContainerItems ) still contains the removed idx., so postpone the update
	if (UWorld* World = ContainerComponent->GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(
			[WeakContainerPtr = TWeakObjectPtr(ContainerComponent->GetOriginContainer())]
			{
				if (WeakContainerPtr.IsValid())
				{
					WeakContainerPtr->NotifyContainerItemsChanged();
				}
			});
	}
}

uint32 UContainerComponent::GetSlotsInUse() const
{
	// todo cache values?
	int32 Result = 0;
	for (const auto& Item : ContainerItems.Items)
	{
		Result += Item.ItemData.GetStaticData()->Slots;
	}

	return Result;
}

uint32 UContainerComponent::GetSlotsAvailable() const
{
	return ItemSlotsCapacity - GetSlotsInUse();
}

FItemTransactionResult UContainerComponent::AddItem(FItemDataDefinition& ItemDataDefinition)
{
	FItemData NewItem(ItemDataDefinition);
	return AddItem(MoveTemp(NewItem));
}

void UContainerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	{
		FDoRepLifetimeParams Params;
		Params.bIsPushBased = true;
		Params.Condition = COND_None;

		DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ContainerItems, Params);
	}
}

/*
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
*/

void UContainerComponent::BeginPlay()
{
	Super::BeginPlay();
	ContainerItems.ContainerComponent = this;
}

/*
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
		ContainerWidget->SetContainerComponent(this);
	}
}
*/

bool UContainerComponent::FindDropTransform(const FItemData& ItemData, FTransform& Result) const
{
	AActor* Owner = GetOwner();
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		Result.SetLocation(Pawn->GetNavAgentLocation());
		return true;
	}

	// Get player pawn
	// TODO review this later
	UWorld* World = GetWorld();
	NULLCHECK_RETURN(World, false);

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		if (TObjectPtr<APawn> Pawn = PC->GetPawn())
		{
			Result.SetLocation(Pawn->GetNavAgentLocation());
			return true;
		}
	}

	// TODO for chests or etc traces will be needed
	return false;
}

TSubclassOf<UContainerWidget> UContainerComponent::GetContainerWidgetClass() const
{
	return ContainerWidgetClass;
}

void UContainerComponent::SetContainerWidgetClass(TSubclassOf<UContainerWidget> Class)
{
	// Should be initialized before the component registration
	ensureAlways(!IsRegistered());

	ContainerWidgetClass = MoveTemp(Class);
}

void UContainerComponent::NotifyContainerItemsChanged_Implementation()
{
	if (AActor* Owner = GetOwner(); Owner && Owner->HasAuthority())
	{
		Owner->ForceNetUpdate();
	}

	if (UContainerComponent* ListenContainer = GetListenContainer())
	{
		ListenContainer->OnContainerItemsChanged.Broadcast();
	}
}

void UContainerComponent::NotifyContainerItemChanged_Implementation(const FContainerItemData& Item)
{
	if (UContainerComponent* ListenContainer = GetListenContainer())
	{
		ListenContainer->OnContainerItemChanged.Broadcast(Item);
	}
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

/*
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

	if (ContainerWidget->IsInViewport())
	{
		ContainerWidget->RemoveFromParent();
		ContainerWidget = nullptr;
	}
	else
	{
		ContainerWidget->AddToViewport();
	}
}
*/

bool UContainerComponent::HasItem(const FContainerItemData& ItemData) const
{
	return ContainerItems.Items.Contains(ItemData);
}

FItemTransactionResult UContainerComponent::AddItem(FItemData&& ItemData)
{
	FContainerItemData AddedItem;
	return AddItem(MoveTemp(ItemData), AddedItem);
}

FItemTransactionResult UContainerComponent::AddItem(FItemData&& ItemData, FContainerItemData& AddedItem)
{
	ensureAlways(GetOwner() && GetOwner()->HasAuthority());

	// todo should we?
	// ItemData->GetStaticData().LoadSynchronous();

	const int32 SlotIndex = GetSlotsInUse();
	ensureAlways(SlotIndex < ItemSlotsCapacity);
	// todo ensure get item at slot == null

	FContainerItemData ContainerItemData(MoveTemp(ItemData), this, SlotIndex);
	AddedItem = ContainerItems.Items.Add_GetRef(MoveTemp(ContainerItemData));

	ContainerItems.MarkItemDirty(AddedItem);
	NotifyContainerItemsChanged();

	return GItemTransactionResult_Success;
}

/*
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

	FItemData NewItemData;
	if (!Data.ItemData.SplitItem(SplitAmount, NewItemData))
	{
		return GItemTransactionResult_Error;
	}

	// The origin has been changed
	ContainerItems.MarkItemDirty(Data);

	FItemTransactionResult Result = AddItem(MoveTemp(NewItemData));
	if (!Result.IsSuccess())
	{
		// todo
		UE_LOG(LogUPContainers, Error,
		       TEXT("Unable to add item to container during split operation - the origin item amount has been reduced"
		       ));
	}
	else
	{
		NotifyContainerItemsChanged();
	}
	return Result;
}
*/

FItemTransactionResult UContainerComponent::MoveItem(FContainerItemData& SourceItem, uint32 AmountToMove)
{
	// Container->Container move
	check(GetOwner()->HasAuthority());

	// FContainerItemData should always have a valid container
	if (!ensureAlways(SourceItem.IsValid()))
	{
		return GItemTransactionResult_Error;
	}

	// Clamp to items real amount
	AmountToMove = FMath::Min(AmountToMove, SourceItem.GetAmount());

	UContainerComponent* SourceContainerComponent = SourceItem.Container.Get();

	const bool bMoveWholeStack = AmountToMove >= SourceItem.GetAmount();
	const bool bSameContainer = SourceItem.Container == this;
	FItemTransactionResult Result;

	if (bSameContainer && bMoveWholeStack)
	{
		ensureAlways(false);
		return GItemTransactionResult_Success;
	}

	FItemData& SourceItemData = SourceItem.ItemData;

	// Try to stack into existing items
	if (!bSameContainer)
	{
		for (FContainerItemData& Item : ContainerItems.Items)
		{
			// How much we can put in this stack
			int32 StackableAmount = SourceItemData.GetStackableAmount(Item.ItemData);
			StackableAmount = FMath::Min((int32)(AmountToMove - Result.MovedAmount), StackableAmount);

			if (StackableAmount == 0)
			{
				continue;
			}

			Item.ModifyAmount(StackableAmount);
			SourceItemData.ModifyAmount(-StackableAmount);

			ContainerItems.MarkItemDirty(Item); // This ( Target ) container
			SourceItem.Container->ContainerItems.MarkItemDirty(SourceItem); // Other ( Source container )

			NotifyContainerItemChanged(Item);

			Result.MovedAmount += StackableAmount;

			if (SourceItemData.GetAmount() == 0 || AmountToMove - Result.MovedAmount == 0)
			{
				break;
			}
		}

		// If the item amount is 0, remove the item 
		if (SourceItem.GetAmount() == 0)
		{
			UContainerComponent* SourceItemContainer = SourceItem.Container.Get();
			SourceItemContainer->RemoveItem(SourceItem);

			Result += EItemTransactionResultCode::FullyMoved;
			return Result;
		}

		// Something is left but the amount target is reached
		if (AmountToMove - Result.MovedAmount == 0)
		{
			Result += EItemTransactionResultCode::FullyMoved;
			return Result;
		}
	}

	// Splitting the item OR trying to put what left after stacking requires a free slot
	if (ContainerItems.Items.Num() >= ItemSlotsCapacity)
	{
		Result += EItemTransactionResultCode::NotEnoughCapacity;
		if (Result.MovedAmount > 0)
		{
			Result += EItemTransactionResultCode::PartiallyMoved;
		}

		return Result;
	}

	ensureAlways(SourceItem.GetAmount() > 0);

	// Create a NEW item and move what is left from the origin
	const int32 NewItemAmount = FMath::Min(AmountToMove - Result.MovedAmount, SourceItem.GetAmount());

	FItemDataDefinition NewItemDefinition(SourceItem.ItemData);
	NewItemDefinition.InstanceData.Amount = NewItemAmount;
	ensureAlways(NewItemDefinition.IsValid());

	// Update the source item amount
	SourceItem.ModifyAmount(-NewItemAmount);

	bool bOriginItemWasRemoved = false;
	if (SourceItem.GetAmount() == 0)
	{
		// Source amount depleted, remove
		if (SourceContainerComponent->RemoveItem(SourceItem))
		{
			Result += EItemTransactionResultCode::FullyMoved;
			bOriginItemWasRemoved = true;
		}
		else
		{
			// Couldn't remove the item
			Result += EItemTransactionResultCode::Error;
			return Result;
		}
	}

	// Add a new item
	FItemTransactionResult AddResult = AddItem(NewItemDefinition);
	if (AddResult.IsSuccess())
	{
		Result += EItemTransactionResultCode::FullyMoved;
		Result.MovedAmount += NewItemAmount;
	}
	else
	{
		// Revert
		if (bOriginItemWasRemoved)
		{
			FItemTransactionResult bRevertAdditionResult = SourceContainerComponent->AddItem(NewItemDefinition);
			ensureAlways(bRevertAdditionResult.IsSuccess());
		}
		else
		{
			SourceItem.ItemData.ModifyAmount(NewItemAmount);
		}
	}

	return Result;
}

FItemTransactionResult UContainerComponent::MoveItem(FContainerItemData& SourceItem, AItem* OutItem,
                                                     uint32 AmountToMove)
{
	// Container->World

	check(GetOwner()->HasAuthority());

	if (!SourceItem.IsValid())
	{
		return GItemTransactionResult_Error;
	}

	FTransform Transform;
	if (!FindDropTransform(SourceItem.ItemData, Transform))
	{
		return GItemTransactionResult_Error;
	}

	FItemTransactionResult Result;
	UContainerComponent* SourceContainerComponent = SourceItem.Container.Get();

	AmountToMove = FMath::Min(AmountToMove, SourceItem.GetAmount());
	const bool bMoveWholeStack = SourceItem.GetAmount() == AmountToMove;

	FItemDataDefinition DataDefinition(SourceItem.ItemData);
	DataDefinition.InstanceData.Amount = AmountToMove;

	if (bMoveWholeStack)
	{
		// Remove item from the container. In case of succeed it will be destroyed
		if (!RemoveItem(SourceItem))
		{
			return GItemTransactionResult_Error;
		}
	}
	else
	{
		SourceItem.ModifyAmount(-((int32)AmountToMove));
	}

	if (AItem* ResultItem = UItemFactoryHelper::SpawnItem(GetWorld(), DataDefinition, Transform))
	{
		OutItem = ResultItem;

		Result += EItemTransactionResultCode::FullyMoved;
		Result.MovedAmount = AmountToMove;

		// The source item remains with amount reduced
		if (!bMoveWholeStack)
		{
			SourceContainerComponent->NotifyContainerItemChanged(SourceItem);
		}
	}
	else
	{
		Result += EItemTransactionResultCode::Error;

		// Revert
		if (bMoveWholeStack)
		{
			FItemTransactionResult RevertAdditionResult = SourceContainerComponent->AddItem(DataDefinition);
			ensureAlways(RevertAdditionResult.IsSuccess());
		}
		else
		{
			SourceItem.ModifyAmount(AmountToMove);
		}
	}

	return Result;
}

FItemTransactionResult UContainerComponent::MoveItem(AItem* WorldItem, uint32 AmountToMove)
{
	// World->Container move. 
	check(GetOwner()->HasAuthority());
	NULLCHECK_RETURN(WorldItem, GItemTransactionResult_Error);

	FItemData& SourceItemData = WorldItem->GetItemDataMutable();
	if (!ensureAlways(SourceItemData.IsValid()))
	{
		return GItemTransactionResult_Error;
	}

	AmountToMove = FMath::Min(AmountToMove, SourceItemData.GetAmount());

	FItemTransactionResult Result;

	// Iterate all items in the container and move as many items as possible
	for (FContainerItemData& Item : ContainerItems.Items)
	{
		int32 StackableAmount = SourceItemData.GetStackableAmount(Item.ItemData);
		StackableAmount = FMath::Min(StackableAmount, (int32)(AmountToMove - Result.MovedAmount));

		SourceItemData.ModifyAmount(-StackableAmount);
		Item.ItemData.ModifyAmount(StackableAmount);

		ContainerItems.MarkItemDirty(Item);

		NotifyContainerItemChanged(Item);

		Result.MovedAmount += StackableAmount;

		if (SourceItemData.GetAmount() == 0)
		{
			break;
		}
	}

	// The source item amount depleted
	if (SourceItemData.GetAmount() == 0)
	{
		// Item is completely stacked into a new container
		WorldItem->RemoveFromWorld();
		Result += EItemTransactionResultCode::FullyMoved;

		return Result;
	}

	// The target amount reached
	if (Result.MovedAmount >= AmountToMove)
	{
		Result += EItemTransactionResultCode::FullyMoved;
		return Result;
	}


	// Can't stack anything, remains will be added as new items
	uint32 RemainingAmount = AmountToMove - Result.MovedAmount;
	const uint32 SlotsAvailable = GetSlotsAvailable();
	const uint32 SlotsPerStack = SourceItemData.StaticData->Slots;
	const uint32 ItemsPerStack = SourceItemData.StaticData->MaxAmountPerStack;

	for (uint32 s = SlotsAvailable, a = RemainingAmount; a > 0 && s > SlotsPerStack; s -= SlotsPerStack)
	{
		FItemDataDefinition ItemDefinition(SourceItemData);
		int32 CurrentIterationAmount = FMath::Min(AmountToMove, ItemsPerStack);

		ItemDefinition.InstanceData.Amount = CurrentIterationAmount;

		if (AddItem(ItemDefinition).IsSuccess())
		{
			SourceItemData.ModifyAmount(-CurrentIterationAmount);
			RemainingAmount -= CurrentIterationAmount;

			Result.MovedAmount += CurrentIterationAmount;
		}
	}

	if (RemainingAmount == 0)
	{
		Result += EItemTransactionResultCode::FullyMoved;
	}
	else
	{
		Result += EItemTransactionResultCode::PartiallyMoved;
	}

	if (SourceItemData.GetAmount() == 0)
	{
		WorldItem->RemoveFromWorld();
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
		NotifyContainerItemsChanged();
	}

	return true;
}

uint32 UContainerComponent::GenerateItemHandle() const
{
	uint32 Handle = 0;
	for (auto Item : ContainerItems.Items)
	{
		Handle = FMath::Max(Handle, Item.GetHandle());
	}

	check(Handle < UINT32_MAX)
	return ++Handle;
}

UContainerComponent* UContainerComponent::GetOriginContainer()
{
	return this;
}

UContainerComponent* UContainerComponent::GetListenContainer()
{
	return this;
}

TArray<FContainerItemData> UContainerComponent::GetItems()
{
	return ContainerItems.Items;
}

TArray<FContainerItemData> UContainerComponent::GetItemsForDisplay(AController* InstigatorController)
{
	// There we may differ results, based on the instigator.
	return GetItems();
}

IContainerInterface* UContainerComponent::GetOwnerInterface() const
{
	return Cast<IContainerInterface>(GetOwner());
}

bool UContainerComponent::CanStoreItem(const AController* Instigator, const AItem* Item) const
{
	NULLCHECK_RETURN(Instigator, false);
	NULLCHECK_RETURN(Item, false);

	APawn* Pawn = Instigator->GetPawn();
	UWorld* World = GetWorld();
	NULLCHECK_RETURN(Pawn, false);
	NULLCHECK_RETURN(World, false);

	// Visibility check
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Pawn);
	Params.AddIgnoredActor(Item);

	FHitResult Result;
	// move trace a bit up to avoid ground collision
	const FVector TraceEnd = Item->GetActorLocation() + FVector(0, 0, 1.f);
	const bool bHasObstacle = World->LineTraceSingleByChannel(
		Result,
		Pawn->GetActorLocation(),
		TraceEnd,
		ECC_Visibility,
		Params
	);
	if (bHasObstacle)
	{
		return false;
	}

	return true;
}


void UContainerComponent::TryStoreItem(AController* Instigator, AItem* Item)
{
	if (!IsValid(Item) || !CanStoreItem(Instigator, Item))
	{
		return;
	}

	// Server only
	check(Item->HasAuthority());
	MoveItem(Item);
}

void UContainerComponent::ServerTryStoreItem_Implementation(AController* Instigator, const FContainerItemData& ItemData)
{
	NULLCHECK(Instigator);
	ensureAlways(ItemData.GetContainerComponent() && ItemData.GetContainerComponent()->HasItem(ItemData));

	FContainerItemData& ItemDataMutable = const_cast<FContainerItemData&>(ItemData);
	MoveItem(ItemDataMutable);
}

bool UContainerComponent::TryDropItem(AController* Instigator, const FContainerItemData& Item)
{
	ensureAlways(!GetOwner()->HasAuthority());
	ensureAlways(Item.IsValid());
	ensureAlways(Item.IsInContainer(GetOriginContainer()));

	if (ensureAlways(HasItem(Item)))
	{
		ServerTryDropItem(Instigator, Item);
	}

	return true;
}

void UContainerComponent::ServerTryDropItem_Implementation(AController* Instigator, const FContainerItemData& Item)
{
	ensureAlways(Item.IsValid() && Item.IsInContainer(GetOriginContainer()));

	if (ensureAlways(HasItem(Item)))
	{
		AItem* Result = nullptr;
		MoveItem(const_cast<FContainerItemData&>(Item), Result);
	}
}
