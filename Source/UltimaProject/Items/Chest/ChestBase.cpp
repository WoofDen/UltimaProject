// Fill out your copyright notice in the Description page of Project Settings.

#include "ChestBase.h"

// Game includes
#include "UltimaProject/Framework/UPPlayerController.h"
#include "UltimaProject/Items/Containers/Components/ExternalContainerComponent.h"

AChestBase::AChestBase()
{
	bReplicates = true;

	ContainerComponent = CreateDefaultSubobject<UExternalContainerComponent>(TEXT("ContainerComponent"));
}

UContainerComponent* AChestBase::GetMainContainerComponent_Implementation() const
{
	return ContainerComponent;
}
