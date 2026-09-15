// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Game includes
#include "UltimaProject/Navigation/UPPathFollowingComponent.h"
#include "UltimaProject/UI/HUD/GameplayHUDWidget.h"

// Engine includes
#include "GameFramework/PlayerController.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY()
	TObjectPtr<UUPPathFollowingComponent> PathFollowingComponent;

	/**
	 * List of opened containers
	 * Client version of the array contains all opened containers while server version - only external containers. 
	 * ( Currently, no need to track own containers on the server like inventories or own pursue )
	 */
	TArray<TWeakInterfacePtr<UContainerComponent>> OpenedContainers;

	// List of opened proxy containers. Server only
	// Key is the origin container ( UExternalContainerComponent ) and value is the corresponding proxy for this client
	UPROPERTY()
	TMap<const UContainerComponent*, class UProxyContainerComponent*> OpenedProxyContainers;

#pragma region Containers

public:
	bool IsContainerOpened(const UContainerComponent* ContainerComponent) const;

	void TryOpenContainer(UContainerComponent* ContainerComponent, EContainerRelationType Relation);
	void TryCloseContainer(UContainerComponent* ContainerComponent);
private:
	
	FTimerHandle ContainerAccessibilityTimerHandle;
	
	void UpdateContainerAccessibility();
	
	void OnOpenedContainerAccessibilityUpdated(IContainerOwnerInterface* ContainerInterface);

	UFUNCTION(Server, Unreliable)
	void ServerOpenProxyContainer(UContainerComponent* ContainerComponent);

	UFUNCTION(Server, Unreliable)
	void ServerCloseProxyContainer(UContainerComponent* ContainerComponent);

	// Called when the container was closed by server
	UFUNCTION(Client, Unreliable)
	void ClientForceCloseContainer(UContainerComponent* ContainerComponent);
	
#pragma endregion

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
	void HandleDropAction(UContainerComponent* SourceContainer, int32 ContainerItemHandle, int32 ItemAmount) const;

	UFUNCTION(BlueprintCallable)
	void HandlePickupAction(AItem* SourceItem, int32 ItemAmount, UContainerComponent* TargetContainer) const;
	
	UFUNCTION(BlueprintCallable)
	void HandleActivateAction();

	UFUNCTION(BlueprintCallable)
	void HandleInventoryToggle();

	UFUNCTION(BlueprintCallable)
	void HandleRelocateItem(UContainerComponent* SourceContainer, int32 ContainerItemHandle, UContainerComponent* TargetContainer, int32 ItemAmount);
#pragma endregion

	UGameplayHUDWidget* GetGameplayHUD() const { return GameplayHUDWidgetInstance; }
	class UUPAbilitySystemComponent* GetAbilitySystemComponent() const;
};
