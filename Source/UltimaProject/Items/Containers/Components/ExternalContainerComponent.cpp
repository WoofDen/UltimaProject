// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "ExternalContainerComponent.h"

#include "Net/UnrealNetwork.h"
#include "UltimaProject/Items/Common/ItemFactoryHelper.h"

UExternalContainerComponent::UExternalContainerComponent()
{
	SetIsReplicatedByDefault(true);
}

void UExternalContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Server only
	if (AActor* Owner = GetOwner(); Owner && Owner->HasAuthority())
	{
		for (const auto& Data : DefaultItems)
		{
			UItemFactoryHelper::SpawnItemInContainer(Data, this);
		}
	}
}

void UExternalContainerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, bIsExplicitlyEmpty, COND_None);
}

void UExternalContainerComponent::NotifyContainerItemsChanged_Implementation()
{
	Super::NotifyContainerItemsChanged_Implementation();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		// TODO is locked check
		// Cache the empty flag for clients 
		bIsExplicitlyEmpty = GetNumItems() == 0;
	}
}

bool UExternalContainerComponent::IsEmpty() const
{
	// Items are not replicated to clients for external containers
	return bIsExplicitlyEmpty;
}
