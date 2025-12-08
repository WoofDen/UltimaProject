// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Game includes
#include "UltimaProject/UI/InteractionProgressWidget.h"

// Engine includes
#include "Abilities/GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"

// Generated include
#include "BaseInteractionAbility.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ULTIMAPROJECT_API UBaseInteractionAbility : public UGameplayAbility
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TWeakObjectPtr<UAbilityTask_WaitDelay> DelayTask;

protected:
	UPROPERTY(EditDefaultsOnly)
	float InteractionTime = 3.f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UInteractionProgressWidget> ProgressWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UInteractionProgressWidget> ProgressWidgetInstance;

	UFUNCTION()
	void OnInteractionFinished();

public:
	UBaseInteractionAbility();

	// UGameplayAbility
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;
	// ~UGameplayAbility
};
