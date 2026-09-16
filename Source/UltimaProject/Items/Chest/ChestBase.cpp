// Fill out your copyright notice in the Description page of Project Settings.

#include "ChestBase.h"

// Game includes
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Framework/UPPlayerController.h"
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

bool AChestBase::IsInteractionAccessible(const AController* InstigatorController) const
{
	UContainerComponent* Container = IContainerOwnerInterface::Execute_GetMainContainerComponent(this);
	NULLCHECK_RETURN(Container, false);

	return Container->IsAccessible(InstigatorController);
}

void AChestBase::AttemptInteraction(AController* InstigatorController, EInteractionType Type)
{
	AUPPlayerController* PC = Cast<AUPPlayerController>(InstigatorController);
	NULLCHECK(PC);
	
	if (PC->IsContainerOpened(ContainerComponent))
	{
		PC->TryCloseContainer(ContainerComponent);
	}
	else
	{
		PC->TryOpenContainer(ContainerComponent, EContainerRelationType::InWorldContainer);
	}
}
