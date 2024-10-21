#pragma once

#include "Interfaces\ContainerInterface.h"
#include "Container.generated.h"

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

protected:
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
static FItemTransactionResult GItemTransactionResult_Error{EItemTransactionResultCode::Success};

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

	UPROPERTY(VisibleAnywhere, Replicated)
	TArray<FContainerItemData> Items;

	UPROPERTY(VisibleAnywhere)
	int64 WeightCapacity = 0;

	// Amount of available slots
	UPROPERTY(VisibleAnywhere)
	int64 ItemsCapacity = 10;

	// Find position nearby where we can safely drop an item
	bool FindDropTransform(const UItemData* ItemData, FTransform& Result) const;

public:
	inline static int64 MaxItemsCapacity = MAX_int64;
	inline static int64 MaxWeightCapacity = MAX_int64;

	FORCEINLINE int64 GetWeightCapacity() const
	{
		return WeightCapacity;
	}

	FORCEINLINE int64 GetItemsCapacity() const
	{
		return ItemsCapacity;
	}

	virtual void SetItemsCapacity(const int64 NewValue);
	virtual void SetWeightCapacity(const int64 NewValue);

	// Try add item ( UItemData ) to container
	virtual FItemTransactionResult AddItem(UItemData* ItemData);

	// Container->Container move. Calls UContainer::AddItem
	virtual FItemTransactionResult MoveItem(FContainerItemData& Item);

	// Container->World move
	virtual FItemTransactionResult MoveItem(FContainerItemData& Item, const FVector Location, AItem* OutItem);

	// World->Container move. Calls UContainer::AddItem
	virtual FItemTransactionResult MoveItem(AItem* WorldItem);


	UFUNCTION(BlueprintCallable)
	virtual bool RemoveItem(FContainerItemData& ItemData);

#pragma region ContainerInterface

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual TArray<FContainerItemData> GetItems() override;
#pragma endregion
};
