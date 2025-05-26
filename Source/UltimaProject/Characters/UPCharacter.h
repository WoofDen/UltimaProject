// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "UltimaProject/Framework/UPPlayerController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "UltimaProject/Abilities/UPBaseAttributeSet.h"
#include "UltimaProject/SkillSystem/SkillSystemComponent.h"
#include "UPCharacter.generated.h"

class UInventoryComponent;

UCLASS(Blueprintable)
class ULTIMAPROJECT_API AUPCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AUPCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreInitializeComponents() override;

	// TODO maybe interface ( or modular plugin? )
	UFUNCTION(BlueprintImplementableEvent)
	void OnGameplayReady();

private:
	bool bGameplayReadyStateBroadcasted = false;
	
	void UpdateGameplayReadyState();

#pragma region Camera

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCameraComponent> CameraComponent;

public:
	USpringArmComponent* GetSpringArmComponent() const { return SpringArmComponent; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UCameraComponent* GetCameraComponent() const { return CameraComponent; }
#pragma  endregion
#pragma region Controller

private:
	UPROPERTY()
	AUPPlayerController* PlayerController = nullptr;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AUPPlayerController* GetPlayerController();
#pragma endregion
#pragma region GAS

private:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess=true), Category="Attributes")
	const UUPBaseAttributeSet* BaseAttributeSet = nullptr;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return AbilitySystemComponent.Get();
	};
#pragma endregion
#pragma region Skills

private:
	UPROPERTY()
	TObjectPtr<USkillSystemComponent> SkillSystemComponent;

public:
	USkillSystemComponent* GetSkillSystemComponent() const
	{
		return SkillSystemComponent;
	}

#pragma endregion
#pragma region Input
#pragma endregion
#pragma region Inventory

protected:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_InventoryComponent)
	TObjectPtr<UInventoryComponent> InventoryComponent;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventoryComponent> InventoryComponentClass;

	UFUNCTION()
	void OnRep_InventoryComponent();

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; };
#pragma endregion
};
