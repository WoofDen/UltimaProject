// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
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

UCLASS()
class ULTIMAPROJECT_API UPlantCultureDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 GrowTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FieldSize;
	
	UPROPERTY(EditAnywhere)
	TMap<float, FPlantCultureGrowVisuals> ProgressVisuals;
};
