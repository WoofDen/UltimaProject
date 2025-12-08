// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Engine includes
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

// Generated include
#include "InteractionProgressWidget.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class ULTIMAPROJECT_API UInteractionProgressWidget : public UUserWidget
{
	GENERATED_BODY()
	
	double TimeStarted = 0;
	float TargetTime = 0;
	
protected:
	UPROPERTY(meta=(BindWidget, OptionalWidget=true))
	TObjectPtr<UTextBlock> RemainingTimeText;
	
	UPROPERTY(meta=(BindWidget, OptionalWidget=true))
	TObjectPtr<UProgressBar> ProgressBar;
	
public:
	// UUserWidget
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	// ~UUserWidget
	
	void InitializeWidget(double InTimeStarted, float InTargetTime);
};
