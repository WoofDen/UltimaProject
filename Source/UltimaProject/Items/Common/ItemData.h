// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/Object.h"
#include "ItemData.generated.h"

class AItem;

USTRUCT(BlueprintType)
struct FItemInstanceData
{
	GENERATED_BODY()

	FItemInstanceData();

	UPROPERTY(EditAnywhere)
	float Amount;
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
};

/**
 * ItemData
 * Represent a single item within world. Has to be always as in-world item ( AItem ) or container ( FContainerItemData )
 */
UCLASS(Blueprintable)
class ULTIMAPROJECT_API UItemData : public UObject
{
	GENERATED_BODY()

	friend class AItem;
	friend class UContainer;

protected:
	// Data asset with static props
	UPROPERTY(EditDefaultsOnly, Replicated)
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

	const FItemInstanceData& GetInstanceData() const;

	TSubclassOf<AItem> GetActorClass() const;

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
	virtual TSoftObjectPtr<UStaticMesh> GetStaticMesh() const;
};
