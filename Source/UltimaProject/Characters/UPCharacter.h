// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Game includes
#include "UltimaProject/Framework/UPPlayerController.h"
#include "UltimaProject/GAS/Attributes/UPBaseAttributeSet.h"
#include "UltimaProject/SkillSystem/SkillSystemComponent.h"
#include "UltimaProject/GAS/UPAbilitySystemComponent.h"

// Engine includes
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "AbilitySystemInterface.h"

// Generated include
#include "UPCharacter.generated.h"

class UInventoryComponent;

UCLASS(Blueprintable)
class ULTIMAPROJECT_API AUPCharacter : public ACharacter, public IContainerInterface
{
	GENERATED_BODY()

protected:
	// When player moved ( considerably distance ), died, some form alternations ( TODO )
	FOnContainerAccessibilityUpdated OnPlayerInventoryAccessibilityChanged;

public:
	AUPCharacter();

	// AActor
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	// ~AActor

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
#pragma region Input
#pragma endregion
#pragma region Inventory

protected:
	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_InventoryComponent, Transient)
	TObjectPtr<UInventoryComponent> InventoryComponent;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventoryComponent> InventoryComponentClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UContainerWidget> InventoryWidgetClass;

	UFUNCTION()
	void OnRep_InventoryComponent();

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; };

	// IContainerInterface
	virtual UContainerComponent* GetContainerComponent_Implementation() const override;
	virtual bool CanBeOpened(const AUPPlayerController* InstigatorController) const override;
	virtual FOnContainerAccessibilityUpdated GetAccessibilityChangedDelegate() const override;
	// ~IContainerInterface

#pragma endregion
};
