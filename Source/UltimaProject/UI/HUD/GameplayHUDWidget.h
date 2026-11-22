// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Engine includes
#include "Blueprint/UserWidget.h"

// Game includes
#include "UltimaProject/Framework/UPPlayerController.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "UltimaProject/UI/ContextActions/ContextActionsWidget.h"

// Generated include
#include "GameplayHUDWidget.generated.h"

class UGameLogWidget;

namespace UP::LogStyle
{
	const FString Default("Default");
	const FString Warning("Warning");
	const FString Error("Error");
}

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API UGameplayHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	void UpdateContextMenu();

public:
	// UUserWidget
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	// ~UUserWidget

#pragma region Containers

private:
	TMap<TWeakObjectPtr<const UContainerComponent>, TObjectPtr<UUserWidget>> OpenedContainers;

	FVector2D LastOpenedContainerPosition;

	FVector2D GetNewContainerPosition(const UUserWidget* ContainerWidget, const class UCanvasPanelSlot* CanvasSlot) const;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> ContainersStackWidget;

	UPROPERTY(EditDefaultsOnly, Category="Containers")
	FVector2D ContainerDefaultOffset = FVector2D(50, 100);

	UPROPERTY(EditDefaultsOnly, Category="Containers")
	FVector2D ContainerOffsetStep = FVector2D(50, 50);

public:
	bool IsContainerOpened(UContainerComponent* ContainerComponent);

	bool AddContainerWidget(UContainerComponent* ContainerComponent);

	void CloseContainerWidget(UContainerComponent* ContainerComponent);
#pragma endregion

#pragma region Game logging

private:
	void GameLogInternal(const FText& Text, const FString& Style);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UGameLogWidget> GameLogWidget;

	// UUserWidget
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	// ~UUserWidget

public:
	static void GameLog(APlayerController* PlayerController, const wchar_t* Text, bool bReplicate, const FString& LogFormat);

	UFUNCTION(BlueprintCallable)
	static void GameLog(APlayerController* PlayerController, FText Text, bool bReplicate, const FString& LogFormat);
#pragma endregion

#pragma region Interaction progress

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> InteractionsPanel;

public:
	void AddInteractionWidget(UUserWidget* InteractionWidget);
#pragma endregion

#pragma region Context menu

private:
	// Context menu Item's interaction radius, gets copied from item data DA
	float ContextMenuMaxDistanceAlive = 0;

	UPROPERTY(Transient)
	TObjectPtr<UContextActionsWidget> ContextMenuWidget;

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UContextActionsWidget> ContextActionWidgetClass;

public:
	void ResetContextMenu();
	void ToggleContextMenu(const AUPPlayerController* PC);
	void OnContextActionActivated(TSubclassOf<UContextActionHandler> HandlerClass);
#pragma endregion
};
