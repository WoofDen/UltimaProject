// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractionProgressWidget.h"

void UInteractionProgressWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (TimeStarted == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float TimePast = World->GetTimeSeconds() - TimeStarted;

	if (ProgressBar)
	{
		const float Progress = FMath::Clamp((TimePast / TargetTime), 0.0f, 1.0f);
		ProgressBar->SetPercent(Progress);
	}
	
	if (RemainingTimeText)
	{
		const float RemainingTime = FMath::Max(TargetTime - TimePast, 0);
		
		FNumberFormattingOptions Options;
		Options.SetMaximumFractionalDigits(2);
		
		RemainingTimeText->SetText(FText::AsNumber(RemainingTime, &Options));
	}
}

void UInteractionProgressWidget::InitializeWidget(double InTimeStarted, float InTargetTime)
{
	TimeStarted = InTimeStarted;
	TargetTime = InTargetTime;
}
