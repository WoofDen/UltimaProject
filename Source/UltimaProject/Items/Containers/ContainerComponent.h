#pragma once

// Engine includes
#include "Net/Serialization/FastArraySerializer.h"
#include "UltimaProject/Framework/UPPlayerController.h"
#include "UltimaProject/Items/Common/ItemData.h"

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
	friend class UProxyContainerComponent;

	GENERATED_BODY()

	bool operator==(const FContainerItemData& Other) const;
	FContainerItemData(FItemData&& InitData, UContainerComponent* InitContainer, const int32 InitSlotIndex);
	FContainerItemData();

	uint32 GetHandle() const { return Handle; }
	int32 GetSlot() const { return SlotIndex; }

	// Sugar
	uint32 GetAmount() const { return ItemData.GetAmount(); }
	uint32 GetMaxAmountPerStack() const { return ItemData.GetMaxAmountPerStack(); };
	uint32 SetAmount(const int32 Value) { return ItemData.SetAmount(Value); }
	uint32 ModifyAmount(const int32 Value) { return ItemData.ModifyAmount(Value); }

	bool IsValid() const { return ItemData.IsValid() && Container.Get(); }
	bool IsInContainer(const UContainerComponent* AnotherContainer) const { return Container == AnotherContainer; }

	const FItemData& GetItemData() const { return ItemData; }
	UContainerComponent* GetContainerComponent() const { return Container.Get(); };

protected:
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UContainerComponent> Container = nullptr;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	FItemData ItemData;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	int32 Handle;

	UPROPERTY(NotReplicated)
	int32 SlotIndex = INDEX_NONE;
};

USTRUCT()
struct FContainerItems : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
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
	uint32 MovedAmount = 0;

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
class UContainerComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UItemFactoryHelper;
	friend class UProxyContainerComponent;

	UPROPERTY(VisibleAnywhere, Transient, Replicated)
	FContainerItems ContainerItems;

	uint32 GetSlotsInUse() const;
	uint32 GetSlotsAvailable() const;

	// Creates A NEW item and adds to container
	virtual FItemTransactionResult AddItem(FItemDataDefinition& ItemDataDefinition);

	// Adds an existing item ( UItemData ) to container
	virtual FItemTransactionResult AddItem(FItemData&& ItemData);

	// Adds an existing item ( UItemData ) to container, returns FContainerItemData
	virtual FItemTransactionResult AddItem(FItemData&& ItemData, FContainerItemData& AddedItem);
	virtual bool RemoveItem(FContainerItemData& ItemData);

protected:
	UPROPERTY(EditDefaultsOnly)
	int32 ItemSlotsCapacity = 10;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UContainerWidget> ContainerWidgetClass;

	// UActorComponent
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void BeginPlay() override;
	// ~UActorComponent

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
	virtual bool HasItem(const FContainerItemData& ItemData) const;

	virtual void SetItemsCapacity(const int32 NewValue);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual int32 GetItemCapacity() const;

	virtual TSubclassOf<UContainerWidget> GetContainerWidgetClass() const;
	void SetContainerWidgetClass(TSubclassOf<UContainerWidget> Class);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual TArray<FContainerItemData> GetItems();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual TArray<FContainerItemData> GetItemsForDisplay(AController* InstigatorController);

	IContainerInterface* GetOwnerInterface() const;
	uint32 GenerateItemHandle() const;

#pragma region Server low-level item transactions

protected:
	// Container->Container move. Calls UContainer::AddItem
	virtual FItemTransactionResult MoveItem(FContainerItemData& SourceItem, uint32 AmountToMove = UINT32_MAX);

	// Container->World move
	virtual FItemTransactionResult MoveItem(FContainerItemData& SourceItem, AItem* OutItem, uint32 AmountToMove = UINT32_MAX);

	// World->Container move. Calls UContainer::AddItem
	virtual FItemTransactionResult MoveItem(AItem* WorldItem, uint32 AmountToMove = UINT32_MAX);
#pragma endregion

#pragma region Helpers & validators

public:
	// Returns self unless the container represents another container (see UProxyContainerComponent)
	virtual UContainerComponent* GetOriginContainer();
	virtual UContainerComponent* GetListenContainer();

	// Find a position nearby where we can safely drop an item
	bool FindDropTransform(const FItemData& ItemData, FTransform& Result) const;

	// TODO split the logic - HasAccess to the container and HasAccess to the item
	// Check can we move an external actor item to this container
	virtual bool CanStoreItem(const AController* Instigator, const AItem* Item) const;
#pragma endregion

#pragma region Client top-level item operations

public:
	// Store an external item
	virtual void TryStoreItem(AController* Instigator, AItem* Item);

	UFUNCTION(BlueprintCallable)
	virtual bool TryDropItem(AController* Instigator, UPARAM(ref) const FContainerItemData& Item);

#pragma endregion

#pragma region Server top-level item operations

public:
	// Store item from another container
	UFUNCTION(Server, Unreliable)
	virtual void ServerTryStoreItem(AController* Instigator, const FContainerItemData& ItemData);

	UFUNCTION(Server, Unreliable)
	void ServerTryDropItem(AController* Instigator, const FContainerItemData& Item);
#pragma endregion
};
