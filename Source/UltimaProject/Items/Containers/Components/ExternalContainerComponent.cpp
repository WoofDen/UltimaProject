// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "ExternalContainerComponent.h"
#include "UltimaProject/Items/Common/ItemFactoryHelper.h"

UExternalContainerComponent::UExternalContainerComponent()
{
}

void UExternalContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner(); Owner && Owner->HasAuthority())
	{
		for (const auto& Data : DefaultItems)
		{
			UItemFactoryHelper::SpawnItemInContainer(Data, this);
		}
	}
}
