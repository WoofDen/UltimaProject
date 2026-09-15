// Fill out your copyright notice in the Description page of Project Settings.

#include "ChestBase.h"

// Game includes
#include "UltimaProject/Items/Containers/Components/ExternalContainerComponent.h"

AChestBase::AChestBase()
{
	bReplicates = true;

	ContainerComponent = CreateDefaultSubobject<UExternalContainerComponent>(TEXT("ContainerComponent"));
	if (ensureAlways(ContainerComponent))
	{
		ContainerComponent->SetIsReplicated(false);
	}
}

UContainerComponent* AChestBase::GetMainContainerComponent_Implementation() const
{
	return ContainerComponent;
}
