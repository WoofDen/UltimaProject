// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Engine includes
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "UPAbilitySystemComponent.generated.h"

UCLASS()
class ULTIMAPROJECT_API UUPAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

#pragma region Tags
private:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag PickupAbilityTag;

public:
	FORCEINLINE const FGameplayTag& GetPickupAbilityTag() const { return PickupAbilityTag; }
#pragma endregion

	TWeakInterfacePtr<IAbilitySystemInterface> ASCOwnerInterface;

protected:
	// UAbilitySystemComponent
	virtual void BeginPlay() override;
	// ~UAbilitySystemComponent

public:
	UUPAbilitySystemComponent();

	FGameplayAbilitySpec* FindAbilityByTag(const FGameplayTag& Tag) const;
};
