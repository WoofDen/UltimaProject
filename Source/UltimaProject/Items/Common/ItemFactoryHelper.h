// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Item.h"
#include "ItemData.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
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
	static FItemData& SpawnItemInContainer(const FItemDataDefinition& Definition, UContainerComponent* Container);

	UFUNCTION(BlueprintCallable)
	static FItemData&
	SpawnItemInContainerFromAsset(const UItemDataAsset* ItemDataAsset, UContainerComponent* Container);

	UFUNCTION(BlueprintCallable)
	static AItem* SpawnItem(const UObject* WorldContextObject, const FItemDataDefinition ItemDefinition,
	                        const FTransform& Transform, bool DuplicateItemData = false);
};
