// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Game includes
#include "GameplayAbility_Interaction.h"
#include "UltimaProject/Items/Common/Item.h"
#include "GameplayAbility_Pickup.generated.h"

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_PickupOperation : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AItem> SourceItem;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UContainerComponent> TargetContainer;
	
	UPROPERTY(BlueprintReadWrite)
	int32 ItemAmount;	

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_PickupOperation::StaticStruct();
	}
	
	bool IsValid() const
	{
		return SourceItem.IsValid()
			&& ItemAmount > 0
			&& TargetContainer.IsValid();
	}
};

template <>
struct TStructOpsTypeTraits<
		FGameplayAbilityTargetData_PickupOperation> : public TStructOpsTypeTraitsBase2<
		FGameplayAbilityTargetData_PickupOperation>
{
	enum
	{
		WithNetSerializer = true
		// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};

/**
 * Pickup an actor item ability
 */
UCLASS()
class ULTIMAPROJECT_API UGameplayAbility_Pickup : public UGameplayAbility_Interaction
{
	GENERATED_BODY()
	
	// Server only
	FGameplayAbilityTargetData_PickupOperation Data;

	bool CanPerformPickup();
	void PickupItemInternal();
protected:
	UPROPERTY(EditDefaultsOnly)
	float PickupRadius;
	
	// UGameplayAbility_Interaction
	virtual void OnInteractionFinished() override;
	// ~UGameplayAbility_Interaction
public:
	UGameplayAbility_Pickup();
	
	// UGameplayAbility
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData = nullptr) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	// ~UGameplayAbility
};
