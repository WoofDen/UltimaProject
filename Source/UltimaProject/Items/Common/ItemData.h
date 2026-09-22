// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Game incldues
#include "UltimaProject/GAS/Abilities/Interactions/GameplayAbility_Interaction.h"

// Engine includes
#include "UObject/Object.h"
#include "Engine/DataAsset.h"

// Generated include
#include "ItemData.generated.h"

UENUM()
enum class EItemIntProperty : uint8
{
	IP_Humidity = 0,
	IP_Temperature = 1,
	
	MAX
};

namespace UP::ItemProperty
{
	namespace Temperature
	{
		constexpr uint8 Frozen = UINT8_MAX * .1f;
		constexpr uint8 Cold = UINT8_MAX * .25f;
		constexpr uint8 Default = UINT8_MAX * .5f;
		constexpr uint8 Hot = UINT8_MAX * .75f;
		constexpr uint8 Burning = UINT8_MAX * 0.9;
	}

	namespace Humidity
	{
		constexpr uint8 Default = UINT8_MAX * 0.1f;
		constexpr uint8 Moist = UINT8_MAX * 0.5f;
		constexpr uint8 Wet = UINT8_MAX * 0.7f;
	}
}

class AItem;

USTRUCT(BlueprintType)
struct FItemInstanceData
{
	GENERATED_BODY()

	FItemInstanceData();

	TArray<uint8> IntProps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount;

	bool operator==(const FItemInstanceData& Other) const
	{
		return Amount == Other.Amount
			&& IntProps == Other.IntProps;
	}

	bool IsValid() const;
	void SetProp(EItemIntProperty Prop, uint8 Value);

	int8 GetIntProp(EItemIntProperty Prop) const;
	float GetIntPropNormalized(EItemIntProperty Prop) const;
};

/**
 * Static item data
 */
UCLASS(BlueprintType)
class UItemDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AItem> ActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxAmountPerStack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UObject> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Slots = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility_Interaction> PickupAbilityClass;
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ExposeOnSpawn="true"))
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

	static FItemData EmptyItem;

	bool PreInitialize(FItemData* Source = nullptr);
	bool PreInitialize(const FItemDataDefinition& Definition);

	FItemDataDefinition GetDataDefinition() const;

	const UItemDataAsset* GetStaticData() const;
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
};
