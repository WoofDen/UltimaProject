// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "UltimaProject/Items/Common/Item.h"
#include "ContextActionsWidget.generated.h"

class UContextActionRow;
class UItemDataAsset;
/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API UContextActionsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly)
	UItemDataAsset* DesignItemDA;
#endif

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> ActionsPanel;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UContextActionRow> ActionWidgetClass;

	TWeakObjectPtr<AItem> SourceItem;

public:
	// UUserWidget
	virtual void NativePreConstruct() override;
	// ~UUserWidget

	// Construct a context menu from a world item
	void ConstructFromItem(AItem* Item);
	
	AItem* GetSourceItem() const
	{
		return SourceItem.Get();
	}

};
