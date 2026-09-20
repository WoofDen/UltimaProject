// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbility_Relocate.h"
#include "UltimaProject/Common/GameplayTags.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Framework/UPPlayerController.h"

bool FGameplayAbilityTargetData_RelocateOperation::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	Ar << SourceContainer;
	Ar << TargetContainer;
	Ar << ItemAmount;
	Ar << ItemHandle;

	return true;
}

bool UGameplayAbility_Relocate::CanPerformRelocate()
{
	if (!Data.IsValid())
	{
		return false;
	}

	AUPPlayerController* PC = Cast<AUPPlayerController>(GetActorInfo().PlayerController);
	NULLCHECK_RETURN(PC, false);

	const bool bTargetContainerAccessible=  Data.TargetContainer->IsAccessible(PC);
	const bool bSourceContainerAccessible=  Data.SourceContainer->IsAccessible(PC);
	if (!bTargetContainerAccessible || !bSourceContainerAccessible)
	{
		return false;
	}

	return true;
}

UGameplayAbility_Relocate::UGameplayAbility_Relocate()
{
	SetAssetTags(FGameplayTagContainer(TAG_Ability_Container_Relocate));

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::Type::GameplayEvent;
	TriggerData.TriggerTag = TAG_Ability_Container_Relocate;

	AbilityTriggers.Add(TriggerData);
}

void UGameplayAbility_Relocate::PreActivate(const FGameplayAbilitySpecHandle Handle,
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

	const FGameplayAbilityTargetData_RelocateOperation* DropData = static_cast<const
		FGameplayAbilityTargetData_RelocateOperation*>(TriggerEventData->TargetData.Get(0));

	if (DropData == nullptr || !DropData->IsValid())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	Data = *DropData;

	if (!CanPerformRelocate())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UGameplayAbility_Relocate::OnInteractionFinished()
{
	// Server only
	Super::OnInteractionFinished();

	// Re-validate everything
	if (!CanPerformRelocate())
	{
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
		return;
	}

	Data.TargetContainer->RelocateItem(Data.SourceContainer.Get(), Data.ItemHandle, Data.ItemAmount);

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
