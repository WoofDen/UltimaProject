// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemFactoryHelper.h"
#include "Item.h"
#include "UltimaProject/Framework/UPGameMode.h"

UItemFactoryHelper::UItemFactoryHelper()
{
}

UItemData* UItemFactoryHelper::CreateDefaultItemData(const TSubclassOf<UItemData> Class)
{
	if (!Class)
	{
		return nullptr;
	}

	UItemData* Data = Class->GetDefaultObject<UItemData>();
	if (!Data)
	{
		return nullptr;
	}

	UItemData* ItemData = NewObject<UItemData>(GetTransientPackage(), Class);
	if (!ItemData)
	{
		return nullptr;
	}

	// TODO better handling
	ensure(ItemData->Initialize(Data));
	return ItemData;
}

AItem* UItemFactoryHelper::SpawnDefaultItem(const UObject* WorldContextObject, const TSubclassOf<UItemData> Class,
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
	AUPGameMode* GameMode = World->GetAuthGameMode<AUPGameMode>();
	if (!GameMode || !IsValid(GameMode->ItemBaseClass))
	{
		return nullptr;
	}

	FActorSpawnParameters Parameters;
	AItem* Item = World->SpawnActorDeferred<AItem>(
		GameMode->ItemBaseClass,
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
