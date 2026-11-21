// Fill out your copyright notice in the Description page of Project Settings.

#include "ContextActionHandler_UseInteractAbility.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/GAS/UPAbilitySystemComponent.h"
#include "UltimaProject/GAS/Abilities/GameplayAbility_InteractionBase.h"

struct FGameplayAbilityTargetData_Interact;

bool UContextActionHandler_UseInteractAbility::HandleAction(AUPPlayerController* PC, AItem* Item)
{
	NULLCHECK_RETURN(Item, false);
	
	if (!IsAvailable(PC, Item))
	{
		return false;
	}

	FGameplayAbilityTargetData_Interact* InteractDataPtr = new FGameplayAbilityTargetData_Interact();
	InteractDataPtr->TargetItem = Item;

	UUPAbilitySystemComponent* ASC = PC->GetAbilitySystemComponent();
	NULLCHECK_RETURN(ASC, false);

	FGameplayEventData EventData;
	EventData.Instigator = PC;
	EventData.Target = Item;
	EventData.TargetData = FGameplayAbilityTargetDataHandle(InteractDataPtr);

	return ASC->HandleGameplayEvent(AbilityTag, &EventData) > 0;}
