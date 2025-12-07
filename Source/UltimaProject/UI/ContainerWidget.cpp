// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "ContainerWidget.h"

void UContainerWidget::OnContainerItemsChange()
{
	HandleContainerItemsChange();
}

void UContainerWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (ContainerComponent.IsValid())
	{
		ContainerComponent->OnContainerItemsChanged.RemoveDynamic(this, &ThisClass::OnContainerItemsChange);
	}
}

void UContainerWidget::SetContainerComponent(UContainerComponent* InContainerComponent)
{
	check(!ContainerComponent.IsValid())
	ContainerComponent = InContainerComponent;
	
	if (!ContainerComponent.IsValid())
	{
		UE_LOG(LogUPContainers, Error, TEXT("ContainerComponent is not valid for %s"), *GetNameSafe(this));
		RemoveFromParent();
		return;
	}

	OnContainerInitialized();

	ensureAlways(GetOwningPlayer()->GetNetMode() == NM_Client || GetOwningPlayer()->GetNetMode() == NM_ListenServer);
	ContainerComponent->OnContainerItemsChanged.AddDynamic(this, &ThisClass::OnContainerItemsChange);
}

UContainerComponent* UContainerWidget::GetContainerComponent() const
{
	return ContainerComponent.Get();
}
