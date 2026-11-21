// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayAbility_InteractionBase.h"
#include "GameplayAbility_FillLiquid.generated.h"

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_FillLiquid : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()

	// Actor with a container component
	TWeakObjectPtr<AItem> SourceItem;

	float Amount = FLT_MAX;

	// Container item source of interaction request
	TWeakObjectPtr<UContainerComponent> SourceContainer;
	uint32 ContainerItemHandle = FContainerItemData::InvalidHandle;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_FillLiquid::StaticStruct();
	}

	bool IsValid() const
	{
		return SourceItem.IsValid() || (SourceContainer.IsValid() && SourceContainer->HasItem(ContainerItemHandle));
	}
};

template <>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_FillLiquid> : public TStructOpsTypeTraitsBase2<
		FGameplayAbilityTargetData_FillLiquid>
{
	enum
	{
		WithNetSerializer = true
	};
};

/**
 * @class UGameplayAbility_FillLiquid
 * @brief A gameplay ability for filling a liquid from a source to a target container.
 *
 * This class extends the base interaction ability to provide functionality for transferring
 * liquids between compatible containers within the game. It defines custom logic and overrides
 * parent class behavior to facilitate this functionality.
 *
 * Key Features:
 * - Enables interaction with liquid containers via gameplay abilities.
 * - Implements server-side logic to handle liquid transfer between containers.
 * - Ensures robust handling of target readiness and interaction flow.
 *
 * Inherits from:
 * - UGameplayAbility_InteractionBase
 */
UCLASS()
class ULTIMAPROJECT_API UGameplayAbility_FillLiquid : public UGameplayAbility_InteractionBase
{
	GENERATED_BODY()

	void TryFillFromActor(AActor* SourceActor, float Amount);

protected:
	// ~UGameplayAbility_InteractionBase
	virtual void OnClientDataReady() override;
	virtual void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& GameplayAbilityTargetDataHandle, FGameplayTag GameplayTag) override;
	// ~UGamepalyAbility_InteractionBase end

public:
	UGameplayAbility_FillLiquid();
};
