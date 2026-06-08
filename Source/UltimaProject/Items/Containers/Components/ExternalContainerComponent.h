// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Game includes
#include "UltimaProject/Items/Containers/ContainerComponent.h"

// Generated include
#include "ExternalContainerComponent.generated.h"

/**
 * External container component (chests, shelves, pockets)
 */
UCLASS(ClassGroup=(Containers), meta=(BlueprintSpawnableComponent))
class ULTIMAPROJECT_API UExternalContainerComponent : public UContainerComponent
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TArray<FItemDataDefinition> DefaultItems;

public:
	UExternalContainerComponent();

	virtual void BeginPlay() override;
	// UContainerComponent
	// virtual TArray<FContainerItemData> GetItemsForDisplay(AController* InstigatorController) override;7
	// ~UContainerComponent
};
