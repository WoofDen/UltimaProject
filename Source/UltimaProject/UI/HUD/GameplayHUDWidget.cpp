// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "GameplayHUDWidget.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "UltimaProject/UI/ContainerWidget.h"

// Engine includes
#include "Components/CanvasPanelSlot.h"
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

bool UGameplayHUDWidget::IsContainerOpened(UContainerComponent* ContainerComponent) const
{
	check(ContainerComponent->GetNetMode() != NM_DedicatedServer);
	return OpenedContainers.Contains(ContainerComponent);
}

void UGameplayHUDWidget::AddContainerWidget(UContainerComponent* ContainerComponent)
{
	NULLCHECK(ContainersStackWidget);
	NULLCHECK(ContainerComponent);

	check(ContainerComponent->GetNetMode() != NM_DedicatedServer);

	if (IsContainerOpened(ContainerComponent))
	{
		return;
	}

	TSubclassOf<UContainerWidget> ContainerClass = ContainerComponent->GetContainerWidgetClass();
	if (!IsValid(ContainerClass))
	{
		return;
	}

	UContainerWidget* Widget = CreateWidget<UContainerWidget>(GetOwningPlayer(), ContainerClass);
	NULLCHECK(Widget);

	Widget->SetContainerComponent(ContainerComponent);

	if (UPanelSlot* AddedSlot = ContainersStackWidget->AddChild(Widget))
	{
		if (UCanvasPanelSlot* CanvasSlot = StaticCast<UCanvasPanelSlot*>(AddedSlot))
		{
			CanvasSlot->SetPosition(FVector2D(200));
		}
	}
	OpenedContainers.Add(ContainerComponent, Widget);
	
}

void UGameplayHUDWidget::CloseContainerWidget(UContainerComponent* ContainerComponent)
{
	NULLCHECK(ContainerComponent);
	check(ContainerComponent->GetNetMode() != NM_DedicatedServer);

	TObjectPtr<UUserWidget> ContainerWidget = nullptr;
	OpenedContainers.RemoveAndCopyValue(ContainerComponent, ContainerWidget);

	if (IsValid(ContainerWidget))
	{
		ContainerWidget->RemoveFromParent();
	}
}
