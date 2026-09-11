// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GameplayAbility_Interaction.h"
#include "UltimaProject/Items/Common/Item.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "GameplayAbility_Relocate.generated.h"

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_RelocateOperation : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()

	TWeakObjectPtr<UContainerComponent> SourceContainer;

	TWeakObjectPtr<UContainerComponent> TargetContainer;

	uint32 ItemHandle = FContainerItemData::InvalidHandle;

	int32 ItemAmount = 0;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_RelocateOperation::StaticStruct();
	}

	bool IsValid() const
	{
		return ItemHandle != FContainerItemData::InvalidHandle
			&& ItemAmount > 0
			&& TargetContainer.IsValid()
			&& SourceContainer.IsValid();
	}
};

template <>
struct TStructOpsTypeTraits<
		FGameplayAbilityTargetData_RelocateOperation> : public TStructOpsTypeTraitsBase2<
		FGameplayAbilityTargetData_RelocateOperation>
{
	enum
	{
		WithNetSerializer = true
		// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};

/**
 * Relocate item from one container to another
 */
UCLASS(Blueprintable)
class ULTIMAPROJECT_API UGameplayAbility_Relocate : public UGameplayAbility_Interaction
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayAbilityTargetData_RelocateOperation Data;

	bool CanPerformRelocate();

public:
	UGameplayAbility_Relocate();

	// UGameplayAbility
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                         const FGameplayAbilityActivationInfo ActivationInfo,
	                         FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
	                         const FGameplayEventData* TriggerEventData = nullptr) override;
	// ~UGameplayAbility

	// UGameplayAbility_Interaction
	virtual void OnInteractionFinished() override;
	// ~UGameplayAbility_Interaction
};
