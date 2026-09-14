// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbility_Drop.h"
#include "UltimaProject/Common/GameplayTags.h"
#include "UltimaProject/Common/Macro.h"

bool FGameplayAbilityTargetData_DropOperation::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	int32 Version = 0;
	Ar << Version;

	Ar << SourceContainer;
	Ar << ItemAmount;
	Ar << ContainerItemHandle;

	return true;
}

bool UGameplayAbility_Drop::CanPerformDrop() const
{
	if (!Data.IsValid())
	{
		return false;
	}

	IContainerOwnerInterface* ContainerOwnerInterface = Data.SourceContainer->GetOwnerInterface();
	NULLCHECK_RETURN(ContainerOwnerInterface, false);

	const AUPPlayerController* PC = Cast<AUPPlayerController>(GetActorInfo().PlayerController);
	if (!ContainerOwnerInterface->CanBeOpened(PC))
	{
		return false;
	}

	// TODO distance / reachability check

	return true;
}

UGameplayAbility_Drop::UGameplayAbility_Drop()
{
	SetAssetTags(FGameplayTagContainer(TAG_Ability_Container_Drop));

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::Type::GameplayEvent;
	TriggerData.TriggerTag = TAG_Ability_Container_Drop;

	AbilityTriggers.Add(TriggerData);
}

void UGameplayAbility_Drop::PreActivate(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo,
                                        const FGameplayAbilityActivationInfo ActivationInfo,
                                        FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
                                        const FGameplayEventData* TriggerEventData)
{
	const FGameplayAbilityTargetData_DropOperation* DropData = static_cast<const
		FGameplayAbilityTargetData_DropOperation*>(TriggerEventData->TargetData.Get(0));

	if (DropData == nullptr || !DropData->IsValid() || IsActive())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);

	Data = *DropData;
	if (!CanPerformDrop())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
}

void UGameplayAbility_Drop::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo,
                                            const FGameplayAbilityActivationInfo ActivationInfo,
                                            const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Wait for interaction finish
}

void UGameplayAbility_Drop::OnInteractionFinished()
{
	Super::OnInteractionFinished();

	// Re-validate everything
	if (!CanPerformDrop())
	{
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
		return;
	}

	Data.SourceContainer->DropItem(Data.ContainerItemHandle, Data.ItemAmount);

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
