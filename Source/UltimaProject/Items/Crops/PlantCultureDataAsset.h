// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "UltimaProject/Items/Common/ItemData.h"
#include "PlantCultureDataAsset.generated.h"

/**
 * Describes a plant that could be ... planted
 */

USTRUCT()
struct FPlantCultureGrowVisuals
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMesh> StaticMesh;
	
	// How big
	UPROPERTY(EditAnywhere, meta=(ClampMin = 0.1))
	FVector2D ScaleRange = FVector2D::ZeroVector;
	
	// How many
	UPROPERTY(EditAnywhere)
	FIntVector2 CountRange = FIntVector2::ZeroValue;
};

USTRUCT()
struct FPlantCultureHarvest
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UItemDataAsset> ItemDataAsset;

	UPROPERTY(EditAnywhere)
	float BaseChance = 1.f;
	
	UPROPERTY(EditAnywhere, meta=(ClampMin = 0.1))
	FVector2D BaseAmount = FVector2D::ZeroVector;
};

UCLASS()
class ULTIMAPROJECT_API UPlantCultureDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 GrowTime;
	
	// Field extent ( i.e. each dimension size is FieldSize * 2 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FieldSize;
	
	UPROPERTY(EditAnywhere)
	TMap<float, FPlantCultureGrowVisuals> ProgressVisuals;
	
	UPROPERTY(EditAnywhere)
	TArray<FPlantCultureHarvest> Harvests;
};
