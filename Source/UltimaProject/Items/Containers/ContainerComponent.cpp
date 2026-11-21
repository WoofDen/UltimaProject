// Game includes
#include "ContainerComponent.h"
#include "UltimaProject/UI/ContainerWidget.h"
#include <UltimaProject/Items/Common/ItemFactoryHelper.h>

// Engine includes
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UltimaProject/Common/Globals.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Framework/UPPlayerController.h"

DEFINE_LOG_CATEGORY(LogUPContainers)

uint32 FContainerItemData::InvalidHandle = 0;

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
	ItemData = FItemData::InvalidItemData;
}

void FContainerItemArray::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
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

void FContainerItemArray::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
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

void FContainerItemArray::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
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

void FContainerLiquidArray::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	NULLCHECK_SP(ContainerComponent);

	if (!AddedIndices.IsEmpty())
	{
		// Propagate changes to client
		if (UContainerComponent* ListenContainer = ContainerComponent->GetListenContainer())
		{
			ListenContainer->NotifyLiquidsChanged();
		}
	}
}

void FContainerLiquidArray::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	NULLCHECK_SP(ContainerComponent);

	if (ChangedIndices.IsEmpty())
	{
		return;
	}

	// Propagate changes to client
	if (UContainerComponent* ListenContainer = ContainerComponent->GetListenContainer())
	{
		ListenContainer->NotifyLiquidsChanged();
	}
}

void FContainerLiquidArray::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	NULLCHECK_SP(ContainerComponent);
	if (RemovedIndices.IsEmpty())
	{
		return;
	}

	// Propagate changes to the client
	// The replicated prop. ( LiquidsData ) still contains the removed idx., so postpone the update
	if (UWorld* World = ContainerComponent->GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(
			[WeakContainerPtr = TWeakObjectPtr(ContainerComponent->GetOriginContainer())]
			{
				if (WeakContainerPtr.IsValid())
				{
					WeakContainerPtr->NotifyLiquidsChanged();
				}
			});
	}
}

const TCHAR* UContainerComponent::EnumToString(EContainerCategory Category)
{
	switch (Category)
	{
	case EContainerCategory::Inventory:
		return TEXT("Inventory");
	case EContainerCategory::Foraging:
		return TEXT("Foraging");
	case EContainerCategory::ChestBase:
		return TEXT("ChestBase");
	case EContainerCategory::Vat:
		return TEXT("Vat");
	default:
		break;
	}

	UE_LOG(LogUPContainers, Error, TEXT("Invalid category"));
	return TEXT("");
}

uint32 UContainerComponent::GetSlotsInUse() const
{
	// todo cache values?
	int32 Result = 0;
	for (auto& Item : ContainerItems.Items)
	{
		Result += Item.ItemData.GetStaticData()->Slots;
	}

	return Result;
}

uint32 UContainerComponent::GetSlotsAvailable() const
{
	return GetSlotsTotal() - GetSlotsInUse();
}

UContainerComponent::UContainerComponent()
{
}

FItemTransactionResult::FItemTransactionResult(uint32 InRequestedAmount, uint32 InMovedAmount)
	: RequestedAmount(InRequestedAmount)
	  , MovedAmount(InMovedAmount)
{
}

FItemTransactionResult UContainerComponent::AddItem(FItemDataDefinition& ItemDataDefinition)
{
	FItemData NewItem(ItemDataDefinition);
	return AddItem(MoveTemp(NewItem));
}

FContainerItemData& UContainerComponent::GetItemMutable(uint32 Handle) const
{
	return const_cast<FContainerItemData&>(GetItem(Handle));
}

void UContainerComponent::OnRep_Category()
{
	ensureAlways(Category != EContainerCategory::None);
	OnClientReady();
}

void UContainerComponent::OnClientReady()
{
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

	DOREPLIFETIME_CONDITION(ThisClass, Category, COND_InitialOnly);
}

bool UContainerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	return WroteSomething;
}

UContainerCategoriesDataAsset* UContainerComponent::GetCategoryData() const
{
	UAssetManager& AssetManager = UAssetManager::Get();

	// Load if not in memory
	const FPrimaryAssetId PrimaryAssetId("ContainerCategory", EnumToString(Category));
	if (TSharedPtr<FStreamableHandle> StreamHandle = AssetManager.LoadPrimaryAsset(PrimaryAssetId))
	{
		StreamHandle->WaitUntilComplete();
	}

	return UAssetManager::Get().GetPrimaryAssetObject<UContainerCategoriesDataAsset>(PrimaryAssetId);
}

void UContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	ContainerItems.ContainerComponent = this;
	LiquidsData.ContainerComponent = this;

	if (AActor* Owner = GetOwner())
	{
		if (Owner->GetNetMode() == NM_Client && Category != EContainerCategory::None)
		{
			OnClientReady();
		}

		if (Owner->HasAuthority())
		{
			IHeartbeatInterface::Register();
		}
	}
}

void UContainerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (AUPPlayerController* UPPC = Cast<AUPPlayerController>(PC); UPPC && UPPC->GetNetMode() == NM_Client)
		{
			UPPC->TryCloseContainer(this);
		}
	}

	if (AActor* Owner = GetOwner())
	{
		if (Owner->HasAuthority())
		{
			IHeartbeatInterface::Unregister();
		}
	}

	Super::EndPlay(EndPlayReason);
}

bool UContainerComponent::FindDropTransform(uint32 ContainerItemDataHandle, FTransform& Result) const
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
	if (UContainerCategoriesDataAsset* DataAsset = GetCategoryData())
	{
		return DataAsset->ContainerWidgetClass;
	}

	return nullptr;
}

float UContainerComponent::GetInteractionRadius() const
{
	// Retrieve interaction data from Item
	// TODO doesn't handle character and non-item cases
	if (AItem* ItemOwner = GetOwner<AItem>())
	{
		if (const UItemDataAsset* ItemData = ItemOwner->GetItemDataConst().GetStaticData())
		{
			return ItemData->InteractionRadius;
		};
	}


	return 0;
}

FVector UContainerComponent::GetContainerOrigin() const
{
	if (const AActor* Owner = GetOwner())
	{
		return Owner->GetActorLocation();
	}

	return UPGlobals::InvalidLocation;
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
		int32 Handle = Item.GetHandle();
		ListenContainer->OnContainerItemChanged.Broadcast(Handle);
	}
}

void UContainerComponent::OnClientOpened(AUPPlayerController* Instigator)
{
}

void UContainerComponent::OnClientContainerClosed(AUPPlayerController* Instigator)
{
}

int32 UContainerComponent::GetSlotsTotal() const
{
	if (UContainerCategoriesDataAsset* DataAsset = GetCategoryData())
	{
		return SlotsCapacityMod + DataAsset->DefaultSlotCapacity;
	}

	return SlotsCapacityMod;
}

float UContainerComponent::GetLiquidCapacity()
{
	if (UContainerCategoriesDataAsset* DataAsset = GetCategoryData())
	{
		return DataAsset->DefaultLiquidCapacity + LiquidCapacityMod;
	}

	return LiquidCapacityMod;
}

float UContainerComponent::GetLiquidCapacityUsed(bool bNormalized)
{
	float Result = 0;
	for (const auto& LiquidData : LiquidsData.Liquids)
	{
		Result += LiquidData.GetAmount();
	}

	Result = FMath::Clamp(Result, 0.f, 1.f);
	if (!bNormalized)
	{
		Result *= GetLiquidCapacity();
	}

	return Result;
}

bool UContainerComponent::HasItem(const uint32 ItemHandle) const
{
	return ContainerItems.Items.ContainsByPredicate([&ItemHandle](const FContainerItemData& ContainerItemData)
	{
		return ContainerItemData.GetHandle() == ItemHandle;
	});
}

const FContainerItemData& UContainerComponent::GetItem(uint32 Handle) const
{
	static FContainerItemData InvalidItem;

	for (const auto& Item : ContainerItems.Items)
	{
		if (Item.GetHandle() == Handle)
		{
			return Item;
		}
	}

	return InvalidItem;
}

FItemTransactionResult UContainerComponent::AddItem(FItemData&& ItemData)
{
	uint32 Handle;
	return AddItem(MoveTemp(ItemData), Handle);
}

FItemTransactionResult UContainerComponent::AddItem(FItemData&& ItemData, uint32& ResultHandle)
{
	ensureAlways(GetOwner() && GetOwner()->HasAuthority());

	const int32 SlotIndex = GetSlotsInUse();
	ensureAlways(SlotIndex < GetSlotsTotal());
	// todo ensure get item at slot == null

	FContainerItemData ContainerItemData(MoveTemp(ItemData), this, SlotIndex);
	FContainerItemData& AddedItem = ContainerItems.Items.Add_GetRef(MoveTemp(ContainerItemData));

	ContainerItems.MarkItemDirty(AddedItem);
	NotifyContainerItemsChanged();

	ResultHandle = AddedItem.GetHandle();

	FItemTransactionResult Result(1, 1);
	return Result;
}

FItemTransactionResult UContainerComponent::MoveItem(UContainerComponent* SourceContainer,
                                                     uint32 Handle,
                                                     uint32 AmountToMove)
{
	// Container->Container move
	check(GetOwner() && GetOwner()->HasAuthority());
	NULLCHECK_RETURN(SourceContainer, InvalidItemTransactionResult);

	FContainerItemData& SourceItem = SourceContainer->GetItemMutable(Handle);

	// FContainerItemData should always have a valid container
	if (!ensureAlways(SourceItem.IsValid()) || AmountToMove <= 0)
	{
		return InvalidItemTransactionResult;
	}

	// Clamp to items real amount
	AmountToMove = FMath::Min(AmountToMove, SourceItem.GetAmount());

	UContainerComponent* SourceContainerComponent = SourceItem.Container.Get();

	const bool bMoveWholeStack = AmountToMove >= SourceItem.GetAmount();
	const bool bSameContainer = SourceItem.Container == this;
	FItemTransactionResult Result(AmountToMove);

	if (bSameContainer && bMoveWholeStack)
	{
		return FItemTransactionResult(AmountToMove, AmountToMove);
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
			SourceContainer->ContainerItems.MarkItemDirty(SourceItem); // Other ( Source container )

			NotifyContainerItemChanged(Item);

			Result.MovedAmount += StackableAmount;

			if (SourceItemData.GetAmount() == 0 || AmountToMove - Result.MovedAmount == 0)
			{
				break;
			}
		}

		// The source item amount is 0, remove the item
		if (SourceItem.GetAmount() == 0)
		{
			SourceContainer->RemoveItem(SourceItem);

			Result.MovedAmount = AmountToMove;
			Result.SourceRecordsRemoved++;

			return Result;
		}

		// Something is left but the amount target is reached
		if (AmountToMove - Result.MovedAmount == 0)
		{
			Result.MovedAmount = AmountToMove;
			return Result;
		}
	}

	// Splitting the item OR trying to put what left after stacking requires a free slot
	if (ContainerItems.Items.Num() >= GetSlotsTotal())
	{
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
			bOriginItemWasRemoved = true;
		}
		else
		{
			// Couldn't remove the item
			return Result;
		}
	}

	// Add a new item
	FItemTransactionResult AddResult = AddItem(NewItemDefinition);
	if (ensureAlways(AddResult.IsSuccess(false)))
	{
		Result.MovedAmount += NewItemAmount;
	}
	else
	{
		// Revert
		if (bOriginItemWasRemoved)
		{
			FItemTransactionResult bRevertAdditionResult = SourceContainerComponent->AddItem(NewItemDefinition);
			ensureAlways(bRevertAdditionResult.IsSuccess(false));
		}
		else
		{
			SourceItem.ItemData.ModifyAmount(NewItemAmount);
		}
	}

	return Result;
}

FItemTransactionResult UContainerComponent::MoveItem(uint32 ContainerItemHandle,
                                                     AItem* OutItem,
                                                     uint32 AmountToMove)
{
	// Container->World

	check(GetOwner()->HasAuthority());

	FContainerItemData& SourceItem = GetItemMutable(ContainerItemHandle);
	if (!SourceItem.IsValid())
	{
		return InvalidItemTransactionResult;
	}

	FTransform Transform;
	if (!FindDropTransform(ContainerItemHandle, Transform))
	{
		return InvalidItemTransactionResult;
	}

	FItemTransactionResult Result(AmountToMove);
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
			Result.SourceRecordsRemoved++;
			return InvalidItemTransactionResult;
		}
	}
	else
	{
		SourceItem.ModifyAmount(-static_cast<int32>(AmountToMove));
	}

	if (AItem* ResultItem = UItemFactoryHelper::SpawnItem(GetWorld(), DataDefinition, Transform))
	{
		OutItem = ResultItem;

		Result.MovedAmount = AmountToMove;

		// The source item remains with amount reduced
		if (!bMoveWholeStack)
		{
			SourceContainerComponent->NotifyContainerItemChanged(SourceItem);
		}
	}
	else
	{
		// Revert
		if (bMoveWholeStack)
		{
			FItemTransactionResult RevertAdditionResult = SourceContainerComponent->AddItem(DataDefinition);
			ensureAlways(RevertAdditionResult.IsSuccess(false));
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
	NULLCHECK_RETURN(WorldItem, InvalidItemTransactionResult);

	FItemData& SourceItemData = WorldItem->GetItemDataMutable();

	if (!ensureAlways(SourceItemData.IsValid()))
	{
		return InvalidItemTransactionResult;
	}

	AmountToMove = FMath::Min(AmountToMove, SourceItemData.GetAmount());

	FItemTransactionResult Result(AmountToMove);

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

		return Result;
	}

	// The target amount reached
	if (Result.MovedAmount >= AmountToMove)
	{
		return Result;
	}

	// Can't stack anything, remains will be added as new items
	uint32 RemainingAmount = AmountToMove - Result.MovedAmount;
	const uint32 SlotsAvailable = GetSlotsAvailable();
	const uint32 SlotsPerStack = SourceItemData.GetStaticData()->Slots;
	const uint32 ItemsPerStack = SourceItemData.GetStaticData()->MaxAmountPerStack;

	for (uint32 s = SlotsAvailable, a = RemainingAmount; a > 0 && s >= SlotsPerStack; s -= SlotsPerStack, a =
	     RemainingAmount)
	{
		FItemDataDefinition ItemDefinition(SourceItemData);
		int32 CurrentIterationAmount = FMath::Min(AmountToMove, ItemsPerStack);

		ItemDefinition.InstanceData.Amount = CurrentIterationAmount;

		if (AddItem(ItemDefinition).IsSuccess(false))
		{
			SourceItemData.ModifyAmount(-CurrentIterationAmount);
			RemainingAmount -= CurrentIterationAmount;

			Result.MovedAmount += CurrentIterationAmount;
		}
	}

	if (SourceItemData.GetAmount() == 0)
	{
		WorldItem->RemoveFromWorld();
	}

	return Result;
}

FItemTransactionResult UContainerComponent::MoveLiquid(UContainerComponent* SourceContainer, float SourceAmountReal)
{
	// Container -> Container move
	check(GetOwner()->HasAuthority());
	NULLCHECK_RETURN(SourceContainer, InvalidItemTransactionResult);

	const float TargetCapacityTotal = GetLiquidCapacity();

	const float TargetCapacityAvailable = TargetCapacityTotal - GetLiquidCapacityUsed(false); // how much can fit
	const float SourceCapacityUsed = SourceContainer->GetLiquidCapacityUsed(false); // how much the source has

	const float SafeAmountReal = FMath::Min3(TargetCapacityAvailable, SourceAmountReal, SourceCapacityUsed);

	const float SourceCapacityTotal = SourceContainer->GetLiquidCapacity();
	if (SourceCapacityTotal <= 0 || SafeAmountReal <= 0.f)
	{
		return InvalidItemTransactionResult;
	}

	// How much we remove from Source
	const float SourceAmountNormalized = SafeAmountReal / SourceCapacityTotal;

	FItemTransactionResult Result(SafeAmountReal);

	// Removed liquids. Key is a not normalized amount of liquid towards this container
	TArray<TPair<float, FLiquidDescriptor>> ExtractedLiquids;

	// Remove liquid from source
	for (auto It = SourceContainer->LiquidsData.Liquids.CreateIterator(); It; ++It)
	{
		/**
		 * Math example
		 * - Source container is a bottle of 100(ml) capacity, which is filled on 25 with water
		 * - Target container is a bottle of 500(ml) capacity
		 * - Move half of source(0.5) to target
		 */

		// 0.25 * 0.5 = 0.125 -> % of source container we remove
		float RealAmount = It->GetAmount();
		float RemoveAmount = RealAmount * SourceAmountNormalized;

		if ((It->GetAmount() - RemoveAmount) < UP::Liquids::MinNormalizedValue)
		{
			RemoveAmount = It->GetAmount();
		}

		It->AddAmount(-RemoveAmount);

		// (0.25 * 100) / 500 = 0.05 -> % of this container we need to add

		// Convert source amount fraction to target amount fraction
		const float AmountToAddNormalized = (RemoveAmount * SourceCapacityTotal) / TargetCapacityTotal;
		ExtractedLiquids.Add({AmountToAddNormalized, It->Descriptor});

		// Remove liquid from source if nothing left
		if (It->GetAmount() < UP::Liquids::MinNormalizedValue)
		{
			It.RemoveCurrent();
			SourceContainer->LiquidsData.MarkArrayDirty();
		}
		else
		{
			SourceContainer->LiquidsData.MarkItemDirty(*It);
		}
	}

	for (const auto ExtractedLiquid : ExtractedLiquids)
	{
		// All cuts and checks have to be done at this point and we assume this addition will always succeed
		MoveLiquid(ExtractedLiquid.Value, ExtractedLiquid.Key);
		Result.MovedAmount += ExtractedLiquid.Key * TargetCapacityTotal;
	}

	NotifyLiquidsChanged();

	return Result;
}

FItemTransactionResult UContainerComponent::MoveLiquid(const FLiquidDescriptor& LiquidDescriptor, float SafeAmountNormalized)
{
	SafeAmountNormalized = FMath::Clamp(SafeAmountNormalized, 0.0f, 1.0f);
	float RealAmount = GetLiquidCapacity() * SafeAmountNormalized;

	// Try merge existing liquid
	for (FContainerLiquidData& ExistingLiquid : LiquidsData.Liquids)
	{
		if (ExistingLiquid.Descriptor == LiquidDescriptor)
		{
			ExistingLiquid.AddAmount(SafeAmountNormalized);
			LiquidsData.MarkItemDirty(ExistingLiquid);

			return FItemTransactionResult(RealAmount, RealAmount);
		}
	}

	// If no existing liquid with the same props, add a new one
	// Hardcore liquids limit
	constexpr int32 LiquidsLimit = 100;
	if (LiquidsData.Liquids.Num() >= LiquidsLimit)
	{
		// Destroy the smallest one
		int32 SmallestIdx = 0;
		float SmallestAmount = 0.f;
		for (int32 i = 0; i < LiquidsData.Liquids.Num(); i++)
		{
			if (LiquidsData.Liquids[i].GetAmount() < SmallestAmount)
			{
				SmallestIdx = i;
				SmallestAmount = LiquidsData.Liquids[i].GetAmount();
			}
		}

		LiquidsData.Liquids.RemoveAt(SmallestIdx);
		LiquidsData.MarkArrayDirty();
	}

	FContainerLiquidData Data;
	Data.SetAmount(SafeAmountNormalized);
	Data.Descriptor = LiquidDescriptor;

	FContainerLiquidData& AddedItem = LiquidsData.Liquids.Add_GetRef(Data);
	LiquidsData.MarkItemDirty(AddedItem);

	FItemTransactionResult Result(RealAmount, RealAmount);
	return Result;
}

bool UContainerComponent::RemoveItem(FContainerItemData& ItemData)
{
	ensureAlways(GetOwner() && GetOwner()->HasAuthority());

	// Invalidate item early as the next array removal might change the referenced value
	ItemData.Container = nullptr;

	if (ContainerItems.Items.RemoveSingle(ItemData) > 0)
	{
		ContainerItems.MarkArrayDirty();
		NotifyContainerItemsChanged();
	}
	else
	{
		// Restore
		ItemData.Container = this;
	}

	return true;
}

void UContainerComponent::NotifyLiquidsChanged_Implementation()
{
	if (UContainerComponent* ListenContainer = GetListenContainer())
	{
		ListenContainer->OnLiquidsChanged.Broadcast();
	}
}

void UContainerComponent::SetCategory(EContainerCategory InCategory)
{
	Category = InCategory;
}

uint32 UContainerComponent::GenerateItemHandle() const
{
	uint32 Handle = FContainerItemData::InvalidHandle + 1;
	for (auto Item : ContainerItems.Items)
	{
		Handle = FMath::Max(Handle, Item.GetHandle());
	}

	check(Handle < UINT32_MAX)
	return ++Handle;
}

FItemTransactionResult UContainerComponent::RemoveLiquid(const float AmountNormalized)
{
	if (AmountNormalized <= 0 || LiquidsData.Liquids.Num() == 0)
	{
		return InvalidItemTransactionResult;
	}

	const float Capacity = GetLiquidCapacity();
	const float RealAmount = AmountNormalized * Capacity;

	FItemTransactionResult Result(RealAmount);

	for (auto It = LiquidsData.Liquids.CreateIterator(); It; ++It)
	{
		float RemoveAmountNormalized = FMath::Min(AmountNormalized, It->GetAmount());
		It->AddAmount(-RemoveAmountNormalized);

		if (It->GetAmount() < UP::Liquids::MinNormalizedValue)
		{
			It.RemoveCurrent();
			Result.TargetRecordsRemoved++;
		}

		Result.MovedAmount += RemoveAmountNormalized;
	}

	return Result;
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
	if (!IsAccessible(InstigatorController))
	{
		// Anti-cheat check
		return {};
	}

	for (auto& Item : ContainerItems.Items)
	{
		Item.ItemData.LoadStaticData();
	}

	// There we may differ results, based on the instigator.
	return GetItems();
}

const TArray<FContainerLiquidData>& UContainerComponent::GetLiquids() const
{
	return LiquidsData.Liquids;
}

IContainerOwnerInterface* UContainerComponent::GetOwnerInterface() const
{
	return Cast<IContainerOwnerInterface>(GetOwner());
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

bool UContainerComponent::IsAccessible(const AController* Instigator) const
{
	NULLCHECK_RETURN(Instigator, false);

	const APawn* Pawn = Instigator->GetPawn();
	const AActor* Owner = GetOwner();
	NULLCHECK_RETURN(Pawn, false);
	NULLCHECK_RETURN(Owner, false);

	const float InteractionRadius = GetInteractionRadius();
	const float Distance = FVector::Distance(Pawn->GetActorLocation(), GetContainerOrigin());

	if (Distance > InteractionRadius)
	{
		return false;
	}

	return true;
}

uint32 UContainerComponent::GetNumItems() const
{
	return ContainerItems.Items.Num();
}

bool UContainerComponent::IsEmpty() const
{
	return GetNumItems() == 0;
}

void UContainerComponent::StoreItem(AItem* WorldItem, uint32 Amount)
{
	check(GetOwner() && GetOwner()->HasAuthority());

	MoveItem(WorldItem, Amount);
}

void UContainerComponent::RelocateItem(UContainerComponent* SourceContainer, uint32 Handle, uint32 Amount)
{
	check(GetOwner() && GetOwner()->HasAuthority());

	MoveItem(SourceContainer, Handle, Amount);
}

AItem* UContainerComponent::DropItem(uint32 Handle, uint32 Amount)
{
	check(GetOwner() && GetOwner()->HasAuthority());

	AItem* Result = nullptr;
	MoveItem(Handle, Result, Amount);

	return Result;
}

uint32 UContainerComponent::SpawnItem(const FItemDataDefinition& ItemDataDefinition)
{
	check(GetOwner() && GetOwner()->HasAuthority());

	uint32 ResultHandle = FContainerItemData::InvalidHandle;
	AddItem(ItemDataDefinition, ResultHandle);

	return ResultHandle;
}

void UContainerComponent::StoreLiquid(UContainerComponent* ContainerComponent, float Amount)
{
	check(GetOwner() && GetOwner()->HasAuthority());

	Amount = FMath::Clamp(Amount, 0, FLT_MAX);

	FItemTransactionResult Result = MoveLiquid(ContainerComponent, Amount);
	if (Result.MovedAmount != 0)
	{
		NotifyLiquidsChanged();
	}
}

void UContainerComponent::SpawnLiquid(const FLiquidDescriptor& LiquidDescriptor, float Amount, bool bForceOverflow)
{
	if (Amount <= 0)
	{
		return;
	}

	const float TargetCapacityTotal = GetLiquidCapacity();
	const float TargetCapacityAvailable = TargetCapacityTotal - GetLiquidCapacityUsed(false);

	float SafeAmount;
	// Resolve container liquids overflow
	if (bForceOverflow)
	{
		// Store all the amount. If not enough space, free it by removing the origin liquid
		SafeAmount = Amount;

		if (SafeAmount > TargetCapacityAvailable)
		{
			const float FreeAmountNormalized = (Amount - TargetCapacityAvailable) / TargetCapacityTotal;
			RemoveLiquid(FreeAmountNormalized);
		}
	}
	else
	{
		// Store as much as possible without overflowing the container
		SafeAmount = FMath::Min(TargetCapacityAvailable, Amount);
	}

	auto Result = MoveLiquid(LiquidDescriptor, SafeAmount);
	if (Result.IsSuccess(true))
	{
		NotifyLiquidsChanged();
	}
}
