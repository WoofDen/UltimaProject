// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Game includes
#include "UltimaProject/Navigation/UPPathFollowingComponent.h"
#include "UltimaProject/UI/HUD/GameplayHUDWidget.h"

// Engine includes
#include "GameFramework/PlayerController.h"
#include "UltimaProject/Items/Containers/ContainerTypes.h"

// Generated include
#include "UPPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API AUPPlayerController : public APlayerController
{
	GENERATED_BODY()

	AUPPlayerController();

	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<UUPPathFollowingComponent> PathFollowingComponent;

	void TryOpenContainer(IContainerInterface* ContainerInterface, EContainerRelationType Relation);

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayHUDWidget> GameplayHUDWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UGameplayHUDWidget> GameplayHUDWidgetInstance;

public:
#pragma region Input
	UFUNCTION(BlueprintCallable)
	void MoveToCursor();

	UFUNCTION(BlueprintCallable)
	void HandlePickupAction() const;

	UFUNCTION(BlueprintCallable)
	void HandleActivateAction();

	UFUNCTION(BlueprintCallable)
	void HandleInventoryToggle();
#pragma endregion
	
	UGameplayHUDWidget* GetGameplayHUD() const { return GameplayHUDWidgetInstance; }
};
