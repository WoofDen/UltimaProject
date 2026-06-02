// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemFactoryHelper.h"
#include "Item.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Framework/UPGameMode.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"

UItemFactoryHelper::UItemFactoryHelper()
{
}

UItemData* UItemFactoryHelper::SpawnItemInContainer(const TSubclassOf<UItemData> Class, UContainerComponent* Container)
{
	NULLCHECK_RETURN(Class, nullptr);

	UItemData* Data = Class->GetDefaultObject<UItemData>();
	NULLCHECK_RETURN(Data, nullptr);

	UItemData* ItemData = NewObject<UItemData>(GetTransientPackage(), Class);
	NULLCHECK_RETURN(ItemData, nullptr);


	ItemData->Initialize(Data);
	if (!Container->AddItem(ItemData).IsSuccess())
	{
		ItemData->MarkAsGarbage();
		return nullptr;
	}

	return ItemData;
}

UItemData* UItemFactoryHelper::SpawnItemInContainerFromAsset(const UItemDataAsset* ItemDataAsset,
                                                       UContainerComponent* Container)
{
	NULLCHECK_RETURN(ItemDataAsset, nullptr);

	UItemData* ItemData = NewObject<UItemData>(GetTransientPackage(), UItemData::StaticClass());
	NULLCHECK_RETURN(ItemData, nullptr);

	ItemData->SetStaticData(ItemDataAsset);
	ItemData->Initialize();
	ItemData->SetAmount(1);

	if (!Container->AddItem(ItemData).IsSuccess())
	{
		ItemData->MarkAsGarbage();
		return nullptr;
	}

	return ItemData;
}

AItem* UItemFactoryHelper::SpawnItemInWorld(const UObject* WorldContextObject, const TSubclassOf<UItemData> Class,
                                            const FTransform& Transform)
{
	check(Class);

	UItemData* DefaultItemData = Class->GetDefaultObject<UItemData>();
	if (!DefaultItemData)
	{
		return nullptr;
	}

	return SpawnItem(WorldContextObject, DefaultItemData, Transform);
}

AItem* UItemFactoryHelper::SpawnItem(const UObject* WorldContextObject, UItemData* ItemData,
                                     const FTransform& Transform, bool DuplicateItemData/* = false*/)
{
	check(ItemData);

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	if (!ensureAlways(ItemData->GetActorClass()))
	{
		return nullptr;
	}

	FActorSpawnParameters Parameters;
	AItem* Item = World->SpawnActorDeferred<AItem>(
		ItemData->GetActorClass(),
		Transform,
		nullptr
	);

	if (!ensure(Item))
	{
		return nullptr;
	}

	UItemData* NewItemData;
	if (DuplicateItemData)
	{
		NewItemData = DuplicateObject(ItemData, Item);
		if (!NewItemData)
		{
			Item->Destroy();
			return nullptr;
		}
	}
	else
	{
		NewItemData = ItemData;
	}

	Item->SetItemData(NewItemData);
	Item->FinishSpawning(Transform);
	return Item;
}
