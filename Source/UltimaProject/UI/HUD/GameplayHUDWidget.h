// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Engine includes
#include "Blueprint/UserWidget.h"
#include "Components/RichTextBlock.h"
#include "UltimaProject/Framework/UPPlayerController.h"
#include "UltimaProject/Items/Containers/Interfaces/ContainerOwnerInterface.h"

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

	TMap<TWeakObjectPtr<const UContainerComponent>, TObjectPtr<UUserWidget>> OpenedContainers;

	FVector2D LastOpenedContainerPosition;

	FVector2D GetNewContainerPosition(const UUserWidget* ContainerWidget, const class UCanvasPanelSlot* CanvasSlot) const;

	void GameLogInternal(const FText& Text, const FString& Style);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> InteractionsPanel;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UGameLogWidget> GameLogWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> ContainersStackWidget;

	UPROPERTY(EditDefaultsOnly, Category="Containers")
	FVector2D ContainerDefaultOffset = FVector2D(50, 100);

	UPROPERTY(EditDefaultsOnly, Category="Containers")
	FVector2D ContainerOffsetStep = FVector2D(50, 50);

public:
	// UUserWidget
	virtual void NativeConstruct() override;
	// ~UUserWidget

	void AddInteractionWidget(UUserWidget* InteractionWidget);

	bool IsContainerOpened(UContainerComponent* ContainerComponent);

	bool AddContainerWidget(UContainerComponent* ContainerComponent);

	void CloseContainerWidget(UContainerComponent* ContainerComponent);

	static void GameLog(APlayerController* PlayerController, const wchar_t* Text, bool bReplicate, const FString& LogFormat);

	UFUNCTION(BlueprintCallable)
	static void GameLog(APlayerController* PlayerController, FText Text, bool bReplicate, const FString& LogFormat);
};
