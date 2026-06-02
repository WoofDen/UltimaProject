// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "GameplayHUDWidget.h"

// Engine includes
#include "Components/PanelWidget.h"
#include "UltimaProject/Common/Macro.h"

void UGameplayHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (InteractionsPanel)
	{
		InteractionsPanel->ClearChildren();
	}
}

void UGameplayHUDWidget::AddInteractionWidget(UUserWidget* InteractionWidget)
{
	ensureAlways(InteractionWidget);
	ensureAlways(InteractionsPanel);

	if (InteractionsPanel && InteractionWidget)
	{
		InteractionsPanel->AddChild(InteractionWidget);
	}
}

void UGameplayHUDWidget::AddContainerWidget(IContainerInterface* ContainerInterface)
{
	NULLCHECK(ContainerInterface);

	UContainerComponent* ContainerComponent = ContainerInterface->GetContainerComponent();
	AddContainerWidget(ContainerComponent);
}

void UGameplayHUDWidget::AddContainerWidget(UContainerComponent* ContainerComponent)
{
	NULLCHECK(ContainerComponent);
	
}
