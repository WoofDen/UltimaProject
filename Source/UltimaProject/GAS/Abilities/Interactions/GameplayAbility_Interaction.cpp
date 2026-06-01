// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "GameplayAbility_Interaction.h"
#include "UltimaProject/UI/InteractionProgressWidget.h"
#include "UltimaProject/Framework/UPPlayerController.h"

// Engine includes
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"

bool UGameplayAbility_Interaction::IsInteractionFinished() const
{
	// Cannot rely on the WaitDelayTaskInstance as it will be moved to Finished only after OnFinish delegate
	return bInteractionFinished;
}

void UGameplayAbility_Interaction::OnInteractionFinished()
{
	check(K2_HasAuthority());
	bInteractionFinished = true;
}

UGameplayAbility_Interaction::UGameplayAbility_Interaction()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGameplayAbility_Interaction::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                                   const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	check(ActorInfo);
	ensureAlways(ActorInfo->OwnerActor.IsValid());

	// (Server) Setup a delay task
	if (ActorInfo->OwnerActor->GetNetMode() == NM_DedicatedServer)
	{
		check(ActorInfo->OwnerActor.IsValid());
		WaitDelayTaskInstance = UAbilityTask_WaitDelay::WaitDelay(this, InteractionTime);

		if (!WaitDelayTaskInstance)
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}

		WaitDelayTaskInstance->OnFinish.AddDynamic(this, &ThisClass::OnInteractionFinished);
		WaitDelayTaskInstance->ReadyForActivation();
	}
	// (Client) Setup interaction UI
	else if (ActorInfo->OwnerActor->GetNetMode() == NM_Client)
	{
		AUPPlayerController* OwnerController = Cast<AUPPlayerController>(ActorInfo->PlayerController.Get());
		if (!ensureAlways(IsValid(OwnerController)))
		{
			return;
		}

		if (!IsValid(ProgressWidgetInstance))
		{
			ProgressWidgetInstance = CreateWidget<UInteractionProgressWidget>(OwnerController, ProgressWidgetClass);
		}

		if (ensureAlways(ProgressWidgetInstance))
		{
			// Set up the timings
			if (UWorld* World = OwnerController->GetWorld())
			{
				ProgressWidgetInstance->InitializeWidget(World->GetTimeSeconds(), InteractionTime);
				ProgressWidgetInstance->SetInteractionName(InteractionName);
			}

			// Add the progress widget to HUD
			if (UGameplayHUDWidget* GameplayHUDWidget = OwnerController->GetGameplayHUD())
			{
				GameplayHUDWidget->AddInteractionWidget(ProgressWidgetInstance);
			}
		}
	}
}

void UGameplayAbility_Interaction::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayAbilityActivationInfo ActivationInfo,
                                              bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (IsValid(WaitDelayTaskInstance))
	{
		WaitDelayTaskInstance->ExternalCancel();
	}

	if (IsValid(ProgressWidgetInstance))
	{
		ProgressWidgetInstance->RemoveFromParent();

		ProgressWidgetInstance->MarkAsGarbage();
		ProgressWidgetInstance = nullptr;
	}

	check(ActorInfo);
	ensureAlways(ActorInfo->OwnerActor.IsValid());
}
