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
	UPROPERTY(EditAnywhere)
	TArray<FItemDataDefinition> DefaultItems;

	UPROPERTY(VisibleAnywhere, Replicated)
	bool bIsExplicitlyEmpty = false;

public:
	UExternalContainerComponent();

	virtual void BeginPlay() override;

	// UActorComponent
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	// ~UActorComponent

	// UContainerComponent
	virtual void NotifyContainerItemsChanged_Implementation() override;
	virtual bool IsEmpty() const override;
	// ~UContainerComponent
};
