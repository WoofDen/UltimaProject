// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "ContainerWidget.h"

void UContainerWidget::Initialize(UContainerComponent* InContainerComponent)
{
	ContainerComponent = InContainerComponent;
	ensureAlways(ContainerComponent.IsValid());
	
	OnContainerInitialized();
}

UContainerComponent* UContainerWidget::GetContainerComponent() const
{
	return ContainerComponent.Get();
}
