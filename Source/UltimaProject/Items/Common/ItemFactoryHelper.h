// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "ItemData.h"
#include "UltimaProject/Items/Containers/Container.h"
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
	static bool SpawnItemInContainer(const TSubclassOf<UItemData> Class, UContainer* Container);

	// Creates in-world item
	UFUNCTION(BlueprintCallable)
	static AItem* SpawnItemInWorld(const UObject* WorldContextObject, const TSubclassOf<UItemData> Class,
	const FTransform& Transform);
	
	UFUNCTION(BlueprintCallable)
	static AItem* SpawnItem(const UObject* WorldContextObject, UItemData* ItemData,
								   const FTransform& Transform, bool DuplicateItemData = false);
};
