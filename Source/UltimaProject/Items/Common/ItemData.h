// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/Object.h"
#include "ItemData.generated.h"

USTRUCT()
struct FItemInstanceData
{
	GENERATED_BODY()

	FItemInstanceData();

	UPROPERTY(EditDefaultsOnly)
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
	UPROPERTY(EditDefaultsOnly)
	FText Name;

	UPROPERTY(EditDefaultsOnly)
	int64 MaxAmountPerStack;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UTexture2D> Icon;
};

/**
 * InstanceItemData
 */
UCLASS(Blueprintable)
class ULTIMAPROJECT_API UItemData : public UObject
{
	GENERATED_BODY()

	// Data asset with static props
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<const UItemDataAsset> StaticData;

	// Item runtime values ( amount, durability, etc )
	UPROPERTY(EditDefaultsOnly)
	FItemInstanceData InstanceData;

public:
	UItemData();

	virtual bool Initialize(UItemData* Source = nullptr);

	TObjectPtr<const UItemDataAsset> GetStaticData() const;

	UFUNCTION(BlueprintCallable)
	virtual UTexture2D* GetViewIcon() const;

	UFUNCTION(BlueprintCallable)
	virtual int64 GetAmount() const;

	UFUNCTION(BlueprintCallable)
	virtual int64 GetMaxAmountPerStack() const;

	UFUNCTION(Blueprintable)
	virtual int64 SetAmount(const int64 Value);
};
