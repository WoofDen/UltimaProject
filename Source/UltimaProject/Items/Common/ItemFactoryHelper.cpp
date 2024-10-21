// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemFactoryHelper.h"
#include "Item.h"
#include "UltimaProject/Framework/UPGameMode.h"
#include "UltimaProject/Items/Containers/Container.h"

UItemFactoryHelper::UItemFactoryHelper()
{
}

bool UItemFactoryHelper::SpawnItemInContainer(const TSubclassOf<UItemData> Class, UContainer* Container)
{
	if (!Class)
	{
		return false;
	}

	UItemData* Data = Class->GetDefaultObject<UItemData>();
	if (!Data)
	{
		return false;
	}

	UItemData* ItemData = NewObject<UItemData>(GetTransientPackage(), Class);
	if (!ItemData)
	{
		return false;
	}


	// TODO better handling
	ensure(ItemData->Initialize(Data));
	return Container->AddItem(ItemData).IsSuccess();
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

	if (!IsValid(ItemData->GetActorClass()))
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
