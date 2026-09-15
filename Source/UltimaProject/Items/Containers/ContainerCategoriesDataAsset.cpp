// Fill out your copyright notice in the Description page of Project Settings.

#include "ContainerCategoriesDataAsset.h"

FPrimaryAssetId UContainerCategoriesDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("ContainerCategory", GetFName());
}
