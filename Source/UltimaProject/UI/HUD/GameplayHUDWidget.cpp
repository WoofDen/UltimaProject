// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "GameplayHUDWidget.h"

// Engine includes
#include "Components/PanelWidget.h"

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
