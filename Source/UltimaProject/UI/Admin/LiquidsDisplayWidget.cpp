#include "LiquidsDisplayWidget.h"
#include "SLiquidsDisplay.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"

TMap<TWeakObjectPtr<UContainerComponent>, TWeakObjectPtr<ULiquidsDisplayWidget>> ULiquidsDisplayWidget::ActiveWidgets;

ULiquidsDisplayWidget* ULiquidsDisplayWidget::ShowLiquids(UContainerComponent* Container, float WorldZOffset)
{
	if (!Container)
	{
		return nullptr;
	}

	// One panel per container
	if (const TWeakObjectPtr<ULiquidsDisplayWidget>* Existing = ActiveWidgets.Find(Container))
	{
		if (ULiquidsDisplayWidget* ExistingWidget = Existing->Get())
		{
			ExistingWidget->WorldZOffset = WorldZOffset;
			return ExistingWidget;
		}
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(Container, 0);
	if (!PC || !PC->IsLocalController())
	{
		return nullptr; // dedicated server / no local player
	}

	ULiquidsDisplayWidget* Widget = CreateWidget<ULiquidsDisplayWidget>(PC);
	if (!Widget)
	{
		return nullptr;
	}

	Widget->Container = Container;
	Widget->WorldZOffset = WorldZOffset;

	// Anchor the widget's bottom-center to the projected point, so it sits above the actor
	Widget->SetAnchorsInViewport(FAnchors(0.f, 0.f));
	Widget->SetAlignmentInViewport(FVector2D(0.5f, 1.f));
	Widget->SetVisibility(ESlateVisibility::Collapsed);
	Widget->AddToViewport(10);

	ActiveWidgets.Add(Container, Widget);

	// Avoid a one-frame flash at (0,0)
	if (Widget->UpdateScreenPosition())
	{
		Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	return Widget;
}

void ULiquidsDisplayWidget::HideLiquids(UContainerComponent* Container)
{
	if (!Container)
	{
		return;
	}

	if (const TWeakObjectPtr<ULiquidsDisplayWidget>* Existing = ActiveWidgets.Find(Container))
	{
		if (ULiquidsDisplayWidget* Widget = Existing->Get())
		{
			Widget->RemoveFromParent(); // NativeDestruct cleans the map
		}
	}
	ActiveWidgets.Remove(Container);
}

TSharedRef<SWidget> ULiquidsDisplayWidget::RebuildWidget()
{
	SlateWidget = SNew(SLiquidsDisplay);
	SlateWidget->SetContainer(Container.Get());
	return SlateWidget.ToSharedRef();
}

void ULiquidsDisplayWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	SlateWidget.Reset();
}

void ULiquidsDisplayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!Container.IsValid() || !Container->GetOwner())
	{
		RemoveFromParent();
		return;
	}

	const bool bOnScreen = UpdateScreenPosition();
	const ESlateVisibility Wanted = bOnScreen ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
	if (GetVisibility() != Wanted)
	{
		SetVisibility(Wanted);
	}
}

void ULiquidsDisplayWidget::NativeDestruct()
{
	for (auto It = ActiveWidgets.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid() || It.Value().Get() == this)
		{
			It.RemoveCurrent();
		}
	}

	Super::NativeDestruct();
}

bool ULiquidsDisplayWidget::UpdateScreenPosition()
{
	const UContainerComponent* C = Container.Get();
	const AActor* Owner = C ? C->GetOwner() : nullptr;
	APlayerController* PC = GetOwningPlayer();
	if (!Owner || !PC)
	{
		return false;
	}

	// Top of the owner's bounds + extra offset
	FVector Origin, Extent;
	Owner->GetActorBounds(true, Origin, Extent);
	const FVector WorldPosition = Origin + FVector(0.f, 0.f, Extent.Z + WorldZOffset);

	FVector2D ScreenPosition;
	if (!PC->ProjectWorldLocationToScreen(WorldPosition, ScreenPosition, /*bPlayerViewportRelative*/ true))
	{
		return false; // behind the camera
	}

	// bRemoveDPIScale = true: ProjectWorldLocationToScreen returns pixels, the viewport slot expects Slate units
	SetPositionInViewport(ScreenPosition, true);
	return true;
}
