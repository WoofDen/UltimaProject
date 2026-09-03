// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Game includes
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "ProxyContainerComponent.generated.h"

/** 
 * Proxies are local player representation of external containers
 * When a player opens an external container (a chest) a temproral proxy container is created and provides its replicated content as long as it is open
 * The items array will be always empty there to avoid dupe risks
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ULTIMAPROJECT_API UProxyContainerComponent : public UContainerComponent
{
	GENERATED_BODY()

	UFUNCTION()
	void OnRep_OriginContainer();

	UFUNCTION()
	void OnRep_OwnerController();
	
	UFUNCTION()
	void OnRep_ProxyContainerItems();
	
	UFUNCTION()
	void OnOriginContainerItemsChanged();
	
	UPROPERTY(Transient, Replicated, ReplicatedUsing=OnRep_ProxyContainerItems)
	FContainerItems ProxyContainerItems;
	
	bool bInitialized = false;

protected:
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_OriginContainer)
	TWeakObjectPtr<UContainerComponent> OriginContainer;

	UPROPERTY(Replicated, ReplicatedUsing=OnRep_OwnerController)
	TWeakObjectPtr<AUPPlayerController> OwnerController;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UProxyContainerComponent();

	void InitializeServer(AUPPlayerController* InOwner, UContainerComponent* InContainerComponent);
	void InitializeClient();

	// UActorComponent
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	// ~UActorComponent

	// UContainerComponent
protected:
	virtual FItemTransactionResult MoveItem(AItem* WorldItem, uint32 AmountToMove) override;
	virtual FItemTransactionResult MoveItem(FContainerItemData& SourceItem, uint32 AmountToMove) override;
	virtual FItemTransactionResult MoveItem(FContainerItemData& Item, AItem* OutItem, uint32 AmountToMove) override;
	
	virtual bool HasItem(const FContainerItemData& ItemData) const override;
	virtual UContainerComponent* GetOriginContainer() override;
	virtual TSubclassOf<UContainerWidget> GetContainerWidgetClass() const override;
	
public:
	virtual TArray<FContainerItemData> GetItemsForDisplay(AController* InstigatorController) override;

	// ~UContainerComponent
};
