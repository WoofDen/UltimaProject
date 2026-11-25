// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UltimaProject/Items/Containers/ContainerTypes.h"
#include "UObject/Object.h"
#include "ItemData.generated.h"

class UContextActionHandler;

UENUM(BlueprintType)
enum class EItemNumericProperty : uint8
{
	Humidity = 0,
	Temperature = 1,
	Grow = 2,
	Durability = 3,
	Decay = 4,

	MAX UMETA(Hidden)
};

class AItem;
class UContainerComponent;

/**
 * FItemLocation
 * Describes ItemData location
 * If the item is an in-world item, the Item property will be set
 * Otherwise, ContainerComponent & ContainerItemHandle expected to be set
 */
USTRUCT(BlueprintType)
struct FItemDataLocation
{
	GENERATED_BODY()

	FItemDataLocation() = default;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AItem> Item;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<class UContainerComponent> ContainerComponent;

	UPROPERTY(BlueprintReadOnly)
	int32 ContainerItemHandle = 0;
};

USTRUCT()
struct FItemProperties
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<EItemNumericProperty> PropertyKeys;

	UPROPERTY()
	TArray<float> PropertyValues;

	UPROPERTY(EditAnywhere)
	FLiquidDescriptor SolidSubstance;

	bool operator==(const FItemProperties& Other) const
	{
		return PropertyKeys == Other.PropertyKeys
			&& PropertyValues == Other.PropertyValues
			&& SolidSubstance == Other.SolidSubstance;
	}
};

USTRUCT(BlueprintType)
struct FItemInstanceData
{
	GENERATED_BODY()

	FItemInstanceData();
	FItemInstanceData(uint32 InAmount);

	UPROPERTY(EditAnywhere)
	FItemProperties Properties;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount;

	bool operator==(const FItemInstanceData& Other) const
	{
		return Amount == Other.Amount
			&& Properties == Other.Properties;
	}

	bool IsValid() const;

	// Don't add methods here but FItemData
};

// Used to describe actions within an item's context menu (Use, Fill, Destroy, etc)
USTRUCT(BlueprintType)
struct FItemContextAction
{
	GENERATED_BODY()

	virtual ~FItemContextAction() = default;

	UPROPERTY(EditAnywhere)
	FText Label;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UContextActionHandler> ContextActionHandlerClass;
};

/**
 * Static item data
 */
UCLASS(BlueprintType)
class UItemDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UItemDataAsset();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AItem> ActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="General")
	FText Name;

	// Using item, call context menus, activate or open containers radius
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="General")
	float InteractionRadius = 500.f;

	UPROPERTY(EditDefaultsOnly, Category="General")
	FItemProperties DefaultProperties;

	UPROPERTY(EditDefaultsOnly, Category="General")
	TSubclassOf<class UHeartbeatItemProcessor> HeartbeatProcessorClass;

	UPROPERTY(EditDefaultsOnly, Category="General")
	TArray<FItemContextAction> ContextActions;

	// Could be stored in inventory
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Containers")
	bool bPickable = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Containers", meta=(EditCondition = bPickable, EditConditionHides))
	int32 MaxAmountPerStack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Containers", meta=(EditCondition = bPickable, EditConditionHides))
	TSoftObjectPtr<UObject> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Containers", meta=(EditCondition = bPickable, EditConditionHides))
	int32 Slots = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Overrides")
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Overrides")
	bool bOverrideWorldTransform = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Overrides", meta=(EditCondition = bOverrideWorldTransform, EditConditionHides))
	FTransform WorldMeshTransform;
};

// Item data definition without an existing item
// Used only for item spawning
USTRUCT(BlueprintType)
struct FItemDataDefinition
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	mutable TObjectPtr<const UItemDataAsset> StaticData;

public:
	FItemDataDefinition();
	FItemDataDefinition(const FItemData& Item);
	FItemDataDefinition(TSoftObjectPtr<const UItemDataAsset> StaticDataIn, FItemInstanceData InstanceDataIn);

	// Data asset with static props
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<const UItemDataAsset> StaticDataSoftPtr;

	// Item runtime values ( amount, durability, etc )
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemInstanceData InstanceData;

	bool IsValid() const
	{
		return StaticDataSoftPtr.IsValid() && InstanceData.IsValid();
	};

	void ResetCache() const
	{
		StaticData = nullptr;
	}

	const UItemDataAsset* GetStaticData() const
	{
		if (!StaticData)
		{
			StaticData = StaticDataSoftPtr.LoadSynchronous();
		}

		return StaticData;
	}
};

/**
 * ItemData
 * Represent a single item within a world. Exists only as in-world item ( AItem ) or in-container item ( FContainerItemData )
 */
USTRUCT(Blueprintable, BlueprintType)
struct ULTIMAPROJECT_API FItemData
{
	GENERATED_BODY()

	friend class AItem;
	friend class UContainerComponent;

protected:
	// Data asset with static props
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<const UItemDataAsset> StaticDataSoftPtr;

	UPROPERTY(Transient, BlueprintReadOnly)
	mutable TObjectPtr<const UItemDataAsset> StaticData;

	// Item runtime values ( amount, durability, etc )
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (ExposeOnSpawn="true"))
	FItemInstanceData InstanceData;

public:
	FItemData();
	FItemData(const FItemDataDefinition& Definition);
	virtual ~FItemData() = default;

	// ItemData cannot be compared, compare in-world actors or FContainerItemData
	bool operator==(const FItemData&) const = delete;

	static FItemData InvalidItemData;

	bool PreInitialize(FItemData* Source = nullptr);
	bool PreInitialize(const FItemDataDefinition& Definition);

	FItemDataDefinition GetDataDefinition() const;

	const UItemDataAsset* GetStaticData() const;

	bool Is(const TSoftObjectPtr<UItemDataAsset>& DataAssetSoftPtr) const;

	void SetStaticData(const UItemDataAsset* InStaticData);
	void LoadStaticData();

	const FItemInstanceData& GetInstanceData() const;

	TSubclassOf<AItem> GetActorClass() const;

	bool IsValid() const;

	// Get a number of items that can be moved TO the TargetItem
	virtual int32 GetStackableAmount(const FItemData& TargetItem) const;

	virtual FText GetDisplayName() const;

	UObject* GetViewIcon() const;

	virtual uint32 GetAmount() const;

	virtual uint32 GetMaxAmountPerStack() const;

	virtual uint32 SetAmount(const uint32 Value);

	virtual uint32 ModifyAmount(const int32 Value);

	virtual TSoftObjectPtr<UStaticMesh> GetStaticMesh() const;

	// Props
	void SetNumericProp(EItemNumericProperty Prop, float Value);
	float GetNumericProp(EItemNumericProperty Prop, float DefaultValue = 0.f) const;
	bool IsSupportProp(EItemNumericProperty Prop) const;
	void InitializeDefaultProperties();
};
