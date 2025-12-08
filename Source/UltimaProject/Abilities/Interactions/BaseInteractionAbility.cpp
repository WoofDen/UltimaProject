// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "BaseInteractionAbility.h"

// Engine includes
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"

void UBaseInteractionAbility::OnInteractionFinished()
{
	check(GetActorInfo().OwnerActor.IsValid() && GetActorInfo().OwnerActor->HasAuthority())

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

UBaseInteractionAbility::UBaseInteractionAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UBaseInteractionAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayAbilityActivationInfo ActivationInfo,
                                              const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	check(ActorInfo);

	// Create the progress widget
	if (!IsValid(ProgressWidgetInstance) && ActorInfo->OwnerActor->GetNetMode() != NM_DedicatedServer)
	{
		ensureAlways(IsValid(ProgressWidgetClass));
		ensureAlways(ActorInfo->OwnerActor.IsValid());

		APawn* Pawn = Cast<APawn>(ActorInfo->OwnerActor.Get());
		APlayerController* PC = nullptr;
		if (Pawn)
		{
			PC = Pawn->GetController<APlayerController>();
		}

		ProgressWidgetInstance = CreateWidget<UInteractionProgressWidget>(PC, ProgressWidgetClass);
	}

	if (UWorld* World = GetWorld(); World && ProgressWidgetInstance)
	{
		ProgressWidgetInstance->InitializeWidget(World->GetTimeSeconds(), InteractionTime);
		ProgressWidgetInstance->AddToViewport();
	}

	check(ActorInfo->OwnerActor.IsValid());
	if (ActorInfo->OwnerActor->HasAuthority())
	{
		DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, InteractionTime);
		if (!ensureAlways(DelayTask.IsValid()))
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, false);
			return;
		}

		DelayTask->OnFinish.AddDynamic(this, &ThisClass::OnInteractionFinished);
		DelayTask->Activate();
	}
}

void UBaseInteractionAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo,
                                         const FGameplayAbilityActivationInfo ActivationInfo,
                                         bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (ProgressWidgetInstance)
	{
		ProgressWidgetInstance->RemoveFromParent();
	}
}
