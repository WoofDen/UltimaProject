// Fill out your copyright notice in the Description page of Project Settings.

#include "ContextActionHandler_OpenContainer.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Framework/UPPlayerController.h"

bool UContextActionHandler_OpenContainer::HandleAction(AUPPlayerController* PC, AItem* Item)
{
	NULLCHECK_RETURN(PC, false);

	UContainerComponent* ContainerComponent = IContainerOwnerInterface::Execute_GetMainContainerComponent(Item);

	if (PC->IsContainerOpened(ContainerComponent))
	{
		PC->TryCloseContainer(ContainerComponent);
	}
	else
	{
		PC->TryOpenContainer(ContainerComponent, EContainerRelationType::InWorldContainer);
	}

	return true;
}

bool UContextActionHandler_OpenContainer::IsAvailable(AUPPlayerController* PC, AItem* Item)
{
	UContainerComponent* ContainerComponent = IContainerOwnerInterface::Execute_GetMainContainerComponent(Item);
	NULLCHECK_RETURN(ContainerComponent, false);
	
	return ContainerComponent->IsAccessible(PC);
}
