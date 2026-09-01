// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Game incldues
#include "UltimaProject/GAS/Abilities/Interactions/GameplayAbility_Interaction.h"

// Engine includes
#include "UObject/Object.h"
#include "Engine/DataAsset.h"

// Generated include
#include "ItemData.generated.h"

class AItem;

USTRUCT(BlueprintType)
struct FItemInstanceData
{
	GENERATED_BODY()

	FItemInstanceData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Amount;

	bool operator==(const FItemInstanceData& Other) const
	{
		return Amount == Other.Amount;
	}
	
	bool IsValid() const;
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
	int64 MaxAmountPerStack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int64 Slots = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility_Interaction> PickupAbilityClass;
};

// Item data definition without an existing item
// Used only for item spawning
USTRUCT(BlueprintType)
struct FItemDataDefinition
{
	GENERATED_BODY()

	// Data asset with static props
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<const UItemDataAsset> StaticData;

	// Item runtime values ( amount, durability, etc )
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemInstanceData InstanceData;
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
	TSoftObjectPtr<const UItemDataAsset> StaticData;

	// Item runtime values ( amount, durability, etc )
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (ExposeOnSpawn="true"))
	FItemInstanceData InstanceData;

	// Create a duplicate item data with amount. The origin object amount will be reduced
	bool SplitItem(const int64 SplitAmount, FItemData& ResultItem);

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

	TSoftObjectPtr<const UItemDataAsset> GetStaticData() const;
	void SetStaticData(const UItemDataAsset* InStaticData);

	const FItemInstanceData& GetInstanceData() const;

	TSubclassOf<AItem> GetActorClass() const;
	
	bool IsValid() const;

	// Get a number of items that can be moved TO the TargetItem
	virtual int32 GetStackableAmount(const FItemData& TargetItem) const;

	virtual FText GetDisplayName() const;

	virtual UTexture2D* GetViewIcon() const;

	virtual int64 GetAmount() const;

	virtual int64 GetMaxAmountPerStack() const;

	virtual int64 SetAmount(const int64 Value);

	virtual int64 ModifyAmount(const int64 Value);

	virtual TSoftObjectPtr<UStaticMesh> GetStaticMesh() const;
};
