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

	UPROPERTY(EditAnywhere)
	float Amount;
	
	bool operator==(const FItemInstanceData& Other) const
	{
		return Amount == Other.Amount;
	}
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

/**
 * ItemData
 * Represent a single item within a world or a pre-set ( in case of inherited BP ) of item settings
 * Has to be always as an in-world item ( AItem ) or container ( FContainerItemData )
 */
UCLASS(Blueprintable, BlueprintType)
class ULTIMAPROJECT_API UItemData : public UObject
{
	GENERATED_BODY()

	friend class AItem;
	friend class UContainerComponent;

protected:
	// Data asset with static props
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, meta = (ExposeOnSpawn="true"))
	TSoftObjectPtr<const UItemDataAsset> StaticData;

	// Item runtime values ( amount, durability, etc )
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated)
	FItemInstanceData InstanceData;

	// Create a duplicate item data with amount. The origin object amount will be reduced
	UItemData* SplitItem(const int64 SplitAmount);

public:
	UItemData();
	UItemData(FObjectInitializer& Initializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;

	virtual bool Initialize(UItemData* Source = nullptr);

	TSoftObjectPtr<const UItemDataAsset> GetStaticData() const;
	void SetStaticData(const UItemDataAsset* InStaticData);

	const FItemInstanceData& GetInstanceData() const;

	TSubclassOf<AItem> GetActorClass() const;
	
	// Get a number of items that can be moved TO the TargetItem
	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual int32 GetStackableAmount(const UItemData* TargetItem) const;

	UFUNCTION(BlueprintCallable)
	virtual FText GetDisplayName() const;

	UFUNCTION(BlueprintCallable)
	virtual UTexture2D* GetViewIcon() const;

	UFUNCTION(BlueprintCallable)
	virtual int64 GetAmount() const;

	UFUNCTION(BlueprintCallable)
	virtual int64 GetMaxAmountPerStack() const;

	UFUNCTION(BlueprintCallable)
	virtual int64 SetAmount(const int64 Value);
	
	UFUNCTION(BlueprintCallable)
	virtual int64 ModifyAmount(const int64 Value);

	UFUNCTION(BlueprintCallable)
	virtual TSoftObjectPtr<UStaticMesh> GetStaticMesh() const;
};
