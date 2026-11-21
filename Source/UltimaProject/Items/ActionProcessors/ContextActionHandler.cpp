// Fill out your copyright notice in the Description page of Project Settings.

#include "ContextActionHandler.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Framework/UPPlayerController.h"
#include "UltimaProject/GAS/UPAbilitySystemComponent.h"

bool UContextActionHandler::HandleActionFromCDO(AUPPlayerController* PC, TSubclassOf<UContextActionHandler> HandlerClass, AItem* Item)
{
	UContextActionHandler* HandlerCDO = HandlerClass->GetDefaultObject<UContextActionHandler>();
	ensure(HandlerCDO);
	NULLCHECK_RETURN(HandlerCDO, false);

	return HandlerCDO->HandleAction(PC, Item);
}

bool UContextActionHandler::HandleAction(AUPPlayerController* PC, AItem* Item)
{
	return true;
}

bool UContextActionHandler::HandleAction(AUPPlayerController* PC, FContainerItemData& ItemData)
{
	return true;
}

bool UContextActionHandler::IsAvailable(AUPPlayerController* PC, AItem* Item)
{
	return true;
}
