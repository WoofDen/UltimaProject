// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbility_FillLiquid.h"
#include "UltimaProject/Common/GameplayTags.h"

UGameplayAbility_FillLiquid::UGameplayAbility_FillLiquid()
{
	SetAssetTags(FGameplayTagContainer(TAG_Ability_Interaction_FillLiquid));

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::Type::GameplayEvent;
	TriggerData.TriggerTag = TAG_Ability_Interaction_FillLiquid;

	AbilityTriggers.Add(TriggerData);
}
