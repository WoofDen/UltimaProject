// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "GameLogWidget.generated.h"

class URichTextBlock;

UCLASS()
class ULTIMAPROJECT_API UGameLogWidget : public UUserWidget
{
	GENERATED_BODY()
	
	FTimerHandle LogClearTimerHandle;
	
	TArray<float> EntriesTimeAdded;

	float GetTimeSeconds() const;
	void CleanOldLogEntries();

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> LogPanel;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<URichTextBlock> LogPanelItem;

	UPROPERTY(EditDefaultsOnly)
	float EntryLifetime = 10.f;

public:
	// UUserWidget
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// ~UUserWidget

	void AddLogEntry(const FText& Log, const FString& Style);
};
