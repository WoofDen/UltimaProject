// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Game includes
#include "UltimaProject/Items/Containers/Components/ExternalContainerComponent.h"
#include "UltimaProject/Items/Containers/Interfaces/ContainerOwnerInterface.h"

// Generated includes
#include "ChestBase.generated.h"

// Base for any in-world container
UCLASS(Blueprintable)
class ULTIMAPROJECT_API AChestBase : public AActor, public IContainerOwnerInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UExternalContainerComponent> ContainerComponent;

public:
	AChestBase();

	// IContainerInterface
	virtual UContainerComponent* GetMainContainerComponent_Implementation() const override;

	//UFUNCTION(Server, Unreliable)
	//virtual void CallServerTryStoreItem(AController* InstigatorController, const FContainerItemData& ItemData) override;
	// ~IContainerInterface
};
