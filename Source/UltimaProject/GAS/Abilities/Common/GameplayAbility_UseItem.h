// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InputAction.h"
#include "Abilities/GameplayAbility.h"
#include "UltimaProject/Common/Globals.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "GameplayAbility_UseItem.generated.h"

class UInputMappingContext;

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_UseItem : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()

	TWeakObjectPtr<AItem> TargetItem;
	TWeakObjectPtr<UContainerComponent> TargetContainer;
	uint32 ContainerItemHandle = FContainerItemData::InvalidHandle;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_UseItem::StaticStruct();
	}

	bool IsValid() const
	{
		return TargetItem.IsValid() || (TargetContainer.IsValid() && TargetContainer->HasItem(ContainerItemHandle));
	}

	bool IsPlacedInWorld() const { return TargetItem.IsValid(); }
};

template <>
struct TStructOpsTypeTraits<
		FGameplayAbilityTargetData_UseItem> : public TStructOpsTypeTraitsBase2<
		FGameplayAbilityTargetData_UseItem>
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
class ULTIMAPROJECT_API UGameplayAbility_UseItem : public UGameplayAbility
{
	GENERATED_BODY()

	friend class UItemLogicProcessor;

	FGameplayAbilityTargetData_UseItem Data;
	FVector ItemOrigin = UPGlobals::InvalidLocation;

	UPROPERTY(Transient)
	const UItemDataAsset* ItemDataAsset;

	TWeakObjectPtr<AUPPlayerController> PlayerController;

	void Initialize();

	uint32 TargetSelectedBindingHandle;
	uint32 TargetCancelledBindingHandle;
	
	UFUNCTION()
	void OnTargetReady(AActor* Actor);

	UFUNCTION()
	void OnTargetCancelled();
protected:
	bool CanUseItem() const;

	const FGameplayAbilityTargetData_UseItem& GetTargetData() const
	{
		return Data;
	}

	UPROPERTY(EditDefaultsOnly)
	float MaxTargetDistance = 100.f;

	UPROPERTY(EditDefaultsOnly)
	UInputMappingContext* TargetingIMC;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* TargetSelectIA;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* TargetCancelIA;

	bool RequestTarget(UClass* TargetClass, TFunction<void(AActor* SelectedTarget)> OnSelectedCallback);
	void CancelTarget();

	// Called when the logic processor get all the targets & data
	void OnDataCollected();

public:
	UGameplayAbility_UseItem();

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
