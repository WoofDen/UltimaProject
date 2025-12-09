// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Engine includes
#include "Blueprint/UserWidget.h"

// Generated include
#include "GameplayHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API UGameplayHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> InteractionsPanel;
	
public:
	// UUserWidget
	virtual void NativeConstruct() override;
	// ~UUserWidget
	
	void AddInteractionWidget(UUserWidget* InteractionWidget);
};
