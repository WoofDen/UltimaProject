// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "UObject/Object.h"
#include "ContextActionHandler.generated.h"

/**
 * ContextActionHandler-s are the handlers for context menu actions ( Use / Destroy / Fill / etc )
 * Class naming should include both item category and action ( _Use, _Use_Crops, _Fill etc )
 */
UCLASS(Blueprintable)
class ULTIMAPROJECT_API UContextActionHandler : public UObject
{
	GENERATED_BODY()

public:
	static bool HandleActionFromCDO(AUPPlayerController* PC, TSubclassOf<UContextActionHandler> HandlerClass, AItem* Item);

	virtual bool HandleAction(AUPPlayerController* PC, AItem* Item);
	virtual bool HandleAction(AUPPlayerController* PC, FContainerItemData& ItemData);

	virtual bool IsAvailable(AUPPlayerController* PC, AItem* Item);
};
