// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemData.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

/**
 * In-world representation of every item
 */
UCLASS(Blueprintable, BlueprintType)
class ULTIMAPROJECT_API AItem : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UItemData> Data;
	
public:
	AItem();
	AItem(UItemData* ItemData);

	bool SetItemData(UItemData* NewData = nullptr);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
