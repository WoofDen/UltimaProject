// Fill out your copyright notice in the Description page of Project Settings.

#include "ContextActionRow.h"

#include "ContextActionsWidget.h"
#include "Components/TextBlock.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Framework/UPPlayerController.h"
#include "UltimaProject/Items/ActionProcessors/ContextActionHandler.h"
#include "UltimaProject/Items/Common/ItemData.h"

FOnButtonPressedEvent* UContextActionRow::GetOnActivatedDelegate() const
{
	if (ActionButton)
	{
		return &ActionButton->OnPressed;
	}

	return nullptr;
}

void UContextActionRow::ConstructFromContextAction(const FItemContextAction& ContextAction)
{
	NULLCHECK(ActionButton);

	HandlerClass = ContextAction.ContextActionHandlerClass;

	if (Label)
	{
		Label->SetText(ContextAction.Label);
	}

	ActionButton->OnPressed.AddDynamic(this, &ThisClass::OnActionButtonPressed);
}

void UContextActionRow::OnActionButtonPressed()
{
	NULLCHECK_SP(ActionsMenuOwner);
	
	AUPPlayerController* OwnerController = CastChecked<AUPPlayerController>(GetOwningPlayer(), ECastCheckedType::NullAllowed);
	NULLCHECK(OwnerController);

	UGameplayHUDWidget* HUD = OwnerController->GetGameplayHUD();
	NULLCHECK(HUD);
	
	HUD->OnContextActionActivated(HandlerClass);
}
