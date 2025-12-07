#pragma once

// Game includes
#include "Interfaces/ContainerInterface.h"

// Engine includes
#include "Net/Serialization/FastArraySerializer.h"

// Generated include
#include "ContainerComponent.generated.h"

// Forward declarations
class UContainerWidget;

DECLARE_LOG_CATEGORY_EXTERN(LogUPContainers, Display, All);

// Info about item stored in a container
USTRUCT(BlueprintType)
struct FContainerItemData : public FFastArraySerializerItem
{
	friend class UContainerComponent;

	GENERATED_BODY()

	bool operator==(const FContainerItemData& Other) const;
	FContainerItemData(UItemData* InitData, UContainerComponent* InitContainer, const int32 InitSlotIndex);
	FContainerItemData();

	int32 GetSlot() const { return SlotIndex; }

	bool IsValid() const { return ItemData && Container.Get(); }
	bool IsInContainer(const UContainerComponent* AnotherContainer) const { return Container == AnotherContainer; }
	const UItemData* GetItemData() const { return ItemData; }

protected:
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UContainerComponent> Container = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UItemData* ItemData = nullptr;

	int32 SlotIndex = INDEX_NONE;
};

USTRUCT()
struct FContainerItems : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FContainerItemData> Items;

	UPROPERTY()
	TWeakObjectPtr<UContainerComponent> ContainerComponent;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FContainerItemData, FContainerItems>(
			Items, DeltaParms, *this);
	}

	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
};

template <>
struct TStructOpsTypeTraits<FContainerItems> : public TStructOpsTypeTraitsBase2<FContainerItems>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EItemTransactionResultCode : uint8
{
	NONE = 0 UMETA(Hidden),
	FullyMoved = 1 << 0,
	PartiallyMoved = 1 << 1, // some amount were moved, some were not
	NotEnoughCapacity = 1 << 2,

	Error = 1 << 7 // Generic error for malformed input, etc.
};

ENUM_CLASS_FLAGS(EItemTransactionResultCode);

USTRUCT(BlueprintType)
struct FItemTransactionResult
{
	GENERATED_BODY()

	int8 ResultCode = 0;

	FItemTransactionResult()
	{
		ResultCode = 0;
	}

	FItemTransactionResult(EItemTransactionResultCode Code)
	{
		ResultCode |= static_cast<uint8>(Code);
	}

	void operator+=(const EItemTransactionResultCode Code)
	{
		ResultCode |= static_cast<uint8>(Code);
	}

	inline bool IsSuccess() const { return ResultCode & static_cast<int8>(EItemTransactionResultCode::FullyMoved); }
};

static FItemTransactionResult GItemTransactionResult_Success{EItemTransactionResultCode::FullyMoved};
static FItemTransactionResult GItemTransactionResult_Error{EItemTransactionResultCode::Error};
static FItemTransactionResult GItemTransactionResult_Capacity{EItemTransactionResultCode::NotEnoughCapacity};

/**
 * Basic container impl. It does not relate on owner/actor and don't perform checks on any external conditions ( owner, player that moves item, etc )
 */
UCLASS(Abstract)
class UContainerComponent : public UActorComponent, public IContainerInterface
{
	GENERATED_BODY()

	friend class UItemFactoryHelper;

	UPROPERTY(VisibleAnywhere, Transient, Replicated)
	FContainerItems ContainerItems;

	int32 GetStoredSlotsCount() const;

	// Try add item ( UItemData ) to container
	virtual FItemTransactionResult AddItem(UItemData* ItemData);
	virtual FItemTransactionResult AddItem(UItemData* ItemData, FContainerItemData& AddedItem);
	virtual bool RemoveItem(FContainerItemData& ItemData);

protected:
	UPROPERTY(EditDefaultsOnly)
	int32 ItemSlotsCapacity = 10;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UContainerWidget> ContainerWidgetClass;
		
	UPROPERTY()
	TObjectPtr<UContainerWidget> ContainerWidget;

	// UActorComponent
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void BeginPlay() override;
	// ~UActorComponent
	
	void InitializeContainerWidget();

	// Find a position nearby where we can safely drop an item
	bool FindDropTransform(const UItemData* ItemData, FTransform& Result) const;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContainerItemChanged, const FContainerItemData&, ContainerItem);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnContainerItemsChanged);

	UPROPERTY(BlueprintAssignable)
	FOnContainerItemChanged OnContainerItemChanged;

	UPROPERTY(BlueprintAssignable)
	FOnContainerItemsChanged OnContainerItemsChanged;

	UFUNCTION(BlueprintNativeEvent)
	void NotifyContainerItemChanged(const FContainerItemData& Item);

	UFUNCTION(BlueprintNativeEvent)
	void NotifyContainerItemsChanged();

	inline static int32 MaxItemsCapacity = MAX_int32;

	FORCEINLINE int32 GetItemsCapacity() const;
	bool HasItem(const FContainerItemData& ItemData) const;

	virtual void SetItemsCapacity(const int32 NewValue);
	
	UFUNCTION(BlueprintCallable,BlueprintPure)
	virtual int32 GetItemCapacity() const;
	
	UFUNCTION(BlueprintCallable)
	void DisplayContainerWidget();

#pragma region Item transactions
	// Split into two items by amount. Second item will be placed to the same container
	virtual FItemTransactionResult SplitItem(UPARAM(ref)  FContainerItemData& Data, const int32 SplitAmount);

	// Container->Container move. Calls UContainer::AddItem
	virtual FItemTransactionResult MoveItem(FContainerItemData& SourceItem);

	// Container->World move
	virtual FItemTransactionResult MoveItem(FContainerItemData& Item, AItem* OutItem);

	// World->Container move. Calls UContainer::AddItem
	virtual FItemTransactionResult MoveItem(AItem* WorldItem);
#pragma endregion 

#pragma region ContainerInterface

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual TArray<FContainerItemData> GetItems() override;
#pragma endregion
};
