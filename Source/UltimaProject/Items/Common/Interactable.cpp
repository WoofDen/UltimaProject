// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactable.h"

bool IInteractable::IsInteractionAccessible(const AController* InstigatorController) const
{
	return true;
}

void IInteractable::AttemptInteraction(AController* InstigatorController, EInteractionType Type)
{
}
