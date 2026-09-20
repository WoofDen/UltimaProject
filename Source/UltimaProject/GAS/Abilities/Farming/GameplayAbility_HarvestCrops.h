// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UltimaProject/GAS/Abilities/Interactions/GameplayAbility_Interaction.h"
#include "UltimaProject/Items/Crops/CropBase.h"
#include "GameplayAbility_HarvestCrops.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API UGameplayAbility_HarvestCrops : public UGameplayAbility_Interaction
{
	GENERATED_BODY()
	
	TWeakObjectPtr<ACropBase> TargetCrop;
	
	TArray<FItemDataDefinition> GenerateHarvest(const TArray<FPlantCultureHarvest>& Harvests) const;
public:
	UGameplayAbility_HarvestCrops();
	
	// UGameplayAbility
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData = nullptr) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	// ~UGameplayAbility
	
	// UGameplayAbility_Interaction
	virtual void OnInteractionFinished() override;
	// ~UGameplayAbility_Interaction
};
