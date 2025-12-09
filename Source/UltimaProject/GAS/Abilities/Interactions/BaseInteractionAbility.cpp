// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "BaseInteractionAbility.h"
#include "UltimaProject/Framework/UPPlayerController.h"

// Engine includes
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"

void UBaseInteractionAbility::InitializeProgressWidget(AUPPlayerController* OwnerController)
{
	if (!IsValid(OwnerController) || !ensureAlways(IsValid(ProgressWidgetClass)))
	{
		return;
	}

	if (!ProgressWidgetInstance)
	{
		ProgressWidgetInstance = CreateWidget<UInteractionProgressWidget>(OwnerController, ProgressWidgetClass);
	}

	if (ensureAlways(ProgressWidgetInstance))
	{
		// Setup the timings
		if (UWorld* World = OwnerController->GetWorld())
		{
			ProgressWidgetInstance->InitializeWidget(World->GetTimeSeconds(), InteractionTime);
		}

		// Add widget to HUD
		if (UGameplayHUDWidget* GameplayHUDWidget = OwnerController->GetGameplayHUD())
		{
			GameplayHUDWidget->AddInteractionWidget(ProgressWidgetInstance);
		}
	}
}

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

	// (Client/LS) Create the progress widget instance
	if (ActorInfo->OwnerActor->GetNetMode() != NM_DedicatedServer)
	{
		check(ActorInfo);
		ensureAlways(ActorInfo->OwnerActor.IsValid());

		if (APawn* Pawn = Cast<APawn>(ActorInfo->OwnerActor.Get()))
		{
			InitializeProgressWidget(Pawn->GetController<AUPPlayerController>());
		}

		if (!ProgressWidgetInstance)
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, false);
			return;
		}
	}

	// (Server) Set up a wait task for the server
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
