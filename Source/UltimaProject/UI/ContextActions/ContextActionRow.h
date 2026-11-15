// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "ContextActionRow.generated.h"

class UContextActionHandler;
class UTextBlock;

/**
 * Widget with a context action
 */
UCLASS()
class ULTIMAPROJECT_API UContextActionRow : public UUserWidget
{
	GENERATED_BODY()

	friend class UContextActionsWidget;

	TSubclassOf<UContextActionHandler> HandlerClass;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ActionButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Label;

	TWeakObjectPtr<UContextActionsWidget> ActionsMenuOwner;

	FOnButtonPressedEvent* GetOnActivatedDelegate() const;

	void ConstructFromContextAction(const struct FItemContextAction& ContextAction);

	UFUNCTION()
	void OnActionButtonPressed();

public:
};
