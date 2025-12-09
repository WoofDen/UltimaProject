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

	TWeakInterfacePtr<IAbilitySystemInterface> ASCOwnerInterface;
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UGameplayAbilitySet> DefaultAbilitySet;
	
	// UAbilitySystemComponent
	virtual void BeginPlay() override;
	// ~UAbilitySystemComponent

public:
	UUPAbilitySystemComponent();
};
