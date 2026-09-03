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
	int32 Amount;

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
	int32 MaxAmountPerStack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

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
	
	FItemDataDefinition();
	FItemDataDefinition(const FItemData& Item);
	FItemDataDefinition(TSoftObjectPtr<const UItemDataAsset> StaticDataIn, FItemInstanceData InstanceDataIn);

	// Data asset with static props
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<const UItemDataAsset> StaticData;

	// Item runtime values ( amount, durability, etc )
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemInstanceData InstanceData;
	
	bool IsValid() const
	{
		return StaticData.IsValid() && InstanceData.IsValid();
	};
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

	virtual uint32 GetAmount() const;

	virtual uint32 GetMaxAmountPerStack() const;

	virtual uint32 SetAmount(const uint32 Value);

	virtual uint32 ModifyAmount(const int32 Value);

	virtual TSoftObjectPtr<UStaticMesh> GetStaticMesh() const;
};
