// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Game includes
#include "GameplayAbility_Interaction.h"
#include "GameplayAbility_Pickup.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API UGameplayAbility_Pickup : public UGameplayAbility_Interaction
{
	GENERATED_BODY()

	class AItem* GetItemUnderCursor();
	bool CanPickupItem(const AItem* Item);
	void ServerOnReplicatedDataReady(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag Tag);
protected:
	UPROPERTY(EditDefaultsOnly)
	float PickupRadius;
	
public:
	// UGameplayAbility
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
