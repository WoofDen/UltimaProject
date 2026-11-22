#pragma once

// Engine includes
#include "ContainerCategoriesDataAsset.h"
#include "ContainerTypes.h"
#include "Interfaces/ContainerOwnerInterface.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "UltimaProject/Items/Common/ItemData.h"
#include "UltimaProject/World/HeartbeatSystem/HeartbeatInterface.h"

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

	static uint32 InvalidHandle;

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
	FItemData& GetItemDataMutable() { return ItemData; }
	UContainerComponent* GetContainerComponent() const { return Container.Get(); };

protected:
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UContainerComponent> Container = nullptr;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	FItemData ItemData;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	int32 Handle = InvalidHandle;

	UPROPERTY(NotReplicated)
	int32 SlotIndex = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct FContainerLiquidData : public FFastArraySerializerItem
{
	GENERATED_BODY()

private:
	UPROPERTY()
	float Amount = 0;

public:
	UPROPERTY()
	FLiquidDescriptor Descriptor;

	float GetAmount() const
	{
		return Amount;
	}

	void AddAmount(const float InAmount)
	{
		Amount = FMath::Max(0.0f, Amount + InAmount);
	}

	void SetAmount(const float InAmount)
	{
		Amount = FMath::Max(0.0f, InAmount);
	}
};

USTRUCT()
struct FContainerItemArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FContainerItemData> Items;

	UPROPERTY()
	TWeakObjectPtr<UContainerComponent> ContainerComponent;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FContainerItemData, FContainerItemArray>(
			Items, DeltaParms, *this);
	}

	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
};

USTRUCT()
struct FContainerLiquidArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FContainerLiquidData> Liquids;

	UPROPERTY()
	TWeakObjectPtr<UContainerComponent> ContainerComponent;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FContainerLiquidData, FContainerLiquidArray>(
			Liquids, DeltaParms, *this);
	}

	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
};

template <>
struct TStructOpsTypeTraits<FContainerItemArray> : public TStructOpsTypeTraitsBase2<FContainerItemArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

template <>
struct TStructOpsTypeTraits<FContainerLiquidArray> : public TStructOpsTypeTraitsBase2<FContainerLiquidArray>
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

	FItemTransactionResult() = default;
	FItemTransactionResult(uint32 InRequestedAmount, uint32 InMovedAmount = 0);

	uint32 RequestedAmount = 0;
	uint32 MovedAmount = 0;

	// uint32 RemovedAsSideEffects = 0;

	uint32 SourceRecordsRemoved = 0;
	uint32 TargetRecordsRemoved = 0;

	inline bool IsSuccess(bool bAllowPartial) const
	{
		return bAllowPartial
			       ? MovedAmount > 0
			       : MovedAmount >= RequestedAmount;
	};
};

inline FItemTransactionResult InvalidItemTransactionResult = FItemTransactionResult(1, 0);

/**
 * Basic container impl. It does not relate on owner/actor and don't perform checks on any external conditions ( owner, player that moves item, etc )
 */
UCLASS(Abstract)
class UContainerComponent :
	public UActorComponent
{
	GENERATED_BODY()

	friend class UItemFactoryHelper;
	friend class UProxyContainerComponent;

	// mutable TWeakObjectPtr<UContainerCategoriesDataAsset> CategoryDataAssetCache;

	// Creates A NEW item and adds to container
	virtual FItemTransactionResult AddItem(FItemDataDefinition& ItemDataDefinition);

	// Adds an existing item ( UItemData ) to container
	virtual FItemTransactionResult AddItem(FItemData&& ItemData);

	// Adds an existing item ( UItemData ) to container, returns FContainerItemData
	virtual FItemTransactionResult AddItem(FItemData&& ItemData, uint32& ResultHandle);
	virtual bool RemoveItem(FContainerItemData& ItemData);

	static const TCHAR* EnumToString(EContainerCategory Category);

protected:
	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_Category)
	EContainerCategory Category = EContainerCategory::None;

	UPROPERTY(VisibleAnywhere, Replicated)
	FContainerItemArray ContainerItems;

	UPROPERTY(VisibleAnywhere, Replicated)
	FContainerLiquidArray LiquidsData;

	UPROPERTY(VisibleAnywhere)
	int32 SlotsCapacityMod = 0;

	UPROPERTY(EditAnywhere)
	int32 LiquidCapacityMod = 0;

	FContainerItemData& GetItemMutable(uint32 Handle) const;

	UFUNCTION()
	void OnRep_Category();

	// When all the essential data was replicated
	virtual void OnClientReady();

	// UActorComponent
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	// ~UActorComponent

	UContainerCategoriesDataAsset* GetCategoryData() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual int32 GetSlotsTotal() const;

	uint32 GetSlotsInUse() const;
	uint32 GetSlotsAvailable() const;

public:
	UContainerComponent();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContainerItemChanged, int32, Handle);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnContainerItemsChanged);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLiquidsChanged);

	UPROPERTY(BlueprintAssignable)
	FOnContainerItemChanged OnContainerItemChanged;

	UPROPERTY(BlueprintAssignable)
	FOnContainerItemsChanged OnContainerItemsChanged;

	UPROPERTY(BlueprintAssignable)
	FOnLiquidsChanged OnLiquidsChanged;

	virtual void OnClientOpened(class AUPPlayerController* Instigator);
	virtual void OnClientContainerClosed(class AUPPlayerController* Instigator);

	UFUNCTION(BlueprintNativeEvent)
	void NotifyContainerItemChanged(const FContainerItemData& Item);

	UFUNCTION(BlueprintNativeEvent)
	void NotifyContainerItemsChanged();

	UFUNCTION(BlueprintNativeEvent)
	void NotifyLiquidsChanged();

	void SetCategory(EContainerCategory InCategory);

	inline static int32 MaxItemsCapacity = MAX_int32;

	virtual bool HasItem(uint32 Handle) const;
	const FContainerItemData& GetItem(uint32 Handle) const;

	virtual TSubclassOf<UContainerWidget> GetContainerWidgetClass() const;
	float GetInteractionRadius() const;

	// Get container location
	virtual FVector GetContainerOrigin() const;
	IContainerOwnerInterface* GetOwnerInterface() const;
	uint32 GenerateItemHandle() const;

#pragma region Accessors

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual TArray<FContainerItemData> GetItems();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual TArray<FContainerItemData> GetItemsForDisplay(AController* InstigatorController);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual const TArray<FContainerLiquidData>& GetLiquids() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual float GetLiquidCapacity();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetLiquidCapacityUsed(bool bNormalized);

#pragma endregion

#pragma region Server low-level ITEM transactions

protected:
	// Container->Container move. Calls UContainer::AddItem
	virtual FItemTransactionResult MoveItem(UContainerComponent* SourceContainer, uint32 Handle, uint32 AmountToMove = UINT32_MAX);

	// Container->World move
	virtual FItemTransactionResult MoveItem(uint32 Handle, AItem* OutItem, uint32 AmountToMove = UINT32_MAX);

	// World->Container move. Calls UContainer::AddItem
	virtual FItemTransactionResult MoveItem(AItem* WorldItem, uint32 AmountToMove = UINT32_MAX);
#pragma endregion

#pragma region Server low-level LIQUID transactions
	// Container->Container liquid move
	virtual FItemTransactionResult MoveLiquid(UContainerComponent* SourceContainer, float SourceAmountReal);

	FItemTransactionResult MoveLiquid(const FLiquidDescriptor& LiquidDescriptor, float SafeAmount);

	FItemTransactionResult RemoveLiquid(const float Amount);
	FItemTransactionResult RemoveLiquid(const float Amount, TMap<FLiquidDescriptor, float>& RemovedLiquids);

#pragma endregion

#pragma region Helpers & validators

public:
	// Returns self unless the container represents another container (see UProxyContainerComponent)
	virtual UContainerComponent* GetOriginContainer();
	virtual UContainerComponent* GetListenContainer();

	// Find a position nearby where we can safely drop an item
	bool FindDropTransform(uint32 ContainerItemDataHandle, FTransform& Result) const;

	// TODO split the logic - HasAccess to the container and HasAccess to the item
	// Check can we move an external actor item to this container
	virtual bool CanStoreItem(const AController* Instigator, const AItem* Item) const;

	bool CanStoreLiquids() const;

	// Can reach & access this container
	virtual bool IsAccessible(const AController* Instigator) const;

	uint32 GetNumItems() const;

	virtual bool IsEmpty() const;
#pragma endregion

#pragma region Server top-level ITEM operations

public:
	// Add item from an in-world actor
	// World->Container
	void StoreItem(AItem* WorldItem, uint32 Amount);

	// Container->Container
	void RelocateItem(UContainerComponent* SourceContainer, uint32 Handle, uint32 Amount);

	// Container->World
	AItem* DropItem(uint32 Handle, uint32 Amount);

	uint32 SpawnItem(const FItemDataDefinition& ItemDataDefinition);
#pragma endregion

#pragma region Server top-level LIQUID operations
	// Move liquid from another container
	void StoreLiquid(UContainerComponent* ContainerComponent, float Amount);

	// Create liquid in container
	void SpawnLiquid(const FLiquidDescriptor& LiquidDescriptor, float Amount, bool bForceOverflow);

	// Remove fraction of liquids from container
	void ConsumeLiquid(const float Amount);

	// Remove a fraction of liquids from the container and return
	TMap<FLiquidDescriptor, float> ExtractLiquid(const float Amount);
#pragma endregion
};
