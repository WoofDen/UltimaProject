// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactable.h"

bool IInteractable::IsInteractionAccessible(const AController* Instigator) const
{
	return true;
}

void IInteractable::AttemptInteraction(AController* Instigator, EInteractionType Type)
{
}
