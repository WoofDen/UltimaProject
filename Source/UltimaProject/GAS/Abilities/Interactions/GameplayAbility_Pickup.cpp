// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "GameplayAbility_Pickup.h"

#include "AbilitySystemComponent.h"
#include "UltimaProject/Characters/UPCharacter.h"
#include "UltimaProject/Common/GameplayTags.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Items/Common/Item.h"
#include "UltimaProject/Items/Containers/Components/InventoryComponent.h"

bool FGameplayAbilityTargetData_PickupOperation::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	Ar << ItemAmount;
	Ar << SourceItem;
	Ar << TargetContainer;

	return true;
}

bool UGameplayAbility_Pickup::CanPerformPickup()
{
	if (!Data.IsValid())
	{
		return false;
	}

	// Check the container is accessible
	IContainerOwnerInterface* ContainerOwnerInterface = Data.TargetContainer->GetOwnerInterface();
	NULLCHECK_RETURN(ContainerOwnerInterface, false);

	AUPPlayerController* PC = Cast<AUPPlayerController>(GetActorInfo().PlayerController);
	NULLCHECK_RETURN(PC, false);

	if (!ContainerOwnerInterface->CanBeOpened(PC))
	{
		return false;
	}

	// Distance check
	// TOOD should it be here?
	float Distance = (GetActorInfo().AvatarActor->GetActorLocation() - Data.SourceItem->GetActorLocation()).Length();
	if (Distance > PickupRadius)
	{
		return false;
	}

	// Inventory capacity & other checks
	if (!Data.TargetContainer->CanStoreItem(PC, Data.SourceItem.Get()))
	{
		return false;
	}

	return true;
}

void UGameplayAbility_Pickup::PickupItemInternal()
{
	check(K2_HasAuthority()); // Server only
	NULLCHECK_SP(Data.SourceItem);

	if (AUPCharacter* Character = Cast<AUPCharacter>(GetAvatarActorFromActorInfo()))
	{
		Data.TargetContainer->StoreItem(Data.SourceItem.Get(), Data.ItemAmount);
	}

	// Regardless of the result, end the ability
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_Pickup::OnInteractionFinished()
{
	Super::OnInteractionFinished();

	// Re-validate everything
	if (!CanPerformPickup())
	{
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
		return;
	}

	PickupItemInternal();

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

UGameplayAbility_Pickup::UGameplayAbility_Pickup()
{
	SetAssetTags(FGameplayTagContainer(TAG_Ability_Container_Pickup));

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::Type::GameplayEvent;
	TriggerData.TriggerTag = TAG_Ability_Container_Pickup;

	AbilityTriggers.Add(TriggerData);
}

void UGameplayAbility_Pickup::PreActivate(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo,
                                          const FGameplayAbilityActivationInfo ActivationInfo,
                                          FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
                                          const FGameplayEventData* TriggerEventData)
{
	if (TriggerEventData == nullptr || IsActive())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);

	const FGameplayAbilityTargetData_PickupOperation* DropData = static_cast<const
		FGameplayAbilityTargetData_PickupOperation*>(TriggerEventData->TargetData.Get(0));

	if (DropData == nullptr || !DropData->IsValid())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	Data = *DropData;

	if (!CanPerformPickup())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UGameplayAbility_Pickup::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayAbilityActivationInfo ActivationInfo,
                                              const FGameplayEventData* TriggerEventData)
{
	NULLCHECK(TriggerEventData);

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
