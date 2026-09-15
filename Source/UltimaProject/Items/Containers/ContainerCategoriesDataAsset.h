// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ContainerCategoriesDataAsset.generated.h"

// Container category is used to bound containers to its static data ( widgets, interaction radius, ... )
UENUM(Blueprintable)
enum class EContainerCategory : uint8
{
	None = 0,

	Inventory = 1,
	ChestBase,

	Foraging = 10
};


UCLASS()
class ULTIMAPROJECT_API UContainerCategoriesDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EContainerCategory Category;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UContainerWidget> ContainerWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	float InteractionRadius = 100.f;
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
