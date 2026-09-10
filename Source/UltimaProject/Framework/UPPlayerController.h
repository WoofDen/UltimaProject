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

	UPROPERTY()
	TObjectPtr<UUPPathFollowingComponent> PathFollowingComponent;

	/**
	 * List of opened containers
	 * Client version of the array contains all opened containers while server version - only external containers. 
	 * ( Currently, no need to track own containers on the server like inventories or own pursue )
	 */
	TArray<TWeakInterfacePtr<const IContainerOwnerInterface>> OpenedContainers;

	// List of opened proxy containers. Server only
	// Key is the origin container ( UExternalContainerComponent ) and value is the corresponding proxy for this client
	UPROPERTY()
	TMap<class UContainerComponent*, class UProxyContainerComponent*> OpenedProxyContainers;

#pragma region Containers

public:
	bool IsContainerOpened(const IContainerOwnerInterface* ContainerInterface) const;

private:
	void TryOpenContainer(IContainerOwnerInterface* ContainerInterface, EContainerRelationType Relation);
	void TryCloseContainer(IContainerOwnerInterface* ContainerInterface);

	void OnOpenedContainerAccessibilityUpdated(IContainerOwnerInterface* ContainerInterface);

	UFUNCTION(Server, Unreliable)
	void ServerOpenProxyContainer(UObject* ContainerInterfaceObject);

	UFUNCTION(Server, Unreliable)
	void ServerCloseProxyContainer(UObject* ContainerInterfaceObject);

	UFUNCTION(Client, Unreliable)
	void ClientForceCloseContainer(UObject* ContainerInterfaceObject);
	
	bool TryRelocateItem(UContainerComponent* SourceContainer, int32 ContainerItemHandle, UContainerComponent* TargetContainer);
	
	UFUNCTION(Server, Unreliable)
	void ServerTryStoreItem(UContainerComponent* SourceContainer, int32 ContainerItemHandle, UContainerComponent* TargetContainer);
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
	void HandleRelocateItem(UContainerComponent* SourceContainerComponent, int32 ContainerItemHandle, UContainerComponent* TargetContainerComponent);
#pragma endregion

	UGameplayHUDWidget* GetGameplayHUD() const { return GameplayHUDWidgetInstance; }
	class UUPAbilitySystemComponent* GetAbilitySystemComponent() const;
};
