// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "GameplayHUDWidget.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "UltimaProject/UI/ContainerWidget.h"

// Engine includes
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UltimaProject/Common/Globals.h"
#include "UltimaProject/Common/InputHelpersFunctionLibrary.h"
#include "UltimaProject/Framework/UPPlayerController.h"
#include "UltimaProject/Items/ActionProcessors/ContextActionHandler.h"
#include "UltimaProject/UI/GameLogWidget.h"
#include "UltimaProject/UI/ContextActions/ContextActionsWidget.h"

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

void UGameplayHUDWidget::GameLogInternal(const FText& Text, const FString& Style)
{
	NULLCHECK(GameLogWidget);
	GameLogWidget->AddLogEntry(Text, Style);
}

void UGameplayHUDWidget::UpdateContextMenu()
{
	NULLCHECK(ContextMenuWidget);

	APlayerController* PC = GetOwningPlayer();
	NULLCHECK(PC);

	// MIGHT be null if menu is called from a container item
	if (const AItem* ContextMenuWorldItem = ContextMenuWidget->GetSourceItem())
	{
		APawn* Pawn = PC->GetPawn();

		// Close context menu if too far
		bool bDistanceValid = Pawn && FVector::Distance(ContextMenuWorldItem->GetActorLocation(), Pawn->GetActorLocation()) <= ContextMenuMaxDistanceAlive;

		if (!bDistanceValid)
		{
			ResetContextMenu();
			return;
		}

		// Update context menu viewport position if called fron a world item
		FVector2D ScreenLocation;
		PC->ProjectWorldLocationToScreen(ContextMenuWorldItem->GetActorLocation(), ScreenLocation);

		ContextMenuWidget->SetPositionInViewport(ScreenLocation);
	}
}

void UGameplayHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (InteractionsPanel)
	{
		InteractionsPanel->ClearChildren();
	}
}

void UGameplayHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateContextMenu();
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

void UGameplayHUDWidget::ResetContextMenu()
{
	if (ContextMenuWidget)
	{
		ContextMenuWidget->RemoveFromParent();
		ContextMenuWidget->MarkAsGarbage();
		ContextMenuWidget = nullptr;
	}
}

void UGameplayHUDWidget::ToggleContextMenu(const AUPPlayerController* PC)
{
	NULLCHECK(PC);

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	NULLCHECK(LocalPlayer);
	NULLCHECK(LocalPlayer->ViewportClient);

	AItem* Item = Cast<AItem>(UInputHelpersFunctionLibrary::GetActorUnderCursor(PC, ECC_InteractableChannel));
	NULLCHECK(Item);
	
	// If a menu is already active for the same item - remove it
	if (IsValid(ContextMenuWidget) && ContextMenuWidget->GetSourceItem() == Item)
	{
		ResetContextMenu();
		return;
	}

	NULLCHECK(WidgetTree);
	NULLCHECK(ContextActionWidgetClass);

	ContextMenuMaxDistanceAlive = Item->GetItemDataConst().GetStaticData()->InteractionRadius;

	// Get mouse position
	FVector2D MousePosition;
	if (!LocalPlayer->ViewportClient->GetMousePosition(MousePosition))
	{
		return;
	}

	// Validation ok
	// Destroy the existing widget
	ResetContextMenu();

	ContextMenuWidget = WidgetTree->ConstructWidget<UContextActionsWidget>(ContextActionWidgetClass);
	ensureAlways(ContextActionWidgetClass);
	NULLCHECK(ContextMenuWidget)

	ContextMenuWidget->ConstructFromItem(Item);
	ContextMenuWidget->AddToViewport();
	ContextMenuWidget->SetPositionInViewport(MousePosition);
}

void UGameplayHUDWidget::OnContextActionActivated(TSubclassOf<UContextActionHandler> HandlerClass)
{
	NULLCHECK(ContextMenuWidget);

	AItem* SourceItem = ContextMenuWidget->GetSourceItem();

	ResetContextMenu();
	NULLCHECK(SourceItem);

	AUPPlayerController* PC = CastChecked<AUPPlayerController>(GetOwningPlayer(), ECastCheckedType::NullAllowed);

	UContextActionHandler::HandleActionFromCDO(PC, HandlerClass, SourceItem);
}

bool UGameplayHUDWidget::IsContainerOpened(UContainerComponent* ContainerComponent)
{
	NULLCHECK_RETURN_LOG(ContainerComponent, false, Error, "UGameplayHUDWidget::IsContainerOpened null container");
	check(ContainerComponent->GetNetMode() != NM_DedicatedServer);

	return OpenedContainers.Contains(ContainerComponent->GetOriginContainer());
}

bool UGameplayHUDWidget::AddContainerWidget(UContainerComponent* ContainerComponent)
{
	NULLCHECK_RETURN(ContainersStackWidget, false);
	NULLCHECK_RETURN(ContainerComponent, false);

	check(ContainerComponent->GetNetMode() != NM_DedicatedServer);

	if (IsContainerOpened(ContainerComponent))
	{
		return false;
	}

	TSubclassOf<UContainerWidget> ContainerClass = ContainerComponent->GetContainerWidgetClass();
	if (!IsValid(ContainerClass))
	{
		return false;
	}

	UContainerWidget* Widget = CreateWidget<UContainerWidget>(GetOwningPlayer(), ContainerClass);
	NULLCHECK_RETURN(Widget, false);

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
	return true;
}

void UGameplayHUDWidget::CloseContainerWidget(UContainerComponent* ContainerComponent)
{
	NULLCHECK(ContainerComponent);
	check(ContainerComponent->GetNetMode() != NM_DedicatedServer);

	TObjectPtr<UUserWidget> ContainerWidget = nullptr;
	OpenedContainers.RemoveAndCopyValue(ContainerComponent, ContainerWidget);

	if (IsValid(ContainerWidget) && ContainerWidget->IsValidLowLevel())
	{
		ContainerWidget->RemoveFromParent();
	}

	LastOpenedContainerPosition -= ContainerOffsetStep;
}

void UGameplayHUDWidget::GameLog(APlayerController* PlayerController, FText Text, bool bReplicate, const FString& LogFormat)
{
	AUPPlayerController* PC = Cast<AUPPlayerController>(PlayerController);
	NULLCHECK(PC);

	if (PlayerController->GetNetMode() != NM_DedicatedServer)
	{
		if (UGameplayHUDWidget* HUD = PC->GetGameplayHUD())
		{
			HUD->GameLogInternal(Text, LogFormat);
		}
	}
	else if (bReplicate)
	{
		PC->GameLogClient(Text, LogFormat);
	}
}

FReply UGameplayHUDWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UGameplayHUDWidget::GameLog(APlayerController* PlayerController, const wchar_t* Text, bool bReplicate, const FString& LogFormat)
{
	GameLog(PlayerController, FText::FromString(Text), bReplicate, LogFormat);
}
