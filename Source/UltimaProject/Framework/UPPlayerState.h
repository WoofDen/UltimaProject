// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "UPPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API AUPPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
	UFUNCTION()
	void OnPawnChanged(APlayerState* Player, APawn* NewPawn, APawn* OldPawn);
	void UpdateAbilitySystemComponent();

protected:
	// TODO move to pickup ability?
	UPROPERTY(EditAnywhere, meta=(ClampMin=0))
	float PickupRadius;

public:
	AUPPlayerState();

	// AActor
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostInitializeComponents() override;
	// ~AActor

	FORCEINLINE float GetPickupRadius() const
	{
		return PickupRadius;
	}

#pragma region Ability System

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<class UUPAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UUPBaseAttributeSet> DefaultAttributeSet;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UGameplayAbilitySet> DefaultAbilitySet;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
#pragma endregion

#pragma region Skills

private:
	UPROPERTY()
	TObjectPtr<class USkillSystemComponent> SkillSystemComponent;

public:
	USkillSystemComponent* GetSkillSystemComponent() const
	{
		return SkillSystemComponent;
	}

#pragma endregion
};
