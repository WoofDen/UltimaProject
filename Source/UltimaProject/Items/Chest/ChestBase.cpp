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

UContainerComponent* AChestBase::GetContainerComponent_Implementation() const
{
	return ContainerComponent;
}

bool AChestBase::CanBeOpened(const class AUPPlayerController* Controller)
{
	// TODO Locked logic, skip for now, visibility & distance check 
	
	return true; 
}
