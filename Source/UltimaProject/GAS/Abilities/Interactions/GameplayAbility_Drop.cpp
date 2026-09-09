// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbility_Drop.h"
#include "UltimaProject/Common/GameplayTags.h"

bool FGameplayAbilityTargetData_DropOperation::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	int32 Version = 0;
	Ar << Version;

	Ar << SourceContainer;
	Ar << ItemAmount;
	Ar << ContainerItemHandle;

	return true;
}
UGameplayAbility_Drop::UGameplayAbility_Drop()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	AbilityTags.AddTag(TAG_Ability_Container_Drop);

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
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);

	switch (ActivationInfo.ActivationMode)
	{
	case EGameplayAbilityActivationMode::Predicting:
		{
			break;
		}
	case EGameplayAbilityActivationMode::Authority:
		{
			break;
		}
	default:
		{
			ensureAlways(false);
		}
	}
}

void UGameplayAbility_Drop::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo,
                                            const FGameplayAbilityActivationInfo ActivationInfo,
                                            const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	switch (ActivationInfo.ActivationMode)
	{
	case EGameplayAbilityActivationMode::Predicting:
		{
			break;
		}
	case EGameplayAbilityActivationMode::Authority:
		{
			break;
		}
	default:
		{
			ensureAlways(false);
		}
	}
}
