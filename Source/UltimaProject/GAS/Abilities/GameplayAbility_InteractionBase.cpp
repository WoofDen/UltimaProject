// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbility_InteractionBase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "UltimaProject/Common/GameplayTags.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Subsystems/TargetManagerSubsystem.h"
#include "UltimaProject/UI/HUD/GameplayHUDWidget.h"

bool FGameplayAbilityTargetData_Interact::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	Ar << TargetItem;
	Ar << ContainerItemHandle;
	Ar << TargetContainer;

	return true;
}

void UGameplayAbility_InteractionBase::Initialize()
{
	PlayerControllerCached = Cast<AUPPlayerController>(GetActorInfo().PlayerController.Get(false));

	// Load item data...

	// ... from world item
	if (AItem* Item = InteractTargetData.TargetItem.Get())
	{
		const FItemData& ItemData = Item->GetItemDataConst();

		ItemDataAsset = ItemData.GetStaticData();
		ItemOrigin = Item->GetActorLocation();
	}
	// ... from container item
	else
	{
		const FContainerItemData& ContainerItemData = InteractTargetData.TargetContainer->GetItem(InteractTargetData.ContainerItemHandle);
		if (ContainerItemData.IsValid())
		{
			const FItemData& ItemData = ContainerItemData.GetItemData();

			ItemDataAsset = ItemData.GetStaticData();
		}

		if (const IContainerOwnerInterface* ContainerOwner = InteractTargetData.TargetContainer->GetOwnerInterface())
		{
			if (const AActor* Owner = ContainerOwner->GetOwningActor())
			{
				ItemOrigin = Owner->GetActorLocation();
			}
		}
	}
}

void UGameplayAbility_InteractionBase::OnClientDataReady()
{
	check(!GetCurrentActorInfo()->IsNetAuthority());
}

void UGameplayAbility_InteractionBase::OnClientDataCancelled()
{
	CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
}

bool UGameplayAbility_InteractionBase::CanUseItem() const
{
	NULLCHECK_RETURN(ItemDataAsset, false);

	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	NULLCHECK_RETURN(AvatarActor, false);

	const float Distance = FVector::Dist(ItemOrigin, AvatarActor->GetActorLocation());

	if (Distance > ItemDataAsset->InteractionRadius)
	{
		UGameplayHUDWidget::GameLog(PlayerControllerCached.Get(), TEXT("Too far."), false, UP::LogStyle::Error);
		return false;
	}

	return true;
}

UTargetManagerSubsystem* UGameplayAbility_InteractionBase::GetTargetManager()
{
	if (TargetManagerCached.IsValid())
	{
		return TargetManagerCached.Get();
	}

	APlayerController* PC = GetActorInfo().PlayerController.Get();
	NULLCHECK_RETURN(PC, nullptr);

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	NULLCHECK_RETURN(LocalPlayer, nullptr);

	TargetManagerCached = LocalPlayer->GetSubsystem<UTargetManagerSubsystem>();
	return TargetManagerCached.Get();
}

void UGameplayAbility_InteractionBase::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& GameplayAbilityTargetDataHandle, FGameplayTag GameplayTag)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	ABILITYCHECK(ASC, true);
	
	// Free target data
	ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

UGameplayAbility_InteractionBase::UGameplayAbility_InteractionBase()
{
	SetAssetTags(FGameplayTagContainer(TAG_Ability_UseItem));

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	bIsCancelable = true;
	bServerRespectsRemoteAbilityCancellation = true;
}

void UGameplayAbility_InteractionBase::PreActivate(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                                   FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
                                                   const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);

	const FGameplayAbilityTargetData_Interact* TargetData = static_cast<const
		FGameplayAbilityTargetData_Interact*>(TriggerEventData->TargetData.Get(0));

	if (!TargetData || !TargetData->IsValid())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	InteractTargetData = *TargetData;

	Initialize();

	// Init. ok
	// Validate the interaction
	if (!CanUseItem())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}

void UGameplayAbility_InteractionBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                       const FGameplayAbilityActorInfo* ActorInfo,
                                                       const FGameplayAbilityActivationInfo ActivationInfo,
                                                       const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	ABILITYCHECK(ASC, true);

	UTargetManagerSubsystem* TargetManager = GetTargetManager();
	if (GetCurrentActivationInfo().ActivationMode == EGameplayAbilityActivationMode::Predicting)
	{
		TargetManager->OnTargetCancel.BindUObject(this, &UGameplayAbility_InteractionBase::OnClientDataCancelled);
		TargetManager->OnTargetDataReady.BindUObject(this, &UGameplayAbility_InteractionBase::OnClientDataReady);

		TargetManager->StartTargetingScheme(TargetingScheme);
	}

	// SERVER - subscribe to target data sent by client
	if (!TargetingScheme.IsEmpty() && ActorInfo->IsNetAuthority())
	{
		const FPredictionKey PredictionKey =
			ActivationInfo.GetActivationPredictionKey();

		ASC->AbilityTargetDataSetDelegate(
			Handle,
			PredictionKey
		).AddUObject(
			this,
			&ThisClass::OnTargetDataReceived
		);

		// Handle the case where target data arrived before
		// we registered the delegate.
		ASC->CallReplicatedTargetDataDelegatesIfSet(
			Handle,
			PredictionKey
		);
	}
}

void UGameplayAbility_InteractionBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                                  const FGameplayAbilityActorInfo* ActorInfo,
                                                  const FGameplayAbilityActivationInfo ActivationInfo,
                                                  bool bReplicateEndAbility,
                                                  bool bWasCancelled)
{
	if (UTargetManagerSubsystem* TargetManager = GetTargetManager())
	{
		TargetManager->StopTargeting();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
