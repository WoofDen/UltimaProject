#pragma once

#include "Interfaces\ContainerInterface.h"
#include "Container.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnContainerItemsChanged);

DECLARE_LOG_CATEGORY_EXTERN(LogContainers, Display, All);

// Info about item stored in a container
USTRUCT(BlueprintType)
struct FContainerItemData
{
	friend class UContainer;

	GENERATED_BODY()

	bool operator==(const FContainerItemData& Other) const;
	FContainerItemData(UItemData* InitData, const UContainer* InitContainer, const int64 InitSlotIndex);
	FContainerItemData();

	int64 GetSlot() const { return SlotIndex; }

	bool IsValid() const { return ItemData && Container.Get(); }
	bool IsInContainer(const UContainer* AnotherContainer) const { return Container == AnotherContainer; }
	const UItemData* GetItemData() const { return ItemData; }

protected:
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<const UContainer> Container = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UItemData* ItemData = nullptr;

	int64 SlotIndex = INDEX_NONE;
};

UENUM()
enum class EItemTransactionResultCode
{
	NONE,
	Success,

	NotEnoughCapacity,
	Error // Generic error for malformed input, etc.
};

USTRUCT(BlueprintType)
struct FItemTransactionResult
{
	GENERATED_BODY()

	EItemTransactionResultCode Code;

	inline bool IsSuccess() const { return Code == EItemTransactionResultCode::Success; }
};

static FItemTransactionResult GItemTransactionResult_Success{EItemTransactionResultCode::Success};
static FItemTransactionResult GItemTransactionResult_Error{EItemTransactionResultCode::Error};
static FItemTransactionResult GItemTransactionResult_Capacity{EItemTransactionResultCode::NotEnoughCapacity};

/**
 * Basic container impl. It does not relate on owner/actor and don't perform checks on any external conditions ( owner, player that moves item, etc )
 */
UCLASS(Abstract)
class UContainer : public UActorComponent, public IContainerInterface
{
	friend class UItemFactoryHelper;

	GENERATED_BODY()

	int64 GetStoredSlotsCount() const;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	UPROPERTY(VisibleAnywhere, Replicated, ReplicatedUsing=OnRep_Items)
	TArray<FContainerItemData> Items;

	// Amount of available slots
	UPROPERTY(EditDefaultsOnly)
	int64 ItemSlotsCapacity = 10;

	// Find position nearby where we can safely drop an item
	bool FindDropTransform(const UItemData* ItemData, FTransform& Result) const;

	UFUNCTION()
	virtual void OnRep_Items();

	// Try add item ( UItemData ) to container
	virtual FItemTransactionResult AddItem(UItemData* ItemData);

	virtual bool RemoveItem(FContainerItemData& ItemData);

public:
	UPROPERTY(BlueprintAssignable)
	FOnContainerItemsChanged OnContainerItemsChanged;

	inline static int64 MaxItemsCapacity = MAX_int64;

	FORCEINLINE int64 GetItemsCapacity() const
	{
		return ItemSlotsCapacity;
	}

	virtual void SetItemsCapacity(const int64 NewValue);

	bool HasItem(const FContainerItemData& ItemData) const { return Items.Contains(ItemData); };

	// Split into two items by amount. Second item will be placed to the same container
	virtual FItemTransactionResult SplitItem(UPARAM(ref) const FContainerItemData& Data, const int64 SplitAmount);

	// Container->Container move. Calls UContainer::AddItem
	virtual FItemTransactionResult MoveItem(const FContainerItemData& Item);

	// Container->World move
	virtual FItemTransactionResult MoveItem(FContainerItemData& Item, AItem* OutItem);

	// World->Container move. Calls UContainer::AddItem
	virtual FItemTransactionResult MoveItem(AItem* WorldItem);


#pragma region ContainerInterface

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual TArray<FContainerItemData> GetItems() override;
#pragma endregion
};
