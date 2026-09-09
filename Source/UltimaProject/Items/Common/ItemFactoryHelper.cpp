// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemFactoryHelper.h"
#include "Item.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"

UItemFactoryHelper::UItemFactoryHelper()
{
}

bool UItemFactoryHelper::SpawnItemInContainer(const FItemDataDefinition& Definition,
                                                    UContainerComponent* Container)
{
	NULLCHECK_RETURN(Container, false);

	FItemData ItemData = FItemData(Definition);
	ItemData.LoadStaticData();

	if (!Container->AddItem(MoveTemp(ItemData)).IsSuccess())
	{
		return false;
	}

	return true;
}

bool UItemFactoryHelper::SpawnItemInContainerFromAsset(const UItemDataAsset* ItemDataAsset,
                                                             UContainerComponent* Container)
{
	NULLCHECK_RETURN(ItemDataAsset, false);

	FItemData ItemData;

	ItemData.SetStaticData(ItemDataAsset);
	ItemData.PreInitialize();
	ItemData.SetAmount(1);

	if (!Container->AddItem(MoveTemp(ItemData)).IsSuccess())
	{
		return false;
	}

	return true;
}

AItem* UItemFactoryHelper::SpawnItem(const UObject* WorldContextObject, const FItemDataDefinition& ItemData,
                                     const FTransform& Transform, bool DuplicateItemData/* = false*/)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	TSubclassOf<AItem> Actorclass = ItemData.StaticData->ActorClass;
	if (!ensureAlways(Actorclass))
	{
		return nullptr;
	}

	FActorSpawnParameters Parameters;
	AItem* Item = World->SpawnActorDeferred<AItem>(
		Actorclass,
		Transform,
		nullptr
	);

	if (!ensure(Item))
	{
		return nullptr;
	}

	FItemData NewItemData(ItemData);
	if (!Item->SetItemData(MoveTemp(NewItemData)))
	{
		Item->Destroy();
		return nullptr;
	}
	Item->FinishSpawning(Transform);

	return Item;
}
