// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "GameplayHUDWidget.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "UltimaProject/UI/ContainerWidget.h"

// Engine includes
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"

FVector2D UGameplayHUDWidget::GetNewContainerPosition(const UUserWidget* ContainerWidget,
                                                      const UCanvasPanelSlot* CanvasSlot) const
{
	NULLCHECK_RETURN(CanvasSlot, FVector2D::ZeroVector);
	NULLCHECK_RETURN(CanvasSlot->Parent, FVector2D::ZeroVector);
	NULLCHECK_RETURN(ContainerWidget, FVector2D::ZeroVector);

	FVector2D ParentWidgetSize = CanvasSlot->Parent->GetCachedGeometry().GetLocalSize();
	FVector2D ContainerWidgetSize = ContainerWidget->GetDesiredSize();

	// Try to place a new container above the prev one
	FVector2D NewPosition = LastOpenedContainerPosition + ContainerOffsetStep;
	if (NewPosition.X + ContainerWidgetSize.X >= ParentWidgetSize.X)
	{
		NewPosition.X = ContainerDefaultOffset.X;
	}
	if (NewPosition.Y + ContainerWidgetSize.Y >= ParentWidgetSize.Y)
	{
		NewPosition.Y = ContainerDefaultOffset.Y;
	}
	
	return NewPosition;
}

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
	NULLCHECK(InteractionWidget);
	NULLCHECK(InteractionsPanel);

	if (InteractionsPanel && InteractionWidget)
	{
		InteractionsPanel->AddChild(InteractionWidget);
	}
}

bool UGameplayHUDWidget::IsContainerOpened(UContainerComponent* ContainerComponent) const
{
	NULLCHECK_RETURN_LOG(ContainerComponent, false, Error, "UGameplayHUDWidget::IsContainerOpened null container");
	check(ContainerComponent->GetNetMode() != NM_DedicatedServer);
	
	return OpenedContainers.Contains(ContainerComponent->GetOriginContainer());
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
			FVector2D Position = GetNewContainerPosition(Widget, CanvasSlot);

			CanvasSlot->SetPosition(Position);
			LastOpenedContainerPosition = Position;
		}
	}

	OpenedContainers.Add(ContainerComponent->GetOriginContainer(), Widget);
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
	
	LastOpenedContainerPosition -= ContainerOffsetStep;
}
