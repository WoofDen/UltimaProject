// Fill out your copyright notice in the Description page of Project Settings.

#include "GameLogWidget.h"

// Engine includes
#include "Blueprint/WidgetTree.h"
#include "Components/RichTextBlock.h"

// Game includes
#include "Components/VerticalBox.h"
#include "UltimaProject/Common/Macro.h"

float UGameLogWidget::GetTimeSeconds() const
{
	if (UWorld* World = GetWorld())
	{
		return World->GetTimeSeconds();
	}

	return 0.f;
}

void UGameLogWidget::CleanOldLogEntries()
{
	const float CurrentTime = GetTimeSeconds();
	const uint32 TotalEntries = EntriesTimeAdded.Num();
	int32 EntriesToRemove = 0;

	for (uint32 i = 0; i < TotalEntries; i++)
	{
		if (CurrentTime - EntriesTimeAdded[i] > EntryLifetime)
		{
			EntriesToRemove++;
		}
	}

	for (int32 i = 0; i < EntriesToRemove; i++)
	{
		LogPanel->RemoveChildAt(0);
	}
	
	// Copy the rest values into removed ones and shrink the array
	for (uint32 i = EntriesToRemove; i < TotalEntries; i++)
	{
		EntriesTimeAdded[i - EntriesToRemove] = EntriesTimeAdded[i];
	}

	EntriesTimeAdded.SetNum(TotalEntries - EntriesToRemove);
}

void UGameLogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LogPanelItem)
	{
		LogPanel->ClearChildren();
	}

	if (UWorld* World = GetWorld())
	{
		FTimerDelegate Delegate;
		Delegate.BindUObject(this, &ThisClass::CleanOldLogEntries);

		World->GetTimerManager().SetTimer(LogClearTimerHandle, Delegate, 1.f, true);
	}
}

void UGameLogWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LogClearTimerHandle);
	}

	Super::NativeDestruct();
}

void UGameLogWidget::AddLogEntry(const FText& Log, const FString& Style)
{
	NULLCHECK(WidgetTree);
	NULLCHECK(LogPanel);
	NULLCHECK(LogPanelItem);

	URichTextBlock* LogEntry = WidgetTree->ConstructWidget<URichTextBlock>(URichTextBlock::StaticClass());
	NULLCHECK(LogEntry);

	static const FTextFormat TextFormat = FTextFormat::FromString(TEXT("<{0}>{1}</>"));
	const FText LogText = FText::Format(TextFormat, FText::FromString(Style), Log);

	LogEntry->SetTextStyleSet(LogPanelItem->GetTextStyleSet());
	LogEntry->SetText(LogText);
	LogEntry->SetVisibility(ESlateVisibility::Visible);

	LogPanel->AddChildToVerticalBox(LogEntry);
	EntriesTimeAdded.Add(GetTimeSeconds());
}
