// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Game includes
#include "UltimaProject/Items/Containers/Components/ExternalContainerComponent.h"
#include "UltimaProject/Items/Containers/Interfaces/ContainerInterface.h"

// Generated includes
#include "ChestBase.generated.h"

// Base for any in-world container
UCLASS(Blueprintable)
class ULTIMAPROJECT_API AChestBase : public AActor, public IContainerInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UExternalContainerComponent> ContainerComponent;

public:
	AChestBase();

	// IActorContainerInterface
	virtual UContainerComponent* GetContainerComponent_Implementation() const override;
	virtual bool CanBeOpened(const class AUPPlayerController* Controller) override;
	// ~IActorContainerInterface
};
