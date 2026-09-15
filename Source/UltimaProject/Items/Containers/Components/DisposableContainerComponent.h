// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "DisposableContainerComponent.generated.h"

/**
 * Disposable container means it exists only for one user and will be destroyed after it was closed
 * It's attached to the pawn itself and replicated only to the owner
 * Used for one-time rewards ( gathering/foraging skills etc )
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ULTIMAPROJECT_API UDisposableContainerComponent : public UContainerComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float MaxLifetime = 60.f;

	FTimerHandle LifetimeTimerHandle;
	
	// Where the container was opened
	FVector ContainerOrigin = FAISystem::InvalidLocation;

	void OnLifetimeExpired();
	
	void DisposeContainer();

public:
	UDisposableContainerComponent();

	// UActorComponent
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// ~UActorComponent

	// UContainerComponent
	virtual TArray<FContainerItemData> GetItemsForDisplay(AController* InstigatorController) override;
	virtual void OnClientContainerClosed(AUPPlayerController* Instigator) override;
	virtual void OnClientReady() override;
	virtual bool CanStoreItem(const AController* Instigator, const AItem* Item) const override;
	// ~UContainerComponent

	UFUNCTION(BlueprintCallable)
	static UDisposableContainerComponent* CreateDisposableContainer(APlayerController* OwnerController,
	                                                                FString Name,
	                                                                EContainerCategory ContainerCategory,
	                                                                TSubclassOf<UContainerWidget> WidgetClass,
	                                                                TArray<FItemDataDefinition> Items);
};
