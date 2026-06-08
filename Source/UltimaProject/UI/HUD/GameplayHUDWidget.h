// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Engine includes
#include "Blueprint/UserWidget.h"
#include "UltimaProject/Items/Containers/Interfaces/ContainerInterface.h"

// Generated include
#include "GameplayHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API UGameplayHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	TMap<TWeakObjectPtr<UContainerComponent>, TObjectPtr<UUserWidget>> OpenedContainers;


protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> InteractionsPanel;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> ContainersStackWidget;

public:
	// UUserWidget
	virtual void NativeConstruct() override;
	// ~UUserWidget

	void AddInteractionWidget(UUserWidget* InteractionWidget);

	bool IsContainerOpened(UContainerComponent* ContainerComponent) const;
	
	void AddContainerWidget(UContainerComponent* ContainerComponent);
	
	void CloseContainerWidget(UContainerComponent* ContainerComponent);
};
