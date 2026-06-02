// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "GameplayAbility_Pickup.h"

#include "AbilitySystemComponent.h"
#include "UltimaProject/Characters/UPCharacter.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Items/Common/Item.h"
#include "UltimaProject/Items/Containers/Components/InventoryComponent.h"

bool UGameplayAbility_Pickup::CanPickupItem(const AItem* Item)
{
	// Validation
	AUPCharacter* Character = Cast<AUPCharacter>(GetAvatarActorFromActorInfo());
	NULLCHECK_RETURN(Item, false);
	NULLCHECK_RETURN(Character, false);
	NULLCHECK_SP_RETURN(GetActorInfo().AvatarActor, false);
	
	UInventoryComponent* InventoryComponent = Character->GetInventoryComponent();
	NULLCHECK_RETURN(InventoryComponent, false);

	// Distance check
	float Distance = (GetActorInfo().AvatarActor->GetActorLocation() - Item->GetActorLocation()).Length();
	if (Distance > PickupRadius)
	{
		return false;
	}

	// Inventory capacity & other checks
	if (!InventoryComponent->CanStoreItem(Item))
	{
		return false;
	}

	return true;
}

void UGameplayAbility_Pickup::PickupItemInternal()
{
	check(K2_HasAuthority()); // Server only
	NULLCHECK_SP(TargetItem);

	if (AUPCharacter* Character = Cast<AUPCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UInventoryComponent* InventoryComponent = Character->GetInventoryComponent())
		{
			// TODO A check that the item hasn't been picked by someone else between client and server ability activation. Item actor may persist but it doesn't guarantee its valid.
			InventoryComponent->TryStoreItem(TargetItem.Get());
		}
	}

	// Regardless of the result, end the ability
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_Pickup::OnInteractionFinished()
{
	Super::OnInteractionFinished();

	PickupItemInternal();
}

void UGameplayAbility_Pickup::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayAbilityActivationInfo ActivationInfo,
                                              const FGameplayEventData* TriggerEventData)
{
	NULLCHECK(TriggerEventData);
	NULLCHECK(TriggerEventData->Target);

	TargetItem = Cast<AItem>(const_cast<AActor*>(TriggerEventData->Target.Get()));
	if (!TargetItem.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGameplayAbility_Pickup::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo,
                                         const FGameplayAbilityActivationInfo ActivationInfo,
                                         bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AbilityTargetDataSetDelegate(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey()
		).RemoveAll(this);
	}
}
