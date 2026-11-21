// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "UltimaProject/Common/Globals.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "UltimaProject/Subsystems/TargetManagerSubsystem.h"
#include "GameplayAbility_InteractionBase.generated.h"

struct FTargetDescriptor;
class UInputMappingContext;

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_Interact : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()

	// World item source of interaction request
	TWeakObjectPtr<AItem> TargetItem;
	
	// Container item source of interaction request
	TWeakObjectPtr<UContainerComponent> TargetContainer;
	uint32 ContainerItemHandle = FContainerItemData::InvalidHandle;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_Interact::StaticStruct();
	}

	bool IsValid() const
	{
		return TargetItem.IsValid() || (TargetContainer.IsValid() && TargetContainer->HasItem(ContainerItemHandle));
	}

	bool IsPlacedInWorld() const { return TargetItem.IsValid(); }
};

template <>
struct TStructOpsTypeTraits<
		FGameplayAbilityTargetData_Interact> : public TStructOpsTypeTraitsBase2<
		FGameplayAbilityTargetData_Interact>
{
	enum
	{
		WithNetSerializer = true
	};
};

/**
 * Generic ability of item usage. Can be activated from in-world Item or container
 * Manages validations and target collections
 * Item-specific data and logic are placed within a corresponding ItemLogicProcessor 
 */
UCLASS()
class ULTIMAPROJECT_API UGameplayAbility_InteractionBase : public UGameplayAbility
{
	GENERATED_BODY()

	friend class UItemLogicProcessor;

	FGameplayAbilityTargetData_Interact InteractTargetData;
	FVector ItemOrigin = UPGlobals::InvalidLocation;

	UPROPERTY(Transient)
	const UItemDataAsset* ItemDataAsset;

	TWeakObjectPtr<AUPPlayerController> PlayerControllerCached;
	TWeakObjectPtr<UTargetManagerSubsystem> TargetManagerCached;

	void Initialize();

	uint32 TargetSelectedBindingHandle;
	uint32 TargetCancelledBindingHandle;

protected:
	virtual void OnClientDataReady();
	virtual void OnClientDataCancelled();	

	bool CanUseItem() const;

	const FGameplayAbilityTargetData_Interact& GetTargetData() const
	{
		return InteractTargetData;
	}

	// Set of target sequences ( Select target, Where to go, etc )
	// Passed to TargetManagerSubsystem
	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FTargetDescriptor>> TargetingScheme;

	UTargetManagerSubsystem* GetTargetManager();
	
	virtual void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& GameplayAbilityTargetDataHandle, FGameplayTag GameplayTag);
public:
	UGameplayAbility_InteractionBase();

	// UGameplayAbility
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle,
	                         const FGameplayAbilityActorInfo* ActorInfo,
	                         const FGameplayAbilityActivationInfo ActivationInfo,
	                         FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
	                         const FGameplayEventData* TriggerEventData = nullptr) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
	                        const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo,
	                        bool bReplicateEndAbility,
	                        bool bWasCancelled) override;
	// ~UGameplayAbility
};
