// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbility_FillLiquid.h"
#include "AbilitySystemComponent.h"
#include "UltimaProject/Common/GameplayTags.h"
#include "UltimaProject/Common/Macro.h"

class AItem;

UGameplayAbility_FillLiquid::UGameplayAbility_FillLiquid()
{
	SetAssetTags(FGameplayTagContainer(TAG_Ability_Interaction_FillLiquid));

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::Type::GameplayEvent;
	TriggerData.TriggerTag = TAG_Ability_Interaction_FillLiquid;

	AbilityTriggers.Add(TriggerData);
}

bool FGameplayAbilityTargetData_FillLiquid::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;

	Ar << SourceItem;
	Ar << Amount;
	Ar << SourceContainer;
	Ar << ContainerItemHandle;

	return bOutSuccess;
}

void UGameplayAbility_FillLiquid::TryFillFromActor(AActor* SourceActor, float Amount)
{
	check(SourceActor && SourceActor->HasAuthority());

	// Get source container
	UContainerComponent* SourceContainer = nullptr;
	UContainerComponent* TargetContainer = nullptr;

	if (SourceActor && SourceActor->Implements<UContainerOwnerInterface>())
	{
		SourceContainer = IContainerOwnerInterface::Execute_GetMainContainerComponent(SourceActor);
	}

	if (AActor* TargetActor = GetTargetData().TargetItem.Get())
	{
		if (TargetActor->Implements<UContainerOwnerInterface>())
		{
			TargetContainer = IContainerOwnerInterface::Execute_GetMainContainerComponent(TargetActor);
		}
	}

	ABILITYCHECK(SourceContainer, true);
	ABILITYCHECK(TargetContainer, true);

	// Store as much as possible
	TargetContainer->StoreLiquid(SourceContainer, Amount);

	// Finish with success
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_FillLiquid::OnClientDataReady()
{
	// Retrieve selected liquid source actor
	UAbilitySystemComponent* ASC = GetCurrentActorInfo()->AbilitySystemComponent.Get();
	NULLCHECK(ASC)

	UTargetManagerSubsystem* TargetManager = GetTargetManager();
	ABILITYCHECK(TargetManager, true);

	AItem* SelectedItem = TargetManager->GetSelectedActor<AItem>(0);
	ABILITYCHECK(SelectedItem, true);

	// Prepare FGameplayAbilityTargetData
	FGameplayAbilityTargetData_FillLiquid* FillLiquidData = new FGameplayAbilityTargetData_FillLiquid();
	FillLiquidData->SourceItem = SelectedItem;
	FillLiquidData->Amount = FLT_MAX;

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Add(FillLiquidData);

	// Send data to a server
	FGameplayTag ApplicationTag;
	ASC->CallServerSetReplicatedTargetData(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActivationInfo().GetActivationPredictionKey(),
		TargetDataHandle,
		ApplicationTag,
		ASC->ScopedPredictionKey
	);
}

void UGameplayAbility_FillLiquid::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& GameplayAbilityTargetDataHandle, FGameplayTag GameplayTag)
{
	Super::OnTargetDataReceived(GameplayAbilityTargetDataHandle, GameplayTag);

	ABILITYCHECK(GameplayAbilityTargetDataHandle.Num() == 1, true);

	const FGameplayAbilityTargetData_FillLiquid* TargetData = static_cast<const FGameplayAbilityTargetData_FillLiquid*>(GameplayAbilityTargetDataHandle.Get(0));
	ABILITYCHECK(TargetData && TargetData->IsValid(), true);

	if (TargetData->SourceItem.IsValid())
	{
		TryFillFromActor(TargetData->SourceItem.Get(), TargetData->Amount);
	}
}
