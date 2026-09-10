// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayAbility_Interaction.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "GameplayAbility_Drop.generated.h"

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_DropOperation : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 ContainerItemHandle = FContainerItemData::InvalidHandle;

	UPROPERTY(BlueprintReadWrite)
	int32 ItemAmount = 0;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UContainerComponent> SourceContainer;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_DropOperation::StaticStruct();
	}
	
	bool IsValid() const
	{
		return ContainerItemHandle != FContainerItemData::InvalidHandle
			&& ItemAmount > 0
			&& SourceContainer.IsValid();
	}
};

template <>
struct TStructOpsTypeTraits<
		FGameplayAbilityTargetData_DropOperation> : public TStructOpsTypeTraitsBase2<
		FGameplayAbilityTargetData_DropOperation>
{
	enum
	{
		WithNetSerializer = true
		// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};

/**
 * Drop an item from a container ( own or something in reachability )
 */
UCLASS()
class ULTIMAPROJECT_API UGameplayAbility_Drop : public UGameplayAbility_Interaction
{
	GENERATED_BODY()
	
	FGameplayAbilityTargetData_DropOperation Data;
	
	bool CanPerformDrop() const;

public:
	UGameplayAbility_Drop();

	// UGameplayAbility
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                         const FGameplayAbilityActivationInfo ActivationInfo,
	                         FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
	                         const FGameplayEventData* TriggerEventData = nullptr) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	// ~UGameplayAbility
	
	// UGameplayAbility_Interaction
	virtual void OnInteractionFinished() override;
	// ~UGameplayAbility_Interaction
};
