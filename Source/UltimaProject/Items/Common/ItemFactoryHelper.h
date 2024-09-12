// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "ItemData.h"
#include "ItemFactoryHelper.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API UItemFactoryHelper : public UObject
{
	GENERATED_BODY()

public:
	UItemFactoryHelper();

	UFUNCTION(BlueprintCallable)
	static UItemData* CreateDefaultItemData(const TSubclassOf<UItemData> Class);

	// Creates in-world item
	UFUNCTION(BlueprintCallable)
	static AItem* SpawnDefaultItem(const UObject* WorldContextObject, const TSubclassOf<UItemData> Class,
	const FTransform& Transform);
	
	UFUNCTION(BlueprintCallable)
	static AItem* SpawnItem(const UObject* WorldContextObject, UItemData* ItemData,
								   const FTransform& Transform, bool DuplicateItemData = false);
};
