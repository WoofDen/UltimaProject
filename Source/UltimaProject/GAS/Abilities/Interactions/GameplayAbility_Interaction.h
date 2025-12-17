// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Engine includes
#include "Abilities/GameplayAbility.h"

// Generated include
#include "GameplayAbility_Interaction.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ULTIMAPROJECT_API UGameplayAbility_Interaction : public UGameplayAbility
{
	GENERATED_BODY()

	float Time;
	float TimeStarted;
	
	UPROPERTY(Transient)
	TObjectPtr<class UAbilityTask_WaitDelay> WaitDelayTaskInstance;

	UPROPERTY(Transient)
	TObjectPtr<class UInteractionProgressWidget> ProgressWidgetInstance;
protected:
	UPROPERTY(EditDefaultsOnly)
	float InteractionTime = 3.f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInteractionProgressWidget> ProgressWidgetClass;

	UFUNCTION()
	void OnInteractionFinished();

public:
	UGameplayAbility_Interaction();

	// UGameplayAbility
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;
	// ~UGameplayAbility
};
