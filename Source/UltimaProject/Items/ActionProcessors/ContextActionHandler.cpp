// Fill out your copyright notice in the Description page of Project Settings.

#include "ContextActionHandler.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Framework/UPPlayerController.h"
#include "UltimaProject/GAS/UPAbilitySystemComponent.h"
#include "UltimaProject/GAS/Abilities/Interactions/GameplayAbility_InteractionBase.h"

bool UContextActionHandler::HandleAction(AUPPlayerController* PC, AItem* Item)
{
	NULLCHECK_RETURN(Item, false);

	FGameplayAbilityTargetData_Interact* DropDataPtr = new FGameplayAbilityTargetData_Interact();
	DropDataPtr->TargetItem = Item;

	UUPAbilitySystemComponent* ASC = PC->GetAbilitySystemComponent();
	NULLCHECK_RETURN(ASC, false);

	FGameplayEventData EventData;
	EventData.TargetData = FGameplayAbilityTargetDataHandle(DropDataPtr);

	return ASC->HandleGameplayEvent(AbilityTag, &EventData) > 0;
}

bool UContextActionHandler::HandleAction(AUPPlayerController* PC, FContainerItemData& ItemData)
{
	ensureAlways(false);
	return false;
}
