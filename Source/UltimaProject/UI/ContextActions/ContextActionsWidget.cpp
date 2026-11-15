// Fill out your copyright notice in the Description page of Project Settings.

#include "ContextActionsWidget.h"
#include "ContextActionRow.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Framework/UPPlayerController.h"
#include "UltimaProject/Items/ActionProcessors/ContextActionHandler.h"
#include "UltimaProject/Items/Common/ItemData.h"

void UContextActionsWidget::ConstructFromItem(AItem* Item)
{
	NULLCHECK(Item);
	NULLCHECK(ActionsPanel);
	NULLCHECK(ActionWidgetClass);
	NULLCHECK(WidgetTree);
	
	SourceItem = Item;

	const UItemDataAsset* ItemData = Item->GetItemDataConst().GetStaticData();

	ActionsPanel->ClearChildren();

	// Construct menu
	for (const FItemContextAction& ContextAction : ItemData->ContextActions)
	{
		UContextActionRow* Entry = WidgetTree->ConstructWidget<UContextActionRow>(ActionWidgetClass);
		if (ensureAlways(Entry))
		{
			Entry->HandlerClass = ContextAction.ContextActionHandlerClass;
			Entry->ActionsMenuOwner = this;
			Entry->ConstructFromContextAction(ContextAction);
			ActionsPanel->AddChild(Entry);
		}
	}
}

void UContextActionsWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

#if WITH_EDITOR
	if (IsDesignTime() && DesignItemDA)
	{
		ActionsPanel->ClearChildren();

		// Using DesignItemDA build buttons as example
		for (const FItemContextAction& ContextAction : DesignItemDA->ContextActions)
		{
			UContextActionRow* Entry = WidgetTree->ConstructWidget<UContextActionRow>(ActionWidgetClass);
			if (ensureAlways(Entry))
			{
				Entry->ConstructFromContextAction(ContextAction);
				ActionsPanel->AddChild(Entry);
			}
		}
	}
#endif
}
