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

void AChestBase::BeginDestroy()
{
	if (HasAuthority())
	{
		GetAccessibilityChangedDelegate().Broadcast(this);
	}
	
	Super::BeginDestroy();
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

FOnContainerAccessibilityUpdated AChestBase::GetAccessibilityChangedDelegate() const
{
	ensureAlways(HasAuthority());
	return OnChestAccessibilityChanged;
}
