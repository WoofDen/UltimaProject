// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "UPAbilitySystemComponent.h"

// Engine includes
#include "GameplayAbilitySet.h"

UUPAbilitySystemComponent::UUPAbilitySystemComponent()
{
}

void UUPAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
	check(GetOwner());

	ASCOwnerInterface = Cast<IAbilitySystemInterface>(GetOwner());
	check(ASCOwnerInterface.IsValid());

	if (DefaultAbilitySet && GetOwner()->HasAuthority())
	{
		DefaultAbilitySet->GiveAbilities(this);
	}
}
