// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbility_HarvestCrops.h"
#include "UltimaProject/Common/GameplayTags.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Items/Containers/Components/DisposableContainerComponent.h"

TArray<FItemDataDefinition> UGameplayAbility_HarvestCrops::GenerateHarvest(const TArray<FPlantCultureHarvest>& Harvests) const
{
	TArray<FItemDataDefinition> Result;

	for (const FPlantCultureHarvest& HarvestData : Harvests)
	{
		if (FMath::RandRange(0.f, 1.f) > HarvestData.BaseChance)
		{
			continue;
		}

		float Amount = FMath::RandRange(HarvestData.BaseAmount.GetMin(), HarvestData.BaseAmount.GetMax());

		// Round to a single decimal
		Amount = FMath::RoundToFloat(Amount * 10.f) * 0.1f;

		FItemInstanceData InstanceData;
		InstanceData.Amount = Amount;

		FItemDataDefinition Item(HarvestData.ItemDataAsset.LoadSynchronous(), InstanceData);
		Result.Add(MoveTemp(Item));
	}

	return Result;
}

UGameplayAbility_HarvestCrops::UGameplayAbility_HarvestCrops()
{
	SetAssetTags(FGameplayTagContainer(TAG_Ability_Container_HarvestCrops));

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::Type::GameplayEvent;
	TriggerData.TriggerTag = TAG_Ability_Container_HarvestCrops;

	AbilityTriggers.Add(TriggerData);

	InteractionTime = 1.f;
}

void UGameplayAbility_HarvestCrops::PreActivate(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo,
                                                const FGameplayAbilityActivationInfo ActivationInfo,
                                                FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
                                                const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);

	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	ABILITYCHECK(AvatarActor, true);
	ABILITYCHECK(TriggerEventData != nullptr && TriggerEventData->Target != nullptr, true);

	TargetCrop = Cast<ACropBase>(const_cast<AActor*>(TriggerEventData->Target.Get()));
	ABILITYCHECK(TargetCrop.IsValid() && TargetCrop->GetProgress() >= 1.f, true);

	const UPlantCultureDataAsset* CultureDataAsset = TargetCrop->GetCultureDataAsset();
	ABILITYCHECK(CultureDataAsset, true);

	// Check distance
	float Distance = FVector::Dist(AvatarActor->GetActorLocation(), TargetCrop->GetActorLocation());
	Distance -= CultureDataAsset->FieldSize;

	ABILITYCHECK(InteractionRadius >= Distance, true);
}

void UGameplayAbility_HarvestCrops::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo,
                                                    const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGameplayAbility_HarvestCrops::OnInteractionFinished()
{
	Super::OnInteractionFinished();

	APlayerController* PC = GetCurrentActorInfo()->PlayerController.Get();
	if (!TargetCrop.IsValid() || !PC)
	{
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
		return;
	}

	const FString ContainerName = FString::Printf(TEXT("%s harvest"), *TargetCrop->GetActorNameOrLabel());

	auto Items = GenerateHarvest(TargetCrop->GetCultureDataAsset()->Harvests);
	UDisposableContainerComponent::CreateDisposableContainer(PC, ContainerName, EContainerCategory::Foraging, Items);

	TargetCrop->ResetProgress();

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
