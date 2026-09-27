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

	if (!Data.TargetContainer->IsAccessible(PC))
	{
		UGameplayHUDWidget::GameLog(PC, TEXT("Not accessible"), false, UP::LogStyle::Warning);

		return false;
	}

	// Distance check
	float Distance = (GetActorInfo().AvatarActor->GetActorLocation() - Data.SourceItem->GetActorLocation()).Length();
	if (Distance > GetInteractionRadius())
	{
		UGameplayHUDWidget::GameLog(PC, TEXT("Too far"), false, UP::LogStyle::Warning);
		return false;
	}

	// If item itself is a container, do not allow pickup unless its empty
	if (Data.SourceItem->Implements<UContainerOwnerInterface>())
	{
		if (UContainerComponent* ContainerComponent = IContainerOwnerInterface::Execute_GetMainContainerComponent(Data.SourceItem.Get()))
		{
			if (!ContainerComponent->IsEmpty())
			{
				UGameplayHUDWidget::GameLog(PC, TEXT("Empty it first!"), false, UP::LogStyle::Warning);

				return false;
			}
		}
	}

	return true;
}

void UGameplayAbility_Pickup::PickupItemInternal()
{
	check(K2_HasAuthority()); // Server only
	NULLCHECK_SP(Data.SourceItem);

	Data.TargetContainer->StoreItem(Data.SourceItem.Get(), Data.ItemAmount);

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

float UGameplayAbility_Pickup::GetInteractionRadius() const
{
	if (Data.TargetContainer.IsValid())
	{
		return Data.TargetContainer->GetInteractionRadius();
	}

	return Super::GetInteractionRadius();
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
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);
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
                                         bool bReplicateEndAbility,
                                         bool bWasCancelled)
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
