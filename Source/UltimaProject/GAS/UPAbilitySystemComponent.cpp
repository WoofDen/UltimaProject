// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "UPAbilitySystemComponent.h"

// Engine includes
#include "GameplayAbilitySet.h"

UUPAbilitySystemComponent::UUPAbilitySystemComponent()
{
}

FGameplayAbilitySpec* UUPAbilitySystemComponent::FindAbilityByTag(const FGameplayTag& Tag) const
{
	TArray<FGameplayAbilitySpec*> AbilitiesToActivatePtrs;
	FGameplayTagContainer TagsToQuery;
	TagsToQuery.AddTag(Tag);

	GetActivatableGameplayAbilitySpecsByAllMatchingTags(TagsToQuery, AbilitiesToActivatePtrs, false);
	ensureAlways(AbilitiesToActivatePtrs.Num() <= 1);

	if (!AbilitiesToActivatePtrs.IsEmpty())
	{
		return AbilitiesToActivatePtrs[0];
	}
	
	return nullptr;
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
